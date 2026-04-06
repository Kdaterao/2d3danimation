
#include <toonzCalculations.h>




std::vector<PointTI> toonzCalculate::QuadraticBezierCurve(PointTI a, PointTI b, PointTI c, float step) {




    std::vector<PointTI> res;


    float x; //initial point 
    float y; //initial point

    for(float t = 0; t <= 1; t += step){
        x = (1-t)*(1-t)*a.x + 2*(1-t)*t*b.x + t*t*c.x;
        y = (1-t)*(1-t)*a.y + 2*(1-t)*t*b.y + t*t*c.y;
        res.push_back(PointTI(int(x), int(y))); 
        
    }

    res.push_back(c);//makes sure last point alway on the curve!


    return res;
}




std::vector<PointTI> toonzCalculate::QuadraticBezierCurve(PointTI a, PointTF b, PointTI c, float step) {



    std::vector<PointTI> res;


    float x; //initial point 
    float y; //initial point
    
    for(float t = 0; t < 1; t += step){
        x = (1-t)*(1-t)*a.x + 2*(1-t)*t*b.x + t*t*c.x;
        y = (1-t)*(1-t)*a.y + 2*(1-t)*t*b.y + t*t*c.y;
        res.push_back(PointTI(int(x), int(y))); 
    }

    res.push_back(c);//makes sure last point alway on the curve!

    return res;
}