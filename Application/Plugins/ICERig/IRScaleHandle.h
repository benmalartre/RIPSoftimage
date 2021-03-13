/*--------------------------------------------------------------------
	IRScaleHandle.h
---------------------------------------------------------------------*/
#ifndef IR_SCALE_HANDLE
#define IR_SCALE_HANDLE

#include "IRToolHandle.h"

namespace ICERIG {
  class IRScaleHandle : public IRToolHandle 
  {
  public:
    ~IRScaleHandle() {};
    IRScaleHandle() {};
    IRScaleHandle(X3DObject& object, IRToolMode mode);
    void Draw(ToolContext& in_ctxt);
    void MouseDown(ToolContext& in_ctxt);
    void MouseDrag(ToolContext& in_ctxt);
    void MouseUp(ToolContext& in_ctxt);
    CString GetHandleDescription() const { return L"Scale, <Shift> for Uniform"; }

  protected:
    CRefArray     _object;
    CVector3      _pivot;
    CLine         _scaleAxis[4];
    LONG          _activeAxis;
    LONG          _startX, _startY;
  };
}
#endif
