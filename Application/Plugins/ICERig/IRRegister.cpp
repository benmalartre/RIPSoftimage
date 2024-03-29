// ---------------------------------------------------------------------------
//  ICERig Register.cpp
// ---------------------------------------------------------------------------
// Delta Mush Operator from Guillaume Barate
// https://vimeo.com/122981239
// ---------------------------------------------------------------------------
#include "IRRegister.h"
#include "IRDeltaMush.h"
#include "IRCubicBezier.h"
#include "IRLeastSquareFitLine.h"
#include "IRLeastSquareFitPlane.h"

XSI::CStatus RegisterIRSmoothWeights( XSI::PluginRegistrar& in_reg );
XSI::CStatus RegisterIRCubicBezier( XSI::PluginRegistrar& in_reg );
XSI::CStatus RegisterIRCubicBezierGenerator(XSI::PluginRegistrar& in_reg);
XSI::CStatus RegisterIRCubicBezierPoint(XSI::PluginRegistrar& in_reg);
XSI::CStatus RegisterIRBuildElementID( XSI::PluginRegistrar& in_reg );
XSI::CStatus RegisterIRDeltaMushInit(XSI::PluginRegistrar& in_reg);
XSI::CStatus RegisterIRDeltaMush(XSI::PluginRegistrar& in_reg);
XSI::CStatus RegisterIRResampleCurve(XSI::PluginRegistrar& in_reg);
XSI::CStatus RegisterIRFabrik(XSI::PluginRegistrar& in_reg);
XSI::CStatus RegisterIRLeastSquareFitLine(XSI::PluginRegistrar& in_reg);
XSI::CStatus RegisterIRLeastSquareFitPlane(XSI::PluginRegistrar& in_reg);

SICALLBACK XSILoadPlugin( PluginRegistrar& in_reg )
{
  in_reg.PutAuthor(L"benmalartre");
  in_reg.PutName(L"ICERig");
  in_reg.PutVersion(1,0);

  RegisterIRSmoothWeights( in_reg );
  RegisterIRCubicBezier(in_reg);
  RegisterIRCubicBezierGenerator(in_reg);
  RegisterIRCubicBezierPoint(in_reg);
  RegisterIRBuildElementID(in_reg);
  RegisterIRDeltaMushInit(in_reg);
  RegisterIRDeltaMush(in_reg);
  RegisterIRResampleCurve(in_reg);
  RegisterIRFabrik(in_reg);
  RegisterIRLeastSquareFitLine(in_reg);
  RegisterIRLeastSquareFitPlane(in_reg);

  in_reg.RegisterTool(L"IRSetElement");
	in_reg.RegisterTool(L"IRSimpleBrush");
	in_reg.RegisterTool(L"IRManipulator");
	in_reg.RegisterEvent( L"IRSetElement_SelectionChanged", siOnSelectionChange );

  return CStatus::OK;
}

SICALLBACK XSIUnloadPlugin( const PluginRegistrar& in_reg )
{
  CString strPluginName;
  strPluginName = in_reg.GetName();
  Application().LogMessage(strPluginName + L" has been unloaded.",siVerboseMsg);
  return CStatus::OK;
}
