#ifndef RASTERTILE_H
#define RASTERTILE_H

#include <types.h>
#include <memory>





/*

NOTE: our tiles are ONLY squares just for simplicity 


*/


struct RasterTile {

    UCHAR *buffer = nullptr; // raw buffer for the tile 
    bool dirty = false; // keep track of if edited 
    int lockCount = 1; //limiting # of things touching it 


    ~RasterTile() {
        delete[] buffer;
    }

    //----- no copying (deletes it immediatly) -------
    RasterTile(const RasterTile&) = delete;
    RasterTile& operator=(const RasterTile&) = delete;
    
    //---- you can move it ------

    RasterTile(RasterTile&& other) noexcept
        : buffer(other.buffer)
        , dirty(other.dirty)
        , lockCount(other.lockCount)
    {
        other.buffer = nullptr;
    }



    RasterTile(UCHAR* buffer, bool dirty, int lockCount) noexcept
        : buffer(buffer)
        , dirty(dirty)
        , lockCount(lockCount)
    {
    }
};



#endif