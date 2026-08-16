#include <CanvasWidget.h>
#include <iostream>
#include <fstream>
#include <QString>
#include <unordered_map>
#include <utility>







//================================
//          Constructor  
//================================

GLWidget::GLWidget(QWidget* parent) : QOpenGLWidget(parent) {
    
    
    // Simple render loop (~80 FPS)
    QTimer* PaintTimer = new QTimer(this);
    connect(PaintTimer, &QTimer::timeout, 
            this, QOverload<>::of(&GLWidget::update)); //connect
    PaintTimer->start(8); //may need to alter this for reducing or increasing idle time

    
    

    canvas = std::make_unique<Canvas>(canvasWidth, canvasHeight);



}


void GLWidget::updateCursor() {
    int size = brushsize * 2 + 1; // +1 so circle has a center pixel
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::green, 1));  // white outline
    painter.drawEllipse(1, 1, size - 2, size - 2); // inset by 1 so it doesn't clip
    painter.end();

    setCursor(QCursor(pixmap, size/2, size/2));
}



//================================
//     OpenGL handler
//================================


//much of this stuff is going to reorganized later on
void GLWidget::initializeGL() {

    //----- initiateBrush (do before initiating opengl) ------
    initiateBrush();

    //----- setup Opengl ------
    initializeOpenGLFunctions();

    
    setUpdateBehavior(QOpenGLWidget::PartialUpdate);//stops Qwidget from constantly overwriting the screen
    
    

    // shader
    shaderProgram = new toonzShader(
        "/Users/krish/codingStuff/2d3danimation/include/2D/Painter/General/Shader/ShaderScripts/vsCanvas.txt", 
        "/Users/krish/codingStuff/2d3danimation/include/2D/Painter/General/Shader/ShaderScripts/fsCanvas.txt"
    );

    // rasterizer
    rasterizer = new toonzPainterGL(
        TAffine(), 
        currentImage()->getTileLength(), 
        canvasWidth, 
        canvasHeight, 
        sizeof(PixelType), //byte per pixel
        DimensionTI(canvasWidth, canvasHeight), //dimensions of raster
        GL_NEAREST,  //mag filter
        GL_NEAREST, //min filter
        false, // premulitplecation?
        shaderProgram, //shader object
        true //isrgbm
    );

   glClearColor(0.0f, 0.0f, 0.0f, 255.0f);
}


void GLWidget::paintEvent(QPaintEvent *event) {
    {
        PipelineScope _paintEvent(PipelineStage::PaintEvent);
        QOpenGLWidget::paintEvent(event);
    }
    PipelineLogger::instance().endFrame();
}





void GLWidget::paintGL() {

    {

    //----- debug timer ------------
    PipelineScope _paintGL(PipelineStage::PaintGL); // <----- DEBUG


    //------ get objects + layer info + grab dirty tiles ------------------------
    std::vector<TileCoord> *dirty = currentImage()->getDirty(); // dirty tiles for the current image 
    int len = currentImage()->getTileLength();
    const int activeLayer = canvas->getActiveLayerIndex();
    GLuint fbo = defaultFramebufferObject();

    std::vector<TileCoord> coords;
    if (newCanvas) {
        //----- case 1: new canvas (everything is dirty-----
        coords = *canvas->getDirty();
        dirty->clear();
        newCanvas = false;
    } else if (!dirty->empty()) {
        //----- case 2: dirty tiles -----
        coords = *dirty; 
        dirty->clear();
    }

    //---- HANDLE  ------------------------
    const int tilesThisFrame = (int)coords.size();
    if (coords.empty()) {
        //----- case 1: no dirty tiles -----
        PipelineLogger::instance().addPaintGLTiles(0);
    } else {
        //----- case 2: dirty tiles -----

        //define batches (these are handled differntly and thus are store seperately)
        std::unordered_map<int, std::vector<RasterPaintTile>> layerBatches; // batches for each layer (less texture uploads!)
        std::vector<RasterPaintTile> previewBatch; // preview batch for the current image (buffer of what our brush JUST did)

        std::vector<std::pair<Raster<PixelType>*, int>> onionPrev; // onion skin previous frames
        std::vector<std::pair<Raster<PixelType>*, int>> onionNext; // onion skin next frames
        std::vector<std::vector<RasterPaintTile>> onionPrevBatches; // batches for each onion skin previous frame
        std::vector<std::vector<RasterPaintTile>> onionNextBatches; // batches for each onion skin next frame





    //--------- BUILD BATCHES -----------------------------

    //---- Preprocess Onion skin for batching later on -------
    {
    PipelineScope _batch(PipelineStage::BatchBuild); // debug
    
    //grab current layer object 
    RasterLayer* onionLayer = currentLayer(); 

    //handle onion skin
    if (onionLayer && (onionBefore > 0 || onionAfter > 0)) {

        PipelineScope _onion(PipelineStage::Onion);// DEBUG 

        //grab current frame index and frames
        const int fi = onionLayer->getCurrentFrameIndex();
        const auto& frames = onionLayer->getFrames();

        //handle onion skin frames 
        if (fi >= 0 && fi < (int)frames.size()) {
            const int currentImg = frames[fi].imageIndex;
            int seen = currentImg;

            //handle previous frames
            for (int i = fi - 1; i >= 0 && (int)onionPrev.size() < onionBefore; --i) {
                if (frames[i].imageIndex == seen) continue;
                seen = frames[i].imageIndex;
                Raster<PixelType>* img = onionLayer->getImageAtFrame(i); // grab the image at the frame
                if (img) onionPrev.push_back({img, (int)onionPrev.size() + 1}); // add to the previous frames vector
            }

            seen = currentImg; // reset for the next loop

            //handle next frames
            for (int i = fi + 1; i < (int)frames.size() && (int)onionNext.size() < onionAfter; ++i) {
                if (frames[i].imageIndex == seen) continue;
                seen = frames[i].imageIndex;
                Raster<PixelType>* img = onionLayer->getImageAtFrame(i); // grab the image at the frame
                if (img) onionNext.push_back({img, (int)onionNext.size() + 1}); // add to the next frames vector
            }
        }
    }

    onionPrevBatches.resize(onionPrev.size());
    onionNextBatches.resize(onionNext.size());




    //---- BATCH TILES (BUFFER + NON BUFFER + ONION SKIN) --------
    UCHAR* nullTile = currentImage()->getNullTile();
    for (const TileCoord& tile : coords) {
        PipelineScope _stack(PipelineStage::TileStack); // DEBUG

        //define rect for the tile
        RectTI rect{tile.x * len, tile.y * len, (tile.x * len) + len, (tile.y * len) + len};

        //--- NON BUFFER TILES
        auto tiles = canvas->getTile(tile.x, tile.y);

        for (auto& [layerIndex, data] : tiles) {
            layerBatches[layerIndex].push_back(RasterPaintTile{rect, data});
        }


        //---- BUFFER TILES
        {
            PipelineScope _preview(PipelineStage::PreviewLookup);
            UCHAR* buffer = currentImage()->getBufferTile(tile.x * len, tile.y * len, false);
            if (buffer != nullTile) {
                previewBatch.push_back(RasterPaintTile{rect, buffer});
            }
        }



        //---- ONION SKIN
        for (size_t i = 0; i < onionPrev.size(); ++i) {
            UCHAR* data = onionPrev[i].first->getRawData(tile.x * len, tile.y * len, false);
            onionPrevBatches[i].push_back(RasterPaintTile{rect, data});
        }
        for (size_t i = 0; i < onionNext.size(); ++i) {
            UCHAR* data = onionNext[i].first->getRawData(tile.x * len, tile.y * len, false);
            onionNextBatches[i].push_back(RasterPaintTile{rect, data});
        }

        //---- UNMARK DIRTY TILE
        currentImage()->unmarkDirty(tile.x, tile.y);
    }
    }





    //---- ONION SKIN HELPERS ---------------------
    //settings
    auto onionSettings = [](bool previous, int distance) {
        PaintSettings s;
        s.opacity = 0.4f / (float)distance;
        if (previous) {
            s.tintR = 1.0f; s.tintG = 0.3f; s.tintB = 0.3f;
        } else {
            s.tintR = 0.3f; s.tintG = 1.0f; s.tintB = 0.35f;
        }
        return s;
    };

    //paint onion skin
    auto paintOnion = [&]() {
        PipelineScope _onion(PipelineStage::Onion);
        for (int i = (int)onionPrev.size() - 1; i >= 0; --i) {
            rasterizer->PaintRasterTiles(
                kLayerKeyOnion, onionPrevBatches[i], fbo, onionSettings(true, onionPrev[i].second));
        }
        for (int i = (int)onionNext.size() - 1; i >= 0; --i) {
            rasterizer->PaintRasterTiles(
                kLayerKeyOnion, onionNextBatches[i], fbo, onionSettings(false, onionNext[i].second));
        }
    };





    //---------- DEFINE LAYER ORDER FOR PAINTING -----------------
    std::vector<int> paintOrder;

    //---- BACKDROP LAYER
    //NOTE: [ Defined in Canvas.h ]
    if (layerBatches.count(kLayerKeyBackdrop)) { 
        paintOrder.push_back(kLayerKeyBackdrop);
    }


    //---- OTHER LAYERS 
    for (int layerIndex : canvas->layerOrder()) {
        if (layerBatches.count(layerIndex)) paintOrder.push_back(layerIndex);
    }



    //----------- PAINT LAYERS -----------------

    bool paintedActive = false;
    for (int layerIndex : paintOrder) { 

        // onion skin (if current layer)
        if (layerIndex == activeLayer) {
            paintOnion();
            paintedActive = true;
        }

        // layer
        rasterizer->PaintRasterTiles(layerIndex, layerBatches[layerIndex], fbo, false);

        // preview batch (if current layer)
        if (layerIndex == activeLayer && !previewBatch.empty()) { // preview batch
            rasterizer->PaintRasterTiles(kLayerKeyPreview, previewBatch, fbo, true);
        }

    }


    //---- ONION SKIN (if we didnt paint and just switch frames) ---------
    if (!paintedActive) {
        paintOnion();
        if (!previewBatch.empty()) {
            rasterizer->PaintRasterTiles(kLayerKeyPreview, previewBatch, fbo, true);
        }
    }

    //---- LOG TILES PAINTED (DEBUGGING)--------
    PipelineLogger::instance().addPaintGLTiles(tilesThisFrame);
    }

    //---- SYNC GPU (FOR DEBUGGING) --------
    if (PipelineLogger::enabled()) {
        PipelineScope _gpu(PipelineStage::GpuSync);
        glFinish();
    }
    }


    //---- COMMIT BUFFER TILES IF OUR STROKE IS COMPLETE (important for alpha scaling) --------
    if(strokeComplete){
        float alphaScale = curr_color.m / (float)PixelType::maxChannelValue;
        currentImage()->commitBufferTiles(alphaScale);
        currentImage()->clearBufferTiles();
        strokeComplete = false;
    }
    

}









void GLWidget::resizeGL(int w, int h) {

};


//================================
//     Canvas Handlers
//================================


void GLWidget::updateCanvas(){
    PipelineScope _update(PipelineStage::UpdateCanvas);
    int size = points.size();


    for(int i = 0; i + 2 < size; i += 3){
        PointTI P0 = points[i];
        PointTI PM = points[i + 1];
        PointTI P2 = points[i + 2];

        float P1x = (4*PM.x - P0.x - P2.x) / 2.0f;
        float P1y = (4*PM.y - P0.y - P2.y) / 2.0f;
        PointTF P1(P1x, P1y);

        std::vector<PointTF> cache;
        {
            PipelineScope _bezier(PipelineStage::Bezier);
            cache = toonzCalculate::QuadraticBezierCurveFloat(P0, P1, P2, 0.2);
        }

        int csize = cache.size();
        for(int j = 0; j + 1 < csize; j++){
            brush.drawBrush(cache[j], cache[j+1]);
            update();
        }
    }

    if(size % 3 == 2){
        brush.drawBrush(points[size-2], points[size-1]);
        update();
    }
    points.clear();
    
}


void GLWidget::ensureDrawingFrame() {
    PipelineScope _ensure(PipelineStage::EnsureFrame); // debug
    if (!canvas) return;
    RasterLayer* layer = currentLayer();
    if (!layer) return;

    const int t = canvas->getCurrentTime();
    if (layer->hasFrameAtTime(t)) return;

    // Empty slot at playhead → create a new frame (and image) before painting
    layer->addFrame(t);
    layer->switchFrame(layer->frameIndexAtTime(t));

    Brush::setBrush(curr_brushType, brush, currentImage(), curr_color, brushsize);
    applyBrushState();

    newCanvas = true;
    emit timelineContentChanged();
}


//================================
//     Mouse Input Handler 
//================================

void GLWidget::mousePressEvent(QMouseEvent *event) {
    PipelineScope _press(PipelineStage::MousePress); // debug

    if (event->button() == Qt::LeftButton) {

        

        QPoint q = event->pos();

        if(p1.x == -1){
            //----- case1: no initial point -----
            ensureDrawingFrame();
            p1 = PointT(q.x(), q.y());
            p2 = PointT(q.x(), q.y());
            p3 = PointT(q.x(), q.y());

            brush.drawBrush(p1, p2);
            update();
        }
    }
    event->accept();
};



void GLWidget::mouseMoveEvent(QMouseEvent *event) {
    PipelineScope _move(PipelineStage::MouseMove); // debug


    if (event->buttons() & Qt::LeftButton) {

        QPoint q = event->pos();

        if(p1.x == -1){
            //----- case1: no initial point -----
            ensureDrawingFrame();
            p1 = PointT(q.x(), q.y());
            p2 = PointT(q.x(), q.y());
            p3 = PointT(q.x(), q.y());
            breakpoint += 1;

        } else  {
            //bubble up the points
            p1 = p2;
            p2 = p3;
            p3 = PointT(q.x(), q.y());
            breakpoint += 1;

            //only add points every 2 so that p3 is never starting on two triples(prevents overlapping artifacts)
            if(breakpoint == 2){
                points.push_back(p1);
                points.push_back(p2);
                points.push_back(p3);
                updateCanvas();
                breakpoint = 0;
            } 
            


        };
    }
    event->accept();
};


void GLWidget::mouseReleaseEvent(QMouseEvent *event) {
    PipelineScope _release(PipelineStage::MouseRelease); // debug
    if(event->button() == Qt::LeftButton) {
        p1 = PointT(-1, -1);
        p2 = PointT(-1, -1);
        p3 = PointT(-1, -1);
        brush.resetBrush();

        strokeComplete = true;
        updateCanvas();
        update();
        event->accept();
    }
}



    //================================
    //      Brush Handlers
    //================================

     void GLWidget::applyBrushState() {
        brush.setColor(curr_color);
        brush.setSize(brushsize);
        brush.toggleEraser(eraser);
     }

     //------- Create Brush Object ---------
     void GLWidget::initiateBrush(){
       
        eraser = false;
        curr_color = PixelType(0, 0, 0, 255 * krish64);
        curr_brushType = Brush::RasterTypes::BRUSH_BGRM32;

        Brush::setBrush(curr_brushType, brush, currentImage(), curr_color, brushsize);
        applyBrushState();
        updateCursor();
     }


     //------- Select new Brush Type and Size ---------
     void GLWidget::selectBrush(const QString& brushId, Brush::RasterTypes type, int size) {
        
        Q_UNUSED(brushId);
        brushsize = size;
        curr_brushType = type;

        Brush::setBrush(curr_brushType, brush, currentImage(), curr_color, brushsize);
        applyBrushState();
        updateCursor();
     }

    //------- Update Brush Size ---------
    void GLWidget::updateBrushSize(int size){
        brushsize = size;
        brush.setSize(brushsize);
        updateCursor();
     }

    //------- Update Brush Opacity ---------
    void GLWidget::updateBrushOpacity(int opacityPercent) {
        opacityPercent = std::clamp(opacityPercent, 0, 100);
        curr_color.m = static_cast<typename PixelType::Channel>(
            (opacityPercent / 100.0f) * PixelType::maxChannelValue + 0.5f);
        if (!eraser) {
            brush.setColor(curr_color);
        }
        if (rasterizer) {
            PaintSettings preview = rasterizer->getPreviewSettings();
            preview.opacity = opacityPercent / 100.0f;
            rasterizer->setPreviewSettings(preview);
        }
    }

    //------- Update Brush Color ---------
    void GLWidget::updateBrushColor(PixelType Color){
        // Keep opacity from the slider; color triangle always sends full alpha
        Color.m = curr_color.m;
        curr_color = Color;
        if (!eraser) {
            brush.setColor(curr_color);
        }
     }

     //------- Toggle Eraser ---------
     void GLWidget::toggleEraser(bool enable) {
        eraser = enable;
        brush.toggleEraser(eraser);
        if (!eraser) {
            brush.setColor(curr_color);
        }
        updateCursor();
     }


    //================================
    //     Timeline Handlers 
    //================================

    //------- Changing frame on same layer -------
    void GLWidget::onTimeChanged(int time) {

        //guard
        if (!canvas) return;

        //if the time has changed, update the canvas
        if (canvas->getCurrentTime() != time) {
            canvas->setCurrentTime(time);
        }

        //rebind brush to the new image
        Brush::setBrush(curr_brushType, brush, currentImage(), curr_color, brushsize);
        applyBrushState();

        //mark the canvas as dirty to trigger a re-render
        newCanvas = true;
        update();
    }




    //------- Changing active layer -------
    
    void GLWidget::onActiveLayerChanged(int layerIndex) {
        //guard
        if (!canvas) return;

        //if the active layer has changed, go to it
        if (canvas->getActiveLayerIndex() != layerIndex) {
            canvas->setCurrentLayer(layerIndex);
        }

        //rebind brush to the new image
        Brush::setBrush(curr_brushType, brush, currentImage(), curr_color, brushsize);
        applyBrushState();

        //mark the canvas as dirty to trigger a re-render (may remove later)
        newCanvas = true;
        update();
    }



    //------- Timeline Edited -------

    void GLWidget::onTimelineEdited() {
        
        //guard
        if (!canvas) return;

        //rebind brush to the new image (just in case)
        Brush::setBrush(curr_brushType, brush, currentImage(), curr_color, brushsize);
        applyBrushState();

        //mark the canvas as dirty to trigger a re-render (just in case)
        newCanvas = true;
        update();
    }

    void GLWidget::onOnionSkinChanged(int before, int after) {
        onionBefore = std::max(0, before);
        onionAfter = std::max(0, after);
        newCanvas = true;
        update();
    }

     





