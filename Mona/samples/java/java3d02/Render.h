// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __Render__
#define __Render__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  class Render;
  class Vector;
  class Matrix;
}

class Render : public ::java::lang::Object
{
public:
  Render (jint, jint);
  static void Clear ();
  static void SetTransform (::Matrix *);
  static void DrawIndexedPrimitive (jint, JArray< ::Vector *> *, jintArray);
private:
  static ::Vector *CalcNormal (::Vector *, ::Vector *, ::Vector *);
  static void DrawPolygon (::Vector *, ::Vector *, ::Vector *);
  static void ScanEdge (::Vector *, ::Vector *);
public: // actually package-private
  static const jint PRIMITIVE_POLYGON = 1L;
  static ::Matrix *mMaster;
  static jintArray pbuf;
private:
  static jintArray zbuf;
  static jint BUFFERW;
  static jint BUFFERH;
  static jint CENTERX;
  static jint CENTERY;
  static jintArray min;
  static jintArray max;
  static jintArray minz;
public: // actually package-private
  static jintArray maxz;
public:

  static ::java::lang::Class class$;
};

#endif /* __Render__ */