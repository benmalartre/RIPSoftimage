/*--------------------------------------------------------------------
	IRUpVectorHandle.cpp
---------------------------------------------------------------------*/
#include "IRUpVectorHandle.h"

namespace ICERIG {
  void IRUpVectorHandle::MouseMove(ToolContext& in_ctxt)
  {
    //Application().LogMessage(L"IRTranslateHandle : MouseMove...");
    if (!in_ctxt.IsInteracting())
    {
      LONG x, y;
      in_ctxt.GetMousePosition(x, y);

      CLongArray points;
      points.Add(x);
      points.Add(y);

      //_xform = _ctr.GetKinematics().GetGlobal().GetTransform();
      GetAxis();
      GetActiveAxis(in_ctxt);
      CPlane plane;
      //plane.Set
    }
  }

  void IRUpVectorHandle::MouseDown(ToolContext& in_ctxt)
  {
    if (m_elem == NULL)return;

    // Called when mouse button is pressed
    in_ctxt.GetMousePosition(m_startX, m_startY);

    if (GetClosestPositionOnAxis(in_ctxt, m_startX, m_startY, m_initpos) != CStatus::OK)
      return;


    /*
    Application().LogMessage(_ctr.GetFullName()+" have "+(CString)children.GetCount()+" children");
    for(int c=0;c<children.GetCount();c++)Application().LogMessage(children[c].GetAsText());
    */
  }

  void IRUpVectorHandle::MouseDrag(ToolContext& in_ctxt)
  {
    if (m_elem == NULL)return;
    // Called when mouse is moved with a mouse button pressed
    LONG x, y;
    in_ctxt.GetMousePosition(x, y);


    CVector3 position;
    if (GetClosestPositionOnAxis(in_ctxt, x, y, position) != CStatus::OK)return;

    // Compute relative translation
    CVector3 translation;
    translation.Sub(position, m_initpos);



  }

  void IRUpVectorHandle::MouseUp(ToolContext& in_ctxt)
  {
    if (m_elem == NULL)return;

  }

  void IRUpVectorHandle::Draw(ToolContext& in_ctxt)
  {
    // Called when a redraw occurs to allow the tool to show feedback
    if (in_ctxt.IsActiveView())
    {
      /*
      //Application().LogMessage(L"Cls Ctr : "+_ctr.GetFullName());
      if ( _ctr.IsValid() )
      {
        in_ctxt.BeginViewDraw();
        DrawAxis(in_ctxt,IRAxisX);
        DrawAxis(in_ctxt,IRAxisY);
        DrawAxis(in_ctxt,IRAxisZ);
        in_ctxt.EndViewDraw();
      }
      */
    }
  }

  void IRUpVectorHandle::DrawAxis(ToolContext& in_ctxt, IRAxis axis)
  {
    m_viewscl = GetViewScaleFactor(in_ctxt, m_xform.GetTranslation(), IRMoveHandleSize);

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

} // namespace ICERIG