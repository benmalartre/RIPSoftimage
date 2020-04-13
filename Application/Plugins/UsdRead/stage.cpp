#include <GL/glew.h>
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


U2XStage::U2XStage()
  :_filename(""), _lastEvalID(-1), _needReload(true), _time(DBL_MAX)
{
}

U2XStage::~U2XStage()
{
  LOG("DELETE FUCKIN STAGE :D");
  Clear();
}


void U2XStage::SetFilename(const CString& filename)
{
  _rawFilename = filename;
  _filename = pxr::ArchNormPath(_rawFilename.GetAsciiString(), false);
  _needReload = true;
  Clear();
  
  if (pxr::UsdStage::IsSupportedFile(_filename) &&
    pxr::ArchGetFileLength(_filename.c_str()) != -1)
  {
    //pxr::SdfLayerRefPtr rootLayer = pxr::SdfLayer::FindOrOpen(_filename);
    //_stage = pxr::UsdStage::Open(rootLayer);
    _stage = pxr::UsdStage::Open(_filename);
    _root = _stage->GetPseudoRoot();
    Recurse(_root);

    _needReload = false;
  }
}

void U2XStage::SetTime(double time)
{
  if (time != _time)
  {
    _time = time;
    for (auto& prim : _prims)prim->Update(_time);
   
    ComputeBoundingBox(pxr::UsdTimeCode(time));
  }
}

void U2XStage::ComputeBoundingBox(const pxr::UsdTimeCode& timeCode)
{
  TfTokenVector purposes = { UsdGeomTokens->default_, UsdGeomTokens->render };
  if (_root.IsValid())
  {
    pxr::UsdGeomBBoxCache bboxCache(timeCode, purposes);
    _bbox = bboxCache.ComputeWorldBound(_root);
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
    ////LOG("MESH : " + CString(prim.GetPath().GetText()));
    U2XMesh* mesh = new U2XMesh(prim);
    mesh->Init();
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

    //CParameterRefArray& params = in_prim.GetParameters();
    //data.halfWidth = params.GetValue(kWidthName);
    //data.halfHeight = params.GetValue(kHeightName);
    //data.halfLength = params.GetValue(kLengthName);

    // check filename
    CString filename = prim.GetParameterValue(L"Filename");
    if (!HasFilename(filename) || _needReload)
    {
      SetFilename(filename);
    }

    double time = prim.GetParameterValue(L"Time");
    SetTime(time);

    ComputeBoundingBox(pxr::UsdTimeCode(time));
    _lastEvalID = evalID;
  }
}

void U2XStage::Draw(const U2XGLSLProgram& glslProgram)
{

  GLint pgm = glslProgram.Get();
  GLuint modelUniform = glGetUniformLocation(pgm, "model");
  for (int i = 0; i < _prims.size(); ++i)
  {
    _prims[i]->Prepare();
    _prims[i]->Draw(modelUniform);
  }
  
}