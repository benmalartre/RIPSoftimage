#include "xform.h"
#include "utils.h"
#include <pxr/usd/usdGeom/xform.h>

U2XXform::U2XXform(const pxr::UsdPrim& prim, U2XPrim* parent)
  : U2XPrim(prim, parent)
{

}

U2XXform::~U2XXform()
{

}

void U2XXform::Init()
{
  pxr::UsdTimeCode timeCode(pxr::UsdTimeCode::Default());
  GetVisibility(timeCode, true);
  GetXform(timeCode);
}

void U2XXform::Term()
{

}

void U2XXform::Update(double t, bool reinitialize)
{
  pxr::UsdTimeCode timeCode(t);
  GetVisibility(timeCode, reinitialize);
  GetXform(timeCode);
}

void U2XXform::Prepare()
{

}

void U2XXform::Draw()
{

}