#ifndef RASTERLAYER_H
#define RASTERLAYER_H

#include <vector>
#include <iostream>
#include <algorithm>
#include <string>

#include <frame.h>
#include <layer.h>
#include <raster.h>
#include <rasterPixel.h>

using PixelType = PixelRGBM64;

class RasterLayer {

    //----- private  varaibles -----

    
    std::vector<RasterP<PixelType>> images; // holds list of object that point to our images
    std::vector<Frame> frames; // used to specify order and duration of frames
    bool visible;
    int lx; 
    int ly;
    std::string name;

    // quick access indices (stable under RasterLayer move)
    int currentFrameIndex = 0;
    int currentImageIndex = 0;

public:
    //----- public variabls  -------
    int layerID;

    //----- constructors -----
    RasterLayer(int id, int lx, int ly,bool visible = true) 
    : layerID(id), lx(lx), ly(ly), visible(visible),
      name("Layer " + std::to_string(id))
    {
        // only time we defintly know that everything is at index 0
        addFrame(0);
        currentFrameIndex = 0;
        currentImageIndex = frames[0].imageIndex;
    };

    RasterLayer(const RasterLayer&) = delete;
    RasterLayer& operator=(const RasterLayer&) = delete;

    RasterLayer(RasterLayer&&) noexcept = default;
    RasterLayer& operator=(RasterLayer&&) noexcept = default;

    //----- public methods -----

    void setVisible(bool v) { visible = v; }
    bool isVisible() const { return visible; }

    void setName(const std::string& n) { name = n; }
    const std::string& getName() const { return name; }

    //----------------------
    //  IMAGE MANAGEMENT
    //----------------------

    int newImage();

    // returns image from current frame (non-owning; layer keeps unique_ptr)
    Raster<PixelType>* getCurrentImage() {
        return images[currentImageIndex].get();
    };

    // switches to a new frame
    void switchFrame(int frameIndex) {
        if (frameIndex < 0 || frameIndex >= (int)frames.size()) return;
        currentFrameIndex = frameIndex; 
        currentImageIndex = frames[frameIndex].imageIndex;
    };

    //----------------------
    //  FRAME MANAGEMENT
    //----------------------

    //------ GETTERS -------

    Frame *getCurrentFrame() { return &frames[currentFrameIndex];};
    const std::vector<Frame>& getFrames() const { return frames; }
    int frameCount() const { return (int)frames.size(); }
    int frameIndexAtTime(int t) const; //very useful if using setters below

    //------ SETTERS -------

    //adds frame at a given start vertex index
    Layer::dirtyRegion addFrame(int startIndex, int imageIndex = -1);

    //removes image at given vertexindex
    std::vector<Layer::dirtyRegion> removeImage(int index);

    //removes frame at given vertex index
    Layer::dirtyRegion removeFrame(int index);

    // Delete only the portion of frames overlapping [rangeStart, rangeEnd).
    Layer::dirtyRegion deleteTimeRange(int rangeStart, int rangeEnd);

    // Set a frame's absolute time span [newStart, newStart + newDuration).
    Layer::dirtyRegion updateFrame(int index, int newStart, int newDuration);

private:


    //===============================================
    //          CONVENIENCE FUNCTIONS
    //===============================================

    // Merge any adjacent/touching frames that share the same imageIndex.
    void coalesceAdjacentSameImages() {
        if (frames.size() < 2) return;
    
        for (int i = 0; i + 1 < (int)frames.size(); ) {
            Frame& cur = frames[i];
            Frame& next = frames[i + 1];
            // Exclusive end: touching / overlapping same image → merge
            if (cur.imageIndex == next.imageIndex && next.startIndex <= cur.endIndex) {
                cur.endIndex = std::max(cur.endIndex, next.endIndex);
                cur.duration = cur.endIndex - cur.startIndex;
                frames.erase(frames.begin() + i + 1);
                continue;
            }
            ++i;
        }
    }

    // convience function toupdate frame positions after an edit
    void updateFramePositions(int startIndex = 0) {
        for(int i = startIndex; i < (int)frames.size(); i++) {
            frames[i].pos = i;
        }
    }

};




using RasterLayerP = std::unique_ptr<RasterLayer>;




#endif
