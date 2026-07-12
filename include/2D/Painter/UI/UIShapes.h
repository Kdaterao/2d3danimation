
#include <types.h>
#include <toonzGeometry.h>
#include <toonzRasterPixel.h>


#include <vector>




    class VertexRGBM32 {
        public:
            float x;
            float y;

            float r;
            float g;
            float b;
            float m; 

        VertexRGBM32(float i_x, float i_y, float i_r, float i_g, float i_b, float i_m)
            : x(i_x),
              y(i_y),
              r(i_r),
              g(i_g),
              b(i_b),
              m(i_m) {}

    };

    
    //----- abstract Shape class -----
    class Shape {
        protected:
        Shape() {};
        virtual ~Shape() {}


        virtual inline  std::vector<float> getVertices() = 0; //enforce this function name!
    };


    //----- Actual Shapes ------

    class Box: public Shape{

        public:

            // level of vertices on UI
            float level;

            // vertices
            VertexRGBM32 tl; // top-left
            VertexRGBM32 tr; // top-right
            VertexRGBM32 bl; // bottom-left
            VertexRGBM32 br; // bottom-right


            Box(VertexRGBM32 i_tl, VertexRGBM32 i_tr,  VertexRGBM32 i_bl, VertexRGBM32 i_br)
                : tl(i_tl),
                tr(i_tr),
                bl(i_bl),
                br(i_br) {};
                

            std::vector<float> getVertices() override {
                return {
                    // position          // color (r,g,b,m)
                    tr.x, tr.y, level, tr.r, tr.g, tr.b, tr.m, // top-right
                    br.x, br.y, level, br.r, br.g, br.b, br.m, // bottom-right
                    tl.x, tl.y, level, tl.r, tl.g, tl.b, tl.m, // top-left

                    br.x, br.y, level, br.r, br.g, br.b, br.m, // bottom-right
                    bl.x, bl.y, level, bl.r, bl.g, bl.b, bl.m, // bottom-left
                    tl.x, tl.y, level, tl.r, tl.g, tl.b, tl.m  // top-left
                };
            }
    };




    struct Triangle : public Shape {

        public:
            // level of vertices on UI
            float level;

            // vertices
            VertexRGBM32 tm; // top-middle
            VertexRGBM32 bl; // bottom-left
            VertexRGBM32 br; // bottom-right



            Triangle(VertexRGBM32 i_tm,   VertexRGBM32 i_bl, VertexRGBM32 i_br)
                : tm(i_tm),
                  bl(i_bl),
                  br(i_br) {};

            std::vector<float> getVertices() {
                return {
                    // position          // color (r,g,b,m)
                    tm.x, tm.y, level, tm.r, tm.g, tm.b, tm.m, // top-middle
                    br.x, br.y, level, br.r, br.g, br.b, br.m, // bottom-right
                    bl.x, bl.y, level, bl.r, bl.g, bl.b, bl.m, // top-left
                };
            }
    };

