#include <toonzRaster.h>
#include <toonzGeometry.h>


#include <assert.h>




//----- mutex functions -----

void ToonzRaster::lock() { };

void ToonzRaster::unlock() { };


//------ Canvas transformation operations (not implimented, hold off for now as they arent necessary) ------

void ToonzRaster::xMirror() {};

void ToonzRaster::yMirror() {};

void ToonzRaster::rotate180() {};
        
void ToonzRaster::rotate90() {};
        
void ToonzRaster::clear() {};
        
void ToonzRaster::clearOutside(const RectT<int> &rect) {};





