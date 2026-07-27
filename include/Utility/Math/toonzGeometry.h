#ifndef TOONZGEOMETRY_H
#define TOONZGEOMETRY_H

#include <iostream>


//----- typedefs ------

template<typename T>

class DimensionT;

template<typename T>

class PointT;

template<typename T>

class RectT;


class TAffine;

typedef PointT<double> PointTD;
typedef PointT<int> PointTI;
typedef PointT<float> PointTF;
typedef  RectT<double> RectTD;
typedef RectT<int> RectTI;
typedef DimensionT<int> DimensionTI;


//----- classes -----



//dimension class which takes in two 
template<typename T>
class DimensionT {
    public:
        //------ variables -------
        T lx;
        T ly; 

        //------ constructors ------
        DimensionT() : lx(), ly() {}

        DimensionT(T i_lx, T i_ly) : lx(i_lx), ly(i_ly) {}


        //----- utility function -----


        //--- Operator overload functions ---
        bool operator==(const DimensionT& other) const {
        return lx == other.lx && ly == other.ly;
        }

        bool operator!=(const DimensionT& other) const {
        return !(*this == other);
        }


    };



template<typename T>

//class which can hold and do transformation operations on coordinates 
class PointT {


    public: 
        //------ variables -------
        T x;
        T y;

        //------ constructors ------
        PointT() 
            : x(0), 
              y(0) {};


        PointT(T i_x, T i_y) 
            : x(i_x), 
              y(i_y) {};
        //------ operators -------

        float operator*(PointT<T>& a) const{

            return float((x * a.x) + (y *a.y));
        }


        bool operator==(PointT<T>& a) const{

            return (a.x == x && a.y == y);

        }

};



template <typename T>

class RectT {


    public:
        //------ variables -------
        T x0, y0;
        T x1, y1;

        //----- constructors ------ 
        RectT() {};
        RectT(T i_x0, T i_y0, T i_x1, T i_y1) 
            : x0(i_x0), 
              y0(i_y0), 
              x1(i_x1), 
              y1(i_y1) {};



        RectT(const RectT &rect) 
            : x0(rect.x0), 
              y0(rect.y0), 
              x1(rect.x1), 
              y1(rect.y1){};



        //----- methods -----
        inline bool overlaps(const RectT<T> &b) const { 
            return x0 <= b.x1 && x1 >= b.x0 && y0 <= b.y1 && y1 >= b.y0;
        };

        inline T getLx() const {
            if(x1 > x0){

                return (x1- x0);
            } else {
                return (x0 - x1);
            }
        };

        inline T getLy() const {
            if(y1 > y0){

                return (y1 - y0);
            } else {
                return (y0 - y1);
            }
        };

        inline PointT<T> const getP00() {return PointT<T>(x0, y0);};
        inline PointT<T> const getP01() {return PointT<T>(x0, y1);};
        inline PointT<T> const getP10() {return PointT<T>(x1, y0);};
        inline PointT<T> const getP11() {return PointT<T>(x1, y0);};

        inline DimensionT<T> getSize() const { return DimensionT<T>(getLx(), getLy()); };
};




//=============================================================================
//! This is the base class for the affine transformations.(very important to use this format incase we wanna do special transofrmations to a given patch of pixels(lasso tool!))
/*!
 This class performs basic manipulations of affine transformations.
 An affine transformation is a linear transformation followed by a translation.
 
  [a11, a12, a13]
  [a21, a22, a23]

  a13 and a23 represent translation (moving sideways or up and down)
  the other 4 handle rotation, scale and shear
*/

class TAffine {
public:

    //----- variables -----
    double a11, a12, a13;
    double a21, a22, a23;

    //----- constructors -----
    TAffine() 
        : a11(1.0), 
        a12(0.0), 
        a13(0.0), 
        a21(0.0), 
        a22(1.0), 
        a23(0.0){};

    TAffine(double p11, double p12, double p13, double p21, double p22, double p23)
        : a11(p11), 
            a12(p12), 
            a13(p13), 
            a21(p21), 
            a22(p22), 
            a23(p23){};

    TAffine(const TAffine &a)
        : a11(a.a11)
        , a12(a.a12)
        , a13(a.a13)
        , a21(a.a21)
        , a22(a.a22)
        , a23(a.a23){};



    //----- Operator functions -----

    friend std::ostream& operator<<(std::ostream& os, const TAffine& t) {
        os << "[" << t.a11 << "," << t.a12 << "," << t.a13 << "]\n"
        << "[" << t.a21 << "," << t.a22 << "," << t.a23 << "]";
        return os;
    }

    TAffine &operator=(const TAffine &a);


    TAffine operator*(const TAffine &b) const;



    TAffine operator*=(const TAffine &b);

    bool operator==(const TAffine &a) const;


    bool operator!=(const TAffine &a) const;

    PointTD operator*(const PointTD &p) const;

    
    

    //----- functions ------
    TAffine inv() const;


    double det() const;


    bool isIdentity(double err = 1.e-8) const;


    bool isZero(double err = 1.e-8) const;

    bool isTranslation(double err = 1.e-8) const;


    bool isIsotropic(double err = 1.e-8) const;



    PointTD transformDirection(const PointTD &p) const;


    TAffine place(double u, double v, double x, double y) const;

    TAffine place(const PointTD &pIn, const PointTD &pOut) const;

    inline static TAffine identity()
        { return TAffine(); }
    inline static TAffine zero()
        { return TAffine(0, 0, 0, 0, 0, 0); }

    inline static TAffine translation(double x, double y)
        { return TAffine(1, 0, x, 0, 1, y); }


    inline static TAffine scale(double sx, double sy)
        { return TAffine(sx, 0, 0, 0, sy, 0); }
    inline static TAffine scale(double s)
        { return scale(s, s); }




    inline static TAffine shear(double sx, double sy)
        { return TAffine(1, sx, 0, sy, 1, 0); }
};


class TTranslation final : public TAffine {
public:
  TTranslation(){};
  TTranslation(double x, double y) : TAffine(1, 0, x, 0, 1, y){};
  TTranslation(const PointTD &p) : TAffine(1, 0, p.x, 0, 1, p.y){};

};




#endif