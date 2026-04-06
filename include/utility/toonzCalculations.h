#ifndef TOONZCALCULATIONS_H
#define TOONZCALCULATIONS_H

#include <toonzGeometry.h>


namespace toonzCalculate{




    std::vector<PointTI> QuadraticBezierCurve(PointTI a, PointTI b, PointTI c, float step);


    std::vector<PointTI> QuadraticBezierCurve(PointTI a, PointTF b, PointTI c, float step);




};



#endif 



