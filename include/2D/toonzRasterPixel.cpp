#include <toonzRasterPixel.h>




//----- operator methods for 32 bit pixel -----

inline bool ToonzPixelBGRM32::operator==(const ToonzPixelBGRM32 &p) const {
                return *(const UINT32 *)this == *(const UINT32 *)&p;
            }
inline bool ToonzPixelBGRM32::operator!=(const ToonzPixelBGRM32 &p) const {
                return *(const UINT32 *)this != *(const UINT32 *)&p;
            }

inline bool ToonzPixelBGRM32::operator<(const ToonzPixelBGRM32 &p) const {
                return *(const UINT32 *)this < *(const UINT32 *)&p;
            }

inline bool ToonzPixelBGRM32::operator>=(const ToonzPixelBGRM32 &p) const {
                return *(const UINT32 *)this >= *(const UINT32 *)&p;
            }

inline bool ToonzPixelBGRM32::operator>(const ToonzPixelBGRM32 &p) const {
                return *(const UINT32 *)this > *(const UINT32 *)&p;
            }
inline bool ToonzPixelBGRM32::operator<=(const ToonzPixelBGRM32 &p) const {
                return *(const UINT32 *)this <= *(const UINT32 *)&p;
            }

//---- default colors for 32-bit pixel -----


const ToonzPixelBGRM32 ToonzPixelBGRM32::Red(maxChannelValue, 0, 0);
const ToonzPixelBGRM32 ToonzPixelBGRM32::Green(0, maxChannelValue, 0);
const ToonzPixelBGRM32 ToonzPixelBGRM32::Blue(0, 0, maxChannelValue);
const ToonzPixelBGRM32 ToonzPixelBGRM32::Yellow(maxChannelValue, maxChannelValue, 0);
const ToonzPixelBGRM32 ToonzPixelBGRM32::Cyan(0, maxChannelValue, maxChannelValue);
const ToonzPixelBGRM32 ToonzPixelBGRM32::Magenta(maxChannelValue, 0, maxChannelValue);
const ToonzPixelBGRM32 ToonzPixelBGRM32::Gray(maxChannelValue/2, maxChannelValue/2,
                                      maxChannelValue/2);
const ToonzPixelBGRM32 ToonzPixelBGRM32::White(maxChannelValue, maxChannelValue,
                                       maxChannelValue);
const ToonzPixelBGRM32 ToonzPixelBGRM32::Black(0, 0, 0);
const ToonzPixelBGRM32 ToonzPixelBGRM32::Transparent(0, 0, 0, 0);