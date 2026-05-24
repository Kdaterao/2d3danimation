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

        std::vector<T> colorBuf;

        //----- Constructors -----

        toonzBrush() {};

        toonzBrush(ToonzRasterPT<T>  i_raster, T i_color, DimensionT<int> i_brushSize) 
            : raster(i_raster),
              brushSize(i_brushSize),
              color(i_color) {
                rasterSize = raster->getBounds();

                colorBuf.assign(this->rasterSize.x1 + 1, color);
              };
         
        toonzBrush(ToonzRasterPT<T>  i_raster, T i_color, float i_lx, float i_ly)
            : raster(i_raster),
              brushSize(DimensionT<int>(i_lx, i_ly)),
              color(i_color) {
                rasterSize = raster->getBounds();


                colorBuf.assign(this->rasterSize.x1 + 1, color);
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
        void drawPixelDouble(int x, int y, int length){
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
                //////std::cout<<"xoffset: "<<xoffset<<" chunk: "<<chunk<<" length: "<<length << "rest:" << length- xoffset <<std::endl;
                int rest = sizeof(T) * (length - xoffset);
                currPixel = raster->getRawData(x + xoffset, y);
                std::memcpy(currPixel, bufferStart, rest);
            }
            
            //DEBUG
            //UCHAR* c = reinterpret_cast<UCHAR*>(&color);
            //printf("R:%d G:%d B:%d A:%d\n", c[0], c[1], c[2], c[3]);
            
        }



        inline void  drawPixelMemory(int x, int y, int length){

   
            //----- Put initial color in -----
            UCHAR* startPixel = raster->getRawData(x, y);
            std::memcpy(startPixel, colorBuf.data(), length * sizeof(T));
        }

        //Im doing this to enforce name
        virtual void drawBrush(PointTI a, PointTI b) = 0; 
        virtual void resize(int r) = 0;

    public:
    //----- utility functions ------

    //deletes old color and replaces it with new one
    inline void setColor(T i_color){
            color = i_color;
            colorBuf.assign(this->rasterSize.x1 + 1, color);
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

    int radius;



    //----- variables -----
    std::vector<int> circleMin;
    std::vector<int> circleMax;


    public: 

    //----- Constructors -----

    DefaultCircleBrush() {};
    DefaultCircleBrush(ToonzRasterPT<T>  i_raster, T i_color, int r) 
        : toonzBrush<T>(i_raster, i_color, DimensionT<int>(r,r)) {
            radius = int(this->brushSize.lx /2);
            circle(radius);


         }; 


    //----- functions -----

    //handles resizing radius
    void resize(int r) override { 
        this->brushSize.lx = r*2;
        this->brushSize.ly = r*2;

        radius = r;
        circle(radius);

    };







    //handles drawing operation
    void drawBrush(PointTI a, PointTI b) override { 

        PointTF p1 = {a.x * 1.0f, a.y * 1.0f};
        PointTF p2 = {b.x * 1.0f, b.y * 1.0f};

        drawBrush(p1, p2);
    } 




        /*
            What do we fill in...

            two circles 

            one rectangle 
        
        */

void drawBrush(PointTF p1, PointTF p2) { 

        //==============================================
        // pick left side (a) and right side (b)
        //============================================


        PointTF a;
        PointTF b;

        if(p1.x < p2.x){
            a = p1;
            b = p2;
        } else {
            a = p2;
            b = p1;
        }
        

        //=============================================
        // build up rectangle 
        //===============================================


        //points
        int x0 = a.x;
        int y0 = a.y;

        int x1 = b.x;
        int y1 = b.y;



        //slope
        int dx = x1 - x0;
        int dy = y1 -y0;


        float length = 0.96f * std::max(abs(dx), abs(dy)) 
            + 0.40f * std::min(abs(dx), abs(dy));


        float dx_90 = 0;
        float dy_90 = 0;

        //unit, perpendicular slope
        if(length != 0){
            dx_90 = -dy / length;
            dy_90 = dx / length;
        }


      
        /*
            cool little capsule i made :)
             a1 ----- b1 
             |        | 
             a2 ----- b2
        */
            

        // build rectangle of capsule
        PointT a1 = PointT(int((dx_90 * radius) + x0), int((dy_90 * radius) + y0));
        PointT a2 = PointT(int(-(dx_90 * radius) + x0), int(-(dy_90 * radius) + y0)); 

        PointT b1 = PointT(int((dx_90 * radius) + x1), int((dy_90 * radius) + y1));
        PointT b2 = PointT(int(-(dx_90 * radius) + x1), int(-(dy_90 * radius) + y1));
        


        //===============================================
        //   fill in rectangle 
        //===============================================
        
        
        if(a1.y == a2.y || b1.y == b2.y){ //couldve used "b" but i just picked "a"
        //===== case 1: vertical rectangle

            /*
            int length;
            int xMin = std::max(0, std::min(a1.x, a2.x));
            if(xMin == a1.x){
                length = std::min(a2.x, this->rasterSize.x1) - a1.x;
            } else {
                length = std::min(a1.x, this->rasterSize.x1)  - a2.x;
            }

            int yStart = std::min(a1.y, b1.y);
            int yEnd = std::max(a2.y, b2.y);
            for(int y = yStart; y <= yEnd; y++){
                this->drawPixelMemory(xMin, y, length);
            }

            */
        
        

        } else if(a1.x == a2.x || b1.x == b2.x){ //couldve used "b" but i just picked "a"
        //===== case 2: horizontal rectangle
            
        /*
            int yMin = std::min( this->rasterSize.y1, std::max(0, y0 - 2*radius));
            int yMax = std::min( this->rasterSize.y1, std::max(0, y0 + 2*radius));
            int xMin = std::min(this->rasterSize.x1, std::max(0, a1.x));
            int xMax = std::min(this->rasterSize.x1, std::max(0, b2.x));
            int length = xMax - xMin;

            for(int y = yMin; y <= yMax; y++){
                this->drawPixelMemory(xMin, y, length);
            }
        
        */


        } else {
         //===== case3: tilted rectangle 
            
         // we can split into flat top and bottom parralelogram 
         // we then make the top and bottom into a flatbottom/flattop triagnel and then rasterize them as well 


         //flat top and bottom parallolgram requires jsut one one line and then extending it out by 2*radius

         //for the triangle we must get a third point(just the same as one of the points but to the left (or right) by 2*r)
         // we then just use. bresenhams algo to quickly rasterize that part 

         int dx = std::abs(x1 - x0);
         int sx = x0 < x1 ? 1 : -1;
         int dy = -std::abs(y1 - y0);
         int sy = y0 < y1 ? 1 : -1;
         int error = dx + dy;

         while (true){
            int xMin = std::min( this->rasterSize.x1, std::max(0, x0 - 2*radius)); //clamp
            int xMax = std::min( this->rasterSize.x1, std::max(0, x0 + 2*radius)); //clamp
             this->drawPixelMemory( xMin, y0, xMax - xMin);

             int e2 = 2 * error;
             if(e2 >= dy){
                if( x0 == x1) break;
                error += dy;
                x0 += sx;
             }
             if(e2 <= dx){

                if(y0 == y1) break;
                error += dx;
                y0 += sy;
             }

         }
    
        }


        //=================================
        //  fill in circle points
        //=================================
        ////std::cout<<"--------"<<std::endl;
        ////std::cout<<"("<<x0<<","<<y0<<")"<<std::endl;
        ////std::cout<<"("<<x1<<","<<y1<<")"<<std::endl;
        
        /*
        for(int i = 0; i <= 2*radius; i++){
            
            int xcaMin =  std::max(0, ((circleMin[i] - radius) + x0));
            int xcaMax =  std::min(this->rasterSize.x1, ((circleMax[i] - radius) + x0));

            int xcbMin =  std::max(0, ((circleMin[i] - radius) + x1));
            int xcbMax =  std::min(this->rasterSize.x1, ((circleMax[i] - radius) + x1));

            
            if(y0 - radius + i < this->rasterSize.y1 && y0 - radius +  i  >= 0) this->drawPixelMemory( xcaMin,  (y0 - radius+ i),  xcaMax - xcaMin); 
            if(y1 - radius + i < this->rasterSize.y1 && y1 - radius + i  >= 0) this->drawPixelMemory(  xcbMin ,  (y1 - radius+ i), xcbMax - xcbMin  ); 
        }
        */


            
         


    
    }



    //----- helper functions -----

    private: 

       
        //----- builds extreme pair values ------
        void buildPairs(int x,  int y){

            if(y < 0){
                y = 0;
            }

            circleMin[y] = std::min(circleMin[y], x);
            circleMax[y] = std::max(circleMax[y], x);
        }



        //------ MidPoint Circle  algo------
        /*
            NOTE: feeds directly into the circle buffer
        */
        void circle(int r){ 

            this->circleMax = std::vector<int>(r*2 + 1, INT_MIN);
            this->circleMin = std::vector<int>(r*2 + 1, INT_MAX); 

            int x = r; 
            int y = 0;
            int p = 1 - r;

            while (x >= y){  

                assert(-x + r >= 0);
                assert(-y +r >= 0);
                assert(x + r >= 0);
                assert(y + r >= 0);
                buildPairs(x + r, y + r);
                buildPairs(x + r, -y + r);
                buildPairs(-x + r, y + r);
                buildPairs(-x + r, -y + r);
                buildPairs(y + r, -x + r);
                buildPairs(y + r, x + r);
                buildPairs(-y + r, -x + r);
                buildPairs(-y + r, x + r);

                y += 1;
                if (p <= 0){
                    p += 2*y + 1;
                } else {
                    x -= 1;
                    p += 2*(y - x) + 1;
                }
            }
        }



    };









    



#endif