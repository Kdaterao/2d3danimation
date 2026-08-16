#ifndef UIPAINTER_H
#define UIPAINTER_H
#include <QMatrix4x4>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>

#include <assert.h>
#include <iostream>
#include <fstream>

#include <toonzGeometry.h>
#include <toonzTextureManager.h>
#include <toonzShader.h>
#include <types.h>



/*
This is a class that handles drawing custom UI that will use the GPU


Approach:

    OpenGL has a context window, so anything that is stated will remain in the context unless we explicitly delete it....

    This means we can create multiple functions to create different types VAOs (texture(raster) vs vector(2d/3d geometry))

    We can layer these functions when implimenting this class

    to keep track of all the VAOs we can have an ordered map with {n: type, VAO } so we can encode how to handle it in the shaders 




*/




class UIPainter: public QOpenGLWidget, protected QOpenGLFunctions{

    int screenHeight; //full screen height
    int screenWidth; // full screen width 
    GLenum magFilter; //used for zoom in
    GLenum minFilter; //used for zoom out
    GLenum premultiplied; //used for displaying transparency
    float zlayer = 0.0;

    toonzShader *shader; //shader object
    std::vector<GLuint> VAOS; // vao ids
    std::vector<GLuint> buffers; //buffer ids
    std::vector<GLuint> textures; //texture ids
    std::vector<int> vertexCounts; //used for strde
    GLuint VAO; // current vao 


    int count = 0; //debugging purposes

    




    public:

    //----- constructor -----

        UIPainter() {};

        UIPainter(int i_screenWidth, int i_screenHeight, GLenum i_magFilter, GLenum i_minFilter, bool i_premultiplied, toonzShader *i_shader)
            : 
            magFilter(i_magFilter),
            minFilter(i_minFilter),
            premultiplied(i_premultiplied),
            shader(i_shader),
            screenHeight(i_screenHeight),
            screenWidth(i_screenWidth) {
                initializeOpenGLFunctions(); 
                i_shader->use();
        };

        ~UIPainter(){

            cleanUp();
        }

    //----- methods -----


        //void addTexture(RectTI box, UCHAR *rasbuffer, GLuint fbuffer, int wrap, isRGBM, int bpp, DimensionTI rasDim);

        void Paint(GLuint fbuffer, bool normalized);

        template <class T>
        void addGeometry(T shape){


            //----- get vertices -----

            shape.level = zlayer;//update layer!

            std::vector<float> vertices = shape.getVertices();

            //----- create Vertex Buffer Object -----

                        
            unsigned int  VBO, VAO;

            glGenVertexArrays(1, &VAO); 
            glBindVertexArray(VAO);

            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);

            //specify data of EBO and VAO
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

            // position attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            // color attribute
            glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3* sizeof(float)));
            glEnableVertexAttribArray(1);

            //ends the binding of the Array buffer
            glBindBuffer(GL_ARRAY_BUFFER, 0); 

            //------ update layer -----
            int vertexCount = vertices.size() / 7; // 7 floats per vertex
            vertexCounts.push_back(vertexCount);
            buffers.push_back(VBO);
            VAOS.push_back(VAO);
            zlayer += 1.0;

             GLenum err = glGetError();
            if (err != GL_NO_ERROR) {
                std::cout << "[OpenGL asdf after Error] code=" << err << std::endl;
            }


        };

        
        void cleanUp();


     };



#endif 