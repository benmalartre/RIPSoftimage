
#include "commands.h"

namespace SOFTIMAGE
{
	// FindObjects
	CValue FindObjects( const CString& in_path, const CString& in_type )
	{
		CValueArray args(2);
		CValue retval;
		LONG i(0);

		args[i++]= in_path;
		args[i++]= in_type;

		CStatus st = Application().ExecuteCommand( L"FindObjects", args, retval );

		return retval;
	}

	// Get_Environment_Variable
	CString Get_Environment_Variable(const CString& oString)
	{
		CComAPIHandler oXSIUtils_cc;
		oXSIUtils_cc.CreateInstance( L"XSI.Utils" );

		CValue oEnvironment = oXSIUtils_cc.GetProperty( L"Environment" );

		CComAPIHandler oEnvironment_cc(oEnvironment);

		CValueArray args(1);
		args[0] = oString;
		CValue oItem;
		oEnvironment_cc.Invoke( L"Item", CComAPIHandler::PropertyGet, oItem, args );

		return oItem;
	}

	// CreateText
	CValue CreateText()
	{
		CValueArray args(2);
		CValue retval;
		LONG i(0);

		args[i++]= L"Text";
		args[i++]= L"NurbsCurve";

		CStatus st = Application().ExecuteCommand( L"CreatePrim", args, retval );

		return retval;
	}

	// MoveOperatorAfter
	void MoveOperatorAfter( const CString& in_object, const CString& in_operatortomove, const CString& in_operatorreference )
	{
		CValueArray args(3);
		CValue retval;
		LONG i(0);

		args[i++]= in_object;
		args[i++]= in_operatortomove;
		args[i++]= in_operatorreference;

		CStatus st = Application().ExecuteCommand( L"MoveOperatorAfter", args, retval );

		return;
	}

	// MoveOperatorBefore
	void MoveOperatorBefore( const CString& in_object, const CString& in_operatortomove, const CString& in_operatorreference )
	{
		CValueArray args(3);
		CValue retval;
		LONG i(0);

		args[i++]= in_object;
		args[i++]= in_operatortomove;
		args[i++]= in_operatorreference;

		CStatus st = Application().ExecuteCommand( L"MoveOperatorBefore", args, retval );

		return;
	}

	// AddImplicit
	X3DObject AddImplicit( const CString& in_name, CString in_type, long color, float size )
	{
		X3DObject out_object;

		Model oRoot = Application().GetActiveSceneRoot();
		oRoot.AddPrimitive(in_type, in_name, out_object);

		if (in_type == L"Sphere") out_object.GetActivePrimitive().GetParameter(L"Radius").PutValue(size);
		if (in_type == L"Cube") out_object.GetActivePrimitive().GetParameter(L"Length").PutValue(size);

		Property oProperty = out_object.GetProperties().GetItem(L"display");
		// changing propagation of the display property
		// MakeLocal erases the correct object reference, so I need to get it again;
		MakeLocal(CValue(oProperty), siNodePropagation);
		oProperty = out_object.GetProperties().GetItem(L"display");

		// changing wireframe color
		Parameter oWireframe_color = oProperty.GetParameter(L"wirecol");
		oWireframe_color.PutValue( color );

		return out_object;
	}

	/*
	siClassifVisualization	= 1,	Parameter affects the UI visualization of an object (ex: level of detail)
	siClassifPositioning	= 2,	Parameter affects the location, size and position of an object (ex: scaling)
	siClassifAppearance	= 4,		Parameter affects the visual appearance of the default representation of an object (ex: color)
	siClassifMetaData	= 8,
	siClassifModel	= 16,			Parameter affects the mathematical model of an object (ex: interpolation change of a surface)
	siClassifTopo	= 16481,		Generator operators
	siClassifGeometry	= 16449,	Deformer operators
	siClassifUnknown	= 32768		Default classification for normal parameters.
	*/
	// CreateParamDef
	CRef CreateParamDef( const CString in_scriptname, CValue::DataType in_datatype,
							const CValue in_classification, const CValue in_capabilities,
							const CString in_paramname, const CString in_description,
							const CValue in_defaultvalue, const CValue in_minvalue, const CValue in_maxvalue,
							const CValue in_suggmin, const CValue in_suggmax )
	{
		CComAPIHandler oXSI_Factory_cc;
		oXSI_Factory_cc.CreateInstance( L"XSI.Factory");

		CValueArray args(11);
		CValue oDef_val;
		LONG i(0);

		args[i++]= in_scriptname;
		args[i++]= in_datatype;

		args[i++]= in_classification;
		args[i++]= in_capabilities;
		args[i++]= in_paramname;
		args[i++]= in_description;

		args[i++]= in_defaultvalue;
		args[i++]= in_minvalue;
		args[i++]= in_maxvalue;
		args[i++]= in_suggmin;
		args[i++]= in_suggmax;

		oXSI_Factory_cc.Call( L"CreateParamDef", oDef_val, args );
		return oDef_val;
	}

	// GetSelectionList
	CValueArray GetSelectionList()
	{ 

		CValueArray o_oObjects;

		CComAPIHandler oApp_cc( Application().GetRef() );
		CValue retval = oApp_cc.GetProperty( L"Selection" );

		CComAPIHandler oSelection_cc(retval);
		CValue retValCollectionItem;
		CValueArray args(1); 

		CValue oObjects_count = oSelection_cc.GetProperty(L"Count");
		long lObjects_count = oObjects_count;

		for(long i = 0; i < lObjects_count; i++)
		{ 
			args[0] = i; 
			oSelection_cc.Invoke(L"Item", CComAPIHandler::PropertyGet, retValCollectionItem, args);

			CComAPIHandler oCollectionItem_cc(retValCollectionItem);
			CValue retValSubComponent = oCollectionItem_cc.GetProperty(L"Subcomponent");

			// check to see if the selection is a selection of subcomponents
			if(retValSubComponent.m_t == CValue::siEmpty)
			{
				// if it's a x3dobject, add it to CRefArray
				o_oObjects.Add(retValCollectionItem);
			}
			else
			{
				Application().LogMessage(L"subcomponent added");
				// add subcomponent to CRefArray
				o_oObjects.Add(retValSubComponent);
			}
		}
		return o_oObjects;
	}

	// GetParamViaCRef
	Parameter GetParamViaCRef( CString i_oParamFullName )
	{
		CRef CRefParam;
		CRefParam.Set( i_oParamFullName );

		Parameter ParamAtFrame(CRefParam);
		return ParamAtFrame;
	}

	// ApplyOp
	void ApplyOp( const CString& in_presetobj, const CString& in_connectionset, const CValue in_connecttype, siOperationMode in_immediatemode, CRefArray& io_obj, const CValue in_constructionmode )
	{
		CValueArray args(6);
		CValue retval;
		LONG i(0);

		args[i++]= in_presetobj;
		args[i++]= in_connectionset;
		args[i++]= in_connecttype;
		args[i++]= (LONG)in_immediatemode;
		args[i++]= io_obj;
		args[i++]= in_constructionmode;

		CStatus st = Application().ExecuteCommand( L"ApplyOp", args, retval );

		io_obj = retval;

		return;
	}

	// DeleteObj
	void DeleteObj( const CValue& in_inputobj )
	{
		CValueArray args(1);
		CValue retval;
		LONG i(0);

		args[i++]= in_inputobj;

		CStatus st = Application().ExecuteCommand( L"DeleteObj", args, retval );

		return;
	}

	// FreezeObj
	void FreezeObj( const CString& in_inputobj )
	{
		Application app;

		CValue retval;
		CValueArray args(1);
		args[0]= in_inputobj;

		CStatus st = app.ExecuteCommand( L"FreezeObj", args, retval );

		return;
	}

	// SelectObj
	CValue SelectObj( const CValue& in_selectionlist, siSelectMode in_hierarchylevel, bool in_checkobjectselectability )
	{
		CValueArray args(3);
		CValue retval;
		LONG i(0);

		args[i++]= in_selectionlist;
		args[i++]= (long)in_hierarchylevel;
		args[i++]= in_checkobjectselectability;

		CStatus st = Application().ExecuteCommand( L"SelectObj", args, retval );

		return retval;
	}


	// InspectObj
	CValue InspectObj( const CString& in_inputobjs, const CString& in_keywords, const CString& in_title, siInspectMode in_mode, bool in_throw )
	{
		CValueArray args(5);
		CValue retval;
		LONG i(0);

		args[i++]= in_inputobjs;
		args[i++]= in_keywords;
		args[i++]= in_title;
		args[i++]= (long)in_mode;
		args[i++]= in_throw;

		CStatus st = Application().ExecuteCommand( L"InspectObj", args, retval );

		return retval;
	}

	// PickElement
	void PickElement( const CString& in_selfilter, const CString& in_leftmessage, const CString& in_middlemessage, CRef& io_pickedelement, CValue& io_buttonpressed, const CValue& in_selregionmode, CValue& io_modifierpressed )
	{
		CValueArray args(7);
		CValue ret;
		CValueArray ret_array;
		LONG i(0);

		args[i++]= in_selfilter;
		args[i++]= in_leftmessage;
		args[i++]= in_middlemessage;
		args[i++]= io_pickedelement;
		args[i++]= io_buttonpressed;
		args[i++]= in_selregionmode;
		args[i++]= io_modifierpressed;

		CStatus st = Application().ExecuteCommand( L"PickElement", args, ret );
		ret_array = ret;
		io_buttonpressed = ret_array[0];	// button
		io_modifierpressed = ret_array[1];	// modifier
		io_pickedelement = ret_array[2];	// obj

		return;
	}

	// MakeLocal
	CValue MakeLocal( const CValue& in_inputobjs, siPropagationType in_propagationtype )
	{
		CValueArray args(2);
		CValue retval;
		long i(0);

		args[i++]= in_inputobjs;
		args[i++]= (long)in_propagationtype;

		CStatus st = Application().ExecuteCommand( L"SIMakeLocal", args, retval );

		return retval;
	}


	// GetMarking
	CValue GetMarking()
	{
		CValueArray args(0);
		CValue retval;
		LONG i(0);

		CStatus st = Application().ExecuteCommand( L"GetMarking", args, retval );

		return retval;
	}

	// GetOperators
	CRefArray GetOperators( X3DObject i_oOps )
	{
		// Given an X3DObject (e.g. a mesh or nurbs), fills in an array with all the operators connected to its
		// primitive.  This function is suitable for re-use in any plug-in that wants to access construction history
		Primitive primitive = i_oOps.GetActivePrimitive();
		CComAPIHandler comPrimitive( primitive.GetRef() );

		CComAPIHandler constructionHistory = comPrimitive.GetProperty( L"ConstructionHistory" );

		// Currently there isn't a "Count" or "Item" property on the ConstructionHistory 
		// scripting object, so we use Filter to find all operators
		CValue valOperatorCollection  ;
		CValueArray args(3);
		args[1] = CValue( L"Operators" );   // We want all operators (siOperatorFamily)
		constructionHistory.Call( L"Filter", valOperatorCollection, args );


		// Now convert from a OperatorCollection object to a C++ CRefArray
		CComAPIHandler comOpColl = valOperatorCollection ;
		CValue cnt = comOpColl.GetProperty( L"Count" );

		CRefArray ops( (long)cnt );

		for ( long i=0 ; i<(long)cnt; i++ )
		{
			CValue outOp;
			CValueArray itemsArgs;
			itemsArgs.Add( i );
			comOpColl.Invoke(L"Item", CComAPIHandler::PropertyGet, outOp, itemsArgs);

			ops[i] = outOp ;
		}

		return ops ;
	}

	// GetX3DObjectFromSubComponent
	X3DObject GetX3DObjectFromSubComponent( SubComponent& i_oSubComponent )
	{
		X3DObject oX3Dobject;
		CRefArray oNesteds;
		Parameter oNested;
		CString oNested_name, oObj_fullname;
		CRef oRef;

		oNesteds = i_oSubComponent.GetNestedObjects();
		oNested = oNesteds.GetItem(0);
		oNested_name = oNested.GetFullName();
		oObj_fullname = L"";
		for (int i = 0; i < (long)oNested_name.Length() - 5; i++)
		{
			oObj_fullname += oNested_name[i];
		}

		oRef.Set(oObj_fullname);
		oX3Dobject = oRef;
		return oX3Dobject;
	}

	// GetComponentCollectionFromSubComponent
	CLongArray GetIndicesFromSubComponent( CValue& i_oSubComponent )
	{
		CLongArray o_oIndices;
		CValueArray args(1);

		CComAPIHandler oSubComponent_cc( i_oSubComponent );
		CValue componentCollection = oSubComponent_cc.GetProperty(L"ComponentCollection");

		CComAPIHandler oTaggedComponentCollection_cc(componentCollection);
		CValue oSub_count = oTaggedComponentCollection_cc.GetProperty(L"Count");
		long lSub_count = oSub_count;

		// add to the COM component collection to the CRefArray
		CValue oTaggedComponent, oIndex;
		for(long i = 0; i < lSub_count; i++)
		{
			args[0] = i;
			oTaggedComponentCollection_cc.Invoke(L"Item", CComAPIHandler::PropertyGet, oTaggedComponent, args);

			CComAPIHandler oTaggedComponent_cc(oTaggedComponent);
			oIndex = oTaggedComponent_cc.GetProperty(L"Index");

			o_oIndices.Add(oIndex);
		}

		return o_oIndices;
	}

	// FindObjectsByMarkingAndCapabilities
	CParameterRefArray FindObjectsByMarkingAndCapabilities( CRefArray& oRefArray, CValueArray& oMarkeds )
	{
		CParameterRefArray oParameters;
		long lMarkeds = oMarkeds.GetCount(), i = 0;
		if ( (oRefArray.GetCount() > 0) && (lMarkeds > 0) )
		{
			CValue oMarked;
			CString oMarkeds_str = L"";
			for ( i = 0; i < lMarkeds; i++ )
			{
				oMarked = oMarkeds[i];
				oMarkeds_str += oMarked.GetAsText();
				if (i < lMarkeds - 1)
					oMarkeds_str += L",";
			}

			CValueArray args;
			CComAPIHandler oColl_cc;
			CValue oColl_val;

			oColl_cc.CreateInstance( L"XSI.Collection" );

			args.Clear();
			args.Add( oRefArray );
			oColl_cc.Invoke( L"AddItems", CComAPIHandler::Method, oColl_val, args );

			args.Clear();
			args.Add( oMarkeds_str );
			args.Add( (long)XSI::siAnimatable );
			oColl_cc.Invoke( L"FindObjectsByMarkingAndCapabilities", CComAPIHandler::Method, oColl_val, args );

			oColl_cc = oColl_val;
			long lColl = oColl_cc.GetProperty(L"Count");

			Parameter oParameter;
			CValue oParameter_val;
			args.Clear();
			args.Resize(1);

			for( i = 0; i < lColl; i++ )
			{
				args[0] = i;
				oColl_cc.Invoke(L"Item", CComAPIHandler::PropertyGet, oParameter_val, args);
				oParameters.Add(oParameter_val);
			}
		}

		return oParameters;
	}

	//	CreateWeightMap
	//--------------------------------------------
	ClusterProperty CreateWeightMap(CRef& inTarget,CString wmName)
	{
		ClusterProperty wm;
		switch( inTarget.GetClassID() ) 
		{
			case siX3DObjectID:
				{
				CValueArray ar1 = CreateWeightMap( L"", X3DObject(inTarget).GetFullName(), wmName, L"", false );
				wm = ar1[0];
				break;
				}

			case siClusterID:
				{
				CValueArray ar1 = CreateWeightMap( L"", Cluster(inTarget).GetFullName(), wmName, L"", false );
				wm = ar1[0];
				break;
				}
		}
		return wm;
	}

	//	GetPositionFromObjects
	//--------------------------------------------
	CVector3Array GetPositionFromObjects(CRefArray& in_objects)
	{
		CVector3Array outPos;
		CVector3 pos;
		X3DObject temp;
		for(long a=0;a<in_objects.GetCount();a++)
		{
			temp = in_objects[a];
			pos = temp.GetKinematics().GetGlobal().GetTransform().GetTranslation();
			outPos.Add(pos);
		}
		return outPos;
	}


	//	GetCurvePointPositionFromObjects
	//--------------------------------------------
	CVector4Array GetCurvePointPositionFromObjects(CRefArray& in_objects)
	{
		CVector4Array outPos;
		CVector3 pos3;
		CVector4 pos4;
		X3DObject temp;
		for(long a=0;a<in_objects.GetCount();a++)
		{
			temp = in_objects[a];
			pos3 = temp.GetKinematics().GetGlobal().GetTransform().GetTranslation();
			pos4.Set(pos3.GetX(),pos3.GetY(),pos3.GetZ(),1.0);
			outPos.Add(pos4);
		}
		return outPos;
	}

	//	CreateCurveFromObjects
	//--------------------------------------------
	X3DObject CreateCurveFromObjects(CRefArray& in_objects,bool in_degree, bool in_constraint, bool in_close, long in_color, CString in_name)
	{
		X3DObject outCurve;
		CVector4Array oPos = GetCurvePointPositionFromObjects(in_objects);

		CNurbsCurveData datas;
		datas.m_aControlPoints = oPos;
		datas.m_bClosed = in_close;
		if(in_degree == false) datas.m_lDegree = 1;
		else datas.m_lDegree = 3;

		Model oRoot = Application().GetActiveSceneRoot();
		oRoot.AddNurbsCurve(datas,siSINurbs,in_name,outCurve);

		Property oProperty = outCurve.GetProperties().GetItem(L"display");
		// changing propagation of the display property
		// MakeLocal erases the correct object reference, so I need to get it again;
		MakeLocal(CValue(oProperty), siNodePropagation);
		oProperty = outCurve.GetProperties().GetItem(L"display");

		// changing rgb colors
		oProperty.PutParameterValue(L"wirecol",in_color);

		if(in_constraint == true)
		{
			Geometry crvGeom = outCurve.GetActivePrimitive().GetGeometry();
			long pNb = crvGeom.GetPoints().GetCount();
			CLongArray index;
			index.Resize(1);
			for(long c=0;c<pNb;c++)
			{
				Cluster oCluster;
				index[0] = c;
				crvGeom.AddCluster(siVertexCluster,L"",index,oCluster);
				CRefArray outOp;
				ApplyOp (L"ClusterCenter", oCluster.GetFullName() +L";"+ in_objects[c].GetAsText() , 0,siPersistentOperation,outOp,2);
			}
		}

		return outCurve;
	}

	//	AddChild
	//--------------------------------------------
	void AddChild(X3DObject& in_parent, X3DObject& in_child)
	{
		CRefArray children;
		children.Add(in_child);
		in_parent.AddChild(children);
	}

	// AddToGroup
	//---------------------------------------------
	void AddToGroup(Model& in_model,CRefArray& in_objects,CString group_name)
	{
		CRefArray oGroups = in_model.GetGroups(); 
		CRef oGroupRef = oGroups.GetItem(group_name);
		Group oGroup;

		if(oGroupRef.IsValid())
		{
			oGroup = oGroupRef;
			oGroup.AddMembers(in_objects,false);
		}
		else 
		{
			in_model.AddGroup(in_objects,group_name,false,oGroup);
		}
	}
} // namespace SOFTIAMGE