#pragma once

#include <pxr/pxr.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/stageCache.h>
#include <pxr/usd/usd/stageCacheContext.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/matrix4d.h>

#include "common.h"
#include "shader.h"
#include <xsi_string.h>
#include <xsi_customprimitive.h>

class U2XPrim;
extern int U2X_STAGE_ID;
// Prim base class
class U2XStage {
public:
  U2XStage();
  ~U2XStage();

  pxr::UsdStageRefPtr& Get() { return _stage; };
  inline const pxr::GfBBox3d& GetBBox() { return _bbox; };
  size_t GetNumPrims() { return _prims.size(); };
  LONG GetLastEvalID() { return _lastEvalID; };
  void SetLastEvalID(LONG evalID) { _lastEvalID = evalID; };
  bool IsLoaded() { return _isLoaded; };
  bool HasFilename(const XSI::CString& filename, size_t index);
  void SetFilename(const XSI::CString& filename, size_t index);
  void SetFilenames(const XSI::CStringArray& filenames);
  bool Reload();

  void SetTime(double time);
  void Load();
  void Clear();
  void ComputeBoundingBox(const pxr::UsdTimeCode& timeCode);
  void Recurse(const pxr::UsdPrim& prim);
  void Update(XSI::CustomPrimitive& prim);
  void Draw();

protected:
  std::vector<SdfLayerRefPtr>       _layers;
  SdfLayerRefPtr                    _rootLayer;
  std::vector<XSI::CString>         _rawFilenames;
  std::vector<std::string>          _filenames;
  bool                              _isLoaded;
  pxr::UsdStageRefPtr               _stage;
  pxr::TfToken                      _upAxis;
  pxr::UsdPrim                      _root;
  std::vector<U2XPrim*>             _prims;
  double                            _time;
  LONG                              _lastEvalID;
  pxr::GfBBox3d                     _bbox;
  pxr::UsdGeomBBoxCache*            _bboxCache;
  pxr::UsdGeomXformCache*           _xformCache;
};

