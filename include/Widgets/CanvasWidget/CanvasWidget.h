#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include <QMouseEvent>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QTimer>
#include <QMatrix4x4>


#include <QPainter>
#include <QPixmap>
#include <QCursor>

#include <types.h>
#include <rasterPixel.h>
#include <toonzRasterBrush.h>
#include <toonzRasterCircleBrush.h>
#include <painterGL.h>
#include <toonzShader.h>
#include <Brush.h>
#include <variant>
#include <toonzCalculations.h>

#include <raster.h>
#include <QString>



#define USE_UINT16



#ifdef USE_UINT16
using PixelType = PixelRGBM64;
inline int krish64 = 257;
#else
using PixelType = PixelRGBM32;
inline int krish64 = 1;
#endif


class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions {



    //temporary variable
    RasterP<PixelType> testImage;

    // Canvas 
    int canvasWidth = 800;
    int canvasHeight = 600; 
    toonzShader* shaderProgram = nullptr;     
    toonzPainterGL* rasterizer = nullptr; 
    int bpp = sizeof(PixelType);

    //brush
    Brush::RasterBrush<PixelType> brush;
    PixelType curr_color;
    bool eraser;


    // point tracking
    PointTI p1 = PointT(-1,-1);
    PointTI p2 = PointT(-1,-1);
    PointTI p3 = PointT(-1, -1);
    std::vector<PointTI> points;
    int breakpoint = 0;

    // canvasUpdate tracking
    bool start = false;

    bool painting = false;

    


    


public:
    int brushsize = 6; //just doin spaghetti code to let this widget know the intial brush size!

    explicit GLWidget(QWidget* parent = nullptr) ;

protected:
    //------ override functions ------
    void initializeGL() override; 
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
   

private:
    void initiateBrush();
    void initiateCanvas();
     void updateCursor();
    

public slots:
    void updateBrushColor(PixelType Color);
    void toggleEraser(bool enabled); 
    void updateBrushSize (int val);
    void selectBrush(const QString& brushId, Brush::RasterTypes type, int size);
    

    void updateCanvas();
 
    
};


#endif 