
#include <mplSurfaceAdapter.h>



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

    if (tx >= self->tiles_width || ty >= self->tiles_height || tx < 0 || ty < 0) {
        // Give it a tile which we will ignore writes to
        tile_pointer = self->ras->null_tile;

    } else {
        tile_pointer = self->ras->getRawData(request->tx, request->ty);
    }

    request->buffer = tile_pointer;

}

template<class T>
static void tile_request_end(MyPaintTiledSurface *tiled_surface, MyPaintTileRequest *request) {

    mplSurfaceAdapter<T> *self = (mplSurfaceAdapter<T> *)tiled_surface;

    const int tx = request->tx;
    const int ty = request->ty;

    if (tx >= self->tiles_width || ty >= self->tiles_height || tx < 0 || ty < 0) {
        // Wipe any changes done to the null tile
        reset_null_tile(self);
    } else {
        // We hand out direct pointers to our buffer, so for the normal case nothing needs to be done
    }

}


//================================
//      PUBLIC METHODS
//================================
template <class T>

//------ DECONSTRUCTOR (included as vfunc) -------
void mplDestroy(mplSurfaceAdapter<T> *self) {

    //----- Destroy MyPaint object ------
    mypaint_tiled_surface_destroy(&self->parent);

    //----- release pointer to raster -------
    self->ras = nullptr;
    
}





//------ CONSTRUCTOR ------------
template <class T>
mplSurfaceAdapter<T> *mplSurfaceAdapter_new(RasterP<T> ras) {


    //----- check if we actaully have a raster -----
    if (ras == nullptr){
        return nullptr;
    }
        

    //------ create adapter object -------
    mplSurfaceAdapter<T> self = new mplSurfaceAdapter<T>{}; //designating memory in the heap ig
    self.ras = ras;

    mypaint_tiled_surface_init(&self->parent, tile_request_start, tile_request_end);
    

    //----- virtual function --------

    self->parent.parent.destroy = mplDestroy;

    return self;
}; 
