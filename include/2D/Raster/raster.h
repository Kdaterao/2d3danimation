#ifndef RASTER_H
#define RASTER_H


#include <types.h>
#include <rasterTile.h>
#include <unordered_map>
#include <assert.h>
#include <memory>

#include <toonzGeometry.h>

/*

    A Raster is defined here as a canvas built up from many tiles, with each tile having its own buffer 


    how should this buffer be defined??

    Well we want to think about two things 
        
        1) is the image small or big 
        2) how are we handling caching of our image?



    for smaller images (4k or less) --> not heap 
    images larger than 4k --> heap



    what we know 

    - we are not dynamically changing image sizes 

    - we are typically working with images 4k or less (it is an ANIMATION tool)

    - we are working with many different layers and such (meaning that if we had 4 layers for 4k images then that meanst 16k pixels)



    thus

    A dynamic buffer that chooses between heap and an array straight tile buffer based on number total pixels being used



    Raster  --> Map of RasterTile structs

    RasterTile Struct --> ptr of buffer + 



*/

//=================================
//      HELPER DATATYPES 
//=================================


struct TileCoord{
    int x;
    int y;


    bool operator==(const TileCoord& other) const {
        return x == other.x && y == other.y;
    }
};


namespace std {

template<>
struct hash<TileCoord> {
    size_t operator()(const TileCoord& t) const noexcept {
        return std::hash<int>()(t.x)
             ^ (std::hash<int>()(t.y) << 1);
    }
};

}

        
//=================================
//          MAIN CLASS
//=================================

template <class T>

class Raster {  

    //=================================
    //    PRIVATE VARIABLES
    //=================================
    

    //--- tile variables ---
    int lockCount; //max amount of threads that can access image
    int pixelSize;//not all images have to have the same pixel type(bit size!)


    int tile_length = 64;//convert to enum later on ******
    int tile_length_power = 6;//convert to value that is automatically ocmputer later on based on enum  *********
    
    int lx; //x-axis length 
    int ly; //y-axis length

    int tiles_x; // # of tile on x axis 
    int tiles_y; // # of tiles on y axis 

   
    //--- buffer variables ---
    std::vector<TileCoord> dirty = std::vector<TileCoord>();
    std::unordered_map<TileCoord, RasterTile>  tilesMap;
    UCHAR *null_tile; // empty tile for out of bounds requests 


    //=================================
    //    PUBLIC FUNCTIONS 
    //=================================

    public:


        //------------------------------------------
        //   CONSTRUCTOR 
        //------------------------------------------


        Raster(int i_w, int i_h) :
        lx(i_w),
        ly(i_h),
        pixelSize(sizeof(T))
        {

            assert(i_w > 0);
            assert(i_h > 0);

            tiles_x = ceil((float)i_w / tile_length);
            tiles_y = ceil((float)i_h / tile_length);

            int size = tile_length * tile_length * pixelSize;
            null_tile = new UCHAR[size];



        };

        //------------------------------------------
        //   DECONSTRUCTOR 
        //------------------------------------------

        ~Raster(){

        }

        //------------------------------------------
        //   IMAGE ACCESS (just return buffer)g
        //------------------------------------------
        UCHAR* getRawData(int x, int y, bool write)  { //---> write lets our raster know we are writing to it (allows us to mark it as dirty)

            //std::cout<<"getRawData() called"<<std::endl;

            //----- return null tile if out of bounds -------
            if (x >= lx || y >= ly || x < 0 || y < 0) {
                std::cout<<"nul till"<<std::endl;
                return this->null_tile;
            }

            //----- get tile coord (tile_length must be power of 2)-----
            int tx = x >> tile_length_power; 
            int ty = y >> tile_length_power;

            //------ get offset from start of tile buffer ---------
            int xLocal = x - (tx * tile_length);
            int yLocal = y - (ty * tile_length);
            int offset = (yLocal * tile_length + xLocal) * pixelSize;



            //std::cout<<"actual[" <<x <<","<<y<<"]"<<std::endl;
            //std::cout<<"tile postiion[" <<tx <<","<<ty<<"]"<<std::endl;
            //std::cout<<"local position[" <<xLocal <<","<<yLocal<<"]"<<std::endl;
            //std::cout<<"sizeof(T):"<<sizeof(T)<<"pixelsize:"<<pixelSize<<std::endl;
            
            //------- try to get kv of tile from map --------
            auto it = tilesMap.find(TileCoord{tx, ty});


            //------- get a tile buffer ----------
            if (it != tilesMap.end()) {
                // KEY EXISTS :)
                
                if(write && it->second.dirty == false){
                    it->second.dirty = true;
                    dirty.push_back(TileCoord{tx, ty});

                }

                


                return it->second.buffer + offset;
            
            } else {
                // KEY NOT THERE :( 
                RasterTile tile = RasterTile{makeBuffer(), true, 1};
      

                auto [newIt, inserted] = tilesMap.emplace(
                    TileCoord{tx, ty},
                    std::move(tile)
                );

                
                if(write){
                    dirty.push_back(TileCoord{tx, ty});
                }

                return newIt->second.buffer + offset;
            }
            
        };

        //-----------------------------------
        //         Get Tile Offset 
        //-----------------------------------



        

        //-------------------------------------
        //        Mark dirty tiles 
        //-------------------------------------

        void unmarkDirty(int tx, int ty){
            TileCoord key = TileCoord{tx, ty};
            auto it = tilesMap.find(key);
            assert(it != tilesMap.end());
            it->second.dirty = false;

        }

        void markDirty(int tx, int ty)
        {

            TileCoord key = TileCoord{tx, ty};
            auto it = tilesMap.find(key);
            
            assert(it != tilesMap.end());


            if(it->second.dirty == false){
                dirty.push_back(key);
            }

        }
        
        std::vector<TileCoord> *getDirty(){
            return &dirty;
        }


        UCHAR *getNullTile() {
            return null_tile;
        }

        //------------------------------------------
        //      IMAGE FORMAT CONVERSIONS :O
        //------------------------------------------

        /*
            TODO: maybe add some conversion methods just in case 
        */


        //------------------------------------------
        //      IMAGE DIMENSION UTILIY FUNCTIONS
        //------------------------------------------

        inline int getLx() const  {return lx;};

        inline int getLy() const  {return ly;};

        inline DimensionT<int> getSize() const  { return DimensionT<int>(lx, ly); }

        inline PointT<double> getCenterD() const { return PointT<double>(0.5 * lx, 0.5 * ly); }

        inline PointTI getCenter() const { return PointTI(lx / 2, ly / 2); }

        inline RectT<int> getBounds() const { return RectT<int>(0, 0, lx - 1, ly - 1); }

        inline int getPixelSize() const { return pixelSize; }

        inline int getTileLength() const { return tile_length;}



    //======================================
    //  HELPER FUNCTIONS 
    //======================================

    private:

        //----- utility to quickly make a new buffer -------
        UCHAR *makeBuffer(){
            std::cout<<"pixelSize"<<pixelSize<<std::endl;
            int size = tile_length * tile_length * pixelSize;
            UCHAR* buffer = new UCHAR[size];
            memset(buffer, 255, size);

            return buffer;

        }


        

};


//============================
// Protected version of Raster 
//=============================



template <class T>

using RasterP = std::shared_ptr<Raster<T>>;  // "using" does not work with "typedef" so this is the compromise










#endif 