#ifndef mpl_brush_h
#define mpl_brush_h

#include <map>
#include <mypaint-brush.h>
#include <mplSurfaceAdapter.h>
#include <types.h>
#include <toonzGeometry.h>
#include <mypaint-brush.h>
#include <mypaint-brush-settings.h>
#include <rasterPixel.h>

#include <cmath>
    /*

    thin wrapper around mypaint brush to be compatible with our application

    */

template <class T>
class mplBrush {




    MyPaintBrush *brush; //my paintbrush storage 
    mplSurfaceAdapter<T> *surface;


    DimensionTI dim;





    public:

        //-------------------------
        //      CONSTRUCTOR    
        //-------------------------

        mplBrush(RasterP<T> ras, int i_lx, int i_ly):
        dim(DimensionTI{i_lx,i_ly})
        {   
            //----- create a surface adapter object -------
            surface = mplSurfaceAdapter_new(ras);

            //----- create a new brush object ---------
            
            brush = mypaint_brush_new();
            mypaint_brush_from_defaults(brush);
            mypaint_brush_set_base_value(brush, MYPAINT_BRUSH_SETTING_COLOR_H, 0.0);
            mypaint_brush_set_base_value(brush, MYPAINT_BRUSH_SETTING_COLOR_S, 1.0);
            mypaint_brush_set_base_value(brush, MYPAINT_BRUSH_SETTING_COLOR_V, 1.0);

            mypaint_brush_set_base_value(
    brush,
    MYPAINT_BRUSH_SETTING_DABS_PER_SECOND,
    10.0f
);

            std::cout << "dabspersecond"<<mypaint_brush_get_base_value(
    brush,
    MYPAINT_BRUSH_SETTING_DABS_PER_SECOND
) << std::endl;

        };

        //-------------------------
        //      DECONSTRUCTOR
        //-------------------------

        ~mplBrush() {
            mypaint_brush_unref(brush);
            surface = nullptr;
        }
        
        //------------------------- 
        //    DRAW BRUSH METHODS
        //-------------------------


        void drawBrush(PointTI a, PointTI b) {
            PointTF p1 = {a.x * 1.0f , a.y * 1.0f};
            PointTF p2 = {b.x * 1.0f , b.y * 1.0f};

            drawBrush(p1, p2);
        };


        void drawBrush(PointTF a, PointTF b) {

            mypaint_surface_begin_atomic((MyPaintSurface*)surface);
            std::cout<<"set surface!"<<std::endl;

            mypaint_brush_new_stroke(brush);

            
            if(a == b){

            stroke_to(brush, (MyPaintSurface*)surface, b.x, b.y);
            stroke_to(brush, (MyPaintSurface*)surface, b.x, b.y);       
            stroke_to(brush, (MyPaintSurface*)surface, b.x, b.y);



            } else {

            stroke_to(brush, (MyPaintSurface*)surface, a.x, a.y);
            stroke_to(brush, (MyPaintSurface*)surface, b.x, b.y);

            }
            
            std::cout<<"yay stroke !"<<std::endl;

            /* 
                Finalize the surface operation, passing one or more invalidation
                rectangles to get information about which areas were affected by
                the operations between ``surface_begin_atomic`` and ``surface_end_atomic.``
            */


            MyPaintRectangle roi;
            MyPaintRectangles rois;
            rois.num_rectangles = 1;
            rois.rectangles = &roi;

            mypaint_surface_end_atomic((MyPaintSurface *)surface, &rois);
            std::cout<<"yay finsihed!"<<std::endl;

        }


        void resetBrush(){
            mypaint_brush_reset(brush);
        }

        //--------------------------
        //  BRUSH ATTRIBUTE ACCESS (MYPaint Specific)
        //---------------------------

        void setBrushPreset(const char *string) {
            gboolean noErrors = mypaint_brush_from_string(brush, string);

            if(noErrors){
                std::cout << "Brush set properly" << std::endl;
            } else {
                std::cerr << "Brush not set properly" << std::endl;
            }
        };

        void setOpacity(float opacity)
        {
            setSetting(MYPAINT_BRUSH_SETTING_OPAQUE, opacity);
        }

        void setHardness(float hardness)
        {
            setSetting(MYPAINT_BRUSH_SETTING_HARDNESS, hardness);
        }

        void setSmudge(float amount)
        {
            setSetting(MYPAINT_BRUSH_SETTING_SMUDGE, amount);
        }

        void setSmudgeLength(float length)
        {
            setSetting(MYPAINT_BRUSH_SETTING_SMUDGE_LENGTH, length);
        }

        void setEraser(float amount)
        {
            setSetting(MYPAINT_BRUSH_SETTING_ERASER, amount);
        }

        void setDabsPerRadius(float value)
        {
            setSetting(MYPAINT_BRUSH_SETTING_DABS_PER_ACTUAL_RADIUS, value);
        }

        void setDabsPerSecond(float value)
        {
            setSetting(MYPAINT_BRUSH_SETTING_DABS_PER_SECOND, value);
        }

        void setOffsetByRandom(float value)
        {
            setSetting(MYPAINT_BRUSH_SETTING_OFFSET_BY_RANDOM, value);
        }

        void setRadiusByRandom(float value)
        {
            setSetting(MYPAINT_BRUSH_SETTING_RADIUS_BY_RANDOM, value);
        }

        void setEllipticalRatio(float ratio)
        {
            setSetting(MYPAINT_BRUSH_SETTING_ELLIPTICAL_DAB_RATIO, ratio);
        }

        void setEllipticalAngle(float angle)
        {
            setSetting(MYPAINT_BRUSH_SETTING_ELLIPTICAL_DAB_ANGLE, angle);
        }


        //-----------------------------
        //      BRUSHS ATTRIBUTE (our engine specific)
        //-----------------------------

        void setColor(T &color)
        {
            float r = color.r / 255.0f;
            float g = color.g / 255.0f;
            float b = color.b / 255.0f;

            float maxValue = std::max({ r, g, b });
            float minValue = std::min({ r, g, b });
            float delta = maxValue - minValue;

            float h = 0.0f;
            float s = 0.0f;
            float v = maxValue;

            // Saturation
            if (maxValue != 0.0f)
                s = delta / maxValue;

            // Hue
            if (delta != 0.0f)
            {
                if (maxValue == r)
                {
                    h = (g - b) / delta;

                    if (h < 0.0f)
                        h += 6.0f;
                }
                else if (maxValue == g)
                {
                    h = (b - r) / delta + 2.0f;
                }
                else
                {
                    h = (r - g) / delta + 4.0f;
                }

                // HSV hue: 0.0 - 1.0
                h /= 6.0f;
            }

            mypaint_brush_set_base_value(
                brush,
                MYPAINT_BRUSH_SETTING_COLOR_H,
                h
            );

            mypaint_brush_set_base_value(
                brush,
                MYPAINT_BRUSH_SETTING_COLOR_S,
                s
            );

            mypaint_brush_set_base_value(
                brush,
                MYPAINT_BRUSH_SETTING_COLOR_V,
                v
            );
        }

        void resize(float radius)
        {
            radius = std::max(radius, 0.2f);

            setSetting(
                MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC,
                std::log(radius)
            );
        }

    private:
        //-------------------------
        //     HELPERS
        //-------------------------

        //----- setting simplification -------
        void setSetting(MyPaintBrushSetting setting, float value)
        {
            mypaint_brush_set_base_value(brush, setting, value);
        }



        //----- temporary fix until we get pressure n shi fixed up --------
       void stroke_to(MyPaintBrush *brush, MyPaintSurface *surf, float x, float y)
        {
            float viewzoom = 1.0, viewrotation = 0.0, barrel_rotation = 0.0;
            float pressure = 1.0, ytilt = 0.0, xtilt = 0.0, dtime = 1.0/10;
            gboolean linear = FALSE;
            mypaint_brush_stroke_to
            (brush, surf, x, y, pressure, xtilt, ytilt, dtime, viewzoom, viewrotation, barrel_rotation, linear);
        }



};




#endif