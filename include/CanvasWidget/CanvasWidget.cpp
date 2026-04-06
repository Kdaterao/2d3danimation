#include <CanvasWidget.h>
#include <iostream>
#include <fstream>







//================================
//          Constructor  
//================================

GLWidget::GLWidget(QWidget* parent) : QOpenGLWidget(parent) {
    // Simple render loop (~60 FPS)
    QTimer* PaintTimer = new QTimer(this);
    connect(PaintTimer, &QTimer::timeout, 
            this, QOverload<>::of(&GLWidget::update)); //connect
    PaintTimer->start(16); //starts timer which a timeout specified

}



//================================
//     OpenGL handler
//================================


//much of this stuff is going to reorganized later on
void GLWidget::initializeGL() {

    //-------test image (temporary) ------

    testImage = std::make_shared<ToonzRasterT<ToonzPixelBGRM32>>(800, 600);
    testImage->createBlank();

    //----- initiateBrush (do before initiating opengl) ------
    initiateBrush();

    //----- setup Opengl ------
    initializeOpenGLFunctions();

    
    setUpdateBehavior(QOpenGLWidget::PartialUpdate);//stops Qwidget from constantly overwriting the screen
    
    

    // shader
    shaderProgram = new toonzShader(
        "/Users/krish/codingStuff/2d3danimation/include/shaders/vsCanvas.txt", 
        "/Users/krish/codingStuff/2d3danimation/include/shaders/fsCanvas.txt"
    );

    // rasterizer
    rasterizer = new toonzPainterGL(
        TAffine(), testImage->getRawData(),
        800, 800, 600, 4,
        DimensionTI(800,600),
        GL_NEAREST, GL_NEAREST,
        false, shaderProgram
    );

   glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
   start = true;
}


void GLWidget::paintGL() {
    updateCanvas();
    if (start){
    rasterizer->PaintRaster(
        RectTI(0,0,800,600), 
        testImage->getRawData(), 
        defaultFramebufferObject()
    );
    start = false;
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

            /* DEBUG
            for(int k = 0; k <size ; k++){

                std::cout<<"POINTS:"<<"("<<points[k].x<<","<<points[k].y<<")"<<std::endl;
            }
            std::cout<<"****************"<<std::endl;
            */

            //case1: Points are close together
            PointTI P0 = points[i];
            PointTI PM = points[i + 1];
            PointTI P2 = points[i + 2];

            float P1x = (4*PM.x - P0.x - P2.x) / 2.0f;
            float P1y = (4*PM.y - P0.y - P2.y) / 2.0f;
            PointTF P1(P1x, P1y);

            std::vector<PointTI> cache = toonzCalculate::QuadraticBezierCurve(P0, P1, P2, 0.05f); //get interpolated points

            //draw interpolated points
            for(int j = 0; j + 1 < cache.size(); j++){
                brush.drawBrush(cache[j], cache[j+1]);
            }
            start = true;
        }

        // handle leftover points
        if(size % 3 == 2){
            brush.drawBrush(points[size-2], points[size-1]);
            start = true;
        }

        points.clear();
    
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
            start = true;

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
        event->accept();
    }
}



//================================
//      Brush Manager functions 
//================================

     void GLWidget::initiateBrush(){
       
        int size = 30;
        curr_color = ToonzPixelBGRM32(128,128,128,255);
        ToonzPixelBGRM32 color =  ToonzPixelBGRM32(128,128,128,255);

        Brush::setBrush(Brush::RasterTypes::BRUSH_BGRM32, brush, testImage, color, size);
   
     }

     


    void GLWidget::updateBrushSize(int size){
        
        brush.setSize(size);
     }

     
    void GLWidget::updateBrushColor(ToonzPixelBGRM32 Color){
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
            ToonzPixelBGRM32 transparent = ToonzPixelBGRM32(255,255,255,255);
            brush.setColor(transparent);
        }  else {
            eraser = false;
            brush.setColor(curr_color);
        }
     }

     





