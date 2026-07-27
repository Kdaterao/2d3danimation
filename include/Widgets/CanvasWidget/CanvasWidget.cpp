#include <CanvasWidget.h>
#include <iostream>
#include <fstream>







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

    //-------test image (temporary) ------

    testImage = std::make_shared<Raster<PixelType>>(canvasWidth, canvasHeight);
    //testImage->createBlank();

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
        testImage->getTileLength(), 
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

   glClearColor(255.0f, 255.0f, 255.0f, 255.0f);
}


void GLWidget::paintGL() {
    std::vector<TileCoord> *dirty = testImage->getDirty();


    int len = testImage->getTileLength();


    
    if(dirty->size() != 0){

        //std::cout<<"painting tile(s)--------------------------"<<std::endl;

        for(auto &tile : *dirty){

            rasterizer->PaintRaster(
                RectTI{tile.x * len , tile.y * len, (tile.x * len) + len, (tile.y * len) + len},
                testImage->getRawData(tile.x * len, tile.y*len, false), 
                defaultFramebufferObject()
            );

            testImage->unmarkDirty(tile.x, tile.y);
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
//      Brush Manager functions 
//================================

     void GLWidget::initiateBrush(){
       
        curr_color = PixelType(128,128,128,255);
        PixelType color =  PixelType(128,128,128,255);

        Brush::setBrush(Brush::RasterTypes::BRUSH_BGRM32, brush, testImage, color, brushsize);
        updateCursor();
     }

     


    void GLWidget::updateBrushSize(int size){
        brushsize = size;
        brush.setSize(brushsize);
        updateCursor();
     }

     
    void GLWidget::updateBrushColor(PixelType Color){
        if(eraser){
            curr_color = Color;
        } else {
            curr_color = Color;
            brush.setColor(Color);
        }
     }

     void GLWidget::toggleEraser(bool enable) {

        if (enable) {
            eraser = true;
            // Transparent brush
            PixelType transparent = PixelType(255 * krish64,255 * krish64,255 * krish64, 255 * krish64);
            brush.setColor(transparent);
        }  else {
            eraser = false;
            brush.setColor(curr_color);
        }
        updateCursor();
     }

     





