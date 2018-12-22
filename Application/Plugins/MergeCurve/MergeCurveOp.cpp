/*

		MergeCurve Operator.cpp

 */


#include <xsi_ref.h>
#include <xsi_status.h>
#include <xsi_application.h>
#include <xsi_context.h>
#include <xsi_customoperator.h>
#include <xsi_customproperty.h>
#include <xsi_factory.h>
#include <xsi_kinematicstate.h>
#include <xsi_nurbscurve.h>
#include <xsi_nurbscurvelist.h>
#include <xsi_operatorcontext.h>
#include <xsi_parameter.h>
#include <xsi_point.h>
#include <xsi_ppgeventcontext.h>
#include <xsi_ppgitem.h>
#include <xsi_ppglayout.h>
#include <xsi_primitive.h>
#include <xsi_string.h>
#include <xsi_transformation.h>
#include <xsi_value.h>
#include <xsi_vector3.h>
#include <xsi_ppglayout.h>


using namespace XSI;
using namespace MATH;

/*
  	points = []
    ncp = []
    kn = []
    nkn = []
    closed = []
    degree = []

    for curve in curves:

        curve_matrix = curve.Kinematics.Global.Transform.Matrix4

        for nurbscrv in curve.ActivePrimitive.Geometry.Curves:

            ncp.append(nurbscrv.ControlPoints.Count)
            kn.extend(nurbscrv.Knots.Array)
            nkn.append(len(nurbscrv.Knots.Array))
            closed.append(isClosed(nurbscrv))
            degree.append(nurbscrv.Degree)

            for point in nurbscrv.ControlPoints:

                point_pos = point.Position
                point_pos.MulByMatrix4InPlace(curve_matrix)

                points.extend([point_pos.X, point_pos.Y,point_pos.Z, 1])

    if len(ncp) > 1:
        curve = addCurve2(xsi.ActiveSceneRoot, "curve", points, ncp, kn, nkn, closed, degree)
    else:
        curve = addCurve(xsi.ActiveSceneRoot, "curve", points, closed[0], degree[0])

 */



SICALLBACK MergeCurveOp_Define(CRef& in_ctxt)
{
	Context ctxt(in_ctxt);
	CustomOperator op ( ctxt.GetSource() );
	Parameter param;
	/*
	Property prop = op.GetPPGLayout().getP;
	op.AddGridData();
	op.AddParameter(factory.CreateParamDef(L"CurveList", CValue::siString, false), param);
	*/
	return CStatus::OK;
}

SICALLBACK MergeCurveOp_DefineLayout(CRef& in_ctxt)
{
	
	Context ctxt ( in_ctxt );
	PPGLayout layout ( ctxt.GetSource() );
	PPGItem item;

	layout.Clear();


	item = layout.AddItem(L"Mute", L"mute", siControlCheck);

	item = layout.AddGroup(L"curve controls");
	layout.AddButton("AddCurve","Add");
	layout.AddButton("RemoveCurve","Remove");
	/*
	item = layout.AddItem(L"reverse", L"reverse", siControlCheck);
	item = layout.AddItem(L"start", L"start", siControlNumber);
	item = layout.AddItem(L"end", L"end", siControlNumber);
	*/
	layout.EndGroup();
	

	return CStatus::OK;
}


SICALLBACK MergeCurveOp_PPGEvent(const CRef& in_ctxt)
{
	Application app;
	PPGEventContext ctxt ( in_ctxt );
	PPGEventContext::PPGEvent event ( ctxt.GetEventID() );

	if (event == PPGEventContext::siOnInit)
	{
		CustomProperty prop ( ctxt.GetSource() );
	}
	else if (event == PPGEventContext::siButtonClicked)
	{
		CustomProperty prop ( ctxt.GetSource() );
		CString button ( ctxt.GetAttribute(L"Button") );
		app.LogMessage(button.GetAsciiString());
	}
	else if (event == PPGEventContext::siParameterChange)
	{
		Parameter param ( ctxt.GetSource() );
	}
	else if (event == PPGEventContext::siTabChange)
	{
		CustomProperty prop ( ctxt.GetSource() );
		CString tab ( ctxt.GetAttribute(L"Tab") );
	}
	else if (event == PPGEventContext::siOnClosed)
	{
		CustomProperty prop ( ctxt.GetSource() );
	}

	return CStatus::OK;
}


SICALLBACK MergeCurveOp_Update(CRef& in_ctxt)
{
	OperatorContext ctxt ( in_ctxt );
	CustomOperator op(ctxt.GetSource());
	CNurbsCurveDataArray outDataArray;
	CVector3 tmp3;

	//NbCurve Connected
	ULONG nbCurves = op.GetNumPortsInGroup(0);
	for(ULONG i=0;i<nbCurves;i++)
	{
		// Group 0: curves primitives
		Primitive in_crvPrim ( ctxt.GetInputValue(i, 0, 0) );
		NurbsCurveList crvList(in_crvPrim.GetGeometry());

		// Group 1: curves kinematics
		KinematicState in_crvKine ( ctxt.GetInputValue(i, 1, 0) );
		CTransformation tra(in_crvKine.GetTransform());

		CNurbsCurveRefArray crvArray = crvList.GetCurves();

		for(LONG j=0;j<crvArray.GetCount();j++)
		{
			NurbsCurve crv = crvArray[j];
			CNurbsCurveData crvData;
			crv.Get(siSINurbs,crvData);
			CVector4 tmp4;
			for(LONG k=0;k<crvData.m_aControlPoints.GetCount();k++)
			{
				tmp4 = crvData.m_aControlPoints[k];
				tmp3.Set(tmp4.GetX(),tmp4.GetY(),tmp4.GetZ());
				tmp3.MulByTransformationInPlace(tra);
				crvData.m_aControlPoints[k].Set(tmp3.GetX(),tmp3.GetY(),tmp3.GetZ(),tmp4.GetW());

			}

			outDataArray.Add(crvData);
		}
	}

	// Output Curve List
	Primitive out_crv ( ctxt.GetOutputTarget() );
	NurbsCurveList outCrvList(out_crv.GetGeometry());
	return outCrvList.Set(outDataArray, siSINurbs);
}
