#ifndef RASTERPIXEL_H
#define RASTERPIXEL_H
#include <types.h>

//----- forward declarations -----

//! b,g,r,m 4 byte/pixel (this order since opengl macos)
class PixelBGRM32;
class PixelBGRM64;






//---------------------------------
//          BGRM
//---------------------------------


//----- 32 bit pixel class -----

class PixelBGRM32 { 

    public: 
        //----- variables -----

        static const int maxChannelValue = 0xff; //8 bit integer in hexadecimal (study up on this lol)
        typedef unsigned char Channel;

        //--- pixel order (it matters because we temporarily turn our class into a 32 bit unsigned integer when inputting mask) ---

        unsigned char b, g, r, m;  //common for opengl and macos!
        


        //----- constructors -----
        
        PixelBGRM32() 
            : b(0), 
              g(0), 
              r(0), 
              m(maxChannelValue){};

        PixelBGRM32(const PixelBGRM32 &pix)
            : b(pix.b), 
              g(pix.g), 
              r(pix.r), 
              m(pix.m){};


        PixelBGRM32(int i_r, int i_g, int i_b, int i_m = maxChannelValue)
            : b(i_b), 
              g(i_g), 
              r(i_r), 
              m(i_m){};

        

        // "*(UINT32 *)this "  treats our bgrm variables as a single  32 bit integer, allowing for a shortcut to quickly input mask varaibles via an equals sign --> doesnt change class or anything just an input trick!
        //notes: we do constructor delegation beforehand allowing for us to access the object inside the constructor!!!
        PixelBGRM32(UINT32 mask) 
            : PixelBGRM32() { *(UINT32 *)this = mask; }; 

            
        //----- operator methods -----

        inline bool operator==(const PixelBGRM32 &p) const {
                        return *(const UINT32 *)this == *(const UINT32 *)&p;
                    }
        inline bool operator!=(const PixelBGRM32 &p) const {
                        return *(const UINT32 *)this != *(const UINT32 *)&p;
                    }

        inline bool operator<(const PixelBGRM32 &p) const {
                        return *(const UINT32 *)this < *(const UINT32 *)&p;
                    }

        inline bool operator>=(const PixelBGRM32 &p) const {
                        return *(const UINT32 *)this >= *(const UINT32 *)&p;
                    }

        inline bool operator>(const PixelBGRM32 &p) const {
                        return *(const UINT32 *)this > *(const UINT32 *)&p;
                    }
        inline bool operator<=(const PixelBGRM32 &p) const {
                        return *(const UINT32 *)this <= *(const UINT32 *)&p;
                    }

        //----- default colors -----


        static const PixelBGRM32 Red;
        static const PixelBGRM32 Green;
        static const PixelBGRM32 Blue;
        static const PixelBGRM32 Yellow;
        static const PixelBGRM32 Cyan;
        static const PixelBGRM32 Magenta;
        static const PixelBGRM32 Gray;
        static const PixelBGRM32 White;
        static const PixelBGRM32 Black;
        static const PixelBGRM32 Transparent;
                    
        
};



//----- 64 bit pixel class -----

class PixelBGRM64 { 

    public: 
        //----- variables -----

        static const int maxChannelValue = 0xffff; //16 bit integer in hexadecimal
        typedef UINT16 Channel;

        //--- pixel order (it matters because we temporarily turn our class into a 32 bit unsigned integer when inputting mask) ---

        UINT16 b, g, r, m;  //common for opengl and macos!
        


        //----- constructors -----
        
        PixelBGRM64() 
            : b(0), 
              g(0), 
              r(0), 
              m(maxChannelValue){};

        PixelBGRM64(const PixelBGRM64 &pix)
            : b(pix.b), 
              g(pix.g), 
              r(pix.r), 
              m(pix.m){};


        PixelBGRM64(int i_r, int i_g, int i_b, int i_m = maxChannelValue)
            : b(i_b), 
              g(i_g), 
              r(i_r), 
              m(i_m){};

        

        // "*(UINT32 *)this "  treats our bgrm variables as a single  32 bit integer, allowing for a shortcut to quickly input mask varaibles via an equals sign --> doesnt change class or anything just an input trick!
        //notes: we do constructor delegation beforehand allowing for us to access the object inside the constructor!!!
        PixelBGRM64(UINT64 mask) 
            : PixelBGRM64() { *(UINT64 *)this = mask; }; 

            
        //----- operator methods -----

        inline bool operator==(const PixelBGRM64 &p) const {
                        return *(const UINT64 *)this == *(const UINT64 *)&p;
                    }
        inline bool operator!=(const PixelBGRM64 &p) const {
                        return *(const UINT64 *)this != *(const UINT64 *)&p;
                    }

        inline bool operator<(const PixelBGRM64 &p) const {
                        return *(const UINT64 *)this < *(const UINT64 *)&p;
                    }

        inline bool operator>=(const PixelBGRM64 &p) const {
                        return *(const UINT64 *)this >= *(const UINT64 *)&p;
                    }

        inline bool operator>(const PixelBGRM64 &p) const {
                        return *(const UINT64 *)this > *(const UINT64 *)&p;
                    }
        inline bool operator<=(const PixelBGRM64 &p) const {
                        return *(const UINT64 *)this <= *(const UINT64 *)&p;
                    }

        //----- default colors -----
   
};




//---------------------------------
//          RGBM
//---------------------------------

//-------- 32 bits ---------------

class PixelRGBM32 { 

    public: 
        //----- variables -----

        static const int maxChannelValue = 0xffff; //16 bit integer in hexadecimal
        typedef UINT16 Channel;

        //--- pixel order (it matters because we temporarily turn our class into a 32 bit unsigned integer when inputting mask) ---

        UCHAR r, g, b, m;  //common for opengl and macos!
        


        //----- constructors -----
        
        PixelRGBM32() 
            : b(0), 
              g(0), 
              r(0), 
              m(maxChannelValue){};

        PixelRGBM32(const PixelRGBM32 &pix)
            : b(pix.b), 
              g(pix.g), 
              r(pix.r), 
              m(pix.m){};


        PixelRGBM32(int i_r, int i_g, int i_b, int i_m = maxChannelValue)
            : b(i_b), 
              g(i_g), 
              r(i_r), 
              m(i_m){};

        

        // "*(UINT32 *)this "  treats our bgrm variables as a single  32 bit integer, allowing for a shortcut to quickly input mask varaibles via an equals sign --> doesnt change class or anything just an input trick!
        //notes: we do constructor delegation beforehand allowing for us to access the object inside the constructor!!!
        PixelRGBM32(UINT32 mask) 
            : PixelRGBM32() { *(UINT32 *)this = mask; }; 

            
        //----- operator methods -----

        inline bool operator==(const PixelRGBM32 &p) const {
                        return *(const UINT32 *)this == *(const UINT32 *)&p;
                    }
        inline bool operator!=(const PixelRGBM32 &p) const {
                        return *(const UINT32 *)this != *(const UINT32 *)&p;
                    }

        inline bool operator<(const PixelRGBM32 &p) const {
                        return *(const UINT32 *)this < *(const UINT32 *)&p;
                    }

        inline bool operator>=(const PixelRGBM32 &p) const {
                        return *(const UINT32 *)this >= *(const UINT32 *)&p;
                    }

        inline bool operator>(const PixelRGBM32 &p) const {
                        return *(const UINT32 *)this > *(const UINT32 *)&p;
                    }
        inline bool operator<=(const PixelRGBM32 &p) const {
                        return *(const UINT32 *)this <= *(const UINT32 *)&p;
                    }
};


//-------- 64 bits ---------------

class PixelRGBM64 { 

    public: 
        //----- variables -----

        static const int maxChannelValue = 0xffff; //16 bit integer in hexadecimal
        typedef UINT16 Channel;

        //--- pixel order (it matters because we temporarily turn our class into a 32 bit unsigned integer when inputting mask) ---

        UINT16 r, g, b, m;  //common for opengl and macos!
        


        //----- constructors -----
        
        PixelRGBM64() 
            : b(0), 
              g(0), 
              r(0), 
              m(maxChannelValue){};

        PixelRGBM64(const PixelRGBM64 &pix)
            : b(pix.b), 
              g(pix.g), 
              r(pix.r), 
              m(pix.m){};


        PixelRGBM64(int i_r, int i_g, int i_b, int i_m = maxChannelValue)
            : b(i_b), 
              g(i_g), 
              r(i_r), 
              m(i_m){};

        

        // "*(UINT32 *)this "  treats our bgrm variables as a single  32 bit integer, allowing for a shortcut to quickly input mask varaibles via an equals sign --> doesnt change class or anything just an input trick!
        //notes: we do constructor delegation beforehand allowing for us to access the object inside the constructor!!!
        PixelRGBM64(UINT64 mask) 
            : PixelRGBM64() { *(UINT64 *)this = mask; }; 

            
        //----- operator methods -----

        inline bool operator==(const PixelRGBM64 &p) const {
                        return *(const UINT64 *)this == *(const UINT64 *)&p;
                    }
        inline bool operator!=(const PixelRGBM64 &p) const {
                        return *(const UINT64 *)this != *(const UINT64 *)&p;
                    }

        inline bool operator<(const PixelRGBM64 &p) const {
                        return *(const UINT64 *)this < *(const UINT64 *)&p;
                    }

        inline bool operator>=(const PixelRGBM64 &p) const {
                        return *(const UINT64 *)this >= *(const UINT64 *)&p;
                    }

        inline bool operator>(const PixelRGBM64 &p) const {
                        return *(const UINT64 *)this > *(const UINT64 *)&p;
                    }
        inline bool operator<=(const PixelRGBM64 &p) const {
                        return *(const UINT64 *)this <= *(const UINT64 *)&p;
                    }
   
};




#endif