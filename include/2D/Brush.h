#ifndef TOONZBRUSHTYPES_H
#define TOONZBRUSHTYPES_H

#include <toonzRasterBrush.h>
#include <variant>



namespace Brush {

    //------ RasterBrush: enum + variant structure ------

    enum class RasterTypes{

        NONE,
        BRUSH_BGRM32

    };

    using BrushVariant = std::variant<
            DefaultCircleBrush<ToonzPixelBGRM32>
        >;
    //------- RasterBrush  ------

    struct RasterBrush{


        std::unique_ptr<BrushVariant> curr;



        void drawBrush(PointTI a, PointTI b) {
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

    //------ set Brush function ------

    template<class T>

    void setBrush(Brush::RasterTypes newType, Brush::RasterBrush &brush, ToonzRasterPT<T> image, T color, int size){

        switch(newType){

        case RasterTypes::NONE:

            break;

        case RasterTypes::BRUSH_BGRM32:
            
        
            DefaultCircleBrush<ToonzPixelBGRM32> newBrush = DefaultCircleBrush<ToonzPixelBGRM32>(image, color , size);
            if(brush.curr){
                *brush.curr = std::move(newBrush);
            } else {
                brush.curr = std::make_unique<BrushVariant>(DefaultCircleBrush<ToonzPixelBGRM32>(image, color, size));
            }
            break;

     };
    
    }

};


#endif