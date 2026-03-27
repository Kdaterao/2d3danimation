#include <gtest/gtest.h>
#include <QMouseEvent>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QTimer>
#include <types.h>
#include <toonzRasterPixel.h>
#include <toonzRaster.h>
#include <toonzRasterBrush.h>
#include <toonzPainterGL.h>
#include <iostream>



 void Circle(int r, std::vector<PointTI> &circle){ 

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
            circle.push_back(PointT(x, y));
            circle.push_back(PointT(x, -y));
            circle.push_back(PointT(-x, y));
            circle.push_back(PointT(-x, -y));
            circle.push_back(PointT(y, -x));
            circle.push_back(PointT(y, x));
            circle.push_back(PointT(-y, -x));
            circle.push_back(PointT(-y, x));

            //----- next block -----
            
            x += 1;

            }


}


//----- builds extreme pair values ------
void buildPairs(int x,  int y, std::unordered_map<int, std::array<int, 2>> &fillPairs){

            //----- case 1: row has element ----
            if (fillPairs.find(y) != fillPairs.end()){
                std::array<int, 2> &row = fillPairs[y]; 
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
                fillPairs[y] = {x, INT_MIN};
            }

}



void drawLine(PointT<int> a, PointT<int> b, std::vector<PointTI> &line){  
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
                line.push_back(PointT(x0, y0));
                std::cout<<"linedrawn:"<<"("<<x0<<","<<y0<<")"<<std::endl;
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


// Demonstrate some basic assertions.
TEST(PixelTest, sizeofpixel) {


  ToonzPixelBGRM32 T = ToonzPixelBGRM32();
  EXPECT_EQ(4, sizeof(T));
}


//from print statemetns we need to handle negative cirlce values 
TEST(BrushTest, halfcircle){

  std::vector<PointTI> circle = {};
  std::vector<PointTI> line = {};
  std::unordered_map<int, std::array<int, 2>> fillpairs = {};
  Circle(10, circle);


  for(const auto& point : circle){
      int x0 = 90;
      int y0 = 90;
      int x1 = 100;
      int y1 = 100;
      std::cout<<"Circledrawn:"<<"("<<point.x <<","<<point.y  <<")"<<std::endl;
      std::cout<<"Circledrawn:"<<"("<<point.x <<","<<point.y <<")"<<std::endl;
      buildPairs(point.x + x0, point.y +y0, fillpairs);
      buildPairs(point.x + x1, point.y +y1, fillpairs);

  }

  

  for(const auto& row : fillpairs){
            const int y = row.first;
            const int min = row.second[0];
            const int max = row.second[1];

            for(int x = min; x <= max; x++ ){
                std::cout<<"("<<x<<","<<y<<")"<<",";
            }
            std::cout<<std::endl;
  }


  
    
}
