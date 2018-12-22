/*--------------------------------------------------------------------
	IRTranslateHandle.h
---------------------------------------------------------------------*/
#ifndef IR_TRANSLATE_HANDLE
#define IR_TRANSLATE_HANDLE

#include "IRToolHandle.h"

class IRTranslateHandle : public IRToolHandle 
{
public: 
	IRTranslateHandle(){};
	~IRTranslateHandle(){};
	IRTranslateHandle( X3DObject& object, IRToolMode mode );
	
	void Draw( ToolContext &in_ctxt );
	void MouseMove(ToolContext& in_ctxt);
	void MouseDown( ToolContext& in_ctxt );
	void MouseDrag( ToolContext& in_ctxt );
	void MouseUp( ToolContext& in_ctxt );
	CString GetHandleDescription() const { return L"Scale, <Shift> for Uniform"; }

protected:
	void DrawAxis(ToolContext& in_ctxt,IRAxis axis);
	void GetAxis();
	void GetActiveAxis(ToolContext& in_ctxt);
	CStatus GetClosestPositionOnAxis(ToolContext& in_ctxt,LONG x, LONG y,CVector3& io_pos);

private:  
	LONG			_startX;
	LONG			_startY;
	CPlane			_plane;
	CTransformation _tra;
	float			_axissize;
	IRAxis			_activeaxis;
	CLine			_axis[3];
	CVector3		_origin;
	CVector3		_initpos;
};

#endif
