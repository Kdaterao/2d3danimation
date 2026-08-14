#include <rasterLayer.h>

#include <raster.h>








//===============================================
//===============================================
//              IMAGE MANAGEMENT
//===============================================
//===============================================



//-------------------
//  NEW IMAGE
//-------------------


int RasterLayer::newImage() {
    images.push_back(std::make_unique<Raster<PixelType>>(lx, ly));
    return (int)images.size() - 1;
}




//===============================================
//===============================================
//              FRAME MANAGEMENT
//===============================================
//===============================================


//===============================================
//             FRAME INDEX AT TIME
//===============================================

int RasterLayer::frameIndexAtTime(int t) const {
    if (frames.empty()) return 0;
    int fallback = 0;
    for (int i = 0; i < (int)frames.size(); ++i) {
        const Frame& f = frames[i];
        if (f.startIndex <= t && t < f.endIndex) return i;
        if (f.startIndex <= t) fallback = i;
    }
    return fallback;
}

bool RasterLayer::hasFrameAtTime(int t) const {
    for (const Frame& f : frames) {
        if (f.startIndex <= t && t < f.endIndex) return true;
    }
    return false;
}


//===============================================
//              ADD FRAME
//===============================================

// Restored original case layout (1a / 1b / 1c / gap), rewritten to use
// indices instead of invalidated iterators. endIndex is exclusive.
Layer::dirtyRegion RasterLayer::addFrame(int startIndex, int imageIndex) {

    //=========== INITIALIZE VARIABLES ==============

    //duration
    const int duration = 1;

    //dirty region
    Layer::dirtyRegion dirtyRegion;

    //overlap + vector insertion position
    int overlap = -1;
    int insertAt = (int)frames.size();
    for (int i = 0; i < (int)frames.size(); ++i) {
        const Frame& f = frames[i]; 
        if (f.startIndex <= startIndex && startIndex < f.endIndex) {
            overlap = i;
            break;
        }
        if (f.startIndex > startIndex) {
            insertAt = i;
            break;
        }
    }



    //=========== CREATE NEW IMAGE (IF ALLOWED) ==============

    //------ guard: cannot create over a duration-1 cell ------

    //check if overlap is a duration-1 cell  
    if (overlap >= 0 && frames[overlap].duration == 1) {
        return Layer::dirtyRegion(frames[overlap].startIndex, frames[overlap].endIndex);
    }


    //----- Get image index + allocate image -----
    if (imageIndex == -1) {
        imageIndex = newImage();
    }


    //=========== CREATE NEW FRAME ==============

    Frame newFrame = Frame(imageIndex, startIndex, duration);

    //----- Case 1: No frames in layer yet ------
    if (frames.empty()) {

        //insert our new frame
        frames.push_back(newFrame);

         //return dirty + update frame positions
        updateFramePositions();
        return Layer::dirtyRegion(newFrame.startIndex, newFrame.endIndex);
    }


    //----- Case 2: other frames but no overlap -----
    if (overlap < 0) {

        //If there is a previous frame and a gap, extend it up to the new frame
        if (insertAt > 0) {
            Frame& prev = frames[insertAt - 1];
            if (prev.endIndex < startIndex) {
                dirtyRegion = Layer::dirtyRegion(prev.endIndex, newFrame.endIndex);
                prev.endIndex = startIndex;
                prev.duration = prev.endIndex - prev.startIndex;
            } else {
                dirtyRegion = Layer::dirtyRegion(newFrame.startIndex, newFrame.endIndex);
            }
        } else {
            dirtyRegion = Layer::dirtyRegion(newFrame.startIndex, newFrame.endIndex);
        }


        // insert our new frame
        frames.insert(frames.begin() + insertAt, newFrame);


        //return dirty + update frame positions
        updateFramePositions();
        return dirtyRegion;
    }

    //----- case 3: frame overlaps with the new frame -----
    Frame& existing = frames[overlap];

    // same image at same start → no-op
    if (existing.imageIndex == newFrame.imageIndex && existing.startIndex == startIndex) {
        std::cout << "you added a frame with an image that already has that exact same frame" << std::endl;
        return Layer::dirtyRegion(existing.startIndex, existing.endIndex);
    }

    if (existing.startIndex == startIndex) {
        //----- case 3a : frame starts at the same time as the new frame -----
       
        //insert our new frame
        frames.insert(frames.begin() + overlap, newFrame);


        //get the existiing frame and update it 
        Frame& shifted = frames[overlap + 1];
        shifted.startIndex = startIndex + 1;
        shifted.duration = shifted.duration - 1;
        shifted.endIndex = shifted.startIndex + shifted.duration;
        
        //get dirty region
        dirtyRegion = Layer::dirtyRegion(newFrame.startIndex, shifted.endIndex);

    } else if (existing.endIndex == startIndex) {
        //----- case 3b : new frame starts exactly where existing ends (adjacent) -----

        // Nothing to trim; insert immediately after existing
        frames.insert(frames.begin() + overlap + 1, newFrame);
        dirtyRegion = Layer::dirtyRegion(existing.startIndex, newFrame.endIndex);

    } else {
        //----- case 3c : new frame somewhere in the middle of existing frame -----

        //get the existing frame's information (start, end, duration, image idnex)
        const int oldStart = existing.startIndex;
        const int oldEnd = existing.endIndex;
        const int oldImage = existing.imageIndex;

        const int beforeDur = startIndex - oldStart;
        const int afterDur = oldEnd - (startIndex + duration); // used for if there is a split


        //insert our before stub frame and our new frame 
        frames[overlap] = Frame(oldImage, oldStart, beforeDur);
        frames.insert(frames.begin() + overlap + 1, newFrame);

        //conditionally insert our after frame (based on afterDur)
        if (afterDur > 0) {
            frames.insert(frames.begin() + overlap + 2, Frame(oldImage, startIndex + duration, afterDur));
            dirtyRegion = Layer::dirtyRegion(oldStart, oldEnd);
        } else {
            dirtyRegion = Layer::dirtyRegion(oldStart, newFrame.endIndex);
        }

    }

    //return dirty + update frame positions
    updateFramePositions();
    return dirtyRegion;
}


//===============================================
//              REMOVE IMAGE
//===============================================

std::vector<Layer::dirtyRegion> RasterLayer::removeImage(int index) {


    //--=-- variables  -----
    std::vector<Layer::dirtyRegion> dirtyRegions;


    //----- go through and delete all matching frames --------

    for (auto it = frames.begin(); it != frames.end(); ) {
        if (it->imageIndex == index) {

            dirtyRegions.push_back(Layer::dirtyRegion(it->startIndex, it->endIndex)); //capture dirty region BEFORE erase
            it = frames.erase(it);
            
        } else {
            ++it;
        }
    }

    //---- return dirty + update frame positions------ 
    updateFramePositions();
    return dirtyRegions;
}


//===============================================
//               REMOVE FRAME
//===============================================
Layer::dirtyRegion RasterLayer::removeFrame(int index) {

    //========= GUARDS ==================

    //------ guard: index is out of bounds --------
    if (index < 0 || index >= (int)frames.size()) {
        return Layer::dirtyRegion();
    }
    //------ guard: layer must keep at least one frame --------
    if (frames.size() <= 1) {
        return Layer::dirtyRegion(frames[0].startIndex, frames[0].endIndex);
    }



    //========= REMOVE FRAME ==================

    Layer::dirtyRegion dirtyRegion(frames[index].startIndex, frames[index].endIndex);
    frames.erase(frames.begin() + index);

    //========= (FEATURE) EXTEND PREVIOUS FRAME ==================

    // Extend previous frame to fill the hole up to the next frame (timeline delete UX)
    if (index > 0 && (index - 1) < (int)frames.size()) {
        Frame& prev = frames[index - 1];
        if (index < (int)frames.size()) {
            if (prev.endIndex < frames[index].startIndex) {
                prev.endIndex = frames[index].startIndex;
                prev.duration = prev.endIndex - prev.startIndex;
            }
        }
    }

    //=========update which frame is currently selected ==================

    // guard: current frame index is out of bounds
    if (currentFrameIndex >= (int)frames.size()) {
        currentFrameIndex = (int)frames.size() - 1;
    }

    //update current image index
    currentImageIndex = frames[currentFrameIndex].imageIndex;


    //return dirty + update frame positions
    updateFramePositions();
    return dirtyRegion;
}




//===============================================
//               DELETE TIME RANGE
//===============================================


// Timeline tile-selection delete: carve [rangeStart, rangeEnd) out of frames.
Layer::dirtyRegion RasterLayer::deleteTimeRange(int rangeStart, int rangeEnd) {

    //======= calcuate dirty =============================
    if (rangeEnd <= rangeStart) {
        return Layer::dirtyRegion();
    }
    if (rangeStart < 0) rangeStart = 0;

    const int dirtyStart = rangeStart;
    const int dirtyEnd = rangeEnd;

    //======= Delete Everything in the range =============

    for (int i = (int)frames.size() - 1; i >= 0; --i) {

        Frame f = frames[i];

        //------ Case 1: non overlapping frame --------
        if (f.endIndex <= rangeStart || f.startIndex >= rangeEnd) {
            continue;
        }

        //------- Case 2: fully covered by selection --------
        if (f.startIndex >= rangeStart && f.endIndex <= rangeEnd) {
            frames.erase(frames.begin() + i);
            continue;
        }

        //------ Case 3: Covered only in the middle of the frame --------
        if (f.startIndex < rangeStart && f.endIndex > rangeEnd) {
            Frame left(f.imageIndex, f.startIndex, rangeStart - f.startIndex);
            Frame right(f.imageIndex, rangeEnd, f.endIndex - rangeEnd);
            frames[i] = left;
            frames.insert(frames.begin() + i + 1, right);
            continue;
        }

        //------ Case 4: trims end of frame -------------
        if (f.startIndex < rangeStart) {
            frames[i].endIndex = rangeStart;
            frames[i].duration = rangeStart - f.startIndex;
            continue;
        }


        //------- Case 4: trims start of frame -------
        frames[i].startIndex = rangeEnd; //pushes start to the the end of the range
        frames[i].duration = frames[i].endIndex - rangeEnd; //updates the duration
    }


    //======= UPDATE AFTER DELETION ====================

    //------ guard: layer must keep at least one frame --------
    if (frames.empty()) {
        addFrame(0);
    }

    //------ guard: current frame index is out of bounds --------
    if (currentFrameIndex >= (int)frames.size()) {
        currentFrameIndex = std::max(0, (int)frames.size() - 1);
    }

    //update current image index
    if (!frames.empty()) {
        currentImageIndex = frames[currentFrameIndex].imageIndex;
    }


    //return dirty + update frame positions
    updateFramePositions();
    return Layer::dirtyRegion(dirtyStart, dirtyEnd);
}



//===============================================
//          CHANGE FRAME TIME RANGE
//===============================================

Layer::dirtyRegion RasterLayer::updateFrame(int index, int newStart, int newDuration) {
    


    //------ guard: index is out of bounds --------
    if (index < 0 || index >= (int)frames.size()) {
        return Layer::dirtyRegion();
    }

        

    //========= INITIALIZE VARIABLES ==================

    if (newDuration < 1) {
        std::cout << "Cannot set frame duration to less than 1" << std::endl;
        return Layer::dirtyRegion(frames[index].startIndex, frames[index].endIndex);
    }
    if (newStart < 0) newStart = 0;


    //========= UPDATE FRAME==================

    //----- CASE 1: Only changing the right side of the frame ------
    if (newStart == frames[index].startIndex) {
        int extent = std::max(frames[index].startIndex + newDuration, frames[index].endIndex);
        Layer::dirtyRegion dirtyRegion(frames[index].startIndex, extent);

        const int oldDuration = frames[index].duration;

        frames[index].duration = newDuration;
        frames[index].endIndex = frames[index].startIndex + newDuration;

        //------ case 1 : last frame -------
        if (index == (int)frames.size() - 1) {
            updateFramePositions();
            return dirtyRegion;
        }

        //------ case 2 : shrinks -------
        if (newDuration < oldDuration) {
            updateFramePositions();
            return dirtyRegion;
        }

        //------ case 3 : grows  ------------
        for (int i = index + 1; i < (int)frames.size(); ) {
            Frame& next = frames[i];

            if (next.startIndex >= frames[index].endIndex) {
                break;
            }

            if (next.endIndex <= frames[index].endIndex) {
                frames.erase(frames.begin() + i);
                continue;
            }

            next.startIndex = frames[index].endIndex;
            next.duration = next.endIndex - next.startIndex;
            if (next.duration < 1) {
                frames.erase(frames.begin() + i);
                continue;
            }
            break;
        }

        //merge any touching same-image runs
        coalesceAdjacentSameImages();

        //return dirty region + update frame positions
        updateFramePositions();
        return dirtyRegion;
    }


    //----- CASE 2: Moving and/or resizing the frame ------

    //------ Move Frame --------

    //get information about frame in old position
    Frame moved = frames[index];
    const int oldStart = moved.startIndex;
    const int oldEnd = moved.endIndex;

    //Delete AFTER  getting the information
    frames.erase(frames.begin() + index);

    //rebuild the frame with its new position and duration
    moved.startIndex = newStart;
    moved.duration = newDuration;
    moved.endIndex = newStart + newDuration;


    //----- Resolve overlaps with remaining frames ------

    for (int i = 0; i < (int)frames.size(); ) {
        Frame& o = frames[i];

        //------- Case 1: No overlap ------
        if (o.endIndex <= moved.startIndex || o.startIndex >= moved.endIndex) {

            //if we are past the end of the frame, we can break(no need to check anything else)
            if(o.startIndex > moved.endIndex) {
                break;
            } else {
                //otherwise, we can just move on to the next frame
                ++i;
                continue;
            }
        }

        //------- Case 2: Completely covered ------
        if (o.startIndex >= moved.startIndex && o.endIndex <= moved.endIndex) {
            frames.erase(frames.begin() + i);
            continue;
        }

        //------- Case 3: in the middle of frame (not touching ends) -------
        if (o.startIndex < moved.startIndex && o.endIndex > moved.endIndex) {
            const int rightDur = o.endIndex - moved.endIndex;
            const int rightImage = o.imageIndex;
            o.endIndex = moved.startIndex;
            o.duration = o.endIndex - o.startIndex;
            frames.insert(frames.begin() + i + 1, Frame(rightImage, moved.endIndex, rightDur));
            i += 2;
            continue;
        }

        //------- Case 4: touches right half of the frame -------
        if (o.startIndex < moved.startIndex) {
            o.endIndex = moved.startIndex;
            o.duration = o.endIndex - o.startIndex;
            if (o.duration < 1) {
                frames.erase(frames.begin() + i);
                continue;
            }
            ++i;
            continue;
        }

        //-------- Case 5: touches the left half of the frame
        o.startIndex = moved.endIndex;
        o.duration = o.endIndex - o.startIndex;
        if (o.duration < 1) {
            frames.erase(frames.begin() + i);
            continue;
        }
        ++i;

    }

    //------ finally insert our moved frame into the correct position ------
    int insertAt = (int)frames.size();
    for (int i = 0; i < (int)frames.size(); ++i) {
        if (frames[i].startIndex > moved.startIndex) {
            insertAt = i;
            break;
        }
    }
    frames.insert(frames.begin() + insertAt, moved);


    //====== UPDATE AFTER MOVING/RESIZING ======

    //merge any touching same-image runs
    coalesceAdjacentSameImages();

    //update frame positions
    updateFramePositions();

    // update current frame and image index
    if (currentFrameIndex >= (int)frames.size()) {
        currentFrameIndex = std::max(0, (int)frames.size() - 1);
    }
    if (!frames.empty()) {
        currentImageIndex = frames[currentFrameIndex].imageIndex;
    }

    //return dirty region
    return Layer::dirtyRegion(
        std::min(oldStart, moved.startIndex),
        std::max(oldEnd, moved.endIndex)
    );

}
