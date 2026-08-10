#include <QApplication>
#include <CanvasWidget.h>
#include <ColorTriangleWidget.h>
#include <ToolOptionWidget.h>
#include <rasterPixel.h>


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
    GLWidget  *canvas= new GLWidget();
    canvas->setMinimumSize(800, 600);
    canvas->setMaximumSize(800, 600);
    canvas->resize(800, 600); // sets window size (otherwise goes to default)
    
    //----- Sidebar -----

    QWidget *sidebar = new QWidget();
    sidebar->setFixedWidth(300);

    QVBoxLayout* sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(0, 0, 0, 0);
    sidebarLayout->setSpacing(4);

    //color picker
    ColorTriangleWidget *ColorWidget = new ColorTriangleWidget(sidebar);
    ColorWidget->setFixedSize(300, 300);
            
    sidebarLayout->addWidget(ColorWidget);

    int initialBrushSize = 5;
    ToolOptionWidget* toolOptions = new ToolOptionWidget(initialBrushSize, sidebar);
    sidebarLayout->addWidget(toolOptions, 1);

    canvas->brushsize = initialBrushSize; 

    //----- connect widgets ------

    layout->addWidget(canvas);
    layout->addWidget(sidebar);

    window->setMinimumSize(1100, 650);
    window->resize(1300, 650);

    //----- signal and slots -----
    QObject::connect(ColorWidget, &ColorTriangleWidget::sendColor, canvas, &GLWidget::updateBrushColor);
    QObject::connect(toolOptions, &ToolOptionWidget::eraserToggled, canvas, &GLWidget::toggleEraser);
    QObject::connect(toolOptions, &ToolOptionWidget::brushSizeChanged, canvas, &GLWidget::updateBrushSize);
    QObject::connect(toolOptions, &ToolOptionWidget::brushSelected, canvas, &GLWidget::selectBrush); 

    window->show();

    return app.exec();
}