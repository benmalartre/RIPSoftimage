#include "event.h"
#include "scene.h"
#include "utils.h"
#include "window.h"

extern U2XScene*            U2X_SCENE;

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
       
        
      }
    }
  }

  return XSI::CStatus::False;
}

SICALLBACK UsdReadSceneOpen_OnEvent(const XSI::CRef& in_ref)
{
  if(U2X_SCENE)delete(U2X_SCENE);
  U2X_SCENE = new U2XScene();
  return CStatus::False;
}

SICALLBACK UsdReadObjectAdded_OnEvent(const XSI::CRef& in_ref)
{
  return CStatus::False;
}

SICALLBACK UsdReadObjectRemoved_OnEvent(const XSI::CRef& in_ref)
{
  Context ctxt(in_ref);
  U2X_SCENE->SyncStagesCache();
  return CStatus::False;
}

SICALLBACK UsdReadNewScene_OnEvent(const XSI::CRef& in_ref)
{
  delete(U2X_SCENE);
  U2X_SCENE = new U2XScene();
 
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