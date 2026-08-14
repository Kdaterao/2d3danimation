#ifndef TOONZBRUSH_H
#define TOONZBRUSH_H

#include <types.h>
#include <toonzGeometry.h>
#include <rasterPixel.h>
#include <raster.h>

#include <string.h>
#include <unordered_map>
#include <vector>
#include <array>
#include <iostream>

#include <thread>
#include <vector>


/*
what do we need to do?

    - based on the position of the cursor we draw "stamp" of our brush onto the canvas via a image buffer!

What format is this?

    - This brush is going to be made for raster based images..

Edge cases:

    case 1:"stamp" not within the bounds of the raster image..

            subcase 1: whole stamp outside the bounds

            subcase 2: part of stamp is outside the bounds

    case 2: whole "stamp" is within the bounds of the raster image

    case 3: brush is too large and inputs an extremely large amount pixels in a short amount of time?

            subcase 1:  convex shape

            subcase 2: simple concave shape(only one divot or something)

            subcase 3: complex concave shape(star or something)


    case 4: brush is less than the size of one pixel

            subcase 1:  convex shape

            subcase 2: simple concave shape(only one divot or something)

            subcase 3: complex concave shape(star or something)



IDEA 1: - have a raster image with a padding of the size of the largest possible brush?
        - routinely delete pixels in the padding area(optional make we want to keep padding)

        pros: low amount of checks 
        cons: higher memory overhead

IDEA 2: - We are guarunteed that the brush is never going to tilt 
        - We can create a "cutoff" to reduce checks, meaning that the first pixel which fails means any pixels beyond that
          in the directionm of the box it touches is simply not going to be checked...

        pros: less memory overhead

        cons: high amount of checks

IDEA 3: - Using cpu parrellization for scanfilling, (split up by row recursively)
        - Using cpu parrellization for filling (split up by row recursively)
        - additionlly make larger brushes update the buffer less times a second

        pros: very large brushes are able to put onto the cavas with less lage
        cons: high overhead with smaller brushes

IDEA 4: capsule algo for the cirucular brush, only draw half a circle for the fist and last point, for all other points just draw two points that are a
        perpendicular line to the center line, then connect those to the starting and last point cicles then use a scanfill line algo to fill in the stuff 

       -  what we need--> A perpendicular line --> we can use a transformation matrix for 90 degrees(ccw) [(0,-1),(1,0)] to get this 
       -  convert this to a unit vector then get our two points by doing u * r and u * -r ! 
       - now we should be able to get lines 


       starting point, use bresenham algorithm to create solid,


which ideas to impliment??

IDEA 2 easier implimentation for now, however IDEA 1 may be a better idea...
IDEA 3 for larger brushes
IDEA 4 for default brush

architecutre

base brush class: (Variables, single pixel color change, single pixel width stroke(all things when small enough are da just a pixel))

base brush class -> default circle brush -> Capsule, scan-line fill algo...

*/


//---------------------------------
//    Helper Classes for brush  
//---------------------------------   
        
        
class HalfCoord {
  //variables(although they arent in private or public they are private by default)
  std::unique_ptr<int[]> m_array;  
  int m_radius;

public:



  //Halfcoord constructor that gives a list of half widths for every y level !
  HalfCoord(int radius) : m_radius(radius), m_array(new int[radius + 1]) {
    assert(radius >= 0);


    memset(m_array.get(), 0, (m_radius + 1) * sizeof(int));


    float dCircle = 1.25f - m_radius;  // Initialize decision variable
    int y         = m_radius;          // Initialize scanline index
    int x         = 0;                 // Initialize column index
    do {
      m_array[y] = std::max(x, m_array[y]);
      m_array[x] = y;
      if (dCircle <= 0) {
        dCircle = dCircle + 2 * x + 3;
      } else {
        y--;
        dCircle = dCircle + 2 * (x - y) + 5;
      }
      x++;

    } while (y >= x);
  }



  inline int getCoord(int x) {
    //std::cout<<"x:"<<x<<std::endl;
    //std::cout<<"m_radius:"<<m_radius<<std::endl;
    assert(0 <= x && x <= m_radius);
    return m_array[x];
  };




private:

  HalfCoord(const HalfCoord &) = default;
  HalfCoord &operator=(const HalfCoord &) = default;
};






//----- Base Brush Class -----
/*
    All brushes get these variables, and the drawPixel operation!

    Additionally all brushes from this class must have the "drawBrush function"

*/

template<class T>

class toonzBrush {

    protected:

        //----- Variables -----
        Raster<T>* raster = nullptr; // non-owning; RasterLayer owns the Raster
        RectTI rasterSize; //size of the raster 
        T color; // color of our brush
        DimensionTI brushSize;// lx and ly of brush
        int pixelSize = sizeof(T); //data size of one pixel
        bool eraser = false; //our eraser value (default to false)


        //-----------------------------
        //      CONSTRUCTORS 
        //----------------------------

        toonzBrush() {};

        toonzBrush(Raster<T>* i_raster, T i_color, DimensionT<int> i_brushSize) 
            : raster(i_raster),
              brushSize(i_brushSize),
              color(i_color) {
                rasterSize = raster->getBounds();
              };

        //-----------------------------
        //      DECONSTRUCTOR
        //----------------------------

        ~toonzBrush(){ }


        //-----------------------------
        //  VARIOUS DRAW PIXEL METHODS 
        //-----------------------------

        //----- Inserts only one pixel O(1) --------

        inline void drawPixel(int x, int y){
            UCHAR* raw = raster->getRawData(x, y, true);
            T* pixel = reinterpret_cast<T*>(raw);

            if(eraser){
                std::memset(raw, 0, sizeof(T)); // transparent
            } else {
                pixel->composite(color);
            }
        }

        //------------------------------
        //  Virtual fucntions 
        //------------------------------

        virtual void drawBrush(PointTI a, PointTI b) = 0; 
        virtual void resize(int r) = 0;
        virtual void resetBrush() = 0;
        virtual void startBrush() = 0;
        virtual void toggleEraser(bool value) = 0;


    

    //------------------------
    //  PUBLIC UTILITY FUCNTIONS 
    //------------------------
    public:

        //------ set new brush color ------

        inline void setColor(T i_color){

            // { NOTE } set a toggle so we may switch between 15 bit myapint and our regular shi
                color = i_color;
                
                color.r = color.r >> 1;
                color.g = color.g >> 1;
                color.b = color.b >> 1;
        }
};












































//----- Default Circle Brush -----

/*
    Architecture of Default Circle Brush:

    - Precompute circle and store as array of PointT<int> points  -> Memory O( 2 * pi * r * sizeOf(int))
    - scanfill array stores the most extreme x axis points per a row in a Map -> Memory O(heightOfCapsule * 2 * sizeOf(int))
    
    **********

    take in two points 
            ↓ 
    Make rectangle from 4 points
            ↓
    Draw out exterior lines of rectangle 
            ↓
    Put into scanfillPair buffer
            ↓
    transform circle to correct position
            ↓
    Put into scanfillPair buffer
            ↓
    Do scanfill based on scanfill buffer(multithreaded maybe if circle is very large)
*/

template<class T>

class DefaultCircleBrush : public toonzBrush<T> {

    //----- variables -----
    std::unordered_map<int, std::array<int, 2>> fillPairs; //scanfillPairs --> store min and max x position
    std::vector<PointT<int>> circle; //holds memory of current circle points
    
    std::vector<int> xMin;
    std::vector<int> xMax;
    int yMin;
    int yMax;

    

    public: 

        //----- Constructors -----

        DefaultCircleBrush() {};
        DefaultCircleBrush(Raster<T>* i_raster, T i_color, int r) 
            : toonzBrush<T>(i_raster, i_color, DimensionT<int>(r,r)) {

                HalfCircle(r);
                xMin = std::vector<int>(this->rasterSize.y1, INT_MAX);
                xMax = std::vector<int>(this->rasterSize.y1, INT_MIN);
            }; 


        //----- functions -----

        //handles resizing radius
        void resize(int r) override { 
            this->brushSize.lx = r;
            this->brushSize.ly = r;

            circle.clear();
            HalfCircle(r);
        };


        void toggleEraser(bool value) override {
            this->eraser = value;
        }

        //handles drawing operation

        //overload
        void drawBrush(PointTI a, PointTI b) override { 

            PointTF p1 = {a.x * 1.0f, a.y * 1.0f};
            PointTF p2 = {b.x * 1.0f, b.y * 1.0f};

            drawBrush(p1, p2);
        } 



        

        void drawBrush(PointTF a, PointTF b) { 


            //std::cout<<"a:"<<"("<<a.x<<","<<a.y<<")"<<std::endl;
            //std::cout<<"b:"<<"("<<b.x<<","<<b.y<<")"<<std::endl;

            //----- variables -----
            //radius
            int radius = this->brushSize.lx; //this brush is a square so lx and ly are same thing here

 
            
            //slope
            float dx = b.x - a.x;
            float dy = b.y -a.y;
            float length =  sqrt(dx*dx + dy*dy); // may change to double if not exact enough

            float dx_90 = 0;
            float dy_90 = 0;

            //unit, perpendicular slope
            if(length != 0){
                dx_90 = -dy / length;
                dy_90 = dx / length;
            }



            int lx = this->rasterSize.x1;
            int ly = this->rasterSize.y1;

            
             HalfCoord halfCoord = HalfCoord(radius);





            //---------------------
            //  ONLY A CIRCLE
            //---------------------
                
            if(a.x == b.x && b.x == b.y){
 
                std::cout << "circle" <<std::endl;
                int yMin = std::max((int)a.y - radius, 0);
                int yMax = std::min((int)a.y + radius, ly - 1);

                for(int y = yMin; y <= yMax; y++){

                    //get half width 
                    int deltay = abs(y - a.y);
                    int width = halfCoord.getCoord(deltay);

                    // determine min and max x positions
                    int xMin = std::max((int)a.x - width, 0);
                    int xMax = std::min((int)a.x + width, lx -1);

                    // draw 
                    for(int x = xMin; x <= xMax; x++){
                        this->drawPixel(x,y);
                    }
                }
                return;
            }



            //---------------------
            //  HORIZONTAL LINE
            //---------------------
            if (a.y == b.y){
                std::cout << "horiznta;" <<std::endl;
                int yMin = std::max((int)a.y - radius, 0) - (int)a.y;
                int yMax = std::min((int)a.y + radius, ly - 1) - (int)a.y;
                
                
                if(a.x < b.x){
                    //------ point A on the left of point B -------
                    
                    for(int y = yMin; y <= yMax; y++){

                        // get circle half width 
                        int deltay = abs(y);
                        int width = halfCoord.getCoord(deltay);

                        // determine min and max x positions
                        int xMin = std::max((int)a.x - width, 0);
                        int xMax = std::min((int)b.x + width, lx -1);

                        // draw
                        for(int x = xMin; x <= xMax; x++){
                            this->drawPixel(x, (int)a.y + y);
                        }
                    }
                    
                } else {
                    //------- point A on the Right of point B -------
                    for(int y = yMin; y <= yMax; y++){

                        // get circle half width 
                        int deltay = abs(y);
                        int width = halfCoord.getCoord(deltay);

                        // determine min and max x positions 
                        int xMin = std::max((int)b.x - width, 0);
                        int xMax = std::min((int)a.x + width, lx - 1);

                        // draw
                        for(int x = xMin; x <= xMax; x++){
                            this->drawPixel(x, (int)a.y + y);
                        }
                    }
                }
                return;
            }


            if(a.x == b.x){


                    if(a.y < b.y){
                    std::cout << "vertical"<< std::endl;
                    int xMin = std::max((int)a.x - radius, 0);
                    int xMax = std::min((int)a.x + radius, lx - 1);

                    for(int x = xMin; x <= xMax; x++){
                        
                        //get circle halfwidth 
                        int deltax = abs(x - (int)a.x);
                        int width = halfCoord.getCoord(deltax);

                        //determine min and max x positions
                        int yMin = std::max((int)a.y - width, 0);
                        int yMax = std::min((int)b.y + width, ly);

                        // draw
                        for(int y = yMin; y <= yMax; y++){
                            this->drawPixel(x,y);
                        }

                    }
                } else {

                    std::cout << "vertical"<< std::endl;
                    int xMin = std::max((int)a.x - radius, 0);
                    int xMax = std::min((int)a.x + radius, lx - 1);

                    for(int x = xMin; x <= xMax; x++){
                        
                        //get circle halfwidth 
                        int deltax = abs(x - (int)a.x);
                        int width = halfCoord.getCoord(deltax);

                        //determine min and max x positions
                        int yMin = std::max((int)b.y - width, 0);
                        int yMax = std::min((int)a.y + width, ly);

                        // draw
                        for(int y = yMin; y <= yMax; y++){
                            this->drawPixel(x,y);
                        }

                    }
                }

                    return;
            }





            


            //----- build capsule -----
            
            //  cool little capsule i made :)
            //   *p1 ----- p2* 
            //   * |        |*
            //   *p4 ----- p3*


            // build rectangle of capsule
            PointT p1 = PointT(int((dx_90 * radius) + a.x), int((dy_90 * radius) + a.y));
            PointT p2 = PointT(int((dx_90 * radius) + b.x), int((dy_90 * radius) + b.y));
            PointT p3 = PointT(int(-(dx_90 * radius) + b.x), int(-(dy_90 * radius) + b.y));
            PointT p4 = PointT(int(-(dx_90 * radius) + a.x), int(-(dy_90 * radius) + a.y));



            yMin = std::max(0, std::min(this->rasterSize.y1, (std::min({p1.y, p2.y, p3.y, p4.y}) - radius)));
            yMax = std::max(0, std::min(this->rasterSize.y1, (std::max({p1.y, p2.y, p3.y, p4.y}) + radius)));

            //std::cout<<"ymin"<<yMin<<std::endl;
            //std::cout<<"ymax"<<yMax<<std::endl;


            // debug
            //std::cout<<"p1:"<<"("<<p1.x<<","<<p1.y<<")"<<std::endl;
            //std::cout<<"p2:"<<"("<<p2.x<<","<<p2.y<<")"<<std::endl;
            //std::cout<<"p3:"<<"("<<p3.x<<","<<p3.y<<")"<<std::endl;
            //std::cout<<"p4:"<<"("<<p4.x<<","<<p4.y<<")"<<std::endl;

            // draw lines and build up pairs
            drawLine(p1, p2);
            drawLine(p3, p4);
            
            //--- build halfcircles  of the capsule ---

            for(const auto& point : circle){
                //std::cout<<"Circledrawn:"<<"("<<point.x + a.x<<","<<point.y + a.y<<")"<<std::endl;
                //std::cout<<"Circledrawn:"<<"("<<point.x + b.x<<","<<point.y + b.y<<")"<<std::endl;

                //build up pairs
                buildPairs(point.x + a.x, point.y +a.y);
                buildPairs(point.x + b.x, point.y +b.y);

            }

            //------ scanfill our pairs ------


            for(int y = yMin; y < yMax; y++){
                if(xMin[y] == INT_MAX || xMax[y] == INT_MIN) continue;

                for(int point = xMin[y]; point <= xMax[y]; point++){
                    //----- find position in current rectangle ------
                        

                    //------ paint via a list of cutoffs with memory fill -------
                    
                    
                    this->drawPixel(point, y);
                }
            }
            

            //------ reset ---=---

            std::fill(xMin.begin() + std::max(0, yMin - 1), xMin.begin() + std::min(ly , yMax + 1), INT_MAX);
            std::fill(xMax.begin() + std::max(0, yMin - 1), xMax.begin() + std::min(ly , yMax + 1), INT_MIN);
        
        }


        void resetBrush() {};

        void startBrush() {};


    //----- helper functions -----

    private: 


        void buildPairs(int x,  int y){ 


            if(x < 0){
                x = 0;
            } 
            else if(x >= this->rasterSize.x1){
                x = this->rasterSize.x1 - 1;
            }
            if(y < 0){
                y = 0;
            }
            if(y >= this->rasterSize.y1){
                y = this->rasterSize.y1 - 1;
            }


            xMin[y] = std::min(xMin[y], x);
            xMax[y] = std::max(xMax[y], x);
        }
        


        //------ MidPoint Circle  algo------
        
    



        void HalfCircle(int r) {
            this->circle.clear();
            this->circle.resize(4 + 8 * r); // allocatiing array size allows us to grab points like an array 

            int i = 0;
            int x = r;
            int y = 0;
            int p = 1 - r; // initial decision parameter

            while (y <= x) {
                // 8 symmetric points covering the full circle
                this->circle[i++] = PointT( x,  y);
                this->circle[i++] = PointT(-x,  y);
                this->circle[i++] = PointT( x, -y);
                this->circle[i++] = PointT(-x, -y);
                this->circle[i++] = PointT( y,  x);
                this->circle[i++] = PointT(-y,  x);
                this->circle[i++] = PointT( y, -x);
                this->circle[i++] = PointT(-y, -x);

                y++;
                if (p < 0) {
                    p += 2 * y + 1;
                } else {
                    x--;
                    p += 2 * (y - x) + 1;
                }
            }

            this->circle.resize(i); // trim to actual count
        }


        //------- Bresenham line algo -----
        
          
        
        void drawLine(PointTI a, PointTI b){  
            int x0 = a.x;
            int y0 = a.y;
            int x1 = b.x;
            int y1 = b.y;

            int dx = abs(x1 - x0);
            int dy = abs(y1 - y0);

            int sx = (x0 < x1) ? 1 : -1;
            int sy = (y0 < y1) ? 1 : -1;

            int err = dx - dy;

            while (true) {
                buildPairs(x0, y0);
                //std::cout<<"linedrawn:"<<"("<<x0<<","<<y0<<")"<<std::endl;
                if (x0 == x1 && y0 == y1) break;

                int e2 = 2 * err;

                if (e2 > -dy) {
                    err -= dy;
                    x0 += sx;
                }

                if (e2 < dx) {
                    err += dx;
                    y0 += sy;
                }
            }
        }

};













    









#endif 