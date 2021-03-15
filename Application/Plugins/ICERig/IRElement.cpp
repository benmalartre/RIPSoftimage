/*--------------------------------------------------------------------
	IRElement.cpp
---------------------------------------------------------------------*/
#include "IRElement.h"
#include "IRRegister.h"

namespace ICERIG {
  IRElement::~IRElement()
  {
    ClearJoints();
  }
  void IRElement::ClearJoints()
  {
    std::vector<IRJoint*>::iterator it = m_joints.begin();
    for (; it < m_joints.end(); it++)delete (*it);
      m_joints.clear();
  }

  void IRElement::SetCurve(X3DObject& curve)
  {
    m_crv = curve;
    GetType();
    NurbsCurveList crvlist = m_crv.GetActivePrimitive().GetGeometry();
    CPointRefArray pnts = crvlist.GetPoints();
    m_nbp = pnts.GetCount();

    ClearJoints();
    GetJoints();
    GetWidth();
    GetDepth();
    GetOffset();
    m_activejoint = -1;
    m_widthmodified = m_depthmodified = m_offsetmodified = m_crvmodified = false;
    m_mode = IRMoveMode;
  }

  bool IRElement::CheckNbPoints()
  {
    bool valid = m_joints.size() == m_nbp;
    if (!valid)
      Application().LogMessage(L"[IRElement] Invalid Nb Points for " + m_crv.GetFullName());
    m_valid = valid;
    return valid;
  }

  bool IRElement::IsValid()
  {
    return m_valid;
  }

  void IRElement::GetType()
  {
    if (m_crv.GetProperties().GetItem(L"RigElement").IsValid())
      m_type = IRSkeletonType;

    else if (m_crv.GetProperties().GetItem(L"MuscleElement").IsValid())
      m_type = IRMuscleType;
    else
      m_type = IRNoType;
  }

  void IRElement::GetJoints()
  {
    Primitive prim = m_crv.GetActivePrimitive();
    NurbsCurveList crvlist = prim.GetGeometry();

    // Get Rig Element Property
    Property prop;
    m_crv.GetPropertyFromName("RigElement", prop);
    if (prop.IsValid()) {
      float x = prop.GetParameterValue(L"UpVectorX");
      float y = prop.GetParameterValue(L"UpVectorY");
      float z = prop.GetParameterValue(L"UpVectorZ");
      m_up.Set(x, y, z);
    } else {
      m_up.Set(0, 1, 0);
    }

    // Clear Existing Joint Datas
    ClearJoints();

    // Get Fresh Joint Datas
    CPointRefArray points = crvlist.GetPoints();
    Point a;
    Point b;
    CVector3 up(m_up);
    int subid = 0;
    int last = points.GetCount() - 1;
    CRotation rot;
    CVector3 dir;
    CTransformation t;
    IRJoint* joint;

    for (int i = 0; i < last; i++)
    {
      joint = new IRJoint(subid);
      joint->SetElement(this);
      subid++;

      a = points[i];
      b = points[i + 1];
      dir.Sub(b.GetPosition(), a.GetPosition());


      t.SetTranslation(a.GetPosition());
      rot = GetRotationFromTwoVectors(dir, up);
      t.SetRotation(rot);

      up = RotateVector(CVector3(1, 0, 0), rot.GetQuaternion());

      joint->SetTransform(t);
      joint->SetPosition(t.GetTranslation());
      joint->SetActive(false);
      m_joints.push_back(joint);
    }

    // Append Last point
    joint = new IRJoint(subid);
    joint->SetElement(this);
    subid++;


    a = points[last - 1];
    b = points[last];
    t.SetTranslation(b.GetPosition());

    joint->SetTransform(t);
    joint->SetPosition(t.GetTranslation());
    joint->SetActive(false);
    m_joints.push_back(joint);

    /*
    ClearJoints();
    CString marker;
    int subid = 0;
    for(int i=0;i<hist.GetCount();i++)
    {
      marker = hist.GetItem(i).GetAsText();
      ULONG found = marker.FindString(L"clsctr");

      if(found>0 && found<UINT_MAX)
      {
        IRJoint* joint = new IRJoint();
        joint->SetSubID(subid);
        subid++;
        Operator clsctr(hist.GetItem(i));
        Port port = clsctr.GetPortAt(0,1,0);
        KinematicState kine = port.GetTarget();
        X3DObject ctr = kine.GetParent3DObject();
        _clsctr.Add(ctr);
        CTransformation t = kine.GetTransform();

        joint->SetTransform(t);
        joint->SetPosition(t.GetTranslation());
        joint->SetActive(false);
        _joints.push_back(joint);
      }
    }
    */
    if (!CheckNbPoints())return;

    //update orientation
    for (int i = 0; i < m_joints.size(); i++)
    {
      m_prev = m_joints[i - 1];
      m_next = m_joints[i + 1];

      if (i == 0)m_prev = NULL;
        m_joints[i]->SetPrev(m_prev);

      if (i == m_joints.size() - 1)m_next = NULL;
        m_joints[i]->SetNext(m_next);

      m_joints[i]->GetTangent();
      m_joints[i]->GetOrientation();
      }
  }

  IRJoint* IRElement::GetJoint(ULONG id)
  {
    if (id > (GetNbPoints() - 1))
      return NULL;
    else
      return m_joints[id];
  }

  IRJoint* IRElement::GetParentJoint()
  {
    if (m_parent)
    {
      if (m_parentjointindex >= 0 && m_parentjointindex < m_parent->m_joints.size())
        return m_parent->m_joints[m_parentjointindex];
    }
    return NULL;
  }

  void IRElement::GetWidth()
  {
    IRElement::GetAttribute(IRWidth);
    m_widthmodified = true;
  };

  void IRElement::GetDepth()
  {
    IRElement::GetAttribute(IRDepth);
    m_depthmodified = true;
  };

  void IRElement::GetOffset()
  {
    IRElement::GetAttribute(IROffset);
    m_offsetmodified = true;
  };

  void IRElement::SetWidth()
  {
    IRElement::SetAttribute(IRWidth, L"Width");
  };

  void IRElement::SetDepth()
  {
    IRElement::SetAttribute(IRDepth, L"Depth");
  };

  void IRElement::SetOffset()
  {
    SetAttribute(IROffset, L"Offset");
  };

  void IRElement::SetUpVector(CVector3& upv)
  {
    SetAttribute(IRUpVector, L"UpVector");
  };

  void IRElement::GetAttribute(IRElemAttribute attribute)
  {
    NurbsCurveList crvlist = m_crv.GetActivePrimitive().GetGeometry();
    Cluster cls = crvlist.GetClusters().GetItem(L"ElementData");
    ClusterProperty p;

    if (cls.IsValid())
    {
      switch (attribute)
      {
        case IRWidth:
          p = cls.GetProperties().GetItem(L"WidthMap");

          if (p.IsValid()) {
            CDoubleArray elems = p.GetElements().GetArray();
            for (ULONG i = 0; i < m_nbp; i++)m_joints[i]->SetWidth((float)elems[i]*100);
          }
          break;

        case IRDepth:
          p = cls.GetProperties().GetItem(L"DepthMap");

          if (p.IsValid()) {
            CDoubleArray elems = p.GetElements().GetArray();
            for (ULONG i = 0; i < m_nbp; i++)m_joints[i]->SetDepth((float)elems[i]*100);
          }
          break;

        case IROffset:
          p = cls.GetProperties().GetItem(L"Offset");

          if (p.IsValid()) {
            CVector3 v;
            CDoubleArray elems = p.GetElements().GetArray();
            for (ULONG i = 0; i < m_nbp; i++)
            {
              v.Set(elems[i * 3], elems[i * 3 + 1], elems[i * 3 + 2]);
              m_joints[i]->SetOffset(v);
            }
          }
          break;
      }
    }
  };

  void IRElement::SetAttribute(IRElemAttribute attribute, CString prefix)
  {
    if (!IsValid() || !IsModified(attribute))return;
    NurbsCurveList crvlist = m_crv.GetActivePrimitive().GetGeometry();
    Cluster cls = crvlist.GetClusters().GetItem(L"ElementData");
    if (cls.IsValid()) {
      // Weight Map Type
      if (attribute == IRWidth || attribute == IRDepth) {
        ClusterProperty wm = cls.GetProperties().GetItem(prefix + L"Map");

        if (wm.IsValid())
        {
          CDoubleArray elems = wm.GetElements().GetArray();
          for (ULONG i = 0; i < m_nbp; i++) {
            elems[i] = m_joints[i]->Get(attribute) * 0.01;
          }
          wm.GetElements().PutArray(elems);
          CValueArray	arg;
          arg.Add((CValue)wm);
          Application().ExecuteCommand(L"FreezeObj", arg, CValue());
        }
      } else if (attribute == IROffset) {
        ClusterProperty shape = cls.GetProperties().GetItem(L"Offset");

        if (shape.IsValid()) {
          CDoubleArray elems = shape.GetElements().GetArray();
          CVector3 v;
          for (ULONG i = 0; i < m_nbp; i++) {
            v = m_joints[i]->GetOffset();
            elems[i * 3] = v.GetX();
            elems[i * 3 + 1] = v.GetY();
            elems[i * 3 + 2] = v.GetZ();
          }
          shape.GetElements().PutArray(elems);
          CValueArray	arg;
          arg.Add((CValue)shape);
          Application().ExecuteCommand(L"FreezeObj", arg, CValue());
        }
      } else if (attribute == IRUpVector) {
        Parameter param = m_prop.GetParameter("UpVector");
        param.PutValue(m_up);
      }
    }
  };

  void IRElement::TransferAttribute(IRElemAttribute a, IRElement* other)
  {
    if (m_nbp != other->m_nbp) {
      Application().LogMessage(L"Invalid Symmetrize Elements!!");
      return;
    }

    CString prefix = L"Width";
    if (a == IRDepth)prefix = L"Depth";
    NurbsCurveList crvlist = m_crv.GetActivePrimitive().GetGeometry();
    Cluster cls = crvlist.GetClusters().GetItem(L"ElementData");
    if (cls.IsValid()) {
      if (a == IRDepth || a == IRWidth) {
        ClusterProperty wm = cls.GetProperties().GetItem(prefix + L"Map");
        if (wm.IsValid()) {
          CDoubleArray elems = wm.GetElements().GetArray();
          for (ULONG i = 0; i < m_nbp; i++) {
            elems[i] = other->m_joints[i]->Get(a)*0.01;
          }
          wm.GetElements().PutArray(elems);
          CValueArray	arg;
          arg.Add((CValue)wm);
          Application().ExecuteCommand(L"FreezeObj", arg, CValue());
        }
        else {
          Application().LogMessage(L"Weight Map Invalid");
        }
      } else if (a == IROffset) {
        ClusterProperty shape = cls.GetProperties().GetItem(L"Offset");

        if (shape.IsValid()) {
          CDoubleArray elems = shape.GetElements().GetArray();
          CVector3 v;
          for (ULONG i = 0; i < m_nbp; i++) {
            v = other->m_joints[i]->GetOffset();
            elems[i * 3] = -v.GetX();
            elems[i * 3 + 1] = v.GetY();
            elems[i * 3 + 2] = v.GetZ();
          }
          shape.GetElements().PutArray(elems);
          CValueArray	arg;
          arg.Add((CValue)shape);
          Application().ExecuteCommand(L"FreezeObj", arg, CValue());
        }
      }
    }
    else {
      Application().LogMessage(L"Cluster Invalid");
    }
  };

  bool IRElement::IsModified(IRElemAttribute attribute)
  {
    switch (attribute)
    {
      case IRWidth:return IsWidthModified();
      case IRDepth:return IsDepthModified();
      case IROffset:return IsOffsetModified();
    }
    return false;
  }

  void IRElement::Draw(ToolContext& in_ctxt)
  {
    DrawJoints(in_ctxt);
    DrawSkin(in_ctxt);
    DrawUpVector(in_ctxt);
  }


  void IRElement::DrawUpVector(ToolContext& in_ctxt)
  {
    // Draw UpVector
    CVector3 p1 = m_joints[0]->GetPosition();
    CVector3 p2;
    p2.Add(p1, m_up);
    glBegin(GL_LINES);
    glColor3f(1.0, 1.0, 0.0);
    CVector3 v;
    in_ctxt.WorldToView(p1, v);
    glVertex3f((GLfloat)v.GetX(), (GLfloat)v.GetY(), (GLfloat)v.GetZ());
    in_ctxt.WorldToView(p2, v);
    glVertex3f((GLfloat)v.GetX(), (GLfloat)v.GetY(), (GLfloat)v.GetZ());
    glEnd();
  }

  void IRElement::DrawJoints(ToolContext& in_ctxt)
  {
    //Draw all accessible point
    for (ULONG i = 0; i < m_nbp; i++) {
      m_joints[i]->Draw(in_ctxt, m_mode);
    }
  }

  void IRElement::DrawJoint(ToolContext& in_ctxt, ULONG idx)
  {
    m_joints[idx]->Draw(in_ctxt, m_mode);
  }

  void IRElement::PickJoint(ToolContext& in_ctxt, ULONG idx)
  {
    m_joints[idx]->Pick(in_ctxt, m_mode);
  }

  void IRElement::DrawHandle(ToolContext& in_ctxt)
  {
    if (m_activejoint == -1)return;
  }

  void IRElement::DrawSkin(ToolContext& in_ctxt)
  {
    for (size_t i = 0; i < m_joints.size(); i++)
      m_joints[i]->DrawSkin(in_ctxt);
  }

  void IRElement::SetActiveJoint(LONG idx)
  {
    if (idx > -1)m_activejoint = idx;
    for (ULONG j = 0; j < m_nbp; j++) {
      if (j == idx)m_joints[j]->SetActive(true);
      else m_joints[j]->SetActive(false);
    }
  }

  IRJoint* IRElement::GetActiveJoint()
  {
    if (m_activejoint == -1)return NULL;
    return m_joints[m_activejoint];
  }

  LONG IRElement::GetActiveJoint(ToolContext &in_ctxt)
  {
    LONG x, y;
    LONG l_JointID = -1;
    if (in_ctxt.GetMousePosition(x, y) == CStatus::OK) {
      ULONG pickSize = 32;//pixels
      if (in_ctxt.BeginPickDraw(x, y, pickSize, pickSize) == CStatus::OK) {
        // Setup pick buffer
        #define MAXSELECT 512
        GLuint l_SelectBuffer[MAXSELECT];
        glSelectBuffer(MAXSELECT, l_SelectBuffer);
        glRenderMode(GL_SELECT);
        glInitNames();
        glPushName(0);

        in_ctxt.BeginViewDraw();
        LONG i = 0;
        for (; i < (LONG)GetNbPoints(); ++i) {
          glLoadName((GLuint)i);
          PickJoint(in_ctxt, i);
        }

        in_ctxt.EndViewDraw();

        // Process hits
        glPopName();
        GLuint *ptr = l_SelectBuffer;
        GLuint l_MinDepth = UINT_MAX; // Initialize to farthest
        GLuint l_NumHits = glRenderMode(GL_RENDER);

        for (GLuint i = 0; i < l_NumHits; ++i) {
          GLuint l_NumIDs = *ptr++;
          GLuint l_MinZ = *ptr++;
          GLuint l_MaxZ = *ptr++;
          if (l_NumIDs > 0) {
            ULONG l_ID = (ULONG)*ptr++;
            ptr += l_NumIDs - 1;

            if (l_MinZ < l_MinDepth) {
              // This is the closest handle so far
              l_JointID = l_ID;
              l_MinDepth = l_MinZ;
            }
          }
        }
        SetActiveJoint(l_JointID);
        in_ctxt.EndPickDraw();
      }
    }
    return m_activejoint;
  }

  void IRElement::Symmetrize()
  {
    CString propertyname;
    switch (m_type)
    {
      case IRSkeletonType:
        propertyname = L"RigElement";
        break;
      case IRMuscleType:
        propertyname = L"MuscleElement";
        break;
      case IRNoType:
        return;
    }

    Model model = m_crv.GetModel();
    Property prop = m_crv.GetProperties().GetItem(propertyname);
    Application().LogMessage(L"Property Name : " + propertyname);
    ULONG side = prop.GetParameterValue(L"Side");
    bool symmetrize = prop.GetParameterValue(L"Symmetrize");
    if (!symmetrize || side > 1) {
      Application().LogMessage(L"No Symmetric Element for " + m_crv.GetFullName());
      return;
    }

    CString prefix = L"R_";
    if (side == 1)prefix = L"L_";

    CString name = m_crv.GetName().GetSubString(2);
    name = prefix + name;

    X3DObject target = model.FindChild2(name, "", CStringArray(), true);
    if (target.IsValid()) {
      IRElement* elem = new IRElement(target);
      if (IsWidthModified())elem->TransferAttribute(IRWidth, this);
      if (IsDepthModified())elem->TransferAttribute(IRDepth, this);
      if (IsOffsetModified())elem->TransferAttribute(IROffset, this);
    }
  }

  void IRElement::UpdateCurve()
  {
    if (m_crvmodified) {
      NurbsCurveList curveList = m_crv.GetActivePrimitive().GetGeometry();
      CNurbsCurveDataArray datasArray;
      curveList.Get(siSINurbs, datasArray);
      CNurbsCurveData datas = datasArray[0];
      CVector3 p;

      CVector3Array positions;
      positions.Resize(m_joints.size());
      for (size_t ii = 0; ii < m_joints.size(); ++ii) {
        positions[ii] = m_joints[ii]->GetPosition();
      }
      NurbsCurve curve = curveList.GetCurves().GetItem(0);
      curve.GetControlPoints().PutPositionArray(positions);
      m_crvmodified = false;
    }
  }

  void IRElement::WriteData()
  {
    SetWidth();
    SetDepth();
    SetOffset();
    Symmetrize();
  }
}