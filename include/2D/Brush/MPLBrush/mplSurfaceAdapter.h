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
        // Give it a tile which we will ignore writes to
        tile_pointer = (UINT16 *) self->ras->getNullTile();

    } else {

        tile_pointer = (UINT16 *) self->ras->getRawData(request->tx * 64, request->ty * 64, true); 
    }

    request->buffer = tile_pointer;
}




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
        //------ convert to our desired type -----
    
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