#ifndef PAINTERGL_H
#define PAINTERGL_H

#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QMatrix4x4>

#include <assert.h>
#include <iostream>
#include <fstream>

#include <toonzGeometry.h>
#include <toonzTextureManager.h>
#include <toonzShader.h>
#include <types.h>



/*
This is a class that handles drawing anything to the canvas


Cases: 

    ** For all optimizations it seems we must create a tiling system for the raster so we only update portions of the image for painting..

    case 1: we are displaying a drawing after it has been manipulated
        
        ** draw parts of the image that are drawn on..


    case 2: we are displaying a preview of what a tool could do to a picture (Ex: transforming pixels via lasso tool)

        ** To satisfy all subcases, we must just display it as a seperate texture, so that it does not overlap with image...
        ** As an optimization, we can update parts of the raster image it touches(not the whole thing....)
        
        ** We must continually pass a matrix(technical term is an affine matrix(x,y,z)) into the painter with the points 
            and just transform our buffer before drawing it everytime (so thats fine prolly)

        subcase 1: The transformed area overlaps a non-transformed part of the drawing

        subcase 2: The transformed area does not overlap a non-transformed part of the drawing 


    case 3: we are displaying a static thing on the canvas(grid lines, ect...)

        ** To satisfy all cases we can keep static things on the canvas as a seperate texture, and redraw them after or before canvas (OpenGL doesnt remember layers..)

        subcase 1: it is toggled on
            
            sub_subcase 1: below canvas

            sub_subcase 2: ontop of canvas


        subcase 2: it is toggled off 

            Simply dont render it 

*/
struct PaintSettings {
    float opacity = 1.0f;                              // overall / alpha gain
    float colorScale = 2.0f;                           // RGBM half-range display scale (scales to 16bit)
    float tintR = 1.0f, tintG = 1.0f, tintB = 1.0f;    // RGB multiply
};

class toonzPainterGL: public QOpenGLWidget, protected QOpenGLFunctions{

    TAffine aff; //affine object for transformations preview
    int screenHeight; //full screen height
    int screenWidth; // full screen width 
    int wrap; //wrap of canvas
    int bpp; // "bytes per pixel"
    DimensionTI rasDim; //"Dimension of raster"
    GLenum magFilter; //used for zoom in
    GLenum minFilter; //used for zoom out
    GLenum premultiplied; //used for displaying transparency
    DimensionTI maxSize; // max texture size
    bool isRGBM; //pixel layout
    bool is32;
    int count = 0;
    toonzShader *shader;

    PaintSettings tileSettings;     // regular raster tiles
    PaintSettings previewSettings;  // buffer / tool preview tiles

    public:

    //----- constructor -----

        toonzPainterGL() {};

        toonzPainterGL(TAffine i_aff, int i_wrap, int i_screenWidth, int i_screenHeight, int i_bpp,  const DimensionTI &i_rasDim, GLenum i_magFilter, GLenum i_minFilter, bool i_premultiplied, toonzShader *i_shader, bool i_isRGBM)
            : aff(i_aff),
            wrap(i_wrap),
            bpp(i_bpp),
            rasDim(i_rasDim),
            magFilter(i_magFilter),
            minFilter(i_minFilter),
            premultiplied(i_premultiplied),
            shader(i_shader),
            screenHeight(i_screenHeight),
            screenWidth(i_screenWidth),
            isRGBM(i_isRGBM){
                initializeOpenGLFunctions(); 


                // checks size of pixel for is32 (4 bytes = 32! )
                is32 = (i_bpp == 4); 

                i_shader->use();
                maxSize = toonzTextureManager::instance()->getMaxSize(isRGBM);
                std::cout<<"maxsize:"<<"lx:"<<maxSize.lx<<"ly:"<<maxSize.ly<<std::endl;
            };

    //----- methods -----

        void setTileSettings(const PaintSettings& settings) { tileSettings = settings; }
        void setPreviewSettings(const PaintSettings& settings) { previewSettings = settings; }
        PaintSettings& getTileSettings() { return tileSettings; }
        PaintSettings& getPreviewSettings() { return previewSettings; }

        void PaintRaster(RectTI box, UCHAR *rasbuffer, GLuint fbuffer, bool isPreview);

        template <class T>
        void dumpBuffer2(T* buf, int width, int height, int wrap, int num_channels);

     };



#endif 