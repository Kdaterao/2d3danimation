#ifndef CANVAS_H
#define CANVAS_H

#include <vector>
#include <cmath>
#include <iostream>
#include <memory>
#include <algorithm>

#include <rasterLayer.h>
#include <raster.h>
#include <rasterPixel.h>


class Canvas {


    //-------------------------
    //   PRIVATE VARIABLES 
    //-------------------------

    //backdrop
    Raster<PixelRGBM64> backDrop;
    bool backDropEnabled = true;
    PixelRGBM64 backDropColor = PixelRGBM64::White;


    //tiles 
    int tile_length = 64;//convert to enum later on ******
    int tile_length_power = 6;//convert to value that is automatically ocmputer later on based on enum  *********
    int tiles_x; // # of tile on x axis 
    int tiles_y; // # of tiles on y axis 

    //canvas dimension 
    int lx; //x-axis length 
    int ly; //y-axis length


    //canvas layers 
    std::vector<RasterLayerP> Layers;
    std::vector<int> orderOfLayers; // order of layers to be rendered

    //current  layer / time
    int currentLayerIndex; // index of current layer
    int currentTime = 0;

    std::vector<TileCoord> dirty;


    public:



        //---------------------
        //    Constructor
        //---------------------
        Canvas(int i_lx = 1920, int i_ly = 1080);

        // -----------------------------
        //     LAYERS
        // -----------------------------

        int layerCount() const;
        RasterLayer* layerAt(int index);
        int getActiveLayerIndex() const;
        RasterLayer* currentLayer();
        void setCurrentLayer(int index);
        void addLayer();
        void removeLayer(int index);

        // Render order: index 0 = bottom, last = top (same as getTile iteration)
        const std::vector<int>& layerOrder() const;

        // Move layer to a new index in layerOrder() (0 = bottom, last = top)
        void moveLayer(int layerIndex, int toOrder);

        // -----------------------------
        //     TIME / PLAYHEAD
        // -----------------------------

        int getCurrentTime() const;
        void setCurrentTime(int t);

        // -----------------------------
        //     BACKDROP
        // -----------------------------
        void toggleBackDrop(bool enabled);

        // -----------------------------
        //     TILES
        // -----------------------------

        // returns the tiles in order of rendering (bottom -> top)
        std::vector<UCHAR*> getTile(int tx, int ty);
        std::vector<TileCoord> *getDirty();


    private:

        //===============================
        //      CONVENIENCE FUNCTIONS
        //===============================

        void createBlankBackDrop();
        void reRenderCanvas();
};

#endif
