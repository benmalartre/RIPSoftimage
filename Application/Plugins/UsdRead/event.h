#pragma once

#include <sicppsdk.h>
#include <xsi_ref.h>
#include <xsi_status.h>

SICALLBACK UsdReadSceneOpen_OnEvent(const XSI::CRef& in_ref);
SICALLBACK UsdReadObjectAdded_OnEvent(const XSI::CRef& in_ref);
SICALLBACK UsdReadObjectRemoved_OnEvent(const XSI::CRef& in_ref);
SICALLBACK UsdReadNewScene_OnEvent(const XSI::CRef& in_ref);
SICALLBACK UsdReadTimeChange_OnEvent(const XSI::CRef& in_ref);