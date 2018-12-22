/*--------------------------------------------------------------------
	IRElement.h
---------------------------------------------------------------------*/
#ifndef IR_ELEMENT_H
#define IR_ELEMENT_H

#include "IRRegister.h"
#include "IRJoint.h"

namespace ICERIG {
	class IRElement
	{
	public:
		IRElement() {};
		IRElement(X3DObject& curve) { SetCurve(curve); };
		~IRElement();
		void ClearJoints();

		void SetCurve(X3DObject& curve);
		X3DObject& GetCurve() { return m_crv; };
		ULONG GetNbPoints() { return m_nbp; };

		bool IsValid();
		void GetJoints();
		IRJoint* GetJoint(ULONG idx);

		void SetAttribute(IRElemAttribute attribute, CString prefix);
		void GetAttribute(IRElemAttribute attribute);
		void TransferAttribute(IRElemAttribute a, IRElement* other);

		void GetType();
		void GetWidth();
		void GetDepth();
		void GetOffset();
		void SetWidth();
		void SetDepth();
		void SetOffset();
		void SetUpVector(CVector3& upv);

		void SetToolMode(IRToolMode mode) { m_mode = mode; };
		IRToolMode GetToolMode() { return m_mode; };

		void Draw(ToolContext& in_ctxt);
		void DrawUpVector(ToolContext& in_ctxt);
		void DrawJoints(ToolContext& in_ctxt);
		void DrawJoint(ToolContext& in_ctxt, ULONG id);
		void PickJoint(ToolContext& in_ctxt, ULONG id);
		void DrawHandle(ToolContext& in_ctxt);
		void DrawSkin(ToolContext& ctxt);
		void SetActiveJoint(LONG id);

		IRJoint* GetActiveJoint();
		bool IsWidthModified() { return m_widthmodified; };
		bool IsDepthModified() { return m_depthmodified; };
		bool IsOffsetModified() { return m_offsetmodified; };
		bool IsModified(IRElemAttribute attribute);

		void SetWidthModified(bool m) { m_widthmodified = m; };
		void SetDepthModified(bool m) { m_depthmodified = m; };
		void SetOffsetModified(bool m) { m_offsetmodified = m; };
		void SetCurveModified(bool m) { m_crvmodified = m; };
		void Symmetrize();
		void WriteData();
		void UpdateCurve();

		float GetRed() { return m_red; };
		float GetGreen() { return m_green; };
		float GetBlue() { return m_blue; };
		void RandomColor() { m_red = R01(); m_blue = R01(); m_green = R01(); };

		LONG GetActiveJoint(ToolContext &in_ctxt);
		bool CheckNbPoints();
		IRElement* GetParent() { return m_parent; };
		IRJoint* GetParentJoint();
		CVector3 GetUpVector() { return m_up; };

	protected:
		IRElement * m_parent;
		ULONG					m_parentjointindex;
		std::vector<IRJoint*>	m_joints;
		std::vector<IRElement*> m_children;
		IRToolMode				m_mode;
		LONG					m_activejoint;
		IRElemType				m_type;
		CVector3				m_up;

	private:
		CustomProperty m_prop;
		X3DObject m_crv;
		ULONG m_nbp;
		bool m_valid;

		IRJoint* m_prev;
		IRJoint* m_next;
		bool m_widthmodified;
		bool m_depthmodified;
		bool m_offsetmodified;
		bool m_crvmodified;
		bool m_symetrize;
		CRefArray m_object;
		float m_red;
		float m_blue;
		float m_green;

	};
	;
} // end namespace ICERIG
#endif 
