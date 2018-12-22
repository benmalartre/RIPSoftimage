/*--------------------------------------------------------------------
	IROffsetHandle.h
---------------------------------------------------------------------*/
#ifndef IR_OFFSET_HANDLE_H
#define IR_OFFSET_HANDLE_H

#include "IRToolHandle.h"
namespace ICERIG {
	class IROffsetHandle : public IRToolHandle
	{
	public:
		IROffsetHandle() {};
		~IROffsetHandle() {};
		IROffsetHandle(X3DObject& object, IRToolMode mode);

		void Draw(ToolContext &in_ctxt);
		void DrawAxis(ToolContext& in_ctxt, IRAxis axis);
		void DrawOffsets(ToolContext& in_ctxt);
		void MouseMove(ToolContext& in_ctxt);
		void MouseDown(ToolContext& in_ctxt);
		void MouseDrag(ToolContext& in_ctxt);
		void MouseUp(ToolContext& in_ctxt);
		CString GetHandleDescription() const { return L"Scale, <Shift> for Uniform"; }


	protected:
		CStatus InitializeArcball(ToolContext &in_ctxt, LONG x, LONG y, const CVector3 &center, double in_dRadius);
		LONG GetClosestArcballAxis(ToolContext &in_ctxt, LONG x, LONG y, LONG numConstraints, const CVector3 *constraintAxes);
		CRotation GetArcballRotation(ToolContext &in_ctxt, LONG x, LONG y, const CVector3 *constraintAxis = NULL);
		CRotation GetNearestRotationIncrement(const CRotation &in_rot);

	private:
		LONG			m_ballDownX, m_ballDownY;
		CVector3		m_ballCenter;
		double 			m_dBallRadius;
		LONG			m_startX;
		LONG			m_startY;
	};
} // ICERIG
#endif

