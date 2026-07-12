#ifndef mpl_brush_h
#define mpl_brush_h

#include <map>
#include <mypaint-brush.h>
#include <mplSurfaceAdapter.h>
#include <types.h>
#include <toonzGeometry.h>
#include <mypaint-brush.h>
#include <mypaint-brush-settings.h>
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

        mplBrush(int i_lx, int i_ly, RasterP<T> ras):
        dim(DimensionTI{i_lx,i_ly})
        {   
            //----- create a surface adapter object -------
            surface = mplSurfaceAdapter_new(ras);

            //----- create a new brush object ---------
            brush = mypaint_brush_new();
            mypaint_brush_from_defaults(brush);
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


        void drawbrush(int x0, int y0, int x1, int y1) {
            mypaint_surface_begin_atomic((MyPaintSurface*)surface);

            stroke_to(brush, (MyPaintSurface*)surface, x0, y0);
            stroke_to(brush, (MyPaintSurface*)surface, x1, y1);

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
        };

        //--------------------------
        //  BRUSH ATTRIBUTE ACCESS
        //---------------------------

        void setBrushPreset(const char *string) {
            gboolean noErrors = mypaint_brush_from_string(brush, string);

            if(noErrors){
                std::cout << "Brush set properly" << std::endl;
            } else {
                std::cerr << "Brush not set properly" << std::endl;
            }
        };


        void setRadius(float radius)
        {
            setSetting(MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC, radius);
        }

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
            setSetting(MYPAINT_BRUSH_SETTING_DABS_PER_RADIUS, value);
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

        void setColorHSV(float h, float s, float v)
        {
            setSetting(MYPAINT_BRUSH_SETTING_COLOR_H, h);
            setSetting(MYPAINT_BRUSH_SETTING_COLOR_S, s);
            setSetting(MYPAINT_BRUSH_SETTING_COLOR_V, v);
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
            gboolean linear = FALSE
            mypaint_brush_stroke_to
            (brush, surf, x, y, pressure, xtilt, ytilt, dtime, viewzoom, viewrotation, barrel_rotation, linear);
        }



};




#endif