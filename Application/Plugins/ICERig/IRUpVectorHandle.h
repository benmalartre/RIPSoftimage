/*--------------------------------------------------------------------
	IRUpVectorHandle.h
---------------------------------------------------------------------*/
#ifndef IR_UPVECTOR_HANDLE_H
#define IR_UPVECTOR_HANDLE_H

#include "IRToolHandle.h"

namespace ICERIG {
	class IRUpVectorHandle : public IRToolHandle
	{
	public:
		IRUpVectorHandle() {};
		~IRUpVectorHandle() {};
		IRUpVectorHandle(X3DObject& object, IRToolMode mode);

		void Draw(ToolContext &in_ctxt);
		void MouseMove(ToolContext& in_ctxt);
		void MouseDown(ToolContext& in_ctxt);
		void MouseDrag(ToolContext& in_ctxt);
		void MouseUp(ToolContext& in_ctxt);
		CString GetHandleDescription() const { return L"HUHUHU"; }

	protected:
		void DrawAxis(ToolContext& in_ctxt, IRAxis axis);

	private:
		float m_axissize;
	};
} // ICERIG
#endif
