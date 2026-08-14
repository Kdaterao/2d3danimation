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
#include <canvas.h>
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
    Q_OBJECT

  

    // Canvas 
    int canvasWidth = 800;
    int canvasHeight = 600; 
    toonzShader* shaderProgram = nullptr;     
    toonzPainterGL* rasterizer = nullptr; 
    int bpp = sizeof(PixelType);





    //brush
    Brush::RasterBrush<PixelType> brush;
    
    // state variables
    bool eraser;
    PixelType curr_color;
    Brush::RasterTypes curr_brushType;


    // point tracking
    PointTI p1 = PointT(-1,-1);
    PointTI p2 = PointT(-1,-1);
    PointTI p3 = PointT(-1, -1);
    std::vector<PointTI> points;
    int breakpoint = 0;

    // canvasUpdate tracking
    bool start = false;
    bool painting = false;


    // canvas object
    std::unique_ptr<Canvas> canvas = nullptr;

    RasterLayer* currentLayer() { return canvas->currentLayer(); }
    Raster<PixelType>* currentImage() { return currentLayer()->getCurrentImage(); }

    bool newCanvas = true;


public:
    int brushsize = 6; //just doin spaghetti code to let this widget know the intial brush size!

    explicit GLWidget(QWidget* parent = nullptr) ;

    Canvas* getCanvas() { return canvas.get(); }

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
    // Re-apply color / size / eraser after recreating the brush engine
    void applyBrushState();
    

public slots:
    //brush widget
    void updateBrushColor(PixelType Color);
    void toggleEraser(bool enabled); 
    void updateBrushSize (int val);
    void updateBrushOpacity(int opacityPercent); // 0–100
    void selectBrush(const QString& brushId, Brush::RasterTypes type, int size);

    //timeline widget
    void onTimeChanged(int time);
    void onActiveLayerChanged(int layerIndex);
    void onTimelineEdited();

    //canvas widget
    void updateCanvas();

signals:
    // Emitted when drawing creates/edits timeline structure (e.g. auto-add frame)
    void timelineContentChanged();
 
private:
    // If playhead sits on empty time, add a frame there and rebind the brush.
    void ensureDrawingFrame();
    
};


#endif 