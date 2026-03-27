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

                return (y1- y0);
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
    /*Moved to tgeometry.cpp
    {
    a11 = a.a11; a12 = a.a12; a13 = a.a13;
    a21 = a.a21; a22 = a.a22; a23 = a.a23;
    return *this;
    };
    */
    /*!
            Matrix multiplication.
            <p>\f$\left(\begin{array}{cc}\bf{A}&\vec{a}\\\vec{0}&1\end{array}\right)
            \left(\begin{array}{cc}\bf{B}&\vec{b}\\\vec{0}&1\end{array}\right)\f$</p>

    */

    TAffine operator*(const TAffine &b) const;

    /*Moved to in tgeometry.cpp
    {
    return TAffine (
    a11 * b.a11 + a12 * b.a21,
    a11 * b.a12 + a12 * b.a22,
    a11 * b.a13 + a12 * b.a23 + a13,

    a21 * b.a11 + a22 * b.a21,
    a21 * b.a12 + a22 * b.a22,
    a21 * b.a13 + a22 * b.a23 + a23);
    };
    */


    TAffine operator*=(const TAffine &b);
    
    /*Moved to tgeometry.cpp
    {
    return *this = *this * b;
    };
    */
    /*!
            Returns the inverse tansformation as:
            <p>\f$\left(\begin{array}{ccc}\bf{A}^{-1}&-\bf{A}^{-1}&\vec{b}\\\vec{0}&\vec{0}&1\end{array}\right)\f$</p>
    */


    /*!
          Returns \e true if all elements are equals.
    */
    bool operator==(const TAffine &a) const;
    /*Sposto in tgeometry.cpp
    {
    return a11==a.a11 && a12==a.a12 && a13==a.a13 &&
    a21==a.a21 && a22==a.a22 && a23==a.a23;
    };
    */

    /*!
          Returns \e true if at least one element is different.
    */

    bool operator!=(const TAffine &a) const;
    /*Sposto in tgeometry.cpp
    {
    return a11!=a.a11 || a12!=a.a12 || a13!=a.a13 ||
    a21!=a.a21 || a22!=a.a22 || a23!=a.a23;
    };
    */

    /*!
        Returns the transformed point.
    */

    PointTD operator*(const PointTD &p) const;
    /*Sposto in tgeometry.cpp
    {
    return PointTD(p.x*a11+p.y*a12+a13, p.x*a21+p.y*a22+a23);
    };
    */

    
    /*
    RectTD operator*(const RectTD &rect) const;
    /*
            Returns a translated matrix that change the vector (u,v) in (x,y).
    \n	It returns a matrix of the form:
            <p>\f$\left(\begin{array}{ccc}\bf{A}&\vec{x}-\bf{A} \vec{u}\\
            \vec{0}&1\end{array}\right)\f$</p>
    */


    //----- functions ------
    TAffine inv() const;
    /*Moved to tgeometry.cpp
    {
    if(a12 == 0.0 && a21 == 0.0)
    {
    assert(a11 != 0.0);
    assert(a22 != 0.0);
    double inv_a11 = 1.0/a11;
    double inv_a22 = 1.0/a22;
    return TAffine(inv_a11,0, -a13 * inv_a11,
    0,inv_a22, -a23 * inv_a22);
    }
    else if(a11 == 0.0 && a22 == 0.0)
    {
    assert(a12 != 0.0);
    assert(a21 != 0.0);
    double inv_a21 = 1.0/a21;
    double inv_a12 = 1.0/a12;
    return TAffine(0, inv_a21, -a23 * inv_a21,
    inv_a12, 0, -a13 * inv_a12);
    }
    else
    {
    double d = 1./det();
    return TAffine(a22*d,-a12*d, (a12*a23-a22*a13)*d,
    -a21*d, a11*d, (a21*a13-a11*a23)*d);
    }
    };
    */

    double det() const;
    /*Sposto in tgeometry.cpp{
    return a11*a22-a12*a21;
    };
    */

    /*!
            Returns \e true if all elements are equals.
    */


    /*!
            Returns \e true if the transformation is an identity,
            i.e in the error limit \e err leaves the vectors unchanged.
    */

    bool isIdentity(double err = 1.e-8) const;
    /*Sposto in tgeometry.cpp
    {
    return ((a11-1.0)*(a11-1.0)+(a22-1.0)*(a22-1.0)+
    a12*a12+a13*a13+a21*a21+a23*a23) < err;
    };
    */
    /*!
            Returns \e true if in the error limits \e err \f$\bf{A}\f$ is the
        identity matrix.
    */

    bool isZero(double err = 1.e-8) const;

    bool isTranslation(double err = 1.e-8) const;
    /*Sposto in tgeometry.cpp
    {
    return ((a11-1.0)*(a11-1.0)+(a22-1.0)*(a22-1.0)+
    a12*a12+a21*a21) < err;
    };
    */
    /*!
            Returns \e true if in the error limits the matrix \f$\bf{A}\f$ is of
        the form:
            <p>\f$\left(\begin{array}{cc}a&b\\-b&a\end{array}\right)\f$</p>.
    */

    bool isIsotropic(double err = 1.e-8) const;
    /*Sposto in tgeometry.cpp
    {
        return areAlmostEqual(a11, a22, err) && areAlmostEqual(a12, -a21, err);
    };
    */



    /*!
            Transform point without translation
    */
    PointTD transformDirection(const PointTD &p) const;

    /*!
            Retruns the transformed box of the bounding box.
    */

    TAffine place(double u, double v, double x, double y) const;

    /*!
            See above.
    */
    TAffine place(const PointTD &pIn, const PointTD &pOut) const;

    inline static TAffine identity()
        { return TAffine(); }
    inline static TAffine zero()
        { return TAffine(0, 0, 0, 0, 0, 0); }

    inline static TAffine translation(double x, double y)
        { return TAffine(1, 0, x, 0, 1, y); }

        /*
    inline static TAffine translation(const PointTD &p)
        { return translation(p.x, p.y); }
         */

    inline static TAffine scale(double sx, double sy)
        { return TAffine(sx, 0, 0, 0, sy, 0); }
    inline static TAffine scale(double s)
        { return scale(s, s); }

        /*
    inline static TAffine scale(const PointTD &center, double sx, double sy)
        { return translation(center)*scale(sx, sy)*translation(-center); }
    inline static TAffine scale(const PointTD &center, double s)
        { return scale(center, s, s); }

    static TAffine rotation(double angle);
    inline static TAffine rotation(const PointTD &center, double angle)
        { return translation(center)*rotation(angle)*translation(-center); }
        */


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