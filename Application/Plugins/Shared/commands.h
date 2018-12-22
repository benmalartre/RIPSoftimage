
#ifndef FLAG_COMMANDS
#define FLAG_COMMANDS

#include "xsi_common.h"
#include "xsi_3d.h"
#include "constants.h"
#include "functions.h"
#include "string.h"

namespace SOFTIMAGE {
	CValue FindObjects( const CString& in_path, const CString& in_type );
	CString b_Get_Environment_Variable(const CString& oString);
	CValue CreateText();
	void MoveOperatorAfter( const CString& in_object, const CString& in_operatortomove, const CString& in_operatorreference );
	void MoveOperatorBefore( const CString& in_object, const CString& in_operatortomove, const CString& in_operatorreference );
	X3DObject b_AddImplicit( const CString& in_name, CString in_type, long color, float size );
	CRef b_CreateParamDef( const CString in_scriptname, CValue::DataType in_datatype, const CValue in_classification, const CValue in_capabilities, const CString in_paramname, const CString in_description, const CValue in_defaultvalue, const CValue in_minvalue, const CValue in_maxvalue, const CValue in_suggmin, const CValue in_suggmax );
	CValueArray b_GetSelectionList();
	Parameter GetParamViaCRef( CString i_oParamFullName );
	void ApplyOp( const CString& in_presetobj, const CString& in_connectionset, const CValue in_connecttype, siOperationMode in_immediatemode, CRefArray& io_obj, const CValue in_constructionmode );
	void DeleteObj( const CValue& in_inputobj );
	void FreezeObj( const CString& in_inputobj );
	CValue SelectObj( const CValue& in_selectionlist, siSelectMode in_hierarchylevel, bool in_checkobjectselectability );
	CValue InspectObj( const CString& in_inputobjs, const CString& in_keywords, const CString& in_title, siInspectMode in_mode, bool in_throw );
	void PickElement( const CString& in_selfilter, const CString& in_leftmessage, const CString& in_middlemessage, CRef& io_pickedelement, CValue& io_buttonpressed, const CValue& in_selregionmode, CValue& io_modifierpressed );
	CValue MakeLocal( const CValue& in_inputobjs, siPropagationType in_propagationtype );
	CValue GetMarking();
	CRefArray GetOperators( X3DObject i_oOps );
	X3DObject GetX3DObjectFromSubComponent( SubComponent& i_oSubComponent );
	CLongArray GetIndicesFromSubComponent( CValue& i_oSubComponent );
	CParameterRefArray FindObjectsByMarkingAndCapabilities( CRefArray& oRefArray, CValueArray& oMarkeds );
	ClusterProperty b_CreateWeightMap(CRef& inTarget,const CString wmName);

	X3DObject b_CreateCurveFromObjects(CRefArray& in_objects,bool in_degree, bool in_constraint, bool in_close, long in_color, CString in_name);
	CVector3Array b_GetPositionFromObjects(CRefArray& in_objects);
	CVector4Array b_GetCurvePointPositionFromObjects(CRefArray& in_objects);

	void b_AddToGroup(Model& in_model,CRefArray& in_objects,CString group_name);
	void b_AddChild(X3DObject& in_parent, X3DObject& in_child);

} // namespace bSOFTIMAGE

#endif // B_FLAG_COMMANDS