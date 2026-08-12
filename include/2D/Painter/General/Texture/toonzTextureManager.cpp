#include <toonzTextureManager.h>


//points to current instance
toonzTextureManager *toonzTextureManager::instance() {
  if (!m_instance) m_instance = new toonzTextureManager();
  return m_instance;
}


toonzTextureManager::toonzTextureManager() : textureSize(0, 0), m_isRGBM(true) {}

toonzTextureManager *toonzTextureManager::m_instance;

//--- Creates an empty texture ---
DimensionTI toonzTextureManager::createEmptyTexture(DimensionTI reqSize, bool isRGBM, GLenum fmt, GLenum type) {

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


  //--- create empty texture ---
  // Match GPU storage to upload type (8-bit vs 16-bit channels)
  GLint internalFmt = (type == GL_UNSIGNED_SHORT) ? GL_RGBA16 : GL_RGBA8;

  
  glTexImage2D(GL_TEXTURE_2D,  // target (is a 2D texture)
               0,              // is one level only
               internalFmt,    //gpu format storage type
               reqSize.lx,     // size width
               reqSize.ly,     // height
               0,              // size of a border
               fmt,            //input data format
               type,           //input data type(UChar or UINT)
                0);
  //--- return the dimensions of the texture ---
  return DimensionTI(reqSize.lx, reqSize.ly);
}


//add conditional statments once you think about importing images
void toonzTextureManager::getFmtAndType(bool isRGBM, bool is32, GLenum &fmt, GLenum &type){


    //----- pixel format -------
    if(isRGBM){
      fmt =  GL_BGRA;
    } else {
      fmt = GL_RGBA;
    }

    //---- channel size -------
    if(is32){
      type = GL_UNSIGNED_BYTE; //1 byte per channel
    } else {
      type = GL_UNSIGNED_SHORT; //... short = 2 bytes
    }
    

}


DimensionTI toonzTextureManager::getMaxSize(bool isRGBM){

    return DimensionTI(200, 300); //placeholder

}