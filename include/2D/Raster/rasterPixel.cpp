#include <rasterPixel.h>



//---- default colors for 32-bit pixel -----


const PixelBGRM32 PixelBGRM32::Red(maxChannelValue, 0, 0);
const PixelBGRM32 PixelBGRM32::Green(0, maxChannelValue, 0);
const PixelBGRM32 PixelBGRM32::Blue(0, 0, maxChannelValue);
const PixelBGRM32 PixelBGRM32::Yellow(maxChannelValue, maxChannelValue, 0);
const PixelBGRM32 PixelBGRM32::Cyan(0, maxChannelValue, maxChannelValue);
const PixelBGRM32 PixelBGRM32::Magenta(maxChannelValue, 0, maxChannelValue);
const PixelBGRM32 PixelBGRM32::Gray(maxChannelValue/2, maxChannelValue/2,
                                      maxChannelValue/2);
const PixelBGRM32 PixelBGRM32::White(maxChannelValue, maxChannelValue,
                                       maxChannelValue);
const PixelBGRM32 PixelBGRM32::Black(0, 0, 0);
const PixelBGRM32 PixelBGRM32::Transparent(0, 0, 0, 0);



//----- default colors for 64-bit pixel bgrm ------


//----- default colors for 32-bit pixel rgbm ------


//----- default colors for 64-bit pixel rgbm ------

const PixelRGBM64 PixelRGBM64::White(maxChannelValue, maxChannelValue, maxChannelValue, maxChannelValue);
const PixelRGBM64 PixelRGBM64::Black(0, 0, 0, maxChannelValue);
const PixelRGBM64 PixelRGBM64::Transparent(0, 0, 0, 0);
