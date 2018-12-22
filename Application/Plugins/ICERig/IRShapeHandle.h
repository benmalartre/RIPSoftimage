/*--------------------------------------------------------------------
	IRShapeHandle.h
---------------------------------------------------------------------*/
#ifndef IR_SHAPE_HANDLE_H
#define IR_SHAPE_HANDLE_H

#include "IRToolHandle.h"

namespace ICERIG {
	class IRShapeHandle : public IRToolHandle {
	public: // Methods
		~IRShapeHandle() {};
		IRShapeHandle() {};
		IRShapeHandle(X3DObject& object, IRToolMode mode);
		void Draw(ToolContext &in_ctxt);
		void MouseMove(ToolContext& in_ctxt);
		void MouseDown(ToolContext& in_ctxt);
		void MouseDrag(ToolContext& in_ctxt);
		void MouseUp(ToolContext& in_ctxt);
		CString GetHandleDescription() const { return L"Scale, <Shift> for Uniform"; }

		void DrawSection(int id, CMatrix4& view, bool selected);
		void DrawAxis(CMatrix4& view);

	protected:
		void GetAxis(ToolContext &in_ctxt);
		void GetActiveAxis(ToolContext& in_ctxt);

	private: // Data
		CVector3			m_pivot;
		LONG				m_activeAxis;
		LONG				m_startX, m_startY;
		CVector3			m_initpos;
		float				m_initwidth;
		float				m_initdepth;
		float				m_initlength;
		float				m_diff;
	};
} // ICERIG
#endif
