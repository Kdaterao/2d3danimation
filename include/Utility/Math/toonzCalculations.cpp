
#include <toonzCalculations.h>


//============================
//      BEZIER CURVES
//=============================


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



std::vector<PointTF> toonzCalculate::QuadraticBezierCurveFloat(PointTI a, PointTF b, PointTI c, float step) {



    std::vector<PointTF> res;


    float x; //initial point 
    float y; //initial point
    
    for(float t = 0; t < 1; t += step){
        x = (1-t)*(1-t)*a.x + 2*(1-t)*t*b.x + t*t*c.x;
        y = (1-t)*(1-t)*a.y + 2*(1-t)*t*b.y + t*t*c.y;
        res.push_back(PointTF(x, y)); 
    }

    res.push_back(PointTF(c.x*1.0f, c.y*1.0f));//makes sure last point alway on the curve!

    return res;
}




RectTI toonzCalculate::QuadraticBezierBounds(PointTI a, PointTF b, PointTI c) {
    auto extrema = [](float a, float b, float c) -> float {
        float denom = a - 2*b + c;
        if (abs(denom) < 1e-6f) return -1.f; // no extrema, linear
        return (a - b) / denom;
    };

    auto eval = [&](float t, auto ax, auto bx, auto cx) {
        return (1-t)*(1-t)*ax + 2*(1-t)*t*bx + t*t*cx;
    };

    float tx = extrema(a.x, b.x, c.x);
    float ty = extrema(a.y, b.y, c.y);

    float minX = std::min({(float)a.x, (float)c.x});
    float maxX = std::max({(float)a.x, (float)c.x});
    float minY = std::min({(float)a.y, (float)c.y});
    float maxY = std::max({(float)a.y, (float)c.y});

    if (tx >= 0.f && tx <= 1.f) {
        float x = eval(tx, a.x, b.x, c.x);
        minX = std::min(minX, x);
        maxX = std::max(maxX, x);
    }

    if (ty >= 0.f && ty <= 1.f) {
        float y = eval(ty, a.y, b.y, c.y);
        minY = std::min(minY, y);
        maxY = std::max(maxY, y);
    }

    return RectTI{(int)minX, (int)minY, (int)maxX, (int)maxY };
}



//============================
//      DISTANCE
//=============================

float toonzCalculate::Distance(PointTI a, PointTI  b){
    
    int A2 = (a.x - b.x) * (a.x - b.x);
    int B2 = (a.y - b.y) * (a.y - b.y);

    return (float)std::sqrt(A2 + B2);

}

float toonzCalculate::Distance(PointTF a, PointTF b){

        float A2 = (a.x - b.x) * (a.x - b.x);
        float B2 = (a.y - b.y) * (a.y - b.y);

        return std::sqrt(A2 + B2);
        
}