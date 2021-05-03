#include "event.h"
#include "scene.h"
#include "utils.h"
#include "window.h"

extern U2XPrimitiveManager U2X_PRIMITIVES;

SICALLBACK UsdReadValueChange_OnEvent(const XSI::CRef& in_ref)
{
  Context ctxt(in_ref);
  CRef object = ctxt.GetAttribute("Object");

  if (object.IsA(siKinematicStateID)) {
    CString fullname = ctxt.GetAttribute("FullName");
    if (fullname.FindString("kine.global") != UINT_MAX) {
      KinematicState kineState(object);
      X3DObject x3dobject = kineState.GetParent3DObject();
      if (x3dobject.GetType() == "UsdPrimitive") {
        x3dobject.PutParameterValue("Update", CValue(true));
        U2XStage* stage = 
          U2X_PRIMITIVES.Get(x3dobject.GetActivePrimitive().GetObjectID());
        if (stage) {
          /*
          pxr::UsdStageRefPtr usdStage = stage->Get();
          if (usdStage) {
            pxr::UsdGeomXformable xformable(usdStage->GetDefaultPrim());
            if (xformable) {
              MATH::CMatrix4 matrix = kineState.GetTransform().GetMatrix4();
              pxr::GfMatrix4d usdMatrix;
              memcpy((void*)&usdMatrix, (void*)&matrix, 16 * sizeof(double));

              bool resetXformOpExists;
              std::vector<pxr::UsdGeomXformOp> xformOps = xformable.GetOrderedXformOps(&resetXformOpExists);
              if (!xformOps.size()) {
                pxr::UsdGeomXformOp xformOp = xformable.AddTransformOp(pxr::UsdGeomXformOp::PrecisionDouble);
                xformOp.Set(pxr::VtValue(usdMatrix), pxr::UsdTimeCode(CTime().GetTime()));
              }
              else {
                xformOps[0].Set(pxr::VtValue(usdMatrix), pxr::UsdTimeCode(CTime().GetTime()));
              }
            }
          }
          */
        }
        
      }
    }
  }

  return XSI::CStatus::False;
}

SICALLBACK UsdReadSceneOpen_OnEvent(const XSI::CRef& in_ref)
{
  U2X_PRIMITIVES.ClearCache();
  return XSI::CStatus::False;
}

SICALLBACK UsdReadObjectAdded_OnEvent(const XSI::CRef& in_ref)
{
  return CStatus::False;
}

SICALLBACK UsdReadObjectRemoved_OnEvent(const XSI::CRef& in_ref)
{
  Context ctxt(in_ref);
  U2X_PRIMITIVES.CheckCache();
  return CStatus::False;
}

SICALLBACK UsdReadNewScene_OnEvent(const XSI::CRef& in_ref)
{
  U2X_PRIMITIVES.ClearCache();
  for (auto& ui : U2X_UIS) {
    ui->ClearStage();
  }
  return CStatus::False;
}

SICALLBACK UsdReadTimeChange_OnEvent(const XSI::CRef& in_ref)
{
  /*
  LOG("USD READ TIME CHANGE EVENT!!!");
  Model sceneRoot = Application().GetActiveSceneRoot();
  CSIObjectRefArray lightsRef = sceneRoot.FindObjects(XSI::siLightID);

  LOG("FOUND " + CString(lightsRef.GetCount()) + " LIGHTS :D!!");
  */
  return CStatus::False;
}