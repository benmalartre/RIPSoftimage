/*--------------------------------------------------------------------
	IRMoveHandle.h
---------------------------------------------------------------------*/
#ifndef IR_MOVE_HANDLE_H
#define IR_MOVE_HANDLE_H

#include "IRToolHandle.h"

namespace ICERIG {
  class IRMoveHandle : public IRToolHandle
  {
  public:
    IRMoveHandle() {};
    ~IRMoveHandle() {};
    IRMoveHandle(X3DObject& object, IRToolMode mode);

    void Draw(ToolContext &in_ctxt);
    void MouseMove(ToolContext& in_ctxt);
    void MouseDown(ToolContext& in_ctxt);
    void MouseDrag(ToolContext& in_ctxt);
    void MouseUp(ToolContext& in_ctxt);
    CString GetHandleDescription() const { return L"Scale, <Shift> for Uniform"; }

  protected:
    void DrawAxis(ToolContext& in_ctxt, IRAxis axis);

  private:
    float _axissize;
  };
} // ICERIG
#endif
