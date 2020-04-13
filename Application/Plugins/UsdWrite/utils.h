#pragma once

#include "common.h"
#include <limits>
#include <iostream>
#include <string>

static const GfVec3f UNDEFINED_COLOR = { 1.f,0.25f,0.5f};

bool X2UIsModelReferenced(const Model& model);
bool X2UIsModelInstanced(const Model& model);

bool X2UGetObjectVisibility(const X3DObject& obj, double t = DBL_MAX);

GfVec3f X2UGetDisplayColorFromShadingNetwork(const X3DObject& obj);
GfVec3f X2UGetDisplayColorFromWireframeColor(const X3DObject& obj);

void X2UGetObjectBoundingBox(const X3DObject& obj, GfBBox3d& bbox);

void X2UGetLocalTransformAtTime(const X3DObject& obj, GfMatrix4d& ioMatrix, double t = DBL_MAX);
void X2UGetGlobalTransformAtTime(const X3DObject& obj, GfMatrix4d& ioMatrix, double t = DBL_MAX);
void X2UGetLocalTransform(const X3DObject& obj, GfMatrix4d& ioMatrix);
void X2UGetGlobalTransform(const X3DObject& obj, GfMatrix4d& ioMatrix);

ICEAttribute X2UGetICEAttributeFromArray(CRefArray& attributes, const CString& name, int& index);

template<typename T>
GfBBox3d X2UComputeBoundingBox(T* points, size_t numPoints)
{
  GfVec3d bboxMin(
    std::numeric_limits<double>::max(),
    std::numeric_limits<double>::max(),
    std::numeric_limits<double>::max()
  );
  GfVec3d bboxMax(
    -std::numeric_limits<double>::max(),
    -std::numeric_limits<double>::max(),
    -std::numeric_limits<double>::max()
  );
  for (size_t i = 0; i < numPoints; ++i)
  {
    if (points[i * 3    ] < bboxMin[0]) bboxMin[0] = points[i * 3    ];
    if (points[i * 3 + 1] < bboxMin[1]) bboxMin[1] = points[i * 3 + 1];
    if (points[i * 3 + 2] < bboxMin[2]) bboxMin[2] = points[i * 3 + 2];

    if (points[i * 3    ] > bboxMax[0]) bboxMax[0] = points[i * 3    ];
    if (points[i * 3 + 1] > bboxMax[1]) bboxMax[1] = points[i * 3 + 1];
    if (points[i * 3 + 2] > bboxMax[2]) bboxMax[2] = points[i * 3 + 2];
  }
  return GfBBox3d(GfRange3d(bboxMin, bboxMax));
}

// matrix 3
static void X2UConvertMatrix3FloatToDouble(const MATH::CMatrix3f& src, GfMatrix3d& dst)
{
  X2UCopyData<float, double>((float*)src.Get(), (double*)&dst, 9);
}

static void X2UConvertMatrix3DoubleToFloat(const MATH::CMatrix3& src, GfMatrix3f& dst)
{
  X2UCopyData<double, float>((double*)&src, dst.data(), 9);
}

static void X2UConvertMatrix3FloatToFloat(const MATH::CMatrix3f& src, GfMatrix3f& dst)
{
  memcpy((void*)&dst, (void*)&src, 9 * sizeof(float));
}

static void X2UConvertMatrix3DoubleToDouble(const MATH::CMatrix3& src, GfMatrix3d& dst)
{
  memcpy((void*)&dst, (void*)&src, 9 * sizeof(double));
}

// matrix 4 
static void X2UConvertMatrix4FloatToDouble(const MATH::CMatrix4f& src, GfMatrix4d& dst)
{
  X2UCopyData<float, double>((float*)src.Get(), (double*)&dst, 16);
}

static void X2UConvertMatrix4DoubleToFloat(const MATH::CMatrix4& src, GfMatrix4f& dst)
{
  X2UCopyData<double, float>((double*)&src, dst.data(), 16);
}

static void X2UConvertMatrix4FloatToFloat(const MATH::CMatrix4f& src, GfMatrix4f& dst)
{
  memcpy((void*)&dst, (void*)&src, 16 * sizeof(float));
}

static void X2UConvertMatrix4DoubleToDouble(const MATH::CMatrix4& src, GfMatrix4d& dst)
{
  memcpy((void*)&dst, (void*)&src, 16 * sizeof(double));
}

/// Scene time infos
struct X2UTimeInfos {
  double startFrame;
  double endFrame;
  double sampleRate;
  double framesPerSecond;

  void InitFromScene();
  void Set(double start, double end, double rate);
};
