/*--------------------------------------------------------------------
	IRMoveHandle.cpp
---------------------------------------------------------------------*/
#include "IRMoveHandle.h"

namespace ICERIG {
  void IRMoveHandle::MouseMove(ToolContext& in_ctxt)
  {
    if (!in_ctxt.IsInteracting())
    {
      LONG x, y;
      in_ctxt.GetMousePosition(x, y);
      CVector3 viewPos(x, y, 0);
      CVector3 worldPos;
      in_ctxt.CameraToWorld(viewPos, worldPos);
      m_xform.SetTranslation(worldPos);

      GetAxis();
      GetActiveAxis(in_ctxt);
    }
  }


  void IRMoveHandle::MouseDown(ToolContext& in_ctxt)
  {
    if (m_elem == NULL)return;

    // Called when mouse button is pressed
    in_ctxt.GetMousePosition(m_startX, m_startY);

    IRJoint* jnt = m_elem->GetActiveJoint();
    if (jnt) {
      in_ctxt.GetViewPlane(m_plane);
      m_plane.SetOrigin(jnt->GetPosition());
    }

    in_ctxt.SetManipulationPlane(m_plane);

    if (GetClosestPositionOnAxis(in_ctxt, m_startX, m_startY, m_initpos) != CStatus::OK)
      return;
  }

  void IRMoveHandle::MouseDrag(ToolContext& in_ctxt)
  {
    if (m_elem == NULL)return;
    IRJoint* jnt = m_elem->GetActiveJoint();
    LONG x, y;
    in_ctxt.GetMousePosition(x, y);
    CVector3 worldPos;
    in_ctxt.GetWorldPosition(x, y, worldPos);
    jnt->SetPosition(worldPos);
    m_elem->SetCurveModified(true);
  }

  void IRMoveHandle::MouseUp(ToolContext& in_ctxt)
  {
    m_elem->UpdateCurve();
  }

  void IRMoveHandle::Draw(ToolContext& in_ctxt)
  {
    // Called when a redraw occurs to allow the tool to show feedback
    if (in_ctxt.IsActiveView())
    {
      in_ctxt.SetCursor(siDragXYCursor);
    }
  }

  void IRMoveHandle::DrawAxis(ToolContext& in_ctxt, IRAxis axis)
  {
    m_viewscl = GetViewScaleFactor(in_ctxt, m_xform.GetTranslation(), (double)IRMoveHandleSize);

    if (axis == m_activeaxis)
      SetGLColor(IRSelectedHandleColor);
    else
    {
      switch (axis)
      {
      case IRAxisX:
        SetGLColor(IRXHandleColor);
        break;
      case IRAxisY:
        SetGLColor(IRYHandleColor);
        break;
      case IRAxisZ:
        SetGLColor(IRZHandleColor);
        break;
      }
    }
    CVector3 a;
    switch (axis)
    {
    case IRAxisX:
      a.Set(m_viewscl, 0.0, 0.0);
      break;
    case IRAxisY:
      a.Set(0.0, m_viewscl, 0.0);
      break;
    case IRAxisZ:
      a.Set(0.0, 0.0, m_viewscl);
      break;
    }

    CMatrix4 vm, tm;
    in_ctxt.GetWorldToViewMatrix(vm);
    tm = m_xform.GetMatrix4();
    tm.MulInPlace(vm);

    GLdouble mat[16];
    tm.Get(
      mat[0], mat[1], mat[2], mat[3],
      mat[4], mat[5], mat[6], mat[7],
      mat[8], mat[9], mat[10], mat[11],
      mat[12], mat[13], mat[14], mat[15]
    );
    glPushMatrix();
    glMultMatrixd(mat);

    glBegin(GL_LINES);
    glVertex3d(0.0, 0.0, 0.0);
    glVertex3d(a.GetX(), a.GetY(), a.GetZ());
    glEnd();

    glTranslated(a.GetX(), a.GetY(), a.GetZ());
    if (axis == IRAxisX)
      glRotatef(90.0, 0.0, 1.0, 0.0);
    else if (axis == IRAxisY)
      glRotatef(-90.0, 1.0, 0.0, 0.0);
    DrawCone(m_viewscl);
    glPopMatrix();
  }
}