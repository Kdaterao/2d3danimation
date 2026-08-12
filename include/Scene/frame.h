#ifndef FRAME_H
#define FRAME_H

#include <types.h>


// start postiion specified by order in the vector it is placed in
// duration is specified by value in the struct
// image it uses is specified

struct Frame {
    int pos = 0; //position index in the vector it is placed in
    int imageIndex = 0; //points to imageIndex
    int startIndex = 0; // points to start index in the vector it is placed in for quick lookup
    int endIndex = 0; // points to end index in the vector it is placed in for quick lookup
    int duration = 0; // specifies the length of the current frame

    Frame() = default;

    Frame(int imageIndex, int startIndex, int duration)
        : imageIndex(imageIndex)
        , startIndex(startIndex)
        , endIndex(startIndex + duration)
        , duration(duration)
    {}
};


#endif
