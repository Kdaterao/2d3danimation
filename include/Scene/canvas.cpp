#include <canvas.h>
#include <pipelineLogger.h>








//===============================================
//===============================================
//                 CONSTRUCTOR
//===============================================
//===============================================


Canvas::Canvas(int i_lx, int i_ly)
: backDrop(i_lx, i_ly),
  lx(i_lx),
  ly(i_ly),
  currentLayerIndex(0)
{
    //------ (default) create a blank backdrop ------
    createBlankBackDrop();
    Layers.push_back(std::make_unique<RasterLayer>(0, lx, ly));
    orderOfLayers.push_back(0);

    //------ calculate tile grid dimensions ------
    tiles_x = (int)ceil((float)lx / tile_length);
    tiles_y = (int)ceil((float)ly / tile_length);

    //------ specifify all dirty tiles(whole canvas) ------
    reRenderCanvas();
}




//===============================================
//===============================================
//                   LAYERS
//===============================================
//===============================================


//-------------------
//  LAYER COUNT
//-------------------

int Canvas::layerCount() const {
    return (int)Layers.size();
}


//-------------------
//  LAYER AT
//-------------------

RasterLayer* Canvas::layerAt(int index) {
    if (index < 0 || index >= (int)Layers.size()) return nullptr;
    return Layers[index].get();
}


//-------------------
//  ACTIVE LAYER INDEX
//-------------------

int Canvas::getActiveLayerIndex() const {
    return currentLayerIndex;
}


//-------------------
//  CURRENT LAYER
//-------------------

RasterLayer* Canvas::currentLayer() {
    if (Layers.empty()) return nullptr;
    return Layers[currentLayerIndex].get();
}


//-------------------
//  SET CURRENT LAYER
//-------------------

void Canvas::setCurrentLayer(int index) {
    if (Layers.empty()) return;
    currentLayerIndex = std::clamp(index, 0, (int)Layers.size() - 1);
}


//-------------------
//  ADD LAYER
//-------------------

void Canvas::addLayer() {

    //------ create new layer + get id------
    int layerID = (int)Layers.size(); // yes layer id is just vector index

    Layers.push_back(std::make_unique<RasterLayer>(layerID, lx, ly));
    orderOfLayers.push_back(layerID);
    currentLayerIndex = layerID;

    //------ sync new layer to our stack of layer------
    Layers.back()->switchFrame(Layers.back()->frameIndexAtTime(currentTime));


    //------ mark all tiles as dirty ------
    reRenderCanvas();
}


//-------------------
//  REMOVE LAYER
//-------------------

void Canvas::removeLayer(int index) {

    //------ guard: need at least one layer------
    if (index < 0 || index >= (int)Layers.size()) return;
    if (Layers.size() <= 1) return; // keep at least one layer


     //------ Delete layer ------
    

    // remove the actual layer
    Layers.erase(Layers.begin() + index);

   // remove the id from our stack of layers
    orderOfLayers.erase(
        std::remove(orderOfLayers.begin(), orderOfLayers.end(), index),
        orderOfLayers.end());

    
    //------ Update other layer indexes ------

    //update other layer id indexes (so that they are still valid)
    for (int& id : orderOfLayers) {
        if (id > index) --id;
    }

    //update the layer id indexes
    for (int i = 0; i < (int)Layers.size(); ++i) {
        Layers[i]->layerID = i;
    }


    //------ set current layer ---------
    if (currentLayerIndex >= (int)Layers.size()) {
        currentLayerIndex = (int)Layers.size() - 1;
    } else if (currentLayerIndex > index) {
        currentLayerIndex -= 1;
    }
    
    //------ mark all tiles as dirty ------
    reRenderCanvas();
}


//-------------------
//  LAYER ORDER
//-------------------

const std::vector<int>& Canvas::layerOrder() const {
    return orderOfLayers;
}


//-------------------
// MOVES LAYER TO NEW VERTEX INDEX 
//-------------------

void Canvas::moveLayer(int layerIndex, int toOrder) { // toOrder is the new vertex position 

    //------ guard: need at least two layers ------
    const int n = (int)orderOfLayers.size();
    if (n <= 1) return;

    //------ 
    toOrder = std::clamp(toOrder, 0, n - 1);

    //
    int fromOrder = -1;
    for (int i = 0; i < n; ++i) {
        if (orderOfLayers[i] == layerIndex) {
            fromOrder = i;
            break;
        }
    }
    if (fromOrder < 0 || fromOrder == toOrder) return;

    const int id = orderOfLayers[fromOrder];
    orderOfLayers.erase(orderOfLayers.begin() + fromOrder);
    orderOfLayers.insert(orderOfLayers.begin() + toOrder, id);
    reRenderCanvas();
}




//===============================================
//===============================================
//                TIME / PLAYHEAD
//===============================================
//===============================================


//-------------------
//  GET CURRENT TIME
//-------------------

int Canvas::getCurrentTime() const {
    return currentTime;
}


//-------------------
//  SET CURRENT TIME
//-------------------

void Canvas::setCurrentTime(int t) {
    currentTime = std::max(0, t);
    for (auto& layer : Layers) {
        layer->switchFrame(layer->frameIndexAtTime(currentTime));
    }
    reRenderCanvas();
}


//-------------------
//  PLAYBACK END
//-------------------

int Canvas::playbackEnd() const {
    int end = 0;
    for (const auto& layer : Layers) {
        if (!layer) continue;
        for (const Frame& f : layer->getFrames()) {
            end = std::max(end, f.endIndex);
        }
    }
    return end;
}




//===============================================
//===============================================
//                   BACKDROP
//===============================================
//===============================================


//-------------------
//  TOGGLE BACKDROP
//-------------------

void Canvas::toggleBackDrop(bool enabled) {
    backDropEnabled = enabled;
    reRenderCanvas();
}




//===============================================
//===============================================
//                    TILES
//===============================================
//===============================================


//-------------------
//  GET TILE
//-------------------

std::vector<std::pair<int, UCHAR*>> Canvas::getTile(int tx, int ty) {
    PipelineScope _getTile(PipelineStage::GetTile); // debug
    std::vector<std::pair<int, UCHAR*>> tiles;

    if (backDropEnabled) {
        tiles.emplace_back(-1, backDrop.getRawData(tx * tile_length, ty * tile_length, false));
    }

    for (auto layerIndex : orderOfLayers) {
        auto *layer = Layers[layerIndex].get();
        if (!layer->isVisible()) continue;
        auto tile = layer->getCurrentImage()->getRawData(tx * tile_length, ty * tile_length, false);
        if (tile) {
            tiles.emplace_back(layerIndex, tile);
        }
    }
    if (tiles.empty()) {
        std::cerr << "No tiles found for tile at" << tx << ", " << ty << std::endl;
    }
    return tiles;
}


//-------------------
//  GET DIRTY
//-------------------

std::vector<TileCoord>* Canvas::getDirty() {
    return &dirty;
}




//===============================================
//===============================================
//             CONVENIENCE FUNCTIONS
//===============================================
//===============================================


//-------------------
//  CREATE BLANK BACKDROP
//-------------------

void Canvas::createBlankBackDrop() {
    backDrop.fill(backDropColor);
}


//-------------------
//  RE-RENDER CANVAS
//-------------------

void Canvas::reRenderCanvas() {
    dirty.clear();
    for (int tx = 0; tx < tiles_x; tx++) {
        for (int ty = 0; ty < tiles_y; ty++) {
            dirty.push_back(TileCoord{tx, ty});
        }
    }
}
