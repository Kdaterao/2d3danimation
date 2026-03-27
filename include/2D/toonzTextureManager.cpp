#include <toonzTextureManager.h>


//points to current instance
toonzTextureManager *toonzTextureManager::instance() {
  if (!m_instance) m_instance = new toonzTextureManager();
  return m_instance;
}


toonzTextureManager::toonzTextureManager() : textureSize(0, 0), m_isRGBM(true) {}

toonzTextureManager *toonzTextureManager::m_instance;

//--- Creates an empty texture ---
DimensionTI toonzTextureManager::createEmptyTexture(DimensionTI reqSize, bool isRGBM) {

  /*
  //--- older gpus cant handle texture sizes that arent powers of two ---
  int lx                     = 1;
  int ly                     = 1;
  while (lx < reqSize.lx) lx = lx << 1;
  while (ly < reqSize.ly) ly = ly << 1;
  DimensionTI textureSize     = instance()->getMaxSize(isRGBM);
  assert(lx <= textureSize.lx);
  assert(ly <= textureSize.ly);
  */
  //--- get the format and type of our texture ---
  GLenum fmt, type;
  getFmtAndType(isRGBM, fmt, type);
  

  //--- create empty texture ---
  glTexImage2D(GL_TEXTURE_2D,  // target (is a 2D texture)
               0,              // is one level only
               GL_RGBA8,              //  number of component of a pixel
               reqSize.lx,             // size width
               reqSize.ly,             //      height
               0,              // size of a border
               fmt, type, 0);
  //--- return the dimensions of the texture ---
  return DimensionTI(reqSize.lx, reqSize.ly);
}



void toonzTextureManager::getFmtAndType(bool isRGBM, GLenum &fmt, GLenum &type){
  /*
  if (isRGBM) {
    //case1: rgbm
    fmt =  GL_BGRA_EXT;
    type = GL_UNSIGNED_INT_8_8_8_8_REV;
  } else {
    //case2: grayscale
    fmt  = GL_LUMINANCE;
    type = GL_UNSIGNED_BYTE;
  }
    */

    fmt =  GL_BGRA;
    type = GL_UNSIGNED_BYTE;


}


DimensionTI toonzTextureManager::getMaxSize(bool isRGBM){

    return DimensionTI(1000, 1000); //placeholder

}