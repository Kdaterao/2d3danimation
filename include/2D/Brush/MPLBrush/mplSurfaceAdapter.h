#ifndef mplSurfaceAdapter_H
#define mplSurfaceAdapter_H


#include <mypaint-tiled-surface.h>
#include <types.h>
#include <raster.h>
#include <rasterPixel.h>
#include <algorithm>
#include <assert.h>
/*

    Surface adapter that satisfies mypaintlib brushes !

    All methods in this file are meant to be used with our Adapter Stuct

*/

//==============================
//      ADAPTER STRUCT 
//===============================


template <class T> //our pixel type 

struct mplSurfaceAdapter {

    MyPaintTiledSurface parent; //Basically inherited since its the first member in our struct 
    Raster<T>* ras = nullptr; // non-owning; RasterLayer owns the Raster
    
    //----- post processing variables ------
    UINT16 alpha = 65535; //our alpha value (default to 65535)
    bool eraser = false; //our eraser value (default to false)
};


//===============================
//   INTERNAL MYPAINT METHODS 
//===============================

template<class T>
static void tile_request_start(MyPaintTiledSurface *tiled_surface, MyPaintTileRequest *request) {
    /*
        Returns tile buffer
    */
   




    //----- type casting ------
    mplSurfaceAdapter<T> *self = (mplSurfaceAdapter<T> *)tiled_surface; 



    self->ras->flushBufferTile();

    //----- set our request->buffer to our raster buffer --------
    const int tx = request->tx;
    const int ty = request->ty;

    uint16_t *tile_pointer = NULL;

    if (tx >= self->ras->getLx() || ty >= self->ras->getLy() || tx < 0 || ty < 0) {
        // grab null tile 
        tile_pointer = (UINT16 *) self->ras->getNullTile();

    } else {



        if(self->eraser){
            //no need to overlay anything, we are just replacing with transparent pixels now
            tile_pointer = (UINT16 *) self->ras->getRawData(request->tx * 64, request->ty * 64, true); 
        } else {
            //need a buffer for post processing + overaly 
            tile_pointer= (UINT16 *) self->ras->getBufferTile();
        }
        
    }


    //------ assign our tile poitner ------
    request->buffer = tile_pointer;
}


//dst[i].r = std::min(src[i*4 + 0] * 2, 65535);

template<class T>
static void tile_request_end(MyPaintTiledSurface *tiled_surface, MyPaintTileRequest *request) {

    mplSurfaceAdapter<T> *self = (mplSurfaceAdapter<T> *)tiled_surface;

    const int tx = request->tx;
    const int ty = request->ty;

    if (tx >= self->ras->getLx() || ty >= self->ras->getLy() || tx < 0 || ty < 0) {
        // dont really need to wipe null tile we'll delete it no matter what 
    } else if(self->eraser == false) {

        // [NOTE] only do post processing if we are not in eraser mode

        //------ convert back to our pixel format -----

        // MyPaint / buffer tile is BGRM; raster destination is our pixel type T
        PixelRGBM64 *src = (PixelRGBM64 *) request->buffer;


        const int tx = request->tx;
        const int ty = request->ty;

        T *dest = (T *) self->ras->getRawData(request->tx * 64, request->ty * 64, true); 

        for (int i = 0; i < 64*64; i++)
        {
            //apply opacity 
            src[i].m =  src[i].m  * (self->alpha / (float) PixelRGBM64::maxChannelValue);

            // apply our buffer tile to our raster
            dest[i].composite(src[i]);
        }

        // flush our buffer tile
        self->ras->flushBufferTile();
    }

}




//================================
//      PUBLIC METHODS
//================================


//------ DECONSTRUCTOR (included as vfunc) -------
template<class T>
void mplDestroyImpl(mplSurfaceAdapter<T>* self)
{
    mypaint_tiled_surface_destroy(&self->parent);

    self->ras = nullptr;

    delete self;
}

template<class T>
void mplDestroy(MyPaintSurface* surface)
{
    auto* self =
        reinterpret_cast<mplSurfaceAdapter<T>*>(surface);

    mplDestroyImpl(self);
}


//------ CONSTRUCTOR ------------


template <class T>
mplSurfaceAdapter<T> *mplSurfaceAdapter_new(Raster<T>* ras) {

    //--- check if we actaully have a raster ---
    assert(ras != nullptr);
    //--- create adapter object ---
    mplSurfaceAdapter<T> *self = new mplSurfaceAdapter<T>{}; //designating memory in the heap ig
    self->ras = ras;

    mypaint_tiled_surface_init(&self->parent, tile_request_start<T>, tile_request_end<T>);
    
    //--- virtual function ---

    self->parent.parent.destroy = mplDestroy<T>;

    return self;
}; 





//----- returns our adapter as MyPaintSurface object type -------


template<class T>
MyPaintSurface *mplSurfaceAdapter_interface(mplSurfaceAdapter<T> *self)
{
    return (MyPaintSurface *)self;
}


#endif