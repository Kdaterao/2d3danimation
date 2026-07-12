#ifndef TOONZRASTERCIRCLEBRUSH_H
#define TOONZRASTERCIRCLEBRUSH_H   

/*
#include <vector>
#include <assert.h>
#include <toonzGeometry.h>
#include <algorithm>

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
    std::cout<<"x:"<<x<<std::endl;
    std::cout<<"m_radius:"<<m_radius<<std::endl;
    assert(0 <= x && x <= m_radius);
    return m_array[x];
  };




private:

  HalfCoord(const HalfCoord &) = default;
  HalfCoord &operator=(const HalfCoord &) = default;
};


//---------------------------------
//    Main Circle Brush Class 
//---------------------------------



template<class T>

class DefaultCircleBrush : public toonzBrush<T> {



    public: 

        //----- Constructors -----

        DefaultCircleBrush() {};
        DefaultCircleBrush(ToonzRasterPT<T>  i_raster, T i_color, int r) 
            : toonzBrush<T>(i_raster, i_color, DimensionT<int>(r,r)) {

            }; 


        //----- functions -----

        //handles resizing radius
        void resize(int r) override { 
            this->brushSize.lx = r;
            this->brushSize.ly = r;
        };


        //handles drawing operation

        //overload
        void drawBrush(PointTF a, PointTF b) { 

            PointTI p1 = {static_cast<int>(a.x), static_cast<int>(a.y)};
            PointTI p2 = {static_cast<int>(b.x), static_cast<int>(b.y)};

            drawBrush(p1, p2);
        } 



        

        void drawBrush(PointTI a, PointTI b) override { 


            //*************************
            //   Variables to use 
            //*************************



            //radius
            int radius = this->brushSize.lx; //this brush is a square so lx and ly are same thing here


            std::cout << "radius=" << radius << std::endl;


            //halfcoord 

            HalfCoord halfCoord = HalfCoord(radius);

            //points(point0 is always on the left of point1)

            if(a.y > b.y){
                PointTI cache = a;
                a = b;
                b = cache;
            } 


            int lx = this->rasterSize.x1;
            int ly = this->rasterSize.y1;
            

            if(radius < 1) { 

                //*************************
                //  Draw THIN BRUSH STROKE
                //*************************
        
                drawLine(a, b);

            } else { 
                //*************************
                //  Draw THICK BRUSH STROKE
                //*************************

                //---------------------
                //  ONLY A CIRCLE
                //---------------------
                
                if(a.x == b.x && b.x == b.y){
                    std::cout << "circle" <<std::endl;
                    int yMin = std::max(a.y - radius, 0);
                    int yMax = std::min(a.y + radius, ly - 1);

                    for(int y = yMin; y <= yMax; y++){

                        //get half width 
                        int deltay = abs(y - a.y);
                        int width = halfCoord.getCoord(deltay);

                        // determine min and max x positions
                        int xMin = std::max(a.x - width, 0);
                        int xMax = std::min(a.x + width, lx -1);

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
                    int yMin = std::max(a.y - radius, 0) - a.y;
                    int yMax = std::min(a.y + radius, ly - 1) - a.y;
                    
                    
                    if(a.x < b.x){
                        //------ point A on the left of point B -------
                        
                        for(int y = yMin; y <= yMax; y++){

                            // get circle half width 
                            int deltay = abs(y);
                            int width = halfCoord.getCoord(deltay);

                            // determine min and max x positions
                            int xMin = std::max(a.x - width, 0);
                            int xMax = std::min(b.x + width, lx -1);

                            // draw
                            for(int x = xMin; x <= xMax; x++){
                                this->drawPixel(x, a.y + y);
                            }
                        }
                        
                    } else {
                        //------- point A on the Right of point B -------
                        for(int y = yMin; y <= yMax; y++){

                            // get circle half width 
                            int deltay = abs(y);
                            int width = halfCoord.getCoord(deltay);

                            // determine min and max x positions 
                            int xMin = std::max(b.x - width, 0);
                            int xMax = std::min(a.x + width, lx - 1);

                            // draw
                            for(int x = xMin; x <= xMax; x++){
                                this->drawPixel(x, a.y + y);
                            }
                        }
                    }
                    return;
                }

                //----------------------
                //  VERTICAL LINE 
                //-----------------------

                if(a.x == b.x){
                    std::cout << "vertical"<< std::endl;
                    int xMin = std::max(a.x - radius, 0);
                    int xMax = std::min(a.x + radius, lx - 1);

                    for(int x = xMin; x <= xMax; x++){
                        
                        //get circle halfwidth 
                        int deltax = abs(x - a.x);
                        int width = halfCoord.getCoord(deltax);

                        //determine min and max x positions
                        int yMin = std::max(a.y - radius, 0);
                        int yMax = std::min(b.y + radius, ly);

                        // draw
                        for(int y = yMin; y <= yMax; y++){
                            this->drawPixel(x,y);
                        }

                    }

                    return;
                }
                //-----------------------------
                //  TILTED LINE (GENERAL CASE)
                //-----------------------------
                std::cout << "tilted"<< std::endl;

                //------ varaibles -----------



                
                //slope
                float dx = b.x - a.x;
                float dy = b.y - a.y;


                // reflect our stroke or nah (a should be on the left of b )
                int k  = 1;
                if (dx < 0) {
                    dx = -dx;
                    k  = -1;
                }


                float length =  sqrt(dx*dx + dy*dy); // may change to double if not exact enough

                //unit, perpendicular slope
                float dx_90 = 0;
                float dy_90 = 0;

                if(length != 0){
                    dx_90 = -dy / length;
                    dy_90 = dx / length;
                }


               

                PointTF up = PointTF{radius * dx_90, radius * dy_90}; //------> perpendicular cirle tangent point 
                int halfAmplCap = floor(-up.x);   //-----> get cutoff for circle caps 





     
                int cutExt, cutIn; //----> cutoffs for trapezoids + circle caps + paralleocgram
                // Vertices of the parallelogram
                PointTF rightUp;
                PointTF rightDown;
                PointTF leftUp;
                PointTF leftDown;
                double mParall;  //---> Angular coefficient of parallelogram

                if (radius > 1) {
                    for (cutExt = radius; cutExt >= 0 && halfCoord.getCoord(cutExt) <= halfAmplCap; cutExt--);
                    cutIn       = cutExt;  //  see next else statement

                    // --> we are treating the center as the center of the circle for "A"
                    rightUp.x   = dx + halfCoord.getCoord(cutIn);
                    rightUp.y   = dy - cutIn;
                    rightDown.x = halfCoord.getCoord(cutIn);
                    rightDown.y = -cutIn;
                    leftUp.x    = dx - halfCoord.getCoord(cutIn);
                    leftUp.y    = dy + cutIn;
                    leftDown.x  = -halfCoord.getCoord(cutIn);
                    leftDown.y  = cutIn;


                    mParall     = dy / (double)dx;
                } else  
                {
        
                    cutExt = radius; 
                    cutIn = 0; 
                    
                    // --> we are treating the center as the center of the circle for "A"
                    rightUp.x   = dx - up.x;
                    rightUp.y   = dy - up.y;
                    rightDown.x = -up.x;
                    rightDown.y = -up.y;
                    leftUp.x    = dx + up.x;
                    leftUp.y    = dy + up.y;
                    leftDown.x  = up.x;
                    leftDown.y  = up.y;
                    mParall     = dy/dx;
                }



                // -----  Fill circular "caps" ------

                // ----- Fill lower circular cap
                int yMin = std::max(a.y - radius, 0); //  clipping y
                int yMax = std::min(a.y - cutExt - 1, ly - 1);  //  clipping y
                for (int y = yMin; y <= yMax; y++) {
                    int r               = halfCoord.getCoord(a.y - y);
                    int xMin            = std::max(a.x - r, 0);       //  clipping x
                    int xMax            = std::min(a.x + r, lx - 1);  //  clipping x


                    // draw
                    for(int x = xMin; x <= xMax; x++){
                        this->drawPixel(x,y);
                    }
                }
                // ----- Fill upper circular cap
                yMin = std::max(b.y + cutExt + 1, 0);   //  clipping y
                yMax = std::min(b.y + radius, ly - 1);  //  clipping y
                for (int y = yMin; y <= yMax; y++) {
                    int r               = halfCoord.getCoord(y - b.y);
                    int xMin            = std::max(b.x - r, 0);       //  clipping x
                    int xMax            = std::min(b.x + r, lx - 1);  //  clipping x
                    // draw

                    for(int x = xMin; x <= xMax; x++){
                        this->drawPixel(x,y);
                    }
                }


                //------ fill trapezoids ----------- 


                //  Absolute limits of trapezoid scan lines 
                int xSegmMax = round(dx - up.x);
                int xSegmMin = round(up.x);  


                //------ lower trapezoid -----------

         

                // variables 
                int x0 = rightDown.x;
                int y0 = rightDown.y;
                int x1 = rightUp.x;
                int y1 = rightUp.y;

                int sx = (x0 < x1) ? 1 : -1;
                int sy = (y0 < y1) ? 1 : -1;

                int err = dx - dy;

                PointTI prev = PointTI{x0, y0};


                //------ loop  -------
                while (true) {

                    //------ DRAW POINT(S) --------

  

                    if(y0 != prev.y){

                        

                        //------ get y -------

                        int y = prev.y;


                        int global_y = a.y + y;

                        if(global_y < 0){
                            global_y = 0;
                        }
                        if(global_y > ly){
                            global_y = ly - 1;
                        }

                        //----- get x --------
                        int x = prev.x; // x value of our bresenham line between rightup and righdown


                        //----- get x min and x max -------
                
                        int deltay = abs(y);
                        if (deltay > radius) break;
                        
                        
                        int xMin;
                        int xMax;

                        if (k > 0) {
                            xMin = std::max(a.x - halfCoord.getCoord(deltay), 0); //get 
                            xMax = std::min(b.x + std::min(x, xSegmMax), lx - 1);   // lower
                        } else {
                            xMin = std::max(a.x - std::min(x, xSegmMax), 0); //get 
                            xMax = std::min(a.x + halfCoord.getCoord(deltay), lx - 1);
                        }


                        for(int x = xMin; x <= xMax; x++){
                            this->drawPixel(x, global_y);
                        }
                    }

                    prev = PointTI{x0, y0};
                        
                    //------ TAKE STEP ----------

                    if (x0 == x1 && y0 == y1) break;
                    if (x0 > xSegmMax) break;

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




                //------ Upper trapezoid -----------


                
                // variables (same types defined above already so just replace them)
                x0 = leftDown.x;
                y0 = leftDown.y;
                x1 = leftUp.x;
                y1 = leftUp.y;

                sx = (x0 < x1) ? 1 : -1;
                sy = (y0 < y1) ? 1 : -1;

                err = dx - dy;

                prev = PointTI{x0, y0};


                //------ loop  -------
                while (true) {

                    //------ DRAW POINT(S) --------

  

                    if(y0 != prev.y){

                        

                        //------ get y -------

                        int y = prev.y;
                        
                        int global_y = a.y + y;

                        if(global_y < 0){
                            global_y = 0;
                        }
                        if(global_y > ly){
                            global_y = ly - 1;
                        }

                        //----- get x --------
                        int x = prev.x; // x value of our bresenham line between rightup and righdown


                        //----- get x min and x max -------
                
                        int deltay = abs(y);

                        if (deltay > radius) break;
    
                        int xMin;
                        int xMax;

                        if (k > 0) {
                            xMin = std::max(b.x - halfCoord.getCoord(deltay), 0); //get 
                            xMax = std::min(b.x + std::min(x, xSegmMax), lx - 1);   // upper
                        } else {
                            xMin = std::max(b.x - std::min(x, xSegmMax), 0); //get 
                            xMax = std::min(b.x + halfCoord.getCoord(deltay), lx - 1);
                        }


                        for(int x = xMin; x <= xMax; x++){
                            this->drawPixel(x, global_y);
                        }
                    }

                    prev = PointTI{x0, y0};
                        
                    //------ TAKE STEP ----------

                    if (x0 == x1 && y0 == y1) break;
                    if (x0 > xSegmMax) break;

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




//------ fill parallelogram -----------

     //Only the right edge (rightDown -> rightUp) is walked with Bresenham.
    //Since leftDown->leftUp is an exact parallel translate of the same
    //line, the left edge's x at any row is just rightX - offset.


int offset = rightDown.x - leftDown.x;  // constant horizontal gap between the two edges

int rx0 = rightDown.x, ry0 = rightDown.y;
int rx1 = rightUp.x,   ry1 = rightUp.y;
int rsx = (rx0 < rx1) ? 1 : -1;
int rsy = (ry0 < ry1) ? 1 : -1;
int rerr = dx - dy;

PointTI rPrev = PointTI{rx0, ry0};

//------ loop -------
while (true) {

    //------ DRAW ROW --------
    if (ry0 != rPrev.y) {

        int y = rPrev.y;

        // only draw once we're past both trapezoids
        if (y > cutIn && y < (dy - cutIn)) {

            int global_y = a.y + y;
            if (global_y < 0)  global_y = 0;
            if (global_y > ly) global_y = ly - 1;

            int rightX = rPrev.x;
            int leftX  = rightX - offset;

            int xMin, xMax;
            if (k > 0) {
                xMin = std::max(a.x + leftX, 0);
                xMax = std::min(a.x + rightX, lx - 1);
            } else {
                xMin = std::max(a.x - rightX, 0);
                xMax = std::min(a.x - leftX, lx - 1);
            }

            for (int px = xMin; px <= xMax; px++) {
                this->drawPixel(px, global_y);
            }
        }
    }

    rPrev = PointTI{rx0, ry0};

    //------ TAKE STEP ----------
    if (rx0 == rx1 && ry0 == ry1) break;

    int e2 = 2 * rerr;
    if (e2 > -dy) { rerr -= dy; rx0 += rsx; }
    if (e2 < dx)  { rerr += dx; ry0 += rsy; }
}

// final row (otherwise skipped, same issue as before)
{
    int y = rPrev.y;
    if (y > cutIn && y < (dy - cutIn)) {
        int global_y = a.y + y;
        if (global_y < 0)  global_y = 0;
        if (global_y > ly) global_y = ly - 1;

        int rightX = rPrev.x;
        int leftX  = rightX - offset;

        int xMin, xMax;
        if (k > 0) {
            xMin = std::max(a.x + leftX, 0);
            xMax = std::min(a.x + rightX, lx - 1);
        } else {
            xMin = std::max(a.x - rightX, 0);
            xMax = std::min(a.x - leftX, lx - 1);
        }

        for (int px = xMin; px <= xMax; px++) {
            this->drawPixel(px, global_y);
        }
    }
}

                

            }


            


        }


    //----- helper functions -----

    private: 



        //------- Bresenham line algo -----

        void drawLine(PointTI a, PointTI b){ 
            
            //------ variable set up ------
            int x0 = a.x;
            int y0 = a.y;
            int x1 = b.x;
            int y1 = b.y;

            int dx = abs(x1 - x0);
            int dy = abs(y1 - y0);

            int sx = (x0 < x1) ? 1 : -1;
            int sy = (y0 < y1) ? 1 : -1;

            int err = dx - dy;

            //------ line loop ----------
            while (true) {

                //------- draw our point ---------
                int x = x0;
                int y = y0;
                if(x < 0){
                    x = 0;
                } 
                else if(x > this->rasterSize.x1){
                    x = this->rasterSize.x1 - 1;
                }
                if(y < 0){
                    y = 0;
                }
                if(y > this->rasterSize.y1){
                    y = this->rasterSize.y1 - 1;
                }

                this->drawPixel(x, y);
                

                //-------take next step for line -------

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


*/

#endif 


