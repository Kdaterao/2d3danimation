#ifndef LAYER_H
#define LAYER_H

namespace Layer {

// Timeline range that needs a UI / cache refresh after an edit.
struct dirtyRegion {
    int startIndex = 0;
    int endIndex = -1;

    dirtyRegion() = default;
    dirtyRegion(int start, int end) : startIndex(start), endIndex(end) {}
};

enum class Type {
    Raster,
    Vector
};

// WIP: type-erased layer storage (RasterLayer / VectorLayer) lives here later.

} // namespace Layer

#endif
