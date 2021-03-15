/*--------------------------------------------------------------------
	IRJoint.cpp
---------------------------------------------------------------------*/
#include "IRJoint.h"
#include "IRToolHandle.h"
#include "IRMoveHandle.h"
#include "IRShapeHandle.h"

namespace ICERIG {
  float IRJoint::Get(IRElemAttribute a)
  {
    if (a == IRDepth)return m_depth;
    else if (a == IRWidth)return m_width;
    else return NULL;
  }

  CVector3 IRJoint::GetNormal()
  {
    CVector3 norm(0, 1, 0);
    norm.MulByTransformationInPlace(m_tra);
    norm.SubInPlace(GetPosition());
    return norm;
  }

  CVector3 IRJoint::GetScaling()
  {
    CVector3 scl(m_width, 1.0, m_depth);
    m_tra.SetScaling(scl);
    return scl;
  }

  CVector3 IRJoint::GetTangent()
  {
    // first point
    if (m_prev == NULL)
      m_tangent.Sub(m_next->GetPosition(), GetPosition());

    // last point
    else if (m_next == NULL) {
      m_tangent.Sub(GetPosition(), m_prev->GetPosition());
    } else {
      CVector3 a, b;
      a.Sub(GetPosition(), m_prev->GetPosition());
      b.Sub(m_next->GetPosition(), GetPosition());
      m_tangent.LinearlyInterpolate(a, b, 0.5);
    }
    m_tangent.NormalizeInPlace();
    return m_tangent;
  }

  void IRJoint::GetOrientation()
  {
    CVector3 side, back;
    if (GetSubID() == 0) {
      side = m_element->GetUpVector();
    } else {
      IRJoint* prev = m_element->GetJoint(GetSubID() - 1);
      side = CVector3(1, 0, 0);
      side.MulByTransformationInPlace(prev->GetTransform());
      side.SubInPlace(prev->GetPosition());
    }

    back.Cross(m_tangent, side);
    side.Cross(back, m_tangent);
    back.NormalizeInPlace();
    side.NormalizeInPlace();
    CMatrix3 m;
    m.Set(side.GetX(), side.GetY(), side.GetZ(),
    m_tangent.GetX(), m_tangent.GetY(), m_tangent.GetZ(),
    back.GetX(), back.GetY(), back.GetZ());
    m_tra.SetRotationFromMatrix3(m);
  }


  CVector3 IRJoint::GetOffsetPosition()
  {
    CVector3 p;
    p.Add(GetPosition(), m_offset);
    return p;
  }

  CTransformation IRJoint::GetOffsetTransform()
  {
    GetTangent();
    GetOrientation();
    GetScaling();
    CTransformation t;
    CVector3 p;
    p.Add(GetPosition(), m_offset);
    t = m_tra;
    t.SetTranslation(p);
    return t;
  }

  void IRJoint::SetOffset(CVector3& offset, IRAxis& axis)
  {
    Application().LogMessage(L"IR Joint Set Offset Called...");
    switch (axis)
    {
      case IRAxisX:
        m_offset.PutX(offset.GetX());
        break;
      case IRAxisY:
        m_offset.PutY(offset.GetY());
        break;
      case IRAxisZ:
        m_offset.PutZ(offset.GetZ());
        break;
    }
  }


  CVector3 IRJoint::GetAxis(ToolContext& in_ctxt, CString axis)
  {
    CVector3 axe;
    if (axis == L"X")axe.Set(1, 0, 0);
    else if (axis == L"Y")axe.Set(0, 1, 0);
    else axe.Set(0, 0, 1);

    axe.MulByTransformationInPlace(GetOffsetTransform());
    axe.SubInPlace(GetOffsetPosition());
    return axe;
  }

  IRAxis IRJoint::GetActiveAxis(ToolContext& in_ctxt, const IRToolMode& mode)
  {
    LONG x, y;
    in_ctxt.GetMousePosition(x, y);
    CVector3 v;
    in_ctxt.GetWorldPosition(x, y, v);
    CVector3 delta, ax, ay, az;

    delta.Sub(v, GetPosition());
    float l = (float)delta.GetLength();
    if (mode == IRShapeMode) {
      ax = GetAxis(in_ctxt, "X");
      az = GetAxis(in_ctxt, "Z");

      if (az.Dot(delta) < ax.Dot(delta)) {
        m_axis = IRAxisX;
      } else {
        m_axis = IRAxisZ;
      }
    } else if (mode == IRMoveMode || mode == IROffsetMode) {
      double maxProjection = 4;
      CVector3Array axisarray;
      axisarray.Add(CVector3(1, 0, 0));
      axisarray.Add(CVector3(0, 1, 0));
      axisarray.Add(CVector3(0, 0, 1));

      CVector3 initial, axis, p1, p2;
      for (int i = 0; i < 3; i++)
      {
        p1.Add(GetPosition(), m_offset);
        p2.Add(p1, axisarray[i]);
        if (in_ctxt.WorldToView(p1, initial) != CStatus::OK ||
          in_ctxt.WorldToView(p2, axis) != CStatus::OK)
            return IRNoAxis;

        // Convert to a 2D vector (without depth)
        axis.SubInPlace(GetPosition()).PutZ(0.0);
        CVector3 mouse(x, y, 0.0);

        double lenSquared = axis.GetLengthSquared();
        double projectedLen = fabs(mouse.Dot(axis)) * axis.GetLength() / lenSquared;
        if (projectedLen > maxProjection)
        {
          m_axis = (IRAxis)i;
          maxProjection = projectedLen;
        }
      }
    }
    return m_axis;
  }

  CVector3 IRJoint::GetCorner(ULONG id)
  {
    CVector3 corner;
    switch (id)
    {
      case 0:
        corner.Set(-m_width / 2, 0, m_depth / 2);
        break;
      case 1:
        corner.Set(m_width / 2, 0, m_depth / 2);
        break;
      case 2:
        corner.Set(m_width / 2, 0, -m_depth / 2);
        break;
      case 3:
        corner.Set(-m_width / 2, 0, -m_depth / 2);
        break;
    }

    return corner;
  }

  void IRJoint::Draw(ToolContext& ctxt, IRToolMode& mode)
  {
    CVector3 p = GetPosition();
    CVector3 o = GetOffset();
    o.AddInPlace(p);
    CVector3 vp, vo, vn;
    ctxt.WorldToView(p, vp);
    ctxt.WorldToView(o, vo);

    // draw points
    glEnable(GL_POINT_SMOOTH);
    if (mode == IRMoveMode)
    {
      glPointSize(6);
      glBegin(GL_POINTS);
      glColor3f(1.0, 0.0, 0.0);
      glVertex3f((GLfloat)vo.GetX(), (GLfloat)vo.GetY(), (GLfloat)vo.GetZ());
      glEnd();
      glPointSize(12);
      glBegin(GL_POINTS);
      if (GetActive())glColor3f(1.0, 1.0, 0.0);
      glVertex3f((GLfloat)vp.GetX(), (GLfloat)vp.GetY(), (GLfloat)vp.GetZ());
      glEnd();
    } else if (mode == IROffsetMode) {
      glPointSize(6);
      glBegin(GL_POINTS);
      glColor3f(1.0, 0.0, 0.0);
      glVertex3f((GLfloat)vp.GetX(), (GLfloat)vp.GetY(), (GLfloat)vp.GetZ());
      glEnd();
      glPointSize(12);
      glBegin(GL_POINTS);
      if (GetActive())glColor3f(1.0, 1.0, 0.0);
      glVertex3f((GLfloat)vo.GetX(), (GLfloat)vo.GetY(), (GLfloat)vo.GetZ());
      glEnd();
    } else if (mode == IRShapeMode) {
      glPointSize(6);
      glBegin(GL_POINTS);
      glColor3f(1.0, 0.0, 0.0);
      glVertex3f((GLfloat)vp.GetX(), (GLfloat)vp.GetY(), (GLfloat)vp.GetZ());
      glEnd();
      glPointSize(12);
      glBegin(GL_POINTS);
      if (GetActive())glColor3f(1.0, 1.0, 0.0);
      glVertex3f((GLfloat)vo.GetX(), (GLfloat)vo.GetY(), (GLfloat)vo.GetZ());
      glEnd();
    }

    if (mode == IRSkeletonMode) {
      glPointSize(6);
      glBegin(GL_POINTS);
      glColor3f(1.0, 0.0, 0.0);
      glVertex3f((GLfloat)vo.GetX(), (GLfloat)vo.GetY(), (GLfloat)vo.GetZ());
      glEnd();
      glPointSize(12);
      glBegin(GL_POINTS);
      if (GetActive())glColor3f(1.0, 1.0, 0.0);
      glVertex3f((GLfloat)vp.GetX(), (GLfloat)vp.GetY(), (GLfloat)vp.GetZ());
      glEnd();
    } else {
      glBegin(GL_POINTS);
      glPointSize(6);
      glColor3f(1.0, 0.0, 0.0);
      glVertex3f((GLfloat)vo.GetX(), (GLfloat)vo.GetY(), (GLfloat)vo.GetZ());
      glEnd();
    }

    // draw offset edge
    glColor3f(1.0, 1.0, 1.0);
    glLineWidth(2);
    glBegin(GL_LINES);
    glVertex3f((GLfloat)vp.GetX(), (GLfloat)vp.GetY(), (GLfloat)vp.GetZ());
    glVertex3f((GLfloat)vo.GetX(), (GLfloat)vo.GetY(), (GLfloat)vo.GetZ());
    glEnd();

    // draw joint
    if (m_next != NULL)
    {
      p = m_next->GetPosition();
      ctxt.WorldToView(p, vn);
      glColor3f(1.0, 1.0, 0.5);
      glLineWidth(4);
      glBegin(GL_LINES);
      glVertex3f((GLfloat)vp.GetX(), (GLfloat)vp.GetY(), (GLfloat)vp.GetZ());
      glVertex3f((GLfloat)vn.GetX(), (GLfloat)vn.GetY(), (GLfloat)vn.GetZ());
      glEnd();
    }
  }

  void IRJoint::Pick(ToolContext& ctxt, IRToolMode& mode)
  {
    CVector3 p = GetPosition();
    CVector3 o = GetOffset();
    o.AddInPlace(p);
    CVector3 pv, ov;
    ctxt.WorldToView(p, pv);
    ctxt.WorldToView(o, ov);
    glColor3f(1.0, 0.0, 0.0);

    glPointSize(24);
    glEnable(GL_POINT_SMOOTH);
    glBegin(GL_POINTS);

    if (mode == IRMoveMode) {
      glVertex3f((GLfloat)pv.GetX(), (GLfloat)pv.GetY(), (GLfloat)pv.GetZ());
    } else if (mode == IROffsetMode) {
      glVertex3f((GLfloat)ov.GetX(), (GLfloat)ov.GetY(), (GLfloat)ov.GetZ());
    } else if (mode == IRShapeMode) {
      glVertex3f((GLfloat)ov.GetX(), (GLfloat)ov.GetY(), (GLfloat)ov.GetZ());
    } else if (mode == IRUpVectorMode) {
      glVertex3f((GLfloat)pv.GetX(), (GLfloat)pv.GetY(), (GLfloat)pv.GetZ());
    } else {
      glVertex3f((GLfloat)ov.GetX(), (GLfloat)ov.GetY(), (GLfloat)ov.GetZ());
    }
    glEnd();

    /*
    if (_next != NULL)
    {
    p = _next->GetPosition();
    ctxt.WorldToView(p, d);
    glColor3f(1.0, 1.0, 0.5);
    glBegin(GL_LINES);
    glLineWidth(2);
    glVertex3f((GLfloat)v.GetX(), (GLfloat)v.GetY(), (GLfloat)v.GetZ());
    glVertex3f((GLfloat)d.GetX(), (GLfloat)d.GetY(), (GLfloat)d.GetZ());
    glEnd();
    }
    */
  }



  void IRJoint::DrawAxis(ToolContext& ctxt, CVector3 axis, bool selected, float scale)
  {
    if (m_active)
    {
      glBegin(GL_LINES);
      glColor3f(1.0, 0.5, 0.0);

      if (selected)
        glColor3f(1.0, 1.0, 1.0);

      CVector3 start, end, v;

      start = GetOffsetPosition();
      end.Scale(scale*1.25, axis);
      end.MulByTransformationInPlace(GetOffsetTransform());

      ctxt.WorldToView(start, v);
      glVertex3f((GLfloat)v.GetX(), (GLfloat)v.GetY(), (GLfloat)v.GetZ());
      ctxt.WorldToView(end, v);
      glVertex3f((GLfloat)v.GetX(), (GLfloat)v.GetY(), (GLfloat)v.GetZ());

      glEnd();

      glPointSize(4.0f);

      glBegin(GL_POINTS);
      glEnable(GL_POINT_SMOOTH);
      glVertex3f((GLfloat)start.GetX(), (GLfloat)start.GetY(), (GLfloat)start.GetZ());
      glEnd();
    }
  }

  void IRJoint::DrawArrow(ToolContext& ctxt, CVector3 axis, bool selected)
  {
    if (m_active)
    {
      glBegin(GL_LINES);
      glColor3f((GLfloat)axis.GetX(), (GLfloat)axis.GetY(), (GLfloat)axis.GetZ());

      if (selected)
        glColor3f(1.0, 1.0, 1.0);

      CVector3 start, end, v;

      CTransformation t = GetOffsetTransform();
      start.Set(0, 0, 0);
      start.MulByTransformationInPlace(t);
      end = axis;
      end.MulByTransformationInPlace(t);

      ctxt.WorldToView(start, v);
      glVertex3f((GLfloat)v.GetX(), (GLfloat)v.GetY(), (GLfloat)v.GetZ());
      ctxt.WorldToView(end, v);
      glVertex3f((GLfloat)v.GetX(), (GLfloat)v.GetY(), (GLfloat)v.GetZ());

      glEnd();

      glPointSize(4.0f);

      glBegin(GL_POINTS);
      glEnable(GL_POINT_SMOOTH);
      glVertex3f((GLfloat)v.GetX(), (GLfloat)v.GetY(), (GLfloat)v.GetZ());
      glEnd();
    }
  }
  /*
  void IRJoint::DrawEllipse(ToolContext& in_ctxt)
  {

  }
  */
  void IRJoint::DrawSquare(ToolContext& in_ctxt)
  {
    CMatrix4 view;
    in_ctxt.GetWorldToViewMatrix(view);

    // set current color
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0, 1.0, 0, 0.5);
    if (m_active)
      glColor4f(1.0, 0.5, 0, 0.5);

    glBegin(GL_QUADS);

    CVector3 a, b, c, d;
    a = GetCorner(0);
    b = GetCorner(1);
    c = GetCorner(2);
    d = GetCorner(3);

    CTransformation t = m_tra;
    t.SetTranslation(GetOffsetPosition());
    CMatrix4 m = t.GetMatrix4();

    m.MulInPlace(view);

    a.MulByMatrix4InPlace(m);
    b.MulByMatrix4InPlace(m);
    c.MulByMatrix4InPlace(m);
    d.MulByMatrix4InPlace(m);

    glVertex3f((GLfloat)a.GetX(), (GLfloat)a.GetY(), (GLfloat)a.GetZ());
    glVertex3f((GLfloat)b.GetX(), (GLfloat)b.GetY(), (GLfloat)b.GetZ());
    glVertex3f((GLfloat)c.GetX(), (GLfloat)c.GetY(), (GLfloat)c.GetZ());
    glVertex3f((GLfloat)d.GetX(), (GLfloat)d.GetY(), (GLfloat)d.GetZ());

    glEnd();
    glDisable(GL_BLEND);
  }

  void IRJoint::DrawSkin(ToolContext& in_ctxt)
  {
    if (!m_next)return;

    CMatrix4 view;
    in_ctxt.GetWorldToViewMatrix(view);
    // set current color
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(m_element->GetRed(), m_element->GetGreen(), m_element->GetBlue(), .4);

    glBegin(GL_QUADS);
    CVector3 a, b, c, d;

    for (int f = 0; f < 4; f++)
    {
      a = GetCorner(f);
      b = m_next->GetCorner(f);
      c = m_next->GetCorner((f + 1) % 4);
      d = GetCorner((f + 1) % 4);

      CTransformation t = GetOffsetTransform();
      CTransformation t2 = m_next->GetOffsetTransform();
      CMatrix4 m = t.GetMatrix4();
      CMatrix4 m2 = t2.GetMatrix4();

      m.MulInPlace(view);
      m2.MulInPlace(view);

      a.MulByMatrix4InPlace(m);
      b.MulByMatrix4InPlace(m2);
      c.MulByMatrix4InPlace(m2);
      d.MulByMatrix4InPlace(m);

      glVertex3f((GLfloat)a.GetX(), (GLfloat)a.GetY(), (GLfloat)a.GetZ());
      glVertex3f((GLfloat)b.GetX(), (GLfloat)b.GetY(), (GLfloat)b.GetZ());
      glVertex3f((GLfloat)c.GetX(), (GLfloat)c.GetY(), (GLfloat)c.GetZ());
      glVertex3f((GLfloat)d.GetX(), (GLfloat)d.GetY(), (GLfloat)d.GetZ());
    }

    glEnd();
    glDisable(GL_BLEND);
  }
}