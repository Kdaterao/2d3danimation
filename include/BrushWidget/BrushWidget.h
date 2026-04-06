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



/*


// ===================== MAIN WIDGET =====================
class MainWidget : public QWidget {
    Q_OBJECT
public:
    MainWidget(QWidget* parent = nullptr) : QWidget(parent) {
        QVBoxLayout* layout = new QVBoxLayout(this);

      
        layout->addWidget(canvas);

        // Triangle picker
        ColorTriangleWidget* triangle = new ColorTriangleWidget(this);
        triangle->setFixedSize(200, 200);
        layout->addWidget(new QLabel("Color Picker"));
        layout->addWidget(triangle);

        // Eraser toggle
        QCheckBox* eraserToggle = new QCheckBox("Eraser", this);
        layout->addWidget(eraserToggle);

        // Connections
        connect(triangle, &ColorTriangleWidget::colorSelected,
                this, &MainWidget::onColorPicked);

        connect(eraserToggle, &QCheckBox::toggled,
                this, &MainWidget::onEraserToggled);
    }

private slots:
    void onColorPicked(int r, int g, int b) {
        if (eraserOn) return;

        // NOTE: your format is BGR + Alpha
        ToonzPixelBGRM32 color(b, g, r, 255);
        canvas->setBrushColor(color);
    }

    void onEraserToggled(bool enabled) {
        eraserOn = enabled;

        if (enabled) {
            // Transparent brush
            ToonzPixelBGRM32 transparent(0, 0, 0, 0);
            canvas->setBrushColor(transparent);
        }
    }

private:
    DrawingWidget* canvas;
    bool eraserOn = false;
};

*/



#endif