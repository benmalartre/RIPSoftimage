/*--------------------------------------------------------------------
IRSkeletonHandle.cpp
---------------------------------------------------------------------*/
#include "IRSkeletonHandle.h"
#include "IROGLPrimitives.h"

namespace ICERIG {
  void IRSkeletonHandle::MouseMove(ToolContext& in_ctxt)
  {
    if (!in_ctxt.IsInteracting()) {
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


  void IRSkeletonHandle::MouseDown(ToolContext& in_ctxt)
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

  void IRSkeletonHandle::MouseDrag(ToolContext& in_ctxt)
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

  void IRSkeletonHandle::MouseUp(ToolContext& in_ctxt)
  {
    if (m_elem == NULL)return;
    m_elem->UpdateCurve();
  }

  void IRSkeletonHandle::Draw(ToolContext& in_ctxt)
  {
    // Called when a redraw occurs to allow the tool to show feedback
    if (in_ctxt.IsActiveView()) {
      in_ctxt.SetCursor(siDragXYCursor);
      CMatrix4 view;
      in_ctxt.GetWorldToViewMatrix(view);
      glColor3f(1.0, 0.0, 0.0);
      DrawHandle(in_ctxt, view);
    }
  }

  void IRSkeletonHandle::DrawResolutionPlane(ToolContext& in_ctxt)
  {

  }

  void IRSkeletonHandle::DrawHandle(ToolContext& in_ctxt, CMatrix4& view)
  {
    IRJoint* jnt = m_elem->GetActiveJoint();
    if (jnt) {
      CTransformation t = jnt->GetOffsetTransform();
      CMatrix4 tm;

      tm = t.GetMatrix4();
      tm.MulInPlace(view);

      GLdouble mat[16];
      tm.Get(
        mat[0], mat[1], mat[2], mat[3],
        mat[4], mat[5], mat[6], mat[7],
        mat[8], mat[9], mat[10], mat[11],
        mat[12], mat[13], mat[14], mat[15]
      );

      glPushMatrix();
      glMultMatrixd(mat);

      DrawCircle(1.0, 32, true);
      glPopMatrix();
    }
    /*
    m_viewscl = GetViewScaleFactor(in_ctxt, m_xform.GetTranslation(), (double)IRMoveHandleSize);
    CVector3 a;

    switch (m_activeaxis)
    {
      case IRAxisX:
        SetGLColor(IRXHandleColor);
        a.Set(m_viewscl, 0.0, 0.0);
        break;

      case IRAxisZ:
        SetGLColor(IRZHandleColor);
        a.Set(0.0, m_viewscl, 0.0);
        break;

      case IRAxisXZ:
        SetGLColor(IRXZHandleColor);
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
    int nbp = 12;
    float incr = 360 / nbp;
    for (int i = 0; i < 12; i++) {
      glVertex3d(RadiansToDegrees(sinf(i*incr)), 0.0, RadiansToDegrees(cosf(i*incr)));
      glVertex3d(a.GetX(), a.GetY(), a.GetZ());
    }

    glEnd();
  */

  }
}