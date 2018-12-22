/*--------------------------------------------------------------------
IRSkeletonHandle.h
---------------------------------------------------------------------*/
#ifndef IR_SKELETON_HANDLE_H
#define IR_SKELETON_HANDLE_H

#include "IRToolHandle.h"

namespace ICERIG {
	class IRSkeletonHandle : public IRToolHandle
	{
	public:
		IRSkeletonHandle() {};
		~IRSkeletonHandle() {};
		IRSkeletonHandle(X3DObject& object, IRToolMode mode);

		void Draw(ToolContext &in_ctxt);
		void MouseMove(ToolContext& in_ctxt);
		void MouseDown(ToolContext& in_ctxt);
		void MouseDrag(ToolContext& in_ctxt);
		void MouseUp(ToolContext& in_ctxt);
		CString GetHandleDescription() const { return L"Scale, <Shift> for Uniform"; }

	protected:
		void DrawHandle(ToolContext& in_ctxt, CMatrix4& view);
		void DrawResolutionPlane(ToolContext& in_ctxt);
	private:
		float	_axissize;
	};
} // ICERIG
#endif
