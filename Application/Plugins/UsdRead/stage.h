#pragma once

#include <pxr/pxr.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/stageCache.h>
#include <pxr/usd/usd/stageCacheContext.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/matrix4d.h>

#include "common.h"
#include "shader.h"
#include <xsi_string.h>
#include <xsi_customprimitive.h>

class U2XPrim;
extern int U2X_STAGE_ID;

class U2XSelection {
public:
  void AddPrim(const SdfPath& path);
  void RemovePrim(const SdfPath& path);
  void Clear();
  void SetStage(pxr::UsdStageRefPtr& stage) { _stage = stage;}

private:
  pxr::UsdStageRefPtr                         _stage;
  TfHashMap<SdfPath, UsdPrim, SdfPath::Hash>  _items;
};

// Prim base class
class U2XStage {
public:
  U2XStage();
  ~U2XStage();

  pxr::UsdStageRefPtr& Get() { return _stage; };
  void SetObjectID(ULONG objectID) { _objectID = objectID; };
  ULONG GetObjectID() { return _objectID; };
  inline const pxr::GfBBox3d& GetBBox() { return _bbox; };
  size_t GetNumPrims() { return _prims.size(); };
  LONG GetLastEvalID() { return _lastEvalID; };
  void SetLastEvalID(LONG evalID) { _lastEvalID = evalID; };
  bool IsLoaded() { return _isLoaded; };
  bool HasFilename(const XSI::CString& filename, size_t index);
  void SetFilenames(const XSI::CStringArray& filenames);
  void SetTime(double time, bool forceUpdate);
  SdfLayerRefPtr GetRootLayer() { return _rootLayer; };
  void Reload();
  void Clear();
  void ComputeBoundingBox(const pxr::UsdTimeCode& timeCode);
  void Recurse(const pxr::UsdPrim& prim, U2XPrim* parent);
  void Update(XSI::CustomPrimitive& prim);
  void Draw();

protected:
  std::vector<SdfLayerRefPtr>       _layers;
  pxr::UsdGeomXform                 _root;
  pxr::UsdPrim                      _ref;
  SdfLayerRefPtr                    _rootLayer;
  //SdfLayerRefPtr                    _rootLayer;
  std::vector<XSI::CString>         _rawFilenames;
  std::vector<std::string>          _filenames;
  bool                              _isLoaded;
  pxr::UsdStageRefPtr               _stage;
  pxr::TfToken                      _upAxis;
  std::vector<U2XPrim*>             _prims;
  double                            _time;
  LONG                              _lastEvalID;
  pxr::GfBBox3d                     _bbox;
  pxr::UsdGeomBBoxCache*            _bboxCache;
  pxr::UsdGeomXformCache*           _xformCache;
  pxr::GfMatrix4f                   _invXform;
  ULONG                             _objectID;
  U2XSelection                      _selection;
};

