#ifndef TOONZBRUSHTYPES_H
#define TOONZBRUSHTYPES_H

#include <toonzRasterBrush.h>
#include <toonzRasterCircleBrush.h>
#include <variant>
#include <raster.h>
#include <mplBrush.h>
#include <pipelineLogger.h>



namespace Brush {
    

    //===============================================
    //  enum + variant structure for brush toggle 
    //===============================================

    enum class RasterTypes{

        NONE,
        BRUSH_BGRM32,
        MPL_BRUSH

    };

    template <class T>
    using BrushVariant = std::variant<
            DefaultCircleBrush<T>,
            mplBrush<T>
        >;




    //===============================================
    //            Brush Definitions  
    //===============================================

    template <class T>

    struct RasterBrush{


        std::unique_ptr<BrushVariant<T>> curr;



        void drawBrush(PointTI a, PointTI b) {
            //---- debug ------
            PipelineScope _draw(PipelineStage::DrawBrush); 
            PipelineLogger::instance().addSegment(
                static_cast<float>(a.x), static_cast<float>(a.y),
                static_cast<float>(b.x), static_cast<float>(b.y));
            //---- actaul logic -----
            std::visit([&](auto& bsh) {
                bsh.drawBrush(a, b);
            }, *curr);
        }


        void drawBrush(PointTF a, PointTF b) {
            //---- debug ------
            PipelineScope _draw(PipelineStage::DrawBrush); // debug
            PipelineLogger::instance().addSegment(a.x, a.y, b.x, b.y);
            //---- actaul logic -----
            std::visit([&](auto& bsh) {
                bsh.drawBrush(a, b);
            }, *curr);
        }



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

        void setEraserMode(bool enabled) {
            std::visit([&](auto& bsh) {
                bsh.toggleEraser(enabled);
            }, *curr);
        }

        void toggleEraser(bool enabled) {
            setEraserMode(enabled);
        }

        void resetBrush() {
            std::visit([&](auto& bsh) {
                bsh.resetBrush();
            }, *curr);
        }

        void startBrush() {
            std::visit([&](auto& bsh) {
                bsh.startBrush();
            }, *curr);
        }
        
        
    };      


    //===============================================
    //            Brush Toggle/Switch 
    //===============================================
    


    template<class T>
    void setBrush( Brush::RasterTypes newType, auto &brush, Raster<T>* image, T color,int size ) {
        brush.curr.reset();

        switch (newType) {

        case RasterTypes::NONE:
            break;

        case RasterTypes::BRUSH_BGRM32:
            brush.curr =
                std::make_unique<BrushVariant<T>>(
                    std::in_place_type<DefaultCircleBrush<T>>, // we must create brush inplace instead of bopying into our variant since the class hold a pointer to brhs!(deleting old would leave dangling pointer )
                    image,
                    color,
                    size
                );
            break;

        case RasterTypes::MPL_BRUSH:
            brush.curr =
                std::make_unique<BrushVariant<T>>(
                    std::in_place_type<mplBrush<T>>,
                    image,
                    size,
                    size
                );
            break;
        }
    }
};


#endif