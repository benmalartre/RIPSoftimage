#include "event.h"
#include "scene.h"
#include "utils.h"

extern U2IScene*            U2I_SCENE;

SICALLBACK UsdICEValueChange_OnEvent(const XSI::CRef& in_ref)
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

SICALLBACK UsdICESceneOpen_OnEvent(const XSI::CRef& in_ref)
{
  delete(U2I_SCENE);
  U2I_SCENE = new U2IScene();
  return CStatus::False;
}

SICALLBACK UsdICEObjectAdded_OnEvent(const XSI::CRef& in_ref)
{
  return CStatus::False;
}

SICALLBACK UsdICEObjectRemoved_OnEvent(const XSI::CRef& in_ref)
{
  Context ctxt(in_ref);
  U2I_SCENE->SyncStagesCache();
  return CStatus::False;
}

SICALLBACK UsdICENewScene_OnEvent(const XSI::CRef& in_ref)
{
  delete(U2I_SCENE);
  U2I_SCENE = new U2IScene();
 
  return CStatus::False;
}

SICALLBACK UsdICETimeChange_OnEvent(const XSI::CRef& in_ref)
{
  /*
  LOG("USD READ TIME CHANGE EVENT!!!");
  Model sceneRoot = Application().GetActiveSceneRoot();
  CSIObjectRefArray lightsRef = sceneRoot.FindObjects(XSI::siLightID);

  LOG("FOUND " + CString(lightsRef.GetCount()) + " LIGHTS :D!!");
  */
  return CStatus::False;
}