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

int U2X_STAGE_ID = 0;

U2XStage::U2XStage()
  : _lastEvalID(-1), _isLoaded(false), _time(DBL_MAX)
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

bool U2XStage::HasFilename(const CString& filename, size_t index)
{
  if (_rawFilenames.size() == 0 | _rawFilenames.size() > index) return false;
  else return (_rawFilenames[index] == filename);
}

void U2XStage::SetFilename(const CString& filename, size_t index)
{
  if ((index + 1) > _rawFilenames.size())
  {
    _rawFilenames.resize(index + 1);
    _filenames.resize(index + 1);
  }

  _rawFilenames[index] = filename;
  _filenames[index] = pxr::ArchNormPath(filename.GetAsciiString(), false);

  Reload();
}

void U2XStage::SetFilenames(const CStringArray& filenames)
{
  size_t numFiles = filenames.GetCount();
  _rawFilenames.resize(numFiles);
  _filenames.resize(numFiles);

  for (int i = 0; i < numFiles; ++i)
  {
    CString filename = filenames[i];
    _rawFilenames[i] = filename;
    _filenames[i] = pxr::ArchNormPath(filename.GetAsciiString(), false);
  }

  Reload();
}

bool U2XStage::Reload()
{
  _isLoaded = false;

  Clear();
  _layers.clear();

  _stage = pxr::UsdStage::Open(_filenames[0]);
  /*
  bool successfullyOpened = false;
  for (int i = 0; i < _filenames.size(); ++i)
  {
    std::string layerFilename = _filenames[i];
    if (pxr::UsdStage::IsSupportedFile(layerFilename) &&
      pxr::ArchGetFileLength(layerFilename.c_str()) != -1)
    {
      if (i == 0)
      {
        _rootLayer = SdfLayer::CreateAnonymous("U2XStage"+std::to_string(U2X_STAGE_ID));
        U2X_STAGE_ID++;

        pxr::SdfLayerRefPtr baseLayer = SdfLayer::FindOrOpen(layerFilename);
        _layers.push_back(baseLayer);
        _rootLayer->InsertSubLayerPath(baseLayer->GetIdentifier());

        successfullyOpened = true;
      }
      else
      {
        pxr::SdfLayerRefPtr subLayer = pxr::SdfLayer::FindOrOpen(layerFilename);
        _layers.push_back(subLayer);
        _rootLayer->InsertSubLayerPath(subLayer->GetIdentifier());
      }
    }
  }
  */
  if (_stage)
  {
    //_stage = pxr::UsdStage::Open(_rootLayer->GetIdentifier());
    //_stage->SetStartTimeCode(1);
    //_stage->SetEndTimeCode(100);


    //pxr::SdfLayerHandleVector usedLayers = _stage->GetUsedLayers();
    _root = _stage->GetPseudoRoot();

   // pxr::UsdStageRefPtr tmpStage = pxr::UsdStage::Open(_rootLayer);
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
  return _isLoaded;
}

void U2XStage::SetTime(double time)
{
  if (time != _time)
  {
    _xformCache->SetTime(pxr::UsdTimeCode(time));
    for (auto& prim : _prims)
    {
      //prim->Update(time);
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

void U2XStage::Update(XSI::CustomPrimitive& prim)
{
  const LONG objectID = prim.GetObjectID();
  const LONG evalID = prim.GetEvaluationID();
  if (_lastEvalID != evalID)
  {
    // get parameters
    CParameterRefArray& params = prim.GetParameters();

    // check filename
    CString filename = params.GetValue(L"Filename");
    if (!HasFilename(filename, 0) || !_isLoaded)
    {
      SetFilename(filename, 0);
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