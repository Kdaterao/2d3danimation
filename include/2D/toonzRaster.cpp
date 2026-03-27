#include <toonzRaster.h>
#include <utility/toonzGeometry.h>


#include <assert.h>




//----- mutex functions -----

void ToonzRaster::lock() { };

void ToonzRaster::unlock() { };

bool ToonzRaster::isEmpty() const {};

//----- Linear image toggle -----

bool ToonzRaster::CheckLinear() const { return isLinear;  };

void ToonzRaster::setLinear(const bool linear) {
    
    //---case 1: already linear---
    if (isLinear == linear) return;

    //---case 2: not linear---
    isLinear = linear;
    if (parent) {parent->setLinear(linear);};

 };

//------ Canvas transformation operations (not implimented, hold off for now as they arent necessary) ------

void ToonzRaster::xMirror() {};

void ToonzRaster::yMirror() {};

void ToonzRaster::rotate180() {};
        
void ToonzRaster::rotate90() {};
        
void ToonzRaster::clear() {};
        
void ToonzRaster::clearOutside(const RectT<int> &rect) {};





