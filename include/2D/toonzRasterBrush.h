#ifndef TOONZBRUSH_H
#define TOONZBRUSH_H

#include <types.h>
#include <toonzGeometry.h>
#include <toonzRasterPixel.h>
#include <toonzRaster.h>


#include <string.h>
#include <unordered_map>
#include <vector>
#include <array>
#include <iostream>


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


/*
Implimentation of the default circle brush

- Instead of using a stamp styled brush to draw it, we use capsules -> reduces overwritten pixels

- We also make sure that the inside of the shape has no lines inside of it -> (we make less pixel operations this way since we only create one circle instead of 2 circle) (basically less wasteage)

- To do this we make a rectangle, and then 2 half circles

- We use midpoint circle algorithm to the circles

- we use the bresenham's line algorithm to connect the outer rectrangle edges

- we can then just use a very rudimentary raster fill algo to fill in the capsule)

*/

/*
    Storing circle vs recomputing circle every draw...


    Storing circle --> less cpu work, more memory overhead(minimal) --> -  would have to wastefully do compare operations for a whole circle (two halves added together)
                                                                        -  would have to transform both circles to proper points (so addition operations for both circle circumferences )
    
    recompuating --> more cpu work, no memory overhead(like none) --> less compare operations   

                                                                        - need to do multiplecation operations, 2 extra compare operations, addition operations, and minus operations ....


    Seems like it makes more sense to just store one circle when doing resize..

*/

//----- Base Brush Class -----
/*
    All brushes get these variables, and the drawPixel operation!

    Additionally all brushes from this class must have the "drawBrush function"

*/

template<class T>

class toonzBrush {

    protected:

        //----- Variables -----
        ToonzRasterPT<T> raster; //shared_pointer of our raster(dont need * since its a typedef of a pointer!)
        RectTI rasterSize;
        T color; // put ts on the stack
        DimensionTI brushSize;// lx and ly of brush
        int pixelSize = sizeof(T); //data size of one pixel
        bool single; //Checks if size of brush is only one pixel 

        //----- Constructors -----

        toonzBrush() {};

        toonzBrush(ToonzRasterPT<T>  i_raster, T i_color, DimensionT<int> i_brushSize) 
            : raster(i_raster),
              brushSize(i_brushSize),
              color(i_color) {
                rasterSize = raster->getBounds();
              };
         
        toonzBrush(ToonzRasterPT<T>  i_raster, T i_color, float i_lx, float i_ly)
            : raster(i_raster),
              brushSize(DimensionT<int>(i_lx, i_ly)),
              color(i_color) {
                rasterSize = raster->getBounds();
              };

        //----- deconstructor -----

        ~toonzBrush(){ }
        //----- Functions -----

        //utility function for all children
        inline void drawPixel(int x, int y){

            UCHAR* pixel = raster->getRawData(x, y);
            std::memcpy(pixel, &color, sizeof(T));

            //DEBUG
            //UCHAR* c = reinterpret_cast<UCHAR*>(&color);
            //printf("R:%d G:%d B:%d A:%d\n", c[0], c[1], c[2], c[3]);
        }


        //utility function for all children
        inline void drawPixelDouble(int x, int y, int length){
            int chunk = 0;//current chunk of pixels in canvas we have inserted
            int xoffset = 0;
            UCHAR* bufferStart;//first pixel point of the chunk we are copying from
            UCHAR* currPixel;//currnet pixel point

            //----- Put initial color in -----
            currPixel = raster->getRawData(x, y);
            std::memcpy(currPixel, &color, sizeof(T));
            chunk = sizeof(T);
            bufferStart = currPixel;
            xoffset++;
            
            //------ Double Insert ------
            while(xoffset*2 < length){
                currPixel = raster->getRawData(x + xoffset, y);
                std::memcpy(currPixel, bufferStart, chunk);
                xoffset = xoffset * 2;
                chunk = chunk *2;
            }
       
            //----- after we can no longer double insert, we just insert the rest of the pixels(we already have a big enough buffer) ------
            if(xoffset < length){
                std::cout<<"xoffset: "<<xoffset<<" chunk: "<<chunk<<" length: "<<length << "rest:" << length- xoffset <<std::endl;
                int rest = sizeof(T) * (length - xoffset);
                currPixel = raster->getRawData(x + xoffset, y);
                std::memcpy(currPixel, bufferStart, rest);
            }
            
            //DEBUG
            //UCHAR* c = reinterpret_cast<UCHAR*>(&color);
            //printf("R:%d G:%d B:%d A:%d\n", c[0], c[1], c[2], c[3]);
            
        }

        //Im doing this to enforce name
        virtual void drawBrush(PointTI a, PointTI b) = 0; 
        virtual void resize(int r) = 0;

    public:
    //----- utility functions ------

    //deletes old color and replaces it with new one
    inline void setColor(T i_color){
            color = i_color;
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
            ↓
    while doing scanfill send pixels to raster image

    *********
    Why store circle?

        1. Wayyy less multiplication operations that if i were to recompute it everytime

        2. Extra memoryoverhead is very minimal

    Why Capsule instead of 1 circle?

        1. Less pixel overwrites (higher efficiency)
*/
template<class T>

class DefaultCircleBrush : public toonzBrush<T> {

    //----- variables -----
    std::unordered_map<int, std::array<int, 2>> fillPairs; //scanfillPairs --> store min and max x position
    std::vector<PointT<int>> circle; //holds memory of current circle points  

    public: 

    //----- Constructors -----

    DefaultCircleBrush() {};
    DefaultCircleBrush(ToonzRasterPT<T>  i_raster, T i_color, int r) 
        : toonzBrush<T>(i_raster, i_color, DimensionT<int>(r,r)) {

             HalfCircle(r);
         }; 


    //----- functions -----

    //handles resizing radius
    void resize(int r) override { 
        this->brushSize.lx = r;
        this->brushSize.ly = r;

        circle.clear();
        HalfCircle(r);
    };

    //handles drawing operation
    void drawBrush(PointTI a, PointTI b) override { 


        //std::cout<<"a:"<<"("<<a.x<<","<<a.y<<")"<<std::endl;
        //std::cout<<"b:"<<"("<<b.x<<","<<b.y<<")"<<std::endl;

        //----- variables -----
        //radius
        int radius = this->brushSize.lx; //this brush is a square so lx and ly are same thing here

        //points
        int x0 = a.x;
        int y0 = a.y;
        int x1 = b.x;
        int y1 = b.y;
        
        //slope
        int dx = x1 - x0;
        int dy = y1 -y0;
        float length =  sqrt(dx*dx + dy*dy); // may change to double if not exact enough

        //unit, perpendicular slope
        float dx_90 = -dy / length;
        float dy_90 = dx / length;


        //----- build capsule -----
        /*
            cool little capsule i made :)
            *p1 ----- p2* 
           * |        |  *
            *p4 ----- p3*
        */
        

        // build rectangle of capsule
        PointT p1 = PointT(int((dx_90 * radius) + x0), int((dy_90 * radius) + y0));
        PointT p2 = PointT(int((dx_90 * radius) + x1), int((dy_90 * radius) + y1));
        PointT p3 = PointT(int(-(dx_90 * radius) + x1), int(-(dy_90 * radius) + y1));
        PointT p4 = PointT(int(-(dx_90 * radius) + x0), int(-(dy_90 * radius) + y0));

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
            //std::cout<<"Circledrawn:"<<"("<<point.x + x0<<","<<point.y + y0<<")"<<std::endl;
            //std::cout<<"Circledrawn:"<<"("<<point.x + x1<<","<<point.y + y1<<")"<<std::endl;

            //build up pairs
            buildPairs(point.x + x0, point.y +y0);
            buildPairs(point.x + x1, point.y +y1);

        }

        //------ scanfill our pairs ------


        for(const auto& row : fillPairs){
            const int y = row.first;
            const int min = row.second[0];
            const int max = row.second[1];

            for(int x = min; x <= max; x++ ){

                this->drawPixel(x, y);
            }
        }

        this->fillPairs.clear(); //clear out scanfill once finished..

    }




    void drawBrush(PointTF a, PointTF b) { 


        //std::cout<<"a:"<<"("<<a.x<<","<<a.y<<")"<<std::endl;
        //std::cout<<"b:"<<"("<<b.x<<","<<b.y<<")"<<std::endl;

        //----- variables -----
        //radius
        int radius = this->brushSize.lx; //this brush is a square so lx and ly are same thing here

        //points
        float x0 = a.x;
        float y0 = a.y;
        float x1 = b.x;
        float y1 = b.y;
        
        //slope
        float dx = x1 - x0;
        float dy = y1 -y0;
        float length =  sqrt(dx*dx + dy*dy); // may change to double if not exact enough

        //unit, perpendicular slope
        float dx_90 = -dy / length;
        float dy_90 = dx / length;


        //----- build capsule -----
        /*
            cool little capsule i made :)
            *p1 ----- p2* 
           * |        |  *
            *p4 ----- p3*
        */
        

        // build rectangle of capsule
        PointT p1 = PointT(int((dx_90 * radius) + x0), int((dy_90 * radius) + y0));
        PointT p2 = PointT(int((dx_90 * radius) + x1), int((dy_90 * radius) + y1));
        PointT p3 = PointT(int(-(dx_90 * radius) + x1), int(-(dy_90 * radius) + y1));
        PointT p4 = PointT(int(-(dx_90 * radius) + x0), int(-(dy_90 * radius) + y0));

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
            //std::cout<<"Circledrawn:"<<"("<<point.x + x0<<","<<point.y + y0<<")"<<std::endl;
            //std::cout<<"Circledrawn:"<<"("<<point.x + x1<<","<<point.y + y1<<")"<<std::endl;

            //build up pairs
            buildPairs(point.x + x0, point.y +y0);
            buildPairs(point.x + x1, point.y +y1);

        }

        //------ scanfill our pairs ------


        for(const auto& row : fillPairs){
            const int y = row.first;
            const int min = row.second[0];
            const int max = row.second[1];



            this->drawPixelDouble(min, y, max-min);

            
            /*
            for(int x = min; x <= max; x++ ){

                this->drawPixel(x, y);
            }
                */
        }

        this->fillPairs.clear(); //clear out scanfill once finished..

    }






    //----- helper functions -----

    private: 

        //----- builds extreme pair values ------
        void buildPairs(int x,  int y){

            if(x < 0){
                x = 0;
            } 
            else if(x > this->rasterSize.x1){
                x = this->rasterSize.x1;
            }
            if(y < 0){
                y = 0;
            }
            if(y > this->rasterSize.y1){
                y = this->rasterSize.y1;
            }

            //----- case 1: row has element ----
            if (this->fillPairs.find(y) != this->fillPairs.end()){
                std::array<int, 2> &row = this->fillPairs[y]; 
                if(row[1] != INT_MIN) { //our default in the 2nd pos
                    //----- subcase 1: row is filled -----
                    int min = row[0];
                    int max = row[1]; 

                    //store extremes only!
                    if( x < min){ 

                        min = x;
                        row[0] = min;
                        row[1] = max;

                    } else if (x > max) {
                        max = x;
                        row[0] = min;
                        row[1] = max;
                    }
                } else {
                    //----- subcase 2: array only has one element -----
                    int element = row[0];
                    
                    //sort ascending order!
                    if(element > x){
                        row[0] = x;
                        row[1] = element;

                    } else {
                        row[1] = x;
                    }
                }

            } else {
            //------ case 2: row is empty ----
                this->fillPairs[y] = {x, INT_MIN};
            }

        }



        //------ MidPoint Circle  algo------
        /*
            NOTE: feeds directly into the circle buffer
        */
        void HalfCircle(int r){ 

            //---- starting points ----

            //store circle at the (0,0) spot (top left hand corner..)
            int x = 0; 
            int y = -r; 
            int p = -r;

            while (x < -y){ 
            
            //----- change y or nah? -----
                if (p > 0){
                    y += 1;
                    p += 2*(x+y) + 1;
                } else {
                    p += 2*x + 1;
                }
            

            //----- add all quadrants -----
            this->circle.push_back(PointT(x, y));
            this->circle.push_back(PointT(x, -y));
            this->circle.push_back(PointT(-x, y));
            this->circle.push_back(PointT(-x, -y));
            this->circle.push_back(PointT(y, -x));
            this->circle.push_back(PointT(y, x));
            this->circle.push_back(PointT(-y, -x));
            this->circle.push_back(PointT(-y, x));

            //----- next block -----
            
            x += 1;

            }


        }

        //------- Bresenham line algo -----
        /*
            Note:  This function directs all points to the buildPairs function..
        */
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