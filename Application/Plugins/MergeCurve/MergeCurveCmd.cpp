#include <xsi_application.h>
#include <xsi_argument.h>
#include <xsi_command.h>
#include <xsi_context.h>
#include <xsi_customoperator.h>
#include <xsi_factory.h>
#include <xsi_kinematics.h>
#include <xsi_kinematicstate.h>
#include <xsi_primitive.h>
#include <xsi_value.h>
#include <xsi_x3dobject.h>
#include <xsi_model.h>

using namespace XSI;


SICALLBACK MergeCurveCmd_Init(CRef& in_ctxt)
{
	Context ctxt ( in_ctxt );
	Command cmd ( ctxt.GetSource() );
	cmd.EnableReturnValue(true);


	// Add some arguments to the command.
	ArgumentArray args ( cmd.GetArguments() );
	args.AddWithHandler(L"in_selection", siArgHandlerCollection);
	args.Add(L"in_silent", false);


	return CStatus::OK;
}


SICALLBACK MergeCurveCmd_Execute(CRef& in_ctxt)
{
	Context ctxt ( in_ctxt );
	Command cmd ( ctxt.GetSource() );

	Application app;
	Model root = app.GetActiveSceneRoot();

	CRefArray return_ops;


	// Retrieve data from the argument list.
	CValueArray args = ctxt.GetAttribute(L"Arguments") ;
	CValueArray in_selection = args[0];
	bool in_silent ( args[1] );

	// Check that the selection contains at least two curves.
	LONG validElem ( 0 );
	for (LONG i = 0; i < in_selection.GetCount(); ++i)
	{
		if (!in_selection[i].IsEmpty())
		{
			X3DObject obj(in_selection[i]);
			if(obj.GetType() == L"crvlist")
				validElem++;
			else
			{
				app.LogMessage(L"[MergeCurve] Selection is invalid. Must be a collection of curves.", siErrorMsg);
				return CStatus::Fail;
			}
		}
	}

	if (validElem<2)
	{
		app.LogMessage(L"[MergeCurve] Selection is invalid. Must be a collection of curves.", siErrorMsg);
		return CStatus::InvalidArgument;
	}

	// Create Out CurveList
	CNurbsCurveDataArray dataArray;
	CString in_name = L"Skeleton_crv";
	X3DObject out_curve;
	root.AddNurbsCurveList (dataArray, siSINurbs, in_name, out_curve);
	Primitive out_prim = out_curve.GetActivePrimitive();

	// Create the operator.
	CustomOperator op ( app.GetFactory().CreateObject(L"MergeCurveOp") );

	// Create Port Groups
	op.AddPortGroup(L"CurvePrimitiveGroup",0,1000,siCurveFilter);
	op.AddPortGroup(L"CurveKinematicsGroup",0,1000,siCurveFilter);
	op.AddPortGroup(L"OutCurveListGroup",1,1,siCurveFilter);

	// Input Ports
	for(LONG i = 0;i<validElem;i++)
	{
		X3DObject obj(in_selection[i]);
		Primitive in_prim(obj.GetActivePrimitive());
		KinematicState in_kine(obj.GetKinematics().GetGlobal());
		op.AddInputPort(in_prim,L"",0);
		op.AddInputPort(in_kine,L"",1);
	}

	// Output Port
	op.AddOutputPort(out_prim.GetRef(), L"Out", 2);

	// Connect Operator
	op.Connect();

	// Inspect the operators created.
	if (!in_silent)
	{
		CValueArray inArgs(5);
		inArgs[0] = op;

		CValue outVal;

		app.ExecuteCommand(L"InspectObj", inArgs, outVal);
	}

	return ctxt.PutAttribute(L"ReturnValue", (CRef)op);
}

SICALLBACK MergeAddCurveCmd_Init(CRef& in_ctxt)
{
	Context ctxt ( in_ctxt );
	Command cmd ( ctxt.GetSource() );
	cmd.EnableReturnValue(true);

	// Add some arguments to the command.
	ArgumentArray args ( cmd.GetArguments() );
	args.AddWithHandler(L"in_collection", siArgHandlerCollection);

	return CStatus::OK;
}

SICALLBACK MergeAddCurveCmd_Execute(CRef& in_ctxt)
{
	Context ctxt ( in_ctxt );
	Command cmd ( ctxt.GetSource() );

	Application app;
	Model root = app.GetActiveSceneRoot();

	// Retrieve data from the argument list.
	CValueArray args = ctxt.GetAttribute(L"Arguments") ;
	CValueArray in_objects = args[0];

	CustomOperator op;

	// Check that the first curve selected contains lmMergeCurveOp
	bool check = false;
	if (!in_objects[0].IsEmpty())
	{
		X3DObject obj(in_objects[0]);
		if(obj.GetType() == L"crvlist")
		{
			CRefArray nested(obj.GetActivePrimitive().GetNestedObjects());
			for(LONG x=0;x<nested.GetCount();x++)
			{
				CRef subop(nested[x]);
				CString subname = subop.GetAsText();
				if(subop.GetAsText().FindString(L"MergeCurveOp",0))
				{
					check = true;
					break;
				}
			}
			if(!check)
			{
				app.LogMessage(L"[MergeAddCurve] First Curve Selected does not contain MergeCurveOp. Failed!", siErrorMsg);
				return CStatus::Fail;
			}
		}
	}
	return CStatus::OK;
}

