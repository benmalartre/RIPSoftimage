#pragma once

#include <xsi_application.h>
#include <xsi_string.h>
#include <pxr/pxr.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/quatf.h>
#include <pxr/base/gf/rotation.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/matrix4d.h>
#include "common.h"

using namespace XSI;

#define LOG(msg) Application().LogMessage(msg);

static CStringArray U2XGetNamesFromObjectNamesList(const CString& objectNames)
{
  CString strippedNames = objectNames;
  ULONG npos = strippedNames.FindString(L"(");
  if (npos != UINT_MAX)
    strippedNames = strippedNames.GetSubString(npos + 1, strippedNames.Length() - npos);
  npos = strippedNames.ReverseFindString(L")");
  if (npos != UINT_MAX)strippedNames = strippedNames.GetSubString(0, npos);

  CStringArray result = strippedNames.Split(L",");
  return result;
}

template<typename T>
static void U2XRandomXform(T& m)
{
  m = T(1);

  pxr::GfRotation r;
  pxr::GfVec3d axis(
    (float)rand() / (float)RAND_MAX,
    (float)rand() / (float)RAND_MAX,
    (float)rand() / (float)RAND_MAX);
  axis.Normalize();
  double angle = (float)rand() / (float)RAND_MAX * 360;
  r.SetAxisAngle(axis, angle);
  m.SetRotate(r);

  m.SetTranslateOnly(
    pxr::GfVec3f(
    (((float)rand() / (float)RAND_MAX) - 0.5f) * 12.f,
      (((float)rand() / (float)RAND_MAX) - 0.5f) * 12.f,
      (((float)rand() / (float)RAND_MAX) - 0.5f) * 12.f));
}

static void U2XTransformPoints(pxr::VtArray<pxr::GfVec3f>& ioPoints, const pxr::GfMatrix4f& m)
{
  for (int i = 0; i < ioPoints.size(); ++i) {
    ioPoints[i] = m.Transform(ioPoints[i]);
  }
}

static void U2XRandomColor(pxr::VtArray<pxr::GfVec3f>& ioColors)
{
  for (int i = 0; i < ioColors.size(); ++i)
    ioColors[i] = pxr::GfVec3f(
    (float)rand() / (float)RAND_MAX,
      (float)rand() / (float)RAND_MAX,
      (float)rand() / (float)RAND_MAX);
}

static double U2XGetBoundingBoxComponent(const pxr::GfBBox3d& bbox, U2XBoundingBoxComponent comp)
{
  switch (comp)
  {
  case BBOX_LOWER_X:
    return bbox.GetRange().GetMin()[0];
    break;
  case BBOX_LOWER_Y:
    return bbox.GetRange().GetMin()[1];
    break;
  case BBOX_LOWER_Z:
    return bbox.GetRange().GetMin()[2];
    break;
  case BBOX_UPPER_X:
    return bbox.GetRange().GetMax()[0];
    break;
  case BBOX_UPPER_Y:
    return bbox.GetRange().GetMax()[1];
    break;
  case BBOX_UPPER_Z:
    return bbox.GetRange().GetMax()[2];
    break;
  }
}

/// Triangulate a polygonal mesh
/// Samples store a GfVec3i X per triangle vertex where
///  - X[0] Vertex index on original topology
///  - X[1] Face index on original topology
///  - X[2] Sample index on original topology
int
U2XTriangulateMesh(const pxr::VtArray<int>& counts,
  const pxr::VtArray<int>& indices,
  pxr::VtArray<pxr::GfVec3i>& samples);

/// Compute smooth vertex normals on a triangulated polymesh
void
U2XComputeVertexNormals(const pxr::VtArray<pxr::GfVec3f>& positions,
  const pxr::VtArray<int>& counts,
  const pxr::VtArray<int>& indices,
  const pxr::VtArray<pxr::GfVec3i>& samples,
  pxr::VtArray<pxr::GfVec3f>& normals);

/// Compute random vertex color
void
U2XComputeVertexColors(const pxr::VtArray<pxr::GfVec3f>& positions,
  pxr::VtArray<pxr::GfVec3f>& colors);

/// Triangulate data
template<typename T>
void
U2XTriangulateDatas(const pxr::VtArray<pxr::GfVec3i>& samples,
  const pxr::VtArray<T>& datas,
  pxr::VtArray<T>& result)
{
  result.resize(samples.size());
  for (int i = 0; i<samples.size(); ++i)
  {
    result[i] = datas[samples[i][0]];
  }
};

