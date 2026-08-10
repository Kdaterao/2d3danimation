#ifndef mplSurfaceAdapter_H
#define mplSurfaceAdapter_H


#include <mypaint-tiled-surface.h>
#include <types.h>
#include <raster.h>
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
    RasterP<T> ras = nullptr; //pointer to our actual raster image 

};


//===============================
//   INTERNAL MYPAINT METHODS 
//===============================

/*

    NOTE: we convert our tile to mypaint format and then covert back to our 64 byte format 
          this may cause a slight loss on a color channel (like 1 value off which is literally nothing considering that the range is 0-65535)

*/



template<class T>
static void tile_request_start(MyPaintTiledSurface *tiled_surface, MyPaintTileRequest *request) {
    /*
        Returns tile buffer
    */
   




    //----- type casting ------
    mplSurfaceAdapter<T> *self = (mplSurfaceAdapter<T> *)tiled_surface; 



    //----- set our request->buffer to our raster buffer --------
    const int tx = request->tx;
    const int ty = request->ty;

    uint16_t *tile_pointer = NULL;

    if (tx >= self->ras->getLx() || ty >= self->ras->getLy() || tx < 0 || ty < 0) {
        // grab null tile 
        tile_pointer = (UINT16 *) self->ras->getNullTile();

    } else {

        /*

        // convert our pixel to mypaint format (before handing off to mypaint)
        T *src =
            (T*)self->ras->getRawData(
                request->tx * 64,
                request->ty * 64,
                true
                );
        for (int i = 0; i < 64*64; i++)
            {
                src[i].r = src[i].r >> 1, 65535;
                src[i].g = src[i].g >> 1, 65535;
                src[i].b = src[i].b >> 1, 65535;
                src[i].m = src[i].m >> 1, 65535;
            }
        */

        
        // pointer for request object
        tile_pointer = (UINT16 *) self->ras->getRawData(request->tx * 64, request->ty * 64, true); 
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
        // Wipe any changes done to the null tile
        
        //--> uhh idrc what happens to null tile... sry!
        //though we should make sure to allot some space for it to prevent corruption !
    } else {
        //------ convert back to our pixel format -----

        /*
        T *src = (T *) request->buffer;

        for (int i = 0; i < 64*64; i++)
        {
            src[i].r = std::min(src[i].r << 1, 65535);
            src[i].g = std::min(src[i].g << 1, 65535);
            src[i].b = std::min(src[i].b << 1, 65535);
            src[i].m = std::min(src[i].m << 1, 65535);
        }
    
        */

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
mplSurfaceAdapter<T> *mplSurfaceAdapter_new(RasterP<T> ras) {

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