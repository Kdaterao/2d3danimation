#include <QApplication>
#include <CanvasWidget.h>

int main(int argc, char *argv[]) {


    QSurfaceFormat format;
    format.setVersion(4, 1); // Request OpenGL 3.3
    format.setProfile(QSurfaceFormat::CoreProfile); // Use Core Profile
    QSurfaceFormat::setDefaultFormat(format);


    //----- inititialize Q application window -----
    QApplication app(argc, argv);

    //----- Create window object -----
    GLWidget window;
    window.resize(800, 600); // sets window size (otherwise goes to default)
    window.show(); // sets our window to visible 

    return app.exec();
}