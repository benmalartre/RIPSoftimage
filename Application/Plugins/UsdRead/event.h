#pragma once

#include <sicppsdk.h>
#include <xsi_ref.h>
#include <xsi_status.h>
#include <vector>

class U2XWindow;
extern std::vector<U2XWindow*> U2X_UIS;

SICALLBACK UsdReadValueChange_OnEvent(const XSI::CRef& in_ref);
SICALLBACK UsdReadSceneOpen_OnEvent(const XSI::CRef& in_ref);
SICALLBACK UsdReadObjectAdded_OnEvent(const XSI::CRef& in_ref);
SICALLBACK UsdReadObjectRemoved_OnEvent(const XSI::CRef& in_ref);
SICALLBACK UsdReadNewScene_OnEvent(const XSI::CRef& in_ref);
SICALLBACK UsdReadTimeChange_OnEvent(const XSI::CRef& in_ref);