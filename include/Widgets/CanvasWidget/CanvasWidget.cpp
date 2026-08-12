#include <CanvasWidget.h>
#include <iostream>
#include <fstream>
#include <QString>







//================================
//          Constructor  
//================================

GLWidget::GLWidget(QWidget* parent) : QOpenGLWidget(parent) {

    /*
    // Simple render loop (~60 FPS)
    QTimer* PaintTimer = new QTimer(this);
    connect(PaintTimer, &QTimer::timeout, 
            this, QOverload<>::of(&GLWidget::update)); //connect
    PaintTimer->start(16); //starts timer which a timeout specified

    */

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


void GLWidget::paintGL() {
    std::vector<TileCoord> *dirty = currentImage()->getDirty();


    int len = currentImage()->getTileLength();



    //------ switching frames, non brush updates, ect.. -------
    if(newCanvas){
    std::vector<TileCoord> *dirtyCanvas = canvas->getDirty();

        for(auto &tile : *dirtyCanvas){

            std::vector<UCHAR*> tiles = canvas->getTile(tile.x, tile.y);

            for(auto &layer : tiles){
                rasterizer->PaintRaster(
                    RectTI{tile.x * len , tile.y * len, (tile.x * len) + len, (tile.y * len) + len},
                    layer, 
                    defaultFramebufferObject()
                );
            }

            currentImage()->unmarkDirty(tile.x, tile.y);
        }

        dirty->clear();
        newCanvas = false;
    }


    //------- brush/tool updates to the cnavs -------
    if(dirty->size() != 0){

        for(auto &tile : *dirty){

            std::vector<UCHAR*> tiles = canvas->getTile(tile.x, tile.y);

            for(auto &layer : tiles){
                rasterizer->PaintRaster(
                    RectTI{tile.x * len , tile.y * len, (tile.x * len) + len, (tile.y * len) + len},
                    layer, 
                    defaultFramebufferObject()
                );
            }

            currentImage()->unmarkDirty(tile.x, tile.y);
        }
        dirty->clear();
    }
    

}



void GLWidget::resizeGL(int w, int h) {

};


//================================
//     Canvas Update Handler
//================================


void GLWidget::updateCanvas(){
    int size = points.size();


    for(int i = 0; i + 2 < size; i += 3){
        PointTI P0 = points[i];
        PointTI PM = points[i + 1];
        PointTI P2 = points[i + 2];

        float P1x = (4*PM.x - P0.x - P2.x) / 2.0f;
        float P1y = (4*PM.y - P0.y - P2.y) / 2.0f;
        PointTF P1(P1x, P1y);

        std::vector<PointTF> cache = toonzCalculate::QuadraticBezierCurveFloat(P0, P1, P2, 0.2f);



        int csize = cache.size();
        for(int j = 0; j + 1 < csize; j++){
            brush.drawBrush(cache[j], cache[j+1]);
        }
    }

    if(size % 3 == 2){
        brush.drawBrush(points[size-2], points[size-1]);
    }


    points.clear();



    update();
   
}


//================================
//     Mouse Input Handler 
//================================

void GLWidget::mousePressEvent(QMouseEvent *event) {

    if (event->button() == Qt::LeftButton) {

        QPoint q = event->pos();

        if(p1.x == -1){
            //----- case1: no initial point -----
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


    if (event->buttons() & Qt::LeftButton) {

        QPoint q = event->pos();

        if(p1.x == -1){
            //----- case1: no initial point -----
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
    if(event->button() == Qt::LeftButton) {
        p1 = PointT(-1, -1);
        p2 = PointT(-1, -1);
        p3 = PointT(-1, -1);
        brush.resetBrush();
        event->accept();

        
    }
}



    //================================
    //      Brush Handlers
    //================================


     //------- Create Brush Object ---------
     void GLWidget::initiateBrush(){
       
        eraser = false;
        curr_color = PixelType(0, 0, 0, 255 * krish64);
        PixelType color = curr_color;

        Brush::setBrush(Brush::RasterTypes::BRUSH_BGRM32, brush, currentImage(), color, brushsize);
        updateCursor();
     }


     //------- Select new Brush Type and Size ---------
     void GLWidget::selectBrush(const QString& brushId, Brush::RasterTypes type, int size) {
        Q_UNUSED(brushId);
        brushsize = size;
        Brush::setBrush(type, brush, currentImage(), curr_color, brushsize);

        if (eraser) {
            // Hard erase: overwrite pixels with zero matte (see toonzBrush::drawPixel)
            PixelType transparent = PixelType(0, 0, 0, 0);
            brush.setColor(transparent);
        } else {
            brush.setColor(curr_color);
        }
        updateCursor();
     }

    //------- Update Brush Size ---------
    void GLWidget::updateBrushSize(int size){
        brushsize = size;
        brush.setSize(brushsize);
        updateCursor();
     }

    //------- Update Brush Color ---------
    void GLWidget::updateBrushColor(PixelType Color){
        if(eraser){
            curr_color = Color;
        } else {
            curr_color = Color;
            brush.setColor(Color);
        }
     }

     //------- Toggle Eraser ---------
     void GLWidget::toggleEraser(bool enable) {

        if (enable) {
            eraser = true;
            // Hard erase: overwrite pixels with zero matte (transparent), not white
            PixelType transparent = PixelType(0, 0, 0, 0);
            brush.setColor(transparent);
        }  else {
            eraser = false;
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
        Brush::setBrush(Brush::RasterTypes::BRUSH_BGRM32, brush, currentImage(), curr_color, brushsize);
        if (eraser) {
            PixelType transparent = PixelType(0, 0, 0, 0);
            brush.setColor(transparent);
        }

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
        Brush::setBrush(Brush::RasterTypes::BRUSH_BGRM32, brush, currentImage(), curr_color, brushsize);
        if (eraser) {
            PixelType transparent = PixelType(0, 0, 0, 0);
            brush.setColor(transparent);
        }

        //mark the canvas as dirty to trigger a re-render (may remove later)
        newCanvas = true;
        update();
    }



    //------- Timeline Edited -------

    void GLWidget::onTimelineEdited() {
        
        //guard
        if (!canvas) return;

        //rebind brush to the new image (just in case)
        Brush::setBrush(Brush::RasterTypes::BRUSH_BGRM32, brush, currentImage(), curr_color, brushsize);
        if (eraser) {
            PixelType transparent = PixelType(0, 0, 0, 0);
            brush.setColor(transparent);
        }

        //mark the canvas as dirty to trigger a re-render (just in case)
        newCanvas = true;
        update();
    }

     





