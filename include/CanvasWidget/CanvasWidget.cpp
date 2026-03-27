#include <CanvasWidget.h>
#include <iostream>
#include <fstream>


void dumpBuffer(UCHAR* buf, int width, int height, int wrap) {
     std::string path = "/Users/krish/codingStuff/2d3danimation//output/dump.ppm";

    FILE* f = fopen(path.c_str(), "wb");
    if (!f) return;
    
    fprintf(f, "P6\n%d %d\n255\n", width, height);
    for (int row = 0; row < height; row++) {
        UCHAR* rowPtr = buf + row * wrap * 4;
        for (int col = 0; col < width; col++) {
            fwrite(rowPtr + col * 4, 1, 3, f);
        }
    }
    fclose(f);
}




//---- 
GLWidget::GLWidget(QWidget* parent) : QOpenGLWidget(parent) {

    //setMouseTracking(true);  // ✅ track mouse movement
    //setFocusPolicy(Qt::StrongFocus); // ✅ receive keyboard/mouse events

    // Simple render loop (~60 FPS)
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, 
            this, QOverload<>::of(&GLWidget::update)); //connect
    timer->start(16); //starts timer which a timeout specified

}


//much of this stuff is going to reorganized later on
void GLWidget::initializeGL() {
    initializeOpenGLFunctions();

    testImage = std::make_shared<ToonzRasterT<ToonzPixelBGRM32>>(800, 600);
    testImage->createBlank();
    ToonzPixelBGRM32 basecolor = ToonzPixelBGRM32(128,128,128,255);
    ToonzPixelBGRM32 *color = &basecolor;
    brush = DefaultCircleBrush<ToonzPixelBGRM32>(testImage, color, 50);

    // shader
    shaderProgram = new toonzShader(
"/Users/krish/codingStuff/2d3danimation/include/shaders/vs.txt", "/Users/krish/codingStuff/2d3danimation/include/shaders/fs.txt"
    );

    // rasterizer
    rasterizer = new toonzPainterGL(
        TAffine(), testImage->getRawData(),
        800, 800, 600, 4,
        DimensionTI(800,600),
        GL_NEAREST, GL_NEAREST,
        false, shaderProgram
    );

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glReady = true;
}


void GLWidget::paintGL() {



    if (start){
    rasterizer->PaintRaster(
        RectTI(0,0,800,600), 
        testImage->getRawData(), 
        defaultFramebufferObject()
    );
    }
}


void GLWidget::mousePressEvent(QMouseEvent *event) {

    std::cout<<"yay!"<<std::endl;

    if (event->button() == Qt::LeftButton && glReady) {

        QPoint q = event->pos();

        if(p1.x == -1){
            //----- case1: no initial point -----
            p1 = PointT(q.x(), q.y());
            p2 = PointT(q.x(), q.y());

            std::cout<<"POINTS"<<std::endl;
            std::cout<<"("<<p1.x<<","<<p1.x<<")"<<std::endl;
            std::cout<<"("<<p2.x<<","<<p2.x<<")"<<std::endl;
            brush.drawBrush(p1, p2);
            start = true;
        } else {
            p1 = p2;
            p2 = PointT(q.x(), q.y());
            brush.drawBrush(p1, p2);
            start = true;
        };
    }
    event->accept();
};



void GLWidget::mouseMoveEvent(QMouseEvent *event) {


    if (event->buttons() & Qt::LeftButton && glReady) {

        QPoint q = event->pos();

        if(p1.x == -1){
            //----- case1: no initial point -----
            p1 = PointT(q.x(), q.y());
            p2 = PointT(q.x(), q.y());

            std::cout<<"POINTS"<<std::endl;
            std::cout<<"("<<p1.x<<","<<p1.x<<")"<<std::endl;
            std::cout<<"("<<p2.x<<","<<p2.x<<")"<<std::endl;
            brush.drawBrush(p1, p2);
            start = true;
        } else {
            p1 = p2;
            p2 = PointT(q.x(), q.y());
            brush.drawBrush(p1, p2);
            start = true;
        };
    }
    update();
    event->accept();
};


void GLWidget::mouseReleaseEvent(QMouseEvent *event) {
    if(event->button() == Qt::LeftButton) {
        p1 = PointT(-1, -1);
        p2 = PointT(-1, -1);
        event->accept();
    }
}



