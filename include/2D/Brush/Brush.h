#ifndef TOONZBRUSHTYPES_H
#define TOONZBRUSHTYPES_H

#include <toonzRasterBrush.h>
#include <toonzRasterCircleBrush.h>
#include <variant>
//#include <mypaint-brush.h>



namespace Brush {
    

    //===============================================
    //  enum + variant structure for brush toggle 
    //===============================================

    enum class RasterTypes{

        NONE,
        BRUSH_BGRM32,
        //MPL_BRUSH

    };

    using BrushVariant = std::variant<
            DefaultCircleBrush<ToonzPixelBGRM32>
            //,MyPaintBrush
        >;




    //===============================================
    //            Brush Definitions  
    //===============================================

    struct RasterBrush{


        std::unique_ptr<BrushVariant> curr;



        void drawBrush(PointTI a, PointTI b) {
            std::visit([&](auto& bsh) {
                bsh.drawBrush(a, b);
            }, *curr);
        }


        void drawBrush(PointTF a, PointTF b) {
            std::visit([&](auto& bsh) {
                bsh.drawBrush(a, b);
            }, *curr);
        }


        template<class T>

        void setColor(T color) {
            std::visit([&](auto& bsh) {
                bsh.setColor(color);
            }, *curr);
        }


        void setSize(int r) {
            std::visit([&](auto& bsh) {
                bsh.resize(r);
            }, *curr);
        }
        
        
    };      


    //===============================================
    //            Brush Toggle/Switch 
    //===============================================
    

    template<class T>

    void setBrush(Brush::RasterTypes newType, Brush::RasterBrush &brush, ToonzRasterPT<T> image, T color, int size){

        switch(newType){

        case RasterTypes::NONE:

            break;

        case RasterTypes::BRUSH_BGRM32:
             {
                DefaultCircleBrush<ToonzPixelBGRM32> newBrush = DefaultCircleBrush<ToonzPixelBGRM32>(image, color , size);
                if(brush.curr){
                    *brush.curr = std::move(newBrush); // replaces with our new brush 
                } else {
                    brush.curr = std::make_unique<BrushVariant>(DefaultCircleBrush<ToonzPixelBGRM32>(image, color, size)); // makes a smart, unique pointer 
                }
                break;
            }

            /*
        case RasterTypes::MPL_BRUSH:
            {
                auto newBrush =  MyPaintBrush(mypaint_brush_new());

                if(brush.curr){
                    brush.curr = newBrush;
                } else {
                    brush.curr = std::make_unique<BrushVariant>(*newBrush); // makes a smart, unique pointer 
                }

                break;
            }
        

            */
            

     };
    
    }

};


#endif