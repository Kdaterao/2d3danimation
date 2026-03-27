#ifndef TOONZRASTERPIXEL_H
#define TOONZRASTERPIXEL_H
#include <types.h>

//----- forward declarations -----

//! b,g,r,m 4 byte/pixel (this order since opengl macos)
class ToonzPixelBGRM32;


//----- 32 bit pixel class -----

class ToonzPixelBGRM32 { 

    public: 
        //----- variables -----

        static const int maxChannelValue = 0xff; //8 bit integer in hexadecimal (study up on this lol)
        typedef unsigned char Channel;

        //--- pixel order (it matters because we temporarily turn our class into a 32 bit unsigned integer when inputting mask) ---

        unsigned char b, g, r, m;  //common for opengl and macos!
        


        //----- constructors -----
        
        ToonzPixelBGRM32() 
            : b(0), 
              g(0), 
              r(0), 
              m(maxChannelValue){};

        ToonzPixelBGRM32(const ToonzPixelBGRM32 &pix)
            : b(pix.b), 
              g(pix.g), 
              r(pix.r), 
              m(pix.m){};


        ToonzPixelBGRM32(int i_r, int i_g, int i_b, int i_m = maxChannelValue)
            : b(i_b), 
              g(i_g), 
              r(i_r), 
              m(i_m){};

        

        // "*(UINT32 *)this "  treats our bgrm variables as a single  32 bit integer, allowing for a shortcut to quickly input mask varaibles via an equals sign --> doesnt change class or anything just an input trick!
        //notes: we do constructor delegation beforehand allowing for us to access the object inside the constructor!!!
        ToonzPixelBGRM32(UINT32 mask) 
            : ToonzPixelBGRM32() { *(UINT32 *)this = mask; }; 

            
        //----- operator methods -----

        inline bool operator==(const ToonzPixelBGRM32 &p) const;

        inline bool operator!=(const ToonzPixelBGRM32 &p) const; 

        inline bool operator<(const ToonzPixelBGRM32 &p) const; 
            
        inline bool operator>=(const ToonzPixelBGRM32 &p) const;

        inline bool operator>(const ToonzPixelBGRM32 &p) const;

        inline bool operator<=(const ToonzPixelBGRM32 &p) const;


        //----- default colors -----


        static const ToonzPixelBGRM32 Red;
        static const ToonzPixelBGRM32 Green;
        static const ToonzPixelBGRM32 Blue;
        static const ToonzPixelBGRM32 Yellow;
        static const ToonzPixelBGRM32 Cyan;
        static const ToonzPixelBGRM32 Magenta;
        static const ToonzPixelBGRM32 Gray;
        static const ToonzPixelBGRM32 White;
        static const ToonzPixelBGRM32 Black;
        static const ToonzPixelBGRM32 Transparent;
                    
      



};

#endif