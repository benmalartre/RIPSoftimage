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

using namespace XSI;

#define LOG(msg) Application().LogMessage(msg);

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

