#pragma once

#include "common.h"
#include <limits>
#include <iostream>
#include <string>

static const GfVec3f UNDEFINED_COLOR = { 1.f,0.25f,0.5f};

GfVec3f GetDisplayColorFromShadingNetwork(const X3DObject& obj);

void GetObjectBoundingBox(const X3DObject& obj, GfBBox3d& bbox);

template<typename T>
GfBBox3d ComputeBoundingBox(T* points, size_t numPoints)
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
  memcpy((void*)&dst[0][0], (void*)src.Get(), 16 * sizeof(float));
}

static void X2UConvertMatrix4DoubleToDouble(const MATH::CMatrix4& src, GfMatrix4d& dst)
{
  memcpy((void*)&dst, (void*)&src, 16 * sizeof(double));
}

template<typename SRC, typename DST>
void X2UConvertMatrix4(const SRC& src, DST& dst)
{
  size_t srcElemSize = sizeof(src.GetValue(0, 0));
  size_t dstElemSize = sizeof(dst[0][0]);

  if (srcElemSize == dstElemSize)
  {

  }
  for (int i = 0; i < 16; ++i)
  {

  }
}