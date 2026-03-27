#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include <QMouseEvent>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QTimer>
#include <QMatrix4x4>

#include <types.h>
#include <toonzRasterPixel.h>
#include <toonzRaster.h>
#include <toonzRasterBrush.h>
#include <toonzPainterGL.h>
#include <toonzShader.h>

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions {

    ToonzRasterPT<ToonzPixelBGRM32> testImage;
    DefaultCircleBrush<ToonzPixelBGRM32> brush;
    bool glReady = false;
    toonzShader* shaderProgram = nullptr;      
    toonzPainterGL* rasterizer = nullptr; 


    PointTI p1 = PointT(-1,-1);
    PointTI p2 = PointT(-1,-1);
    bool start = false;


public:
    explicit GLWidget(QWidget* parent = nullptr);

protected:
    void initializeGL() override; 
    //void resizeGL(int w, int h) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
};


#endif 