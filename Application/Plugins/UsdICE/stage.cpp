#include <pxr/base/arch/fileSystem.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/points.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/imaging/garch/glApi.h>

#include "stage.h"
#include "prim.h"
#include "utils.h"
#include "scene.h"
#include "nodes.h"


pxr::UsdStageRefPtr U2I_STAGE;
extern U2IScene*    U2I_SCENE;


U2IStage::U2IStage()
  : _isRoot(false), _objectID(-1), _lastEvalID(-1), _isLoaded(false), _time(DBL_MAX)
{
  TfTokenVector purposes = { UsdGeomTokens->default_, UsdGeomTokens->render };
  _bboxCache = new pxr::UsdGeomBBoxCache(pxr::UsdTimeCode::Default(), purposes);
  _xformCache = new pxr::UsdGeomXformCache(pxr::UsdTimeCode::Default());
}

U2IStage::U2IStage(CustomPrimitive& prim)
  : _isRoot(true), _objectID(prim.GetObjectID()), _lastEvalID(-1), _isLoaded(false), _time(DBL_MAX)
{
  TfTokenVector purposes = { UsdGeomTokens->default_, UsdGeomTokens->render };
  _bboxCache = new pxr::UsdGeomBBoxCache(pxr::UsdTimeCode::Default(), purposes);
  _xformCache = new pxr::UsdGeomXformCache(pxr::UsdTimeCode::Default());
}

U2IStage::~U2IStage()
{
  Clear();
  delete _bboxCache;
  delete _xformCache;
  _stage = nullptr;
}

bool U2IStage::HasFilename(const CString& filename, size_t index)
{
  if (!_rawFilenames.size() || index >= _rawFilenames.size()) return false;
  else return (_rawFilenames[index] == filename);
}

void U2IStage::SetFromICE(const CustomPrimitive& prim)
{
  ICEAttribute attr = prim.GetICEAttributeFromName(U2I_PROCEDURAL_ROOT);
  if (attr.IsValid() && attr.IsDefined()) {
    CStringArray customDataTypes = attr.GetCustomDataTypes();
    if (customDataTypes.GetCount() && customDataTypes[0] == "UsdStage") {
      CICEAttributeDataArrayCustomType customData;
      attr.GetDataArray(customData);

      for (ULONG i = 0; i < customData.GetCount(); i++)
      {
        ULONG nSize;
        const CICEAttributeDataArrayCustomType::TData* pBuffer;
        customData.GetData(i, &pBuffer, nSize);

        LOG(CString((void*)pBuffer) + L":" + CString(nSize));
        U2IStageNodeDatas_t* stageData = (U2IStageNodeDatas_t*)(void*)pBuffer;
        std::cout << "STAGE DATA : " << stageData->_uri << std::endl;
      }
    }
    //pxr::UsdStageRefPtr stage = 

    /*


    // Log the data pointer address and size of data.
    CICEAttributeDataArrayCustomType customData;
    attr.GetDataArray( customData );

    
    */
  }
}

void U2IStage::SetFilenames(const CStringArray& filenames)
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

void U2IStage::Reload()
{
  _isLoaded = false;
  Clear();

  //_rootLayer = pxr::SdfLayer::FindOrOpen(_filenames[0]);
  //_stageX = pxr::UsdStage::Open(_rootLayer->GetIdentifier());
  std::string uniqueName = std::to_string(_objectID);
  _stage = pxr::UsdStage::CreateInMemory("U2IStage_" + uniqueName);
  _rootLayer = _stage->GetRootLayer();

  for (size_t i = 0; i < _filenames.size(); ++i)
  {
    std::string filename = _filenames[i];
    if (pxr::UsdStage::IsSupportedFile(filename) &&
      pxr::ArchGetFileLength(filename.c_str()) != -1)
    {
      pxr::SdfLayerRefPtr subLayer = SdfLayer::FindOrOpen(filename);
      _layers.push_back(subLayer);
      _rootLayer->InsertSubLayerPath(subLayer->GetIdentifier());
    }
  }
  
  _stage->SetDefaultPrim(*_stage->Traverse().cbegin());
  U2I_SCENE->ReloadStage(this);

  //pxr::SdfLayerRefPtr rootLayer = pxr::SdfLayer::FindOrOpen(_filename);
  //_stage = pxr::UsdStage::Open(rootLayer);
  
  //_rootLayer = pxr::SdfLayer::FindOrOpen(_filenames[0]);
  //_stage = pxr::UsdStage::Open(_rootLayer->GetIdentifier());
  //_stage->SetEditTarget(_rootLayer);
  /*
  _selection.Clear();
  _selection.SetStage(_stage);
  _upAxis = pxr::UsdGeomGetStageUpAxis(_stage);
 
  if (_upAxis == pxr::UsdGeomTokens->z)
  {
    pxr::UsdGeomXformable xformable(_stage->GetDefaultPrim());
    pxr::UsdGeomXformOp rotateOp = xformable.AddRotateXOp();
    rotateOp.Set(-90.f);
  }
  */
  //Recurse(_root, NULL);
  
  _isLoaded = true;
}

void U2IStage::SetTime(double time, bool forceUpdate)
{
  if (time != _time || forceUpdate)
  {
    _xformCache->SetTime(pxr::UsdTimeCode(time));
    /*
    for (auto& prim : _prims)
    {
      prim->Update(time, forceUpdate);
      prim->SetMatrix(_xformCache->GetLocalToWorldTransform(prim->Get()));
    }
    */
    ComputeBoundingBox(pxr::UsdTimeCode(time));
    _time = time;
  }
}

void U2IStage::ComputeBoundingBox(const pxr::UsdTimeCode& timeCode)
{
  TfTokenVector purposes = { UsdGeomTokens->default_, UsdGeomTokens->render };
  if (_root.GetPrim().IsValid())
  {
    _bboxCache->SetTime(timeCode);
    _bbox = _bboxCache->ComputeWorldBound(_root.GetPrim());
  }
}

void U2IStage::Clear()
{
  for (auto prim : _prims) {
    if (prim)delete prim;
  }
  _prims.clear();
  _layers.clear();
}


void U2IStage::Recurse(const pxr::UsdPrim& prim, U2IPrim* parent)
{
  /*
  U2IPrim* current = NULL;
  if (prim.IsA<pxr::UsdGeomMesh>())
  {
    U2IMesh* mesh = new U2IMesh(prim, parent);
    mesh->Init();
    mesh->SetMatrix(_xformCache->GetLocalToWorldTransform(prim));
    _prims.push_back(mesh);
    current = mesh;
  }
  else if (prim.IsA<pxr::UsdGeomPoints>())
  {
    //LOG("POINTS : " + CString(prim.GetPath().GetText()));
  }
  if (prim.IsA<pxr::UsdGeomXform>())
  {
    U2IXform* xform = new U2IXform(prim, parent);
    xform->Init();
    xform->SetMatrix(_xformCache->GetLocalToWorldTransform(prim));
    _prims.push_back(xform);
    current = xform;
  }
  for (const auto& child : prim.GetChildren())
  {
    Recurse(child, current);
  }
  */
}

bool U2IStage::HasUsdICEAttribute(CustomPrimitive& prim)
{
  ICEAttribute attr(prim.GetICEAttributeFromName(U2I_PROCEDURAL_ROOT));
  if (attr.IsValid() && attr.IsDefined())return true;
  return false;
};

void U2IStage::Update(CustomPrimitive& prim)
{
  const LONG objectID = prim.GetObjectID();
  const LONG evalID = prim.GetEvaluationID();
  CParameterRefArray& params = prim.GetParameters();
  bool forceUpdate = params.GetValue("Update");

  X3DObject parent = prim.GetParent3DObject();
  KinematicState kineState = parent.GetKinematics().GetGlobal();

  if (_lastEvalID != evalID || forceUpdate)
  {
    // get inverse xform
    pxr::GfMatrix4d xfo = *(pxr::GfMatrix4d*)&kineState.GetTransform().GetMatrix4();
    _invXform = pxr::GfMatrix4f(xfo.GetInverse());
    
    // check filename
    CString filename = params.GetValue(L"Filename");
    if (!HasFilename(filename, 0) || !_isLoaded)
    {
      CStringArray filenames;
      filenames.Add(filename);
      SetFilenames(filenames);
    }
    if (_isLoaded)
    {
      double time = params.GetValue(L"Time");
      SetTime(time, forceUpdate);
    }
    prim.PutParameterValue("Update", false);
    _lastEvalID = evalID;
  }
  
  MATH::CMatrix4 matrix = kineState.GetTransform().GetMatrix4();
  if (matrix != _xfo) {
    pxr::UsdGeomXformable xformable(U2I_SCENE->GetRootPrim(_objectID));
    if (xformable) {
      MATH::CMatrix4 matrix = kineState.GetTransform().GetMatrix4();
      pxr::GfMatrix4d usdMatrix;
      memcpy((void*)&usdMatrix, (void*)&matrix, 16 * sizeof(double));

      bool resetXformOpExists;
      std::vector<pxr::UsdGeomXformOp> xformOps = xformable.GetOrderedXformOps(&resetXformOpExists);
      if (!xformOps.size()) {
        pxr::UsdGeomXformOp xformOp = xformable.AddTransformOp(pxr::UsdGeomXformOp::PrecisionDouble);
        xformOp.Set(pxr::VtValue(usdMatrix));
      }
      else {
        xformOps[0].Set(pxr::VtValue(usdMatrix));
      }
    }
    _xfo = matrix;
  }
}

void U2IStage::Draw()
{
  
}