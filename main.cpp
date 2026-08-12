#include <QApplication>
#include <CanvasWidget.h>
#include <ColorTriangleWidget.h>
#include <ToolOptionWidget.h>
#include <TimelineWidget.h>
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

    QVBoxLayout *rootLayout = new QVBoxLayout(window);
    rootLayout->setContentsMargins(8, 8, 8, 8);
    rootLayout->setSpacing(6);

    QWidget *topRow = new QWidget(window);
    QHBoxLayout *layout = new QHBoxLayout(topRow);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    //----- Create window object -----
    GLWidget  *canvas= new GLWidget();
    canvas->setFixedSize(800, 600);
    
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

    //----- Top row: canvas + sidebar -----
    layout->addWidget(canvas);
    layout->addWidget(sidebar);
    rootLayout->addWidget(topRow, 1);

    //----- Timeline -----
    TimelineWidget *timeline = new TimelineWidget(canvas->getCanvas(), window);
    timeline->setMinimumHeight(140);
    timeline->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    rootLayout->addWidget(timeline);

    // Width floor from fixed canvas + sidebar; height comes from layout (canvas 600 + timeline).
    window->setMinimumWidth(1100);
    window->resize(1300, 880);

    //----- signal and slots -----

    //color widget
    QObject::connect(ColorWidget, &ColorTriangleWidget::sendColor, canvas, &GLWidget::updateBrushColor);
    

    //tool options widget
    QObject::connect(toolOptions, &ToolOptionWidget::eraserToggled, canvas, &GLWidget::toggleEraser);
    QObject::connect(toolOptions, &ToolOptionWidget::brushSizeChanged, canvas, &GLWidget::updateBrushSize);
    QObject::connect(toolOptions, &ToolOptionWidget::brushSelected, canvas, &GLWidget::selectBrush);

    //timeline widget
    QObject::connect(timeline, &TimelineWidget::timeChanged, canvas, &GLWidget::onTimeChanged);
    QObject::connect(timeline, &TimelineWidget::activeLayerChanged, canvas, &GLWidget::onActiveLayerChanged);
    QObject::connect(timeline, &TimelineWidget::timelineEdited, canvas, &GLWidget::onTimelineEdited);

    window->show();

    return app.exec();
}
