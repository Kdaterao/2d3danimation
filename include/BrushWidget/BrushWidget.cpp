#include <BrushWidget.h>




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

    std::cout<<"YAT!"<<std::endl;

    ToonzPixelBGRM32 color = ToonzPixelBGRM32(255*u, 255*v, 255*w, 255);
    emit sendColor(color);

    }

 }




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

//------ visual ------


 void ColorTriangleWidget::initializeGL() {
     //----- setup Opengl ------
    initializeOpenGLFunctions();
    setUpdateBehavior(QOpenGLWidget::PartialUpdate);//stops Qwidget from constantly overwriting the screen


    //----- shaders ------

    shaderProgram = new toonzShader(
        "/Users/krish/codingStuff/2d3danimation/include/shaders/vsColorTriangle.txt", 
        "/Users/krish/codingStuff/2d3danimation/include/shaders/fsColorTriangle.txt"
    );

    //----- set up rasterizer ------

    rasterizer =  new UIPainter(this->width(), this->height(),  
                                GL_NEAREST, GL_NEAREST, 
                                false, shaderProgram);


    //----- place holder triangle (will replace with better one later (like the one in opentoonz!)) -----

    VertexRGBM32 v0(0.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f);  // Top vertex: Red
    VertexRGBM32 v1(-1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f); // Bottom-left: Green
    VertexRGBM32 v2(1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);  // Bottom-right: Blue

    Triangle shape = Triangle(v0, v1, v2);



    VertexRGBM32 v0a(0.0f,  0.1f, 1.0f, 1.0f, 1.0f, 1.0f);  // Top vertex: Red
    VertexRGBM32 v1a(-0.1f,-0.1f, 1.0f, 1.0f, 1.0f, 1.0f); // Bottom-left: Green
    VertexRGBM32 v2a(0.1f, -0.1f, 1.0f, 1.0f, 1.0f, 1.0f);  // Bottom-right: Blue

    Triangle shape2 = Triangle(v0a, v1a, v2a);

    rasterizer->addGeometry(shape);
    rasterizer->addGeometry(shape2);
 }



 void ColorTriangleWidget::paintGL(){
    rasterizer->Paint(defaultFramebufferObject(), true);

 };