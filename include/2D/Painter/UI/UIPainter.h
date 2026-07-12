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

    toonzShader *shader;
    std::vector<GLuint> VAOS;
    std::vector<GLuint> buffers;
    std::vector<GLuint> textures;
    std::vector<int> vertexCounts;
    GLuint VAO;


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








        /*
        void testPipline(GLuint fbuffer) {

             glBindFramebuffer(GL_FRAMEBUFFER, fbuffer);


            float vertices[] =              { 0.0f, 0.5f, 0.0f,      1.0f, 0.0f, 0.0f, 1.0f, // top-middle
                                            -0.5f, -0.5f, 0.0f,     0.0f, 1.0f, 0.0f, 1.0f, // bottom-right
                                            0.5f, -0.5f, 0.0f,     0.0f, 0.0f, 1.0f, 1.0f // top-left

                                            };


            //----- create Vertex Buffer Object -----

                        
            unsigned int  VBO, VAO;

            glGenVertexArrays(1, &VAO); 
            glBindVertexArray(VAO);

            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);

            //specify data of EBO and VAO
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            // position attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            // color attribute
            glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3* sizeof(float)));
            glEnableVertexAttribArray(1);

            //ends the binding of the Array buffer
            glBindBuffer(GL_ARRAY_BUFFER, 0); 

            //------ update layer -----
            int vertexCount = 3; // 6 floats per vertex
            vertexCounts.push_back(vertexCount);
            buffers.push_back(VBO);
            VAOS.push_back(VAO);
            zlayer += 1.0;

             GLenum err = glGetError();
            if (err != GL_NO_ERROR) {
                std::cout << "[OpenGL asdf after Error] code=" << err << std::endl;
            }


            
            //----- create orthorgraphic projection -----
            QMatrix4x4 projection;
            //projection.ortho(0.0f, screenWidth, 0.0f, screenHeight, -1.0f, 1.0f);
            projection.setToIdentity();

            QMatrix4x4 model;
            model.translate(
                screenWidth/2,                
                screenHeight/2, 
                1.0f
            );
            model.setToIdentity();

            //----- set shader -----
            shader->use();

            //----- set uniforms  -----

            shader->set4X4M("projection", projection);
            shader->set4X4M("model", model);

            

            //----- draw our layers -----
            int i = 0;
            for(auto& vao : VAOS){
                glBindVertexArray(vao);

                //----- draw image segment -----
                std::cout<<"drawing"<<std::endl;

                glDisable(GL_BLEND);

                glDrawArrays(GL_TRIANGLES, 0, vertexCounts[i]);
                i++;

                        
                GLenum err = glGetError();
                if (err != GL_NO_ERROR) {
                    std::cout << "[OpenGL drawing Error] code=" << err << std::endl;
                }

            }

        }
        */
     };



#endif 