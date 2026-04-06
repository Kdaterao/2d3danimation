#include <UIPainter.h>
#include <toonzTextureManager.h>
#include <iostream>
#include <fstream>
#include <QMatrix4x4>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>




/*
 I want this to be a general purpose painter ofr UI, so that I just plop this in whereever, and I get some specified ui..

 Which operations to support?: 
    Drawing images --> raster buffer --> draw via texture 
    Drawing basic geometry shapes --> vertex buffer --> draw via triangle

    Order... So I also need to define how one can layer these, obviously we do this via the z axis, but I would like it so a person can put as many layers as they want..

    I could do a vertex of pointers as input, and the order of the pointers is the order in how the image gets rendered?

    I can do this by having a vertex of VAOs and them render them in the exact order we define later on

*/






void UIPainter::Paint(GLuint fbuffer, bool normalized) {

    

    glBindFramebuffer(GL_FRAMEBUFFER, fbuffer);

    glClearColor(0.5, 0.5, 0.5, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    //----- create orthorgraphic projection -----
    QMatrix4x4 model;
    QMatrix4x4 projection;
    if(!normalized){

        projection.ortho(0.0f, screenWidth, 0.0f, screenHeight, -1.0f, 1.0f);

        model.translate(
            screenWidth/2,                
            screenHeight/2, 
            1.0f
        );
    }   else {

         projection.setToIdentity();
         model.setToIdentity();
    }


    //----- set shader -----
    shader->use();

    //----- set uniforms  -----

    shader->setInt("texture1", 0);
    shader->set4X4M("projection", projection);
    shader->set4X4M("model", model);

    


    //----- bind textures to shader -----
    /*
    //bind textures to shader
    for (int i = 0; i < textures.size(); ++i) {
            glActiveTexture(GL_TEXTURE0 + i); // activate texture unit i
            glBindTexture(GL_TEXTURE_2D, textures[i]); // bind the texture to GL_TEXTURE_2D
            shader->setInt(("texture" + std::to_string(i)).c_str(), i); // link sampler to correct unit
    }
    */
    //----- draw our layers -----
    int i = 0;
    for(auto& vao : VAOS){
        glBindVertexArray(vao);

        //----- draw image segment -----
        //std::cout<<"drawing"<<std::endl;

        glDisable(GL_BLEND);

        glDrawArrays(GL_TRIANGLES, 0, vertexCounts[i]);
        i++;

                
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cout << "[OpenGL drawing Error] code=" << err << std::endl;
        }

    }

}



void UIPainter::cleanUp(){

    glBindVertexArray(0); 
    glBindTexture(GL_TEXTURE_2D, 0);

    //----- delete buffers + textures ------

    for(auto& vao : VAOS){
        glDeleteBuffers(1, &vao);
    }
    glDeleteVertexArrays(1, &VAO);

    for(auto& buf : buffers){
        glDeleteBuffers(1, &buf);
    }

    for(auto& tex : textures){
        glDeleteTextures(1, &tex);
    }

}




/*
function is (Probably) fully implimented but not really need atm and i dont wanna debug this for now
void toonzPainterGL::addTexture(RectTI box, UCHAR *rasbuffer, GLuint fbuffer, int wrap) {

    //----- Case 1: box is too big (Recursively break down boxes) -----

    //--- if too wide ---
    if (box.getLx() > maxSize.lx){


        //std::cout<<"Box:"<<"("<<(box.x0) <<","<<(box.y0) <<")"<<","<<"("<<(box.x1) <<","<<(box.y1)<<")"<<std::endl;

        int midX = box.x0 + box.getLx() / 2;
        
        
        RectTI leftBox  = RectTI(box.x0, box.y0, midX,  box.y1);  // x1 clamped
        RectTI rightBox = RectTI(midX,   box.y0, box.x1,  box.y1);

        addTexture(leftBox, rasbuffer,fbuffer, wrap);
        addTexture(rightBox, rasbuffer, fbuffer, wrap);
        return;
    } 

    //--- if too tall ---
    if(box.getLy() > maxSize.ly){

        int midY = box.y0 + box.getLy() / 2;

        RectTI bottomBox = RectTI(box.x0, box.y0, box.x1, midY  );
        RectTI topBox    = RectTI(box.x0, midY,   box.x1, box.y1);



        
        addTexture(bottomBox, rasbuffer,fbuffer, wrap);
        addTexture(topBox, rasbuffer, fbuffer, wrap);
        return;
    }

    //----- Texture settings ----- 
    //bind empty texture pointer(must be done before mag_filter and min_filter)
    unsigned int texture1;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);


    //reads buffer with 1 byte alignment
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 
    
    glEnable(GL_BLEND);

    //clamp edges
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    
    //configure magnify and minimize
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);

    //handle premultiplcation
    if (premultiplied){ 
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }


    //----- Fill Empty texture object -----

    //handle format and type of buffer
    GLenum fmt, type;
    toonzTextureManager::instance()->getFmtAndType(isRGBM, fmt, type);


    //create empty texture 
    DimensionTI ts = toonzTextureManager::instance()->createEmptyTexture(box.getSize(), isRGBM, fmt, type);


    //upload pixels to buffer
    int width  = box.getLx(); 
    int height = box.getLy();
    int x      = box.x0;
    int y      = box.y0;


    UCHAR* bufferOffset = rasbuffer + (x + y * wrap) * bpp;

    //toonzPainterGL::dumpBuffer2(bufferOffset, width, height, wrap); //debug function

    // print them before the call
    //std::cout << "fmt=" << fmt << " type=" << type << " wrap=" << wrap << " bpp=" << bpp << std::endl;

    glPixelStorei(GL_UNPACK_ROW_LENGTH, wrap); 
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, fmt, GL_UNSIGNED_BYTE, bufferOffset);//fmt, type
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, fmt, GL_UNSIGNED_BYTE, bufferOffset);

     
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cout << "[OpenGL texture creation error] code=" << err << std::endl;
    }


    //----- create our vertice arrays -----

    float halfWidth  = 0.5 * box.getLx(); //lx*0.5 
    float halfHeight = 0.5 * box.getLy(); //ly*0.5 


    //these are flipped on purpose!
    PointTF v2(-halfWidth, -halfHeight);  // Bottom-left
    PointTF v3( halfWidth, -halfHeight);  // Bottom-right
    PointTF v0(-halfWidth,  halfHeight);  // Top-left
    PointTF v1( halfWidth,  halfHeight);  // Top-right


    float s = (box.getLx()) / (float)ts.lx;//normalized lx
    float t = (box.getLy()) / (float)ts.ly;//normalized ly


    float vertices[] = {
              // position           //texture coords
            v3.x,  v3.y, zlayer,        s,    t, // top right (0)
            v1.x,  v1.y, zlayer,        s, 0.0f, // bottom right (1)
            v2.x,  v2.y, zlayer,     0.0f,    t,     // top left (3)

            v1.x,  v1.y, zlayer,        s, 0.0f, // bottom right (1)
            v0.x,  v0.y, zlayer,     0.0f, 0.0f, // bottom left (2)
            v2.x,  v2.y, zlayer,     0.0f,    t     // top left (3)
    };

 
    //----- create Vertex Buffer Object -----

    unsigned int  VBO;

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    //specify data of EBO and VAO
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texture attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);  

    //ends the binding of the Array buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    //ends the binding of the texture 
    glBindTexture(GL_TEXTURE_2D, 0);

    //------ save VBO  and texture ------
    textures.push_back(texture1);
    buffers.push_back(VBO);
    //------ update layer -----
    zlayer += 1.0;

}

*/



