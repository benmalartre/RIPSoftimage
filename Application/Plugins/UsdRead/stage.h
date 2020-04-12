#pragma once

#include <pxr/pxr.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/base/gf/bbox3d.h>
#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/matrix4d.h>

class U2XPrim;

// Prim base class
class U2XStage {
public:
  U2XStage(const std::string& filename);
  ~U2XStage();

  pxr::UsdStageRefPtr& Get() { return _stage; };
  size_t GetNumPrims() { return _prims.size(); };

protected:
  std::string                  _filename;
  pxr::UsdPrim                 _root;
  pxr::UsdStageRefPtr          _stage;
  LONG                         _lastEvalID;
  bool                         _needReload;
  std::vector<U2XPrim*> _prims;
};

typedef std::shared_ptr<U2XStage> U2XStageSharedPtr;
typedef std::vector<U2XStageSharedPtr> U2XStageSharedPtrList;

