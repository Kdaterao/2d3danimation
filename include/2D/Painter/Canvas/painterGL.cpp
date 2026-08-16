#include <painterGL.h>
#include <toonzTextureManager.h>
#include <pipelineLogger.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <fstream>



//for debugging
template <class T>
void toonzPainterGL::dumpBuffer2(T* buf, int width, int height, int wrap, int num_channels) {

    std::cout<<"dumpbuffer2 running"<<std::endl;
    count += 1;
    std::string path = "/Users/krish/codingStuff/2d3danimation/output/dump";
    path += count;
    path += ".ppm";
      FILE* f = fopen(path.c_str(), "wb");
    if (!f) return;
    
    fprintf(f, "P6\n%d %d\n255\n", width, height);
    for (int row = 0; row < height; row++) {
        T* rowPtr = buf + row * wrap * num_channels;
        for (int col = 0; col < width; col++) {
            fwrite(rowPtr + col * num_channels, 1, 3, f);
        }
    }
    fclose(f);
   
}






//========== Lazy createion of tile VAO ============

void toonzPainterGL::ensureQuadVAO() {
    if (quadReady) return;

    const float vertices[] = {
        // position              // texture coords
         0.5f, -0.5f, 0.0f,      1.0f, 1.0f, // top right  (v3)
         0.5f,  0.5f, 0.0f,      1.0f, 0.0f, // bottom right (v1)
        -0.5f,  0.5f, 0.0f,      0.0f, 0.0f, // bottom left  (v0)
        -0.5f, -0.5f, 0.0f,      0.0f, 1.0f  // top left     (v2)
    };

    const unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);

    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &quadEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STREAM_DRAW);

    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    quadReady = true;
}

//========= Used to delete tile VAO at end of program ================
void toonzPainterGL::destroyQuadVAO() {
    if (!quadReady) return;

    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteBuffers(1, &quadEBO);
    glDeleteBuffers(1, &instanceVBO);

    quadVAO = 0;
    quadVBO = 0;
    quadEBO = 0;
    instanceVBO = 0;
    quadReady = false;
}



toonzPainterGL::LayerTexture& toonzPainterGL::ensureLayerTexture(
    int layerKey, int width, int height, GLenum fmt, GLenum type)
{

    //======= grabs texture id ==========
    LayerTexture& tex = layerTextures[layerKey];
    const bool needCreate = (tex.id == 0); //texture id cannot be 0 thus, we prolly need 

    
    const bool needAlloc = needCreate // see if we need to expand our atlas texture 
        || width > tex.w
        || height > tex.h
        || fmt != tex.fmt
        || type != tex.type;

    if (needCreate) {
        PipelineScope _texGen(PipelineStage::TexGen); // debug
        
        glGenTextures(1, &tex.id);
        glBindTexture(GL_TEXTURE_2D, tex.id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    } else {
        glBindTexture(GL_TEXTURE_2D, tex.id);
    }

    if (needAlloc) {
        PipelineScope _texAlloc(PipelineStage::TexAlloc);
        tex.w = std::max(tex.w, width);
        tex.h = std::max(tex.h, height);
        tex.fmt = fmt;
        tex.type = type;
        toonzTextureManager::instance()->createEmptyTexture(
            DimensionTI(tex.w, tex.h), isRGBM, fmt, type);
    }
    return tex;
}

void toonzPainterGL::destroyLayerTextures() {
    for (auto& [key, tex] : layerTextures) {
        (void)key;
        if (tex.id != 0) glDeleteTextures(1, &tex.id);
    }
    layerTextures.clear();
}

void toonzPainterGL::PaintRaster(RectTI box, UCHAR *rasbuffer, GLuint fbuffer, bool isPreview) {
    PaintRaster(box, rasbuffer, fbuffer, isPreview ? previewSettings : tileSettings);
}

void toonzPainterGL::PaintRaster(RectTI box, UCHAR *rasbuffer, GLuint fbuffer, const PaintSettings& settings) {

    //----- Case 1: box is too big (Recursively break down boxes) -----

    //--- if too wide ---
    if (box.getLx() > maxSize.lx){
        int midX = box.x0 + box.getLx() / 2;
        RectTI leftBox  = RectTI(box.x0, box.y0, midX,  box.y1);
        RectTI rightBox = RectTI(midX,   box.y0, box.x1,  box.y1);
        toonzPainterGL::PaintRaster(leftBox, rasbuffer, fbuffer, settings);
        toonzPainterGL::PaintRaster(rightBox, rasbuffer, fbuffer, settings);
        return;
    }

    //--- if too tall ---
    if(box.getLy() > maxSize.ly){
        int midY = box.y0 + box.getLy() / 2;
        RectTI bottomBox = RectTI(box.x0, box.y0, box.x1, midY  );
        RectTI topBox    = RectTI(box.x0, midY,   box.x1, box.y1);
        toonzPainterGL::PaintRaster(bottomBox, rasbuffer, fbuffer, settings);
        toonzPainterGL::PaintRaster(topBox,    rasbuffer, fbuffer, settings);
        return;
    }

    std::vector<RasterPaintTile> one{{box, rasbuffer}};
    PaintRasterTiles(kLayerKeyScratch, one, fbuffer, settings);
}

void toonzPainterGL::PaintRasterTiles(int layerKey, const std::vector<RasterPaintTile>& tiles, GLuint fbuffer, bool isPreview) {
    PaintRasterTiles(layerKey, tiles, fbuffer, isPreview ? previewSettings : tileSettings);
}

void toonzPainterGL::PaintRasterTiles(
    int layerKey,
    const std::vector<RasterPaintTile>& tiles,
    GLuint fbuffer,
    const PaintSettings& settings)
{
    if (tiles.empty()) return;

    PipelineScope _paintRaster(PipelineStage::PaintRaster);

    struct PackedTile {
        int packX = 0;
        int packY = 0;
        int w = 0;
        int h = 0;
        const RasterPaintTile* src = nullptr;
    };

    std::vector<PackedTile> packed;
    packed.reserve(tiles.size());

    int cursorX = 0;
    int cursorY = 0;
    int rowH = 0;
    int atlasW = 0;
    int atlasH = 0;

    for (const RasterPaintTile& tile : tiles) {
        if (!tile.data) continue;
        const int w = tile.box.getLx();
        const int h = tile.box.getLy();
        if (w <= 0 || h <= 0) continue;
        if (w > maxSize.lx || h > maxSize.ly) {
            PaintRaster(tile.box, tile.data, fbuffer, settings);
            continue;
        }
        if (cursorX > 0 && cursorX + w > maxSize.lx) {
            cursorX = 0;
            cursorY += rowH;
            rowH = 0;
        }
        PackedTile p;
        p.packX = cursorX;
        p.packY = cursorY;
        p.w = w;
        p.h = h;
        p.src = &tile;
        packed.push_back(p);
        cursorX += w;
        rowH = std::max(rowH, h);
        atlasW = std::max(atlasW, cursorX);
        atlasH = cursorY + rowH;
    }

    if (packed.empty() || atlasW <= 0 || atlasH <= 0) return;

    glBindFramebuffer(GL_FRAMEBUFFER, fbuffer);
    ensureQuadVAO();

    GLenum fmt, type;
    toonzTextureManager::instance()->getFmtAndType(isRGBM, is32, fmt, type);
    LayerTexture& tex = ensureLayerTexture(layerKey, atlasW, atlasH, fmt, type);

    {
        PipelineScope _pack(PipelineStage::AtlasPack);
        atlasScratch.assign((size_t)atlasW * (size_t)atlasH * (size_t)bpp, 0);
        const int srcWrap = wrap;
        for (const PackedTile& p : packed) {
            for (int row = 0; row < p.h; ++row) {
                UCHAR* dst = atlasScratch.data() + ((p.packY + row) * atlasW + p.packX) * bpp;
                const UCHAR* src = p.src->data + row * srcWrap * bpp;
                std::memcpy(dst, src, (size_t)p.w * (size_t)bpp);
            }
        }
    }

    {
        PipelineScope _glState(PipelineStage::GlState);
        const float dpr = this->devicePixelRatioF();
        int scx0 = packed[0].src->box.x0;
        int scy0 = packed[0].src->box.y0;
        int scx1 = packed[0].src->box.x1;
        int scy1 = packed[0].src->box.y1;
        for (const PackedTile& p : packed) {
            scx0 = std::min(scx0, p.src->box.x0);
            scy0 = std::min(scy0, p.src->box.y0);
            scx1 = std::max(scx1, p.src->box.x1);
            scy1 = std::max(scy1, p.src->box.y1);
        }
        glEnable(GL_SCISSOR_TEST);
        glScissor(scx0 * dpr, (screenHeight - scy1) * dpr, (scx1 - scx0) * dpr, (scy1 - scy0) * dpr);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glEnable(GL_BLEND);
        if (premultiplied) {
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        } else {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
    }

    {
        PipelineScope _texUpload(PipelineStage::TexUpload);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, atlasW);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, atlasW, atlasH, fmt, type, atlasScratch.data());
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    }

    {
        PipelineScope _err2(PipelineStage::GlGetError);
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cout << "[OpenGL texture creation error] code=" << err << std::endl;
        }
    }

    if (projectionWidth != screenWidth || projectionHeight != screenHeight) {
        projection.setToIdentity();
        projection.ortho(0.0f, screenWidth, 0.0f, screenHeight, -1.0f, 1.0f);
        projectionWidth = screenWidth;
        projectionHeight = screenHeight;
    }

    instanceScratch.resize(packed.size() * 8);
    {
        PipelineScope _inst(PipelineStage::InstanceUpload);
        for (size_t i = 0; i < packed.size(); ++i) {
            const PackedTile& p = packed[i];
            const RectTI& box = p.src->box;
            float* dst = instanceScratch.data() + i * 8;
            dst[0] = (float)box.x0;
            dst[1] = (float)box.y0;
            dst[2] = (float)p.w;
            dst[3] = (float)p.h;
            dst[4] = p.packX / (float)tex.w;
            dst[5] = p.packY / (float)tex.h;
            dst[6] = (p.packX + p.w) / (float)tex.w;
            dst[7] = (p.packY + p.h) / (float)tex.h;
        }

        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(
            GL_ARRAY_BUFFER,
            (GLsizeiptr)(instanceScratch.size() * sizeof(float)),
            instanceScratch.data(),
            GL_STREAM_DRAW
        );
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    {
        PipelineScope _shader(PipelineStage::ShaderBind);
        shader->use();
        shader->setInt("texture1", 0);
        shader->set4X4M("projection", projection);
        shader->setFloat("uScreenHeight", (float)screenHeight);
        shader->setFloat("uOpacity", settings.opacity);
        shader->setFloat("uColorScale", settings.colorScale);
        shader->setVec3("uTint", settings.tintR, settings.tintG, settings.tintB);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex.id);
        glBindVertexArray(quadVAO);
    }

    {
        PipelineScope _draw(PipelineStage::GlDraw);
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, (GLsizei)packed.size());
    }

    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
