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
#include <Brush.h>
#include <variant>
#include <toonzCalculations.h>

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions {

    //temporary variable
    ToonzRasterPT<ToonzPixelBGRM32> testImage;

    // Canvas 
    int width;
    int height; 
    toonzShader* shaderProgram = nullptr;     
    toonzPainterGL* rasterizer = nullptr; 

    //brush
    Brush::RasterBrush brush;
    ToonzPixelBGRM32 curr_color;
    bool eraser;
    

    // point tracking
    PointTI p1 = PointT(-1,-1);
    PointTI p2 = PointT(-1,-1);
    PointTI p3 = PointT(-1, -1);
    std::vector<PointTI> points;
    int breakpoint = 0;

    // canvasUpdate tracking
    bool start = false;


    


public:
    int brushsize = 0; //just doin spaghetti code to let this widget know the intial brush size!

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
    

public slots:
    void updateBrushColor(ToonzPixelBGRM32 Color);
    void toggleEraser(bool enabled); 
    void updateBrushSize (int val);
    

    void updateCanvas();
 
    
};


#endif 