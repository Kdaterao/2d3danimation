#include <ColorTriangleWidget.h>


//---------------------------------
//      Color Triangle Logic      
//---------------------------------

void ColorTriangleWidget::getColor(QMouseEvent *event){

     //------ variables ------
    /*

    Note: I reversed coordinate space of qt so that i could treat "A" as origin
        Triangle
            B
         *    *
        A ***** C

        mouse point = pos (P)
    */

    QPoint q = event->pos(); 
    PointTF pos = PointTF(q.x(), q.y()); //mouse position
    int width = this->width(); //width of widget
    int height = this->height(); // hieght of widget
    PointTF v2 = PointTF(q.x(), height - q.y()); //P - A (had to reverse the coordinates space for the mouse from top left to bottom left )
    PointTF v1 = PointTF(width/2, height); //B - A
    PointTF v0 = PointTF(width, 0); //C -A
\


    //----- calculate baycentric coordinates ------
    /*
        v2* v0 = u(v1 * v0) + w(v0 *v0)
        v2 * v1 = u(v1 * v1) + w(v0 * v1)

        [ a  b ]  [ u ] = [ e ]
        [ c  d  ] [ w ] = [ f ]
        
    */
    
    //get a,b,c,d,e,f
    float a = v1*v0;
    float b = v0*v0;
    float c = v1*v1;
    float d = v0*v1;

    float e = v2*v0;
    float f = v2*v1;

    //solve for u and w via cramer's rule
    float denominator = (a*d) - (b*c);
    float u = ((e*d) - (f*b))/denominator;
    float w = ((a*f) - (e*c))/denominator;

    float v = 1 - u - w; 
    

    if( u >= 0 && u <= 1 && w >= 0 && w <= 1 && v >= 0 && v <= 1 ){

    //std::cout<<"YAT!"<<std::endl;

PixelType color = PixelType(
    255.0f * v * krish64,  // RED   → A
    255.0f * u * krish64,  // GREEN → B
    255.0f * w * krish64,  // BLUE  → C
    255.0f * krish64
);
    emit sendColor(color);

    }

 }

//---------------------------------
//     Input Handling Logic      
//---------------------------------


void ColorTriangleWidget::mousePressEvent(QMouseEvent *event) {

    if(event->buttons() & Qt::LeftButton){
    getColor(event);
    }
    
}

void ColorTriangleWidget::mouseMoveEvent(QMouseEvent *event){
    if(event->buttons() & Qt::LeftButton){
    getColor(event);

    }

}

//-----------------------------------
// Color Triangle Visualization Logic    
//-----------------------------------

 void ColorTriangleWidget::initializeGL() {
     //----- setup Opengl ------
    initializeOpenGLFunctions();
    setUpdateBehavior(QOpenGLWidget::PartialUpdate);//stops Qwidget from constantly overwriting the screen

    //----- shaders ------

    shaderProgram = new toonzShader(
        "/Users/krish/codingStuff/2d3danimation/include/2D/Painter/General/Shader/ShaderScripts/vsColorTriangle.txt", 
        "/Users/krish/codingStuff/2d3danimation/include/2D/Painter/General/Shader/ShaderScripts/fsColorTriangle.txt"
    );

    //----- set up rasterizer (UIPainter) ------

    rasterizer =  new UIPainter(this->width(), this->height(),  
                                GL_NEAREST, GL_NEAREST, 
                                false, shaderProgram);


    //----- place holder triangle (will replace with better one later (like the one in opentoonz!)) -----

    VertexRGBM32 v0(0.0f,  1.0f, 0.0f, 1.0f, 0.0f, 1.0f);  // Top, Green
    VertexRGBM32 v1(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f); // Bottom-left, Blue
    VertexRGBM32 v2(1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f);  // Bottom-right, Red

    Triangle shape = Triangle(v0, v1, v2);
    rasterizer->addGeometry(shape);

 }

 void ColorTriangleWidget::paintGL(){
    rasterizer->Paint(defaultFramebufferObject(), true);
 };
