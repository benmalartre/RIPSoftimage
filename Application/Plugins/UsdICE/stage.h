#ifndef U2I_STAGE_H
#define U2I_STAGE_H

// usd includes
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

// softimage include
#include <xsi_string.h>
#include <xsi_customprimitive.h>

// project includes
#include "common.h"

class U2IPrim;
extern int U2I_STAGE_ID;

// Prim base class
class U2IStage {
public:
  U2IStage();
  U2IStage(CustomPrimitive& prim);
  ~U2IStage();

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
  void SetFromICE(const CustomPrimitive& prim);
  void SetTime(double time, bool forceUpdate);
  pxr::SdfLayerHandle GetRootLayer() { return _stage->GetRootLayer(); };
  bool HasUsdICEAttribute(CustomPrimitive& prim);
  void Reload();
  void Clear();
  void ComputeBoundingBox(const pxr::UsdTimeCode& timeCode);
  void Recurse(const pxr::UsdPrim& prim, U2IPrim* parent);
  void Update(XSI::CustomPrimitive& prim);
  void Draw();

protected:
  std::vector<SdfLayerRefPtr>       _layers;
  pxr::UsdPrim                      _root;
  SdfLayerRefPtr                    _rootLayer;
  std::vector<XSI::CString>         _rawFilenames;
  std::vector<std::string>          _filenames;
  bool                              _isLoaded;
  bool                              _isRoot;
  pxr::UsdStageRefPtr               _stage;
  pxr::TfToken                      _upAxis;
  std::vector<U2IPrim*>             _prims;
  double                            _time;
  LONG                              _lastEvalID;
  pxr::GfBBox3d                     _bbox;
  pxr::UsdGeomBBoxCache*            _bboxCache;
  pxr::UsdGeomXformCache*           _xformCache;
  pxr::GfMatrix4f                   _invXform;
  ULONG                             _objectID;
  MATH::CMatrix4                    _xfo;

};

#endif // U2I_STAGE_H