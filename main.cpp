#include <QApplication>
#include <CanvasWidget/CanvasWidget.h>

int main(int argc, char *argv[]) {

    //----- inititialize Q application window -----
    QApplication app(argc, argv);

    //----- Create window object -----
    GLWidget window;
    window.resize(800, 600); // sets window size (otherwise goes to default)
    window.show(); // sets our window to visible 

    return app.exec();
}