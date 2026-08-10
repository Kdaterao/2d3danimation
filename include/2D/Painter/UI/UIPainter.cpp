#include <UIPainter.h>
#include <toonzTextureManager.h>
#include <iostream>
#include <fstream>
#include <QMatrix4x4>
#include <QOpenGLFunctions>




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


//---- clean up the current objects -----
void UIPainter::cleanUp(){

    glBindVertexArray(0); 
    glBindTexture(GL_TEXTURE_2D, 0);

    zlayer = 0.0;

    //----- delete buffers ------

    for (auto& vao : VAOS) {
        glDeleteVertexArrays(1, &vao);
    }
    VAOS.clear();

    for (auto& buf : buffers) {
        glDeleteBuffers(1, &buf); //
    }
    buffers.clear();
    vertexCounts.clear();

    //------ deletes textures ------
    for(auto& tex : textures){
        glDeleteTextures(1, &tex);
    }
    textures.clear();
}

