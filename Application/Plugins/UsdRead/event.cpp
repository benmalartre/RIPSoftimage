#include "event.h"
#include "scene.h"
#include "utils.h"

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