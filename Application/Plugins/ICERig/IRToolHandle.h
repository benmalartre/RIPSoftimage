/*--------------------------------------------------------------------
	IRToolHandle.h
---------------------------------------------------------------------*/
#ifndef IR_TOOL_HANDLE_H
#define IR_TOOL_HANDLE_H

#include "IRRegister.h"
#include "IRElement.h"

namespace ICERIG {
  class IRToolHandle {
  public:
    IRToolHandle(); // Abstract base class
    ~IRToolHandle();

    // Tool callbacks
    virtual void Setup(IRElement*, IRToolMode mode);
    virtual void Cleanup() {}
    virtual void Draw(ToolContext &in_ctxt) {}
    virtual void MouseMove(ToolContext& in_ctxt) {}
    virtual void MouseDown(ToolContext& in_ctxt) {}
    virtual void MouseDrag(ToolContext& in_ctxt) {}
    virtual void MouseUp(ToolContext& in_ctxt) {}

    // Data access
    IRToolMode GetToolMode() const { return m_mode; }

  protected:      // Methods
    double GetViewScaleFactor(ToolContext &in_ctxt, const CVector3 &pos, double viewSize) const;
    LONG GetClosestDragAxis(ToolContext &in_ctxt, const CVector3 &mouse, CLine axes[], double &out_projectedlength) const;
    double GetClosestIncrement(double in_value, double in_increment) const;

    void IRToolHandle::GetAxis(bool global = false);
    void IRToolHandle::GetActiveAxis(ToolContext& in_ctxt);
    CStatus GetClosestPositionOnAxis(ToolContext& in_ctxt, LONG x, LONG y, CVector3& io_pos);

    X3DObject& GetCurve();

  protected:                    // Data
    IRElement*  m_elem;         // Element
    IRToolMode  m_mode;         // Tool Mode
    bool        m_valid;
    int         m_nbchildren;   // Nb Children
    CTransformation m_xform;    // Handle Transform
    CPlane      m_plane;        // Handle Plane
    IRAxis      m_activeaxis;
    CLine       m_axis[3];
    CVector3    m_origin;
    CVector3    m_initpos;
    float       m_viewscl;

    LONG        m_startX;
    LONG        m_startY;
    bool        m_down;         // is mouse button down
  };
} // ICERIG
#endif // HANDLE_INCLUDED
