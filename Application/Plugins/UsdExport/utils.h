#pragma once

#include "common.h"
#include <limits>

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