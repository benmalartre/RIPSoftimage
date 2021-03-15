///////////////////////////////////////////////////////////////////////////
//
// File: IRShapeHandle.cpp
//
///////////////////////////////////////////////////////////////////////////
#include "IRShapeHandle.h"
#include "IRJoint.h"
#include "IROGLPrimitives.h"

namespace ICERIG {
  void IRShapeHandle::GetAxis(ToolContext& in_ctxt)
  {
    m_origin = m_xform.GetTranslation();
    CMatrix3 matrix = m_xform.GetRotation().GetMatrix();
    CVector3 tangent(1.0, 0.0, 0.0);
    tangent.MulByMatrix3InPlace(matrix);
    m_axis[0].Set(m_origin, tangent);
    tangent.Set(0.0, 1.0, 0.0);
    tangent.MulByMatrix3InPlace(matrix);
    m_axis[1].Set(m_origin, tangent);
    tangent.Set(0.0, 0.0, 1.0);
    tangent.MulByMatrix3InPlace(matrix);
    m_axis[2].Set(m_origin, tangent);
  }

  void IRShapeHandle::GetActiveAxis(ToolContext& in_ctxt)
  {
    LONG x, y;
    in_ctxt.GetMousePosition(x, y);
    CVector3 o;
    in_ctxt.WorldToView(m_origin, o);

    CVector3 mouse(x - o.GetX(), y - o.GetY(), 0);
    m_activeaxis = IRNoAxis; // No axis
    double maxProjection = IRHandleDragThreshold;

    for (int i = 0; i < 3; i++) {
      if (i == 1)continue;
      CVector3 initial, axis;
      if (in_ctxt.WorldToView(m_axis[i].GetOrigin(), initial) != CStatus::OK ||
        in_ctxt.WorldToView(m_axis[i].GetPosition(1.0), axis) != CStatus::OK)
          return;

      // Convert to a 2D vector (without depth)
      axis.SubInPlace(initial).PutZ(0.0);
      double lenSquared = axis.GetLengthSquared();
      if (lenSquared > IRHandleDragThreshold * IRHandleDragThreshold) {
        double projectedLen = fabs(mouse.Dot(axis)) * axis.GetLength() / lenSquared;
        if (projectedLen > maxProjection) {
          m_activeaxis = (IRAxis)i;
          maxProjection = projectedLen;
        }
      }
    }
  }


  void IRShapeHandle::MouseMove(ToolContext& in_ctxt)
  {
    LONG x, y;
    in_ctxt.GetMousePosition(x, y);

    CLongArray points;
    points.Add(x);
    points.Add(y);

    m_xform = m_elem->GetActiveJoint()->GetTransform();
    GetAxis(in_ctxt);
    GetActiveAxis(in_ctxt);
  }

  void IRShapeHandle::MouseDown(ToolContext& in_ctxt)
  {

    // Called when mouse button is pressed
    in_ctxt.GetMousePosition(m_startX, m_startY);

    GetClosestPositionOnAxis(in_ctxt, m_startX, m_startY, m_initpos);

    IRJoint* jnt = m_elem->GetActiveJoint();
    m_initwidth = jnt->GetWidth();
    m_initdepth = jnt->GetDepth();
    m_initlength = m_initpos.GetLength();
    /*
    CVector3 origin;
    in_ctxt.WorldToView(m_origin,origin);
    IRJoint* jnt = m_elem->GetActiveJoint();
    if (jnt) {
      m_plane.Set(jnt->GetOffsetPosition(), jnt->GetTangent());
      in_ctxt.SetManipulationPlane(m_plane);
    }

    m_initlength = (float)m_initpos.GetLength();
    CLine worldRay;
    in_ctxt.GetWorldRay(m_startX, m_startY, worldRay);
    m_plane.Intersect(worldRay, m_initpos);
    */
  }

  void IRShapeHandle::MouseDrag(ToolContext& in_ctxt)
  {

    // Called when mouse is moved with a mouse button pressed
    LONG x, y;
    in_ctxt.GetMousePosition(x, y);

    CVector3 position;
    if (GetClosestPositionOnAxis(in_ctxt, x, y, position) != CStatus::OK)return;

    // compute delta
    CVector3 delta;
    delta.Sub(position, m_initpos);

    float currentlength = delta.GetLength();
    m_diff = currentlength * m_viewscl;

    if (delta.Dot(position) < 0) m_diff *= -1;

    IRJoint* jnt = m_elem->GetActiveJoint();
    if (m_activeaxis == IRAxisX) {
      jnt->SetWidth(m_initwidth + m_diff);
      m_elem->SetWidthModified(true);
    }

    if (m_activeaxis == IRAxisZ) {
      jnt->SetDepth(m_initdepth + m_diff);
      m_elem->SetDepthModified(true);
    }
  }

  void IRShapeHandle::MouseUp(ToolContext& in_ctxt)
  {
    //in_ctxt.EndTransformUpdate( m_objects );
    m_elem->SetWidth();
    m_elem->SetDepth();
  }

  void IRShapeHandle::DrawAxis(CMatrix4& view)
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

      // X Axis
      if (m_activeaxis == 0) {
        SetGLColor(IRSelectedHandleColor);
      } else {
        SetGLColor(IRXHandleColor);
      }

      glLineWidth(2);
      glBegin(GL_LINES);
      glVertex3f(0, 0, 0);
      glVertex3f(2 * m_viewscl, 0, 0);
      glEnd();

      glBegin(GL_POINTS);
      glPointSize(24);
      glVertex3f(2 * m_viewscl, 0.0, 0.0);
      glEnd();

      // Z Axis
      if (m_activeaxis == 2) {
        SetGLColor(IRSelectedHandleColor);
      } else {
        SetGLColor(IRZHandleColor);
      }

      glLineWidth(2);
      glBegin(GL_LINES);
      glVertex3f(0, 0, 0);
      glVertex3f(0, 0, 2 * m_viewscl);
      glEnd();

      glBegin(GL_POINTS);
      glPointSize(24);
      glVertex3f(0.0, 0.0, 2 * m_viewscl);
      glEnd();

      glPopMatrix();
    }
  }

  void IRShapeHandle::DrawSection(int idx, CMatrix4& view, bool selected)
  {
    IRJoint* jnt = m_elem->GetJoint(idx);
    if (!jnt == NULL) {
      jnt->GetScaling();
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
      if (selected)
        SetGLColor(IRSelectedHandleColor);
      else
        glColor3f(0.5, 0.5, 0.5);

      //ogl_drawCircle(1,36,true);
      glLineWidth(2);
      glBegin(GL_LINE_STRIP);
      glVertex3f(-1, 0, -1);
      glVertex3f(-1, 0, 1);
      glVertex3f(1, 0, 1);
      glVertex3f(1, 0, -1);
      glVertex3f(-1, 0, -1);
      glEnd();

      glPopMatrix();
    }
  }

  void IRShapeHandle::Draw(ToolContext& in_ctxt)
  {
    m_viewscl = GetViewScaleFactor(in_ctxt, m_xform.GetTranslation(), (double)IRShapeHandleSize);

    CVector3 pv;
    in_ctxt.WorldToView(m_initpos, pv);

    glBegin(GL_POINTS);
    glPointSize(12);
    glVertex3f(pv.GetX(), pv.GetY(), pv.GetZ());
    glEnd();

    CMatrix4 view;
    in_ctxt.GetWorldToViewMatrix(view);
    int subid = m_elem->GetActiveJoint()->GetSubID();

    for (ULONG i = 0; i < m_elem->GetNbPoints(); i++) {
      bool selected = (subid == i);
      DrawSection(i, view, selected);
      if (selected) DrawAxis(view);
    }
  }
}