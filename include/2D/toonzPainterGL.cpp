#include <toonzPainterGL.h>
#include <toonzTextureManager.h>
#include <iostream>
#include <fstream>

void toonzPainterGL::dumpBuffer2(UCHAR* buf, int width, int height, int wrap) {

    std::cout<<"dumpbuffer2 running"<<std::endl;
    count += 1;
    std::string path = "/Users/krish/codingStuff/2d3danimation/output/dump";
    path += count;
    path += ".ppm";
      FILE* f = fopen(path.c_str(), "wb");
    if (!f) return;
    
    fprintf(f, "P6\n%d %d\n255\n", width, height);
    for (int row = 0; row < height; row++) {
        UCHAR* rowPtr = buf + row * wrap * 4;
        for (int col = 0; col < width; col++) {
            fwrite(rowPtr + col * 4, 1, 3, f);
        }
    }
    fclose(f);



    
}






void toonzPainterGL::PaintRaster(RectTI box, UCHAR *rasbuffer, GLuint fbuffer) {

    //----- Case 1: box is too big (Recursively break down boxes) -----

    //--- if too wide ---
    if (box.getLx() > maxSize.lx){

        int midX = box.x0 + box.getLx() / 2;


        RectTI leftBox  = RectTI(box.x0, box.y0, midX,   box.y1);  // x1 clamped
        RectTI rightBox = RectTI(midX,   box.y0, box.x1,  box.y1);

        toonzPainterGL::PaintRaster(leftBox, rasbuffer, fbuffer);
        toonzPainterGL::PaintRaster(rightBox, rasbuffer, fbuffer);
        return;
    } 

    //--- if too tall ---
    if(box.getLy() > maxSize.ly){

        int midY = box.y0 + box.getLy() / 2;


        RectTI bottomBox = RectTI(box.x0, box.y0, box.x1, midY  );
        RectTI topBox    = RectTI(box.x0, midY,   box.x1, box.y1);

        toonzPainterGL::PaintRaster(bottomBox, rasbuffer, fbuffer);
        toonzPainterGL::PaintRaster(topBox,    rasbuffer, fbuffer);
        return;
    }

    GLenum err;
    //------ case 2: box is small enough to upload pixels as texture -----
   
    //debug
    //std::cout<<"Box:"<<"("<<box.x0<<","<<box.y0<<"),"<<"("<<box.x1<<","<<box.y1<<"),"<<std::endl;

    
    // bind to widget framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, fbuffer);

    // verify(debug)
    /*
    GLenum fbStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    std::cout << "FBO status: " << fbStatus << " FBO id: " << fbuffer << std::endl;
    */

    //----- Texture settings -----

        
    //bind empty texture pointer (IMPORTANT DO BEFORE MIPMAP)
    unsigned int texture1;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    //only change this box on the screen
    //glEnable(GL_SCISSOR_TEST);
    //glScissor(screenWidth - box.x0 - box.getLx(), screenHeight - box.y0 - box.getLy(), box.getLx(), 200); //our buffer coordinate space is top left while opengl is bottom left.

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
    
    err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cout << "[OpenGL texture settings error] code=" << err << std::endl;
    }



    //----- Fill Empty texture object -----

    //handle format and type of buffer
    GLenum fmt, type;
    toonzTextureManager::instance()->getFmtAndType(isRGBM, fmt, type);


    //create empty texture 
    DimensionTI ts = toonzTextureManager::instance()->createEmptyTexture(box.getSize(), isRGBM);


    //upload pixels to buffer
    int width  = box.getLx(); 
    int height = box.getLy();
    int x      = box.x0;
    int y      = box.y0;


    UCHAR* bufferOffset = rasbuffer + (x + y * wrap) * bpp;

    //toonzPainterGL::dumpBuffer2(bufferOffset, width, height, wrap); //debug function

    // print them before the call
    //std::cout << "fmt=" << fmt << " type=" << type << " wrap=" << wrap << " bpp=" << bpp << std::endl;

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    //glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, fmt, type, bufferOffset);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, bufferOffset);

     /*
    err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cout << "[OpenGL texture creation error] code=" << err << std::endl;
    }


    std::cout << "[Debug] Uploading texture data:" 
              << "subwidth=" << width 
              << ", subheight=" << height
              << ", Rect start Coord=" << "(" << x <<"," << y <<")"
              << ", buffer offset=" << (x + y * wrap) * bpp 
              << ", wrap=" << wrap << std::endl;

    char* pcolor = reinterpret_cast<char*>(bufferOffset);


    std::cout<<"color:"<<"("
            <<(int)(unsigned char)pcolor[0]<<","
            <<(int)(unsigned char)pcolor[1]<<","
            <<(int)(unsigned char)pcolor[2]<<","
            <<(int)(unsigned char)pcolor[3]<<","
            <<")"<<","<<
            "size of one char:"<<
            sizeof(pcolor[0])<<
            std::endl;
    */

    
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

    /*
    std::cout << "[Debug] Vertices: "
        << "(" << v0.x << "," << v0.y << "), "
        << "(" << v1.x << "," << v1.y << "), "
        << "(" << v2.x << "," << v2.y << "), "
        << "(" << v3.x << "," << v3.y << ")"
        << ", texture coords s=" << s << ", t=" << t << std::endl;
    */

    float vertices[] = {
              // position        //texture coords
            v3.x,  v3.y, 0.0f,        s,    t, // top right
            v1.x,  v1.y, 0.0f,        s, 0.0f, // bottom right
            v0.x,  v0.y, 0.0f,     0.0f, 0.0f, // bottom left
            v2.x,  v2.y, 0.0f,     0.0f,    t     // top left 
    };

    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    
    //----- create Vertex array object -----

    unsigned int VAO, VBO, EBO;

    //create VAO VBO EBO
    glGenVertexArrays(1, &VAO); //NOTE: everything after vao gets binded
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    //specify data of EBO and VAO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    
    //specify strides vectices[]
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5* sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);  

    //ends the binding of the Array buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    //----- create orthorgraphic projection -----
    QMatrix4x4 projection;
    projection.ortho(0.0f, screenWidth, 0.0f, screenHeight, -1.0f, 1.0f);


    QMatrix4x4 model;
    model.translate(
        box.x0 + box.getLx() * 0.5f,                
        screenHeight - box.y0 - box.getLy() * 0.5f, 
        0.0f
    );


    //----- set shader -----
    shader->use();

    //----- set uniforms  -----

    shader->setInt("texture1", 0);
    shader->set4X4M("projection", projection);
    shader->set4X4M("model", model);

    
    //needed for textures ig
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glBindVertexArray(VAO);





    //----- bind vertext array -----

    glBindVertexArray(VAO);

    //----- draw image segment -----


    glDisable(GL_BLEND);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); 

    /*
    err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cout << "[OpenGL drawing Error] code=" << err << std::endl;
    }
    */

    //----- clean up-----

    //glDisable(GL_SCISSOR_TEST);
    glBindVertexArray(0); 
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &texture1);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

}



