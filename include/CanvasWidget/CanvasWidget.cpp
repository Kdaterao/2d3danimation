

#include <CanvasWidget/CanvasWidget.h>

//---- 
GLWidget::GLWidget(QWidget* parent) : QOpenGLWidget(parent) {
    // Simple render loop (~60 FPS)
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, 
            this, QOverload<>::of(&GLWidget::update)); //connect
    timer->start(16); //starts timer which a timeout specified
}



void GLWidget::initializeGL() {
    initializeOpenGLFunctions(); 

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); //static background color 
}

void GLWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

void GLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);

}