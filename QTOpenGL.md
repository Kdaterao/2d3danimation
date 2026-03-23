# OpenGL pipline within QT


## Overview 

* QT is not like glfw(which is a for loop), and instead works via signal and slots...

* This means that widgets/UI for Opengl is rendered over again, based on whether a specific event was fired 

* QT uses <QOpenGLWidget> to handle windows for opengl

* QT also supplies us with <QOpenGLFunctions>, which are openglfunctions that we know are compatible with qt


## How is OpenGl initialized?

* QOpenGLWidget::initializeGL() is called only once (before the first call of our resizeGL or our painGL) and it contains logic which is used to initialize static elements in our opengl widget

* Example 
```
//note: if we want glClearColor to have a different color every frame, we must put it inside paintGL()...

void GLWidget::initializeGL() {
    // pointer to all OpenGL functions
    initializeOpenGLFunctions();  
    
    // specifiy back color buffer
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f); // static background color
}
```


## Pipeline(based on timer)

* we always repaint our widget by calling the update() function  over and over again

```
      update()  or (window resized) 
                ↓
    Qt event loop schedules paint event
                ↓
    Qt makes OpenGL context current
                ↓
        (if size changed) → resizeGL()
                ↓
            paintGL()  ← your actual render code
                ↓
        Swap buffers / display frame

```


## Example of how to create a basic timer loop 

* for syntax help check out qt.md for specifics

* Note: we set up the timer loop in our constructor so we guaruntee our widget object will always have an update loop!

```
GLWidget::GLWidget(QWidget* parent) : QOpenGLWidget(parent) {
    // Simple render loop (~60 FPS)
    QTimer* timer = new QTimer(this); // timer object
    connect(timer, &QTimer::timeout, 
            this, QOverload<>::of(&GLWidget::update)); //signal and slots...
    timer->start(16); //starts timer which a timeout specified
}

```


## Typical way to create a custom QOpenGLWidget?

* Inherit  QOpenGLWidget(handles window functions) and QOpenGLFunctions(all normal opengl functions that we can use )

* override the methods of the QOpenGLWidget and provide our own custom implimentations

  - Methods are typically [initializeGL(), resizeGL(), paintGL() ] since thats basically what determines the logic of painting the window
```

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
public:
    explicit GLWidget(QWidget* parent = nullptr);

protected:
    void initializeGL() override; // called before 
    void resizeGL(int w, int h) override;
    void paintGL() override;
};

```