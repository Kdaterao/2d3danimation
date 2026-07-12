#ifndef mplSurfaceAdapter_H
#define mplSurfaceAdapter_H


#include <mypaint-tiled-surface.h>
#include <types.h>
#include <raster.h>
/*

    Surface adapter that satisfies mypaintlib brushes !

    All methods in this file are meant to be used with our Adapter Stuct

*/

//==============================
//      ADAPTER STRUCT 
//===============================


template <class T> //our pixel type 

struct mplSurfaceAdapter {

    MyPaintSurface parent; //Basically inherited since its the first member in our struct 
    RasterP<T> *ras = nullptr; //pointer to our actual raster image 

};


//===============================
//   INTERNAL MYPAINT METHODS 
//===============================

template<class T>
static void tile_request_start(MyPaintTiledSurface *tiled_surface, MyPaintTileRequest *request);

template<class T>
static void tile_request_end(MyPaintTiledSurface *tiled_surface, MyPaintTileRequest *request);


//================================
//      PUBLIC METHODS
//================================
template <class T>

//------ DECONSTRUCTOR (included as vfunc) -------
void mplDestroy(mplSurfaceAdapter<T> *self);

//------ CONSTRUCTOR ------------
template <class T>
mplSurfaceAdapter<T> * mplSurfaceAdapter_new(RasterP<T> ras); 


//----- returns our adapter as MyPaintSurface object type -------
template<class T>
MyPaintSurface *mplSurfaceAdapter_interface(mplSurfaceAdapter<T> *self)
{
    return (MyPaintSurface *)self;
}


#endif