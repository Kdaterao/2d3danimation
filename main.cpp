#include <QApplication>
#include <CanvasWidget.h>
#include <BrushWidget.h>


#include <QObject>
#include <QWidget>     
#include <QLayout>  
int main(int argc, char *argv[]) {

    //----- Qt opengl window context -----

    //our specs
    QSurfaceFormat format;
    format.setVersion(3, 3); // Request OpenGL 4.1
    format.setProfile(QSurfaceFormat::CoreProfile); // Use Core Profile

    //set specs as our default 
    QSurfaceFormat::setDefaultFormat(format); 
    
    
    //----- inititialize Q application window -----

    QApplication app(argc, argv);


    QWidget *window = new QWidget();

    QHBoxLayout *layout = new QHBoxLayout(window)
;
    //----- Create window object -----
    GLWidget *canvas = new GLWidget();
    canvas->setMinimumSize(800, 600);
    canvas->setMaximumSize(800, 600);
    canvas->resize(800, 600); // sets window size (otherwise goes to default)
    
    //----- Brush Widget -----

    QWidget *BrushWidget = new QWidget();
    BrushWidget->setFixedSize(300, 300);

    QVBoxLayout* Brushlayout = new QVBoxLayout(BrushWidget);


    //color picker
    ColorTriangleWidget *ColorWidget = new ColorTriangleWidget(BrushWidget);
    ColorWidget->setFixedSize(200, 200);
            
    Brushlayout->addWidget(ColorWidget);

    // Eraser toggle
    QCheckBox* eraserToggle = new QCheckBox("Eraser", BrushWidget);
    Brushlayout->addWidget(eraserToggle);
        
    // Brush Size 
    int initialBrushSize = 5;
    QSlider* brushSizeSlider = new QSlider( BrushWidget);
    brushSizeSlider->setRange(1, 50 );
    brushSizeSlider->setValue(initialBrushSize);
    brushSizeSlider->setOrientation(Qt::Horizontal);
    Brushlayout->addWidget(brushSizeSlider);

    canvas->brushsize = initialBrushSize; 

    //----- connect widgets ------

    layout->addWidget(canvas);
    layout->addWidget(BrushWidget);

    //----- signal and slots -----
    QObject::connect(ColorWidget, &ColorTriangleWidget::sendColor, canvas, &GLWidget::updateBrushColor);
    QObject::connect(eraserToggle, &QCheckBox::toggled, canvas, &GLWidget::toggleEraser);
    QObject::connect(brushSizeSlider, QOverload<int>::of(&QSlider::valueChanged),canvas, &GLWidget::updateBrushSize); 

    window->show();

    return app.exec();
}