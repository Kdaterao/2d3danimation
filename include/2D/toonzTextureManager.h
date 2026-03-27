#pragma once

#ifndef TOONZTEXTUREMANAGER_H
#define TOONZTEXTUREMANAGER_H

#include <toonzGeometry.h>
#include <QOpenGLFunctions>

class toonzTextureManager {
  DimensionTI textureSize;
  bool m_isRGBM;
  static toonzTextureManager *m_instance;  //singleton design pattern (only one texture manager in program..)

  toonzTextureManager();

public:
  static toonzTextureManager *instance();
  DimensionTI getMaxSize(bool isRGBM);
  void getFmtAndType(bool isRGBM, GLenum &fmt, GLenum &type);
  DimensionTI createEmptyTexture(DimensionTI reqSize, bool isRGBM);
};

#endif  
