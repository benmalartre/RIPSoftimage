#ifndef U2X_ENGINE_H
#define U2X_ENGINE_H

#include <pxr/pxr.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/imaging/hd/driver.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <pxr/usdImaging/usdImagingGL/renderParams.h>
#include <pxr/imaging/glf/drawTarget.h>

#include "pxr/base/tf/token.h"
#include "pxr/base/tf/errorMark.h"
#include "pxr/base/gf/matrix4f.h"
#include "pxr/base/gf/frustum.h"
#include "pxr/base/gf/rotation.h"

#include <memory>


class U2XEngine : public pxr::UsdImagingGLEngine {
public:
  U2XEngine(const pxr::HdDriver& driver);
  U2XEngine(const pxr::SdfPath& rootPath,
    const pxr::SdfPathVector& excludedPaths,
    const pxr::SdfPathVector& invisedPaths = pxr::SdfPathVector(),
    const pxr::SdfPath& sceneDelegateID =
    pxr::SdfPath::AbsoluteRootPath(),
    const pxr::HdDriver& driver = pxr::HdDriver());
  ~U2XEngine();

  /*
  pxr::HdSelectionSharedPtr _Pick(pxr::GfVec2i const& startPos,
    pxr::GfVec2i const& endPos, pxr::TfToken const& pickTarget);
    */
private:
  pxr::UsdImagingGLRenderParams     _renderParams;
  pxr::UsdPrim                      _root;
  //pxr::UsdLuxDomeLight              _light;
  pxr::GlfDrawTargetRefPtr          _drawTarget;

};


#endif