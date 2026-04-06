#ifndef BRUSHWIDGET_H
#define  BRUSHWIDGET_H



#include <QWidget>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QSlider>
#include <QCheckBox>
#include <QLabel>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>

#include <toonzRasterPixel.h>
#include <types.h>
#include <toonzRasterBrush.h>
#include <toonzPainterGL.h>
#include <toonzShader.h>
#include <UIPainter.h>
#include <UIShapes.h>

class ColorTriangleWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    toonzShader *shaderProgram = nullptr;
    UIPainter *rasterizer; 
    ToonzPixelBGRM32 color;


     Q_OBJECT

    public :
        explicit ColorTriangleWidget(QWidget* parent = nullptr){};




    protected:

        void initializeGL() override; 
        //void resizeGL(int w, int h) override;
        void paintGL() override;
        void mousePressEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;


        //signal and slots!
        void getColor(QMouseEvent *event);

    private:
        // helper functions 


    signals:
        void sendColor(ToonzPixelBGRM32 Color);
};





#endif