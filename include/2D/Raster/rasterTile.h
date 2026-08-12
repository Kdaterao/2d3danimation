#ifndef RASTERTILE_H
#define RASTERTILE_H

#include <types.h>
#include <memory>
#include <memory>




/*

NOTE: our tiles are ONLY squares just for simplicity 


*/


struct RasterTile {

    std::unique_ptr<UCHAR[]> buffer;  //raw buffer for the tile
    bool dirty = false; // keep track of if edited 
    int lockCount = 1; //limiting # of things touching it 
    
};



#endif