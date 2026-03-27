#ifndef TOONZGL_H
#define TOONZGL_H


#include <toonzGeometry.h>
#include <QOpenGLFunctions>


namespace toonzGL{


    inline void tglMultMatrix(const TAffine aff){
        
    GLdouble m[] = {aff.a11, aff.a21, 0, 0, 
                    aff.a12, aff.a22, 0, 0,
                        0,       0,   1, 0, 
                    aff.a13, aff.a23, 0, 1};


        glMultMatrixd(m);

    }




};



#endif