#ifndef TOONZRASTER_H
#define TOONZRASTER_H

#include <toonzGeometry.h>
#include <types.h>
#include <memory>
#include <assert.h>


//----- Classes Structure -----
/*
ToonzRaster (abstract class) -> 

ToonzRasterT (template version)

ToonzRasterP (abstract class typdef as shared pointer)

ToonzRasterPT (template version typdef as shared pointer)


Notes:  

    - We do not create a wrapper class for our shared pointers, which means we need to manually
    handle std::make_shared and such later on when using these image classes.

    - We make our default buffer a UCHAR since its one byte, allowing us to traverse the buffer with whatever color type we use..

*/

//----- forward declaration -----

class ToonzRaster;
class ToonzRasterP;


//------ ToonzRaster abstract class Definition ------

class ToonzRaster {

    protected:
    
        //----- varaibles -----

        //--- canvas variables ---
        int pixelSize;//not all images have to have the same pixel type(bit size!)
        int lx; //x-axis length 
        int ly; //y-axis length
        int wrap; // length of a row
        int lockCount; //max amount of threads that can access image
        bool isLinear; //linear images have more efficient operation algorthims than non-linear


        //--- buffer variables ---
        ToonzRaster*parent; // pointer to a parent raster if this raster canavs does not own the buffer 
        UCHAR *buffer; //pointer to buffer of the image
        bool bufferOwner; // true if this raster canvas owns a buffer and is responsible for closing it 
        


        //----- constructor ------

        ToonzRaster(int i_lx, int i_ly, int i_pixelSize, int i_wrap, UCHAR *i_buffer, ToonzRaster *i_parent) 
            :lx(i_lx),
             ly(i_ly),
             pixelSize(i_pixelSize),
             wrap(i_wrap),
             buffer(i_buffer),
             parent(i_parent),
             bufferOwner(false),
             isLinear(false) {};


        ToonzRaster(int i_lx, int i_ly, int i_pixelSize)
            :lx(i_lx),
             ly(i_ly),
             wrap(i_lx),
             parent(0),
             bufferOwner(true),   
             buffer(0), 
             lockCount(0), 
             pixelSize(i_pixelSize){};


    private:

        //some other stuff that was in the class(i havent even touch mutex shi rn, so ill leave it be rn)
        
        /*
        static TAtomicVar m_totalMemory;
        TThread::Mutex m_mutex;
        void remap();
        inline void detach() {};
        */


    public:


        //----- deconstructor ------
       
        virtual ~ToonzRaster() {};  

        //----- Canvas Measurement Getter functions ------
        
        inline int getLx() const  {return lx;};

        inline int getLy() const  {return ly;};

        inline DimensionT<int> getSize() const  { return DimensionT<int>(lx, ly); }

        inline int getWrap() const { return wrap; };  

        inline PointT<double> getCenterD() const { return PointT<double>(0.5 * lx, 0.5 * ly); }

        inline PointT<int> getCenter() const { return PointT(lx / 2, ly / 2); }

        inline RectT<int> getBounds() const { return RectT<int>(0, 0, lx - 1, ly - 1); }

        inline int getPixelSize() const { return pixelSize; }
        

        //----- mutex functions (not implimented) -----

        void lock();

        void unlock();

        
        //------ Buffer access functions ------

        inline UCHAR* getRawData() const {return buffer;};
       
        inline UCHAR* getRawData(int x, int y)  {

            //--- case 1: data of section is not correct ---
            assert(0 <= x && x < lx && 0 <= y && y <= ly); 

            //--- case 2: data of section is correct ---
            return buffer + (y * wrap + x) * pixelSize;
        };

        bool isEmpty() const {return getSize() == DimensionT<int>(); };

        //----- Linear toggle functions -----

        bool CheckLinear() const;

        void setLinear(const bool linear);
        
        //------ Canvas transformation operations (not implimented, hold off for now as they arent necessary) ------

        void xMirror();
        void yMirror();
        void rotate180();
        void rotate90();
        void clear();
        void clearOutside(const RectT<int> &rect);


            
        //----- Pure Virtual functions (virtual = 0) -----

        /*
        virtual ToonzRasterP clone() const = 0;

        virtual ToonzRasterP extract(RectT<int> &rect) = 0;

        virtual ToonzRasterP extract(int x0, int y0, int x1, int y1) = 0;

        virtual void createBlank() = 0;

        */


};


//----- ToonzRasterT class (template type is pixel type -> TPixel32, TPixel64, etc) -----

template <class T>

class ToonzRasterT : public ToonzRaster {
    
    protected:


    public: 
        //----- variables ------
        typedef T Pixel; //allows for pixel type not to be T but Pixel when accessing it later on
        
        //----- Constructors -----

        ToonzRasterT(int i_lx, int i_ly) 
                : ToonzRaster(
                    i_lx, 
                    i_ly, 
                    sizeof(T)) { buffer = new UCHAR[lx * ly * sizeof(T)];};
        

        ToonzRasterT(int i_lx, int i_ly, int i_wrap, T *i_buffer, ToonzRasterT<T> *i_parent, bool i_bufferOwner = false)
                : ToonzRaster(
                    i_lx, 
                    i_ly, 
                    sizeof(T), 
                    i_wrap, 
                    reinterpret_cast<UCHAR *>(i_buffer), //reinterpret_cast allows for bit-operations!
                    i_parent, 
                    i_bufferOwner) {};

        //----- deconstructor ------
        ~ToonzRasterT(){};



        //----- Accessing pixel Buffer at specific Row -----

        T *pixelRow(int y = 0) {
                assert(0 <= y && y < getLy());
                return reinterpret_cast<T *>(buffer) + (getWrap() * y); //start + offset
         };


        const T *pixelRow(int y = 0) const { 
            assert(0 <= y && y < getLy());
            return reinterpret_cast<T *>(buffer) + (getWrap() * y); //start + offset
        };


        //----- virtual function overrides (all of these are from a template class that I wann keep generic so we define them here) -----

        /*
        const ToonzRasterP clone() const override { 

            return std::make_shared<ToonzRasterT<T>>(lx, ly);

        };
        */



        /*
            Note: 
                    extract will keep the same "wrap" but a different 
                    lx than before, this means that when using the 
                    data from the new object, it will adjust and keep
                    our pixels in the Rectangle we specified
        */

        /*
        ToonzRasterP extract(int x0, int y0, int x1, int y1) override { };
            RectT rect(x0, y0, x1, y1);   


        
        ToonzRasterP extract(RectT &rect) override { };
            if (isEmpty() || getBounds().overlaps(rect) == false) return TRasterP();
            
            return std::make_shared<ToonzRaster<T>>(rect.getLx(), rect.getLy(), wrap, pixelRow(rect.y0) + rect.x0, this) 

        */
        
        void createBlank(){

            assert(lx > 0 && ly > 0);
            assert(buffer != nullptr);

            std::memset(buffer, 255, ly * wrap * getPixelSize());
        }

        };

/*

    Note: 
           1. Below are types of our class but as smartpointer

           2. These ARE JUST types

           3. when creating a new object do: std::make_shared<ToonzRaster>( **constructor varaibles** );
                                             std::make_shared<ToonzRasterT>( **constructor varaibles** );

*/


//----- ToonzRasterPT classtype (shared pointer with template T) -----

template <class T>

using ToonzRasterPT = std::shared_ptr<ToonzRasterT<T>>;  // "using" does not work with "typedef" so this is the compromise



#endif