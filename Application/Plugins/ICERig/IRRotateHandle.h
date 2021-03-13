/*--------------------------------------------------------------------
	IRRotateHandle.h
---------------------------------------------------------------------*/
#ifndef IR_ROTATE_HANDLE
#define IR_ROTATE_HANDLE

#include "IRToolHandle.h"

namespace ICERIG {
  class IRRotateHandle : public IRToolHandle
  {
  public:
    IRRotateHandle() {};
    ~IRRotateHandle() {};
    IRRotateHandle(X3DObject& object, IRToolMode mode);

    void Draw(ToolContext& in_ctxt);
    void MouseMove(ToolContext& in_ctxt);
    void MouseDown(ToolContext& in_ctxt);
    void MouseDrag(ToolContext& in_ctxt);
    void MouseUp(ToolContext& in_ctxt);
    CString GetHandleDescription() const { return L"Scale, <Shift> for Uniform"; }


  protected:
    CStatus InitializeArcball(ToolContext& in_ctxt, LONG x, LONG y, const CVector3& center, double in_dRadius);
    LONG GetClosestArcballAxis(ToolContext& in_ctxt, LONG x, LONG y, LONG numConstraints, const CVector3* constraintAxes);
    CRotation GetArcballRotation(ToolContext& in_ctxt, LONG x, LONG y, const CVector3* constraintAxis = NULL);
    CRotation GetNearestRotationIncrement(const CRotation& in_rot);

  private:
    LONG			m_ballDownX, m_ballDownY;
    CVector3		m_ballCenter;
    double 			m_dBallRadius;
    LONG			m_startX;
    LONG			m_startY;
  };
}
#endif

