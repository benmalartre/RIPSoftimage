#include <GL/glew.h>
#include "common.h"
#include "stage.h"
#include "prim.h"
#include "mesh.h"
#include "utils.h"
#include "shader.h"

#include <pxr/base/arch/fileSystem.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/points.h>
#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/usdGeom/metrics.h>


U2XStage::U2XStage()
  :_filename(""), _lastEvalID(-1), _isLoaded(false), _time(DBL_MAX)
{
  TfTokenVector purposes = { UsdGeomTokens->default_, UsdGeomTokens->render };
  _bboxCache = new pxr::UsdGeomBBoxCache(pxr::UsdTimeCode::Default(), purposes);
  _xformCache = new pxr::UsdGeomXformCache(pxr::UsdTimeCode::Default());
}

U2XStage::~U2XStage()
{
  Clear();
  delete _bboxCache;
  delete _xformCache;
}


void U2XStage::SetFilename(const CString& filename)
{
  _rawFilename = filename;
  _filename = pxr::ArchNormPath(_rawFilename.GetAsciiString(), false);
  _isLoaded = false;
  Clear();
  
  if (pxr::UsdStage::IsSupportedFile(_filename) &&
    pxr::ArchGetFileLength(_filename.c_str()) != -1)
  {
    //pxr::SdfLayerRefPtr rootLayer = pxr::SdfLayer::FindOrOpen(_filename);
    //_stage = pxr::UsdStage::Open(rootLayer);
    _stage = pxr::UsdStage::Open(_filename);
    _root = _stage->GetPseudoRoot();
    _upAxis = pxr::UsdGeomGetStageUpAxis(_stage);
    if (_upAxis == pxr::UsdGeomTokens->z)
    {
      pxr::UsdGeomXformable xformable(_stage->GetDefaultPrim());
      pxr::UsdGeomXformOp rotateOp = xformable.AddRotateXOp();
      rotateOp.Set(-90.f);
    }
    
    Recurse(_root);

    _isLoaded = true;
  }
}

void U2XStage::SetTime(double time)
{
  if (time != _time)
  {
    _xformCache->SetTime(pxr::UsdTimeCode(time));
    for (auto& prim : _prims)
    {
      prim->Update(time);
      prim->SetMatrix(_xformCache->GetLocalToWorldTransform(prim->Get()));
    }
    ComputeBoundingBox(pxr::UsdTimeCode(time));
    _time = time;
  }
}

void U2XStage::ComputeBoundingBox(const pxr::UsdTimeCode& timeCode)
{
  TfTokenVector purposes = { UsdGeomTokens->default_, UsdGeomTokens->render };
  if (_root.IsValid())
  {
    _bboxCache->SetTime(timeCode);
    _bbox = _bboxCache->ComputeWorldBound(_root);
  }
}

void U2XStage::Clear()
{
  for (auto prim : _prims) {
    if (prim)delete prim;
  }
  _prims.clear();
}

void U2XStage::Load()
{

}

void U2XStage::Recurse(const pxr::UsdPrim& prim)
{
  if (prim.IsA<pxr::UsdGeomMesh>())
  {
    U2XMesh* mesh = new U2XMesh(prim);
    mesh->Init();
    mesh->SetMatrix(_xformCache->GetLocalToWorldTransform(prim));
    _prims.push_back(mesh);
  }
  else if (prim.IsA<pxr::UsdGeomPoints>())
  {
    //LOG("POINTS : " + CString(prim.GetPath().GetText()));
  }
  if (prim.IsA<pxr::UsdGeomXform>())
  {
    //LOG("XFORM : " + CString(prim.GetPath().GetText()));
  }
  for (const auto& child : prim.GetChildren())
  {
    Recurse(child);
  }
}

void U2XStage::Update(CustomPrimitive& prim)
{
  const LONG objectID = prim.GetObjectID();
  const LONG evalID = prim.GetEvaluationID();
  if (_lastEvalID != evalID)
  {
    CParameterRefArray& params = prim.GetParameters();

    // check filename
    CString filename = params.GetValue(L"Filename");
    if (!HasFilename(filename) || !_isLoaded)
    {
      SetFilename(filename);
    }
    if (_isLoaded)
    {
      double time = params.GetValue(L"Time");
      SetTime(time);
    }
    _lastEvalID = evalID;
  }
}

void U2XStage::Draw()
{
  GLint pgm = GLSL_PROGRAM->Get();
  GLint modelUniform = glGetUniformLocation(pgm, "model");
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  for (int i = 0; i < _prims.size(); ++i)
  {
    /// model matrix
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, _prims[i]->GetMatrix());

    _prims[i]->Prepare();
    _prims[i]->Draw();
  }
}