/*-----------------------------------------------------------------
		File : IRJoint.h
-------------------------------------------------------------------*/
#ifndef IR_JOINT_H
#define IR_JOINT_H

#include "IRRegister.h"

namespace ICERIG {
	class IRElement;
	class IRJoint
	{
	public:
		IRJoint(int subIdx) :m_subid(subIdx), m_prev(NULL), m_next(NULL) {};

		float GetWidth() { return m_width; };
		float GetDepth() { return m_depth; };
		float Get(IRElemAttribute a);
		int GetSubID() { return m_subid; };

		CVector3 GetOffset() { return m_offset; };
		CTransformation GetTransform() { return m_tra; };
		bool GetActive() { return m_active; };
		CVector3 GetPosition() { return m_pos; };
		CVector3 GetScaling();
		CVector3 GetNormal();
		CVector3 GetTangent();
		CVector3 GetAxis(ToolContext& in_ctxt, CString axis);
		IRAxis GetActiveAxis(ToolContext& in_ctxt, const IRToolMode& mode);
		void GetOrientation();
		CVector3 GetOffsetPosition();
		CVector3 GetCorner(ULONG id);
		CTransformation GetOffsetTransform();

		void SetWidth(float w) { m_width = w; };
		void SetDepth(float d) { m_depth = d; };
		void SetOffset(CVector3& v) { m_offset = v; };
		void SetOffset(CVector3& v, IRAxis& axis);
		void SetTransform(CTransformation& t) { m_tra = t; };
		void SetPosition(CVector3& v) { m_pos = v; };
		void SetActive(bool active) { m_active = active; };
		void SetSubID(int id) { m_subid = id; };
		void SetElement(IRElement* elem) { m_element = elem; };

		void SetPrev(IRJoint* prv) { m_prev = prv; };
		void SetNext(IRJoint* nxt) { m_next = nxt; };
		void Draw(ToolContext& ctxt, IRToolMode& mode);
		void Pick(ToolContext& ctxt, IRToolMode& mode);
		void DrawSkin(ToolContext& ctxt);

	protected:
		void DrawAxis(ToolContext& ctxt, CVector3 axis, bool selected, float scale);
		void DrawArrow(ToolContext& ctxt, CVector3 axis, bool selected);
		void DrawSquare(ToolContext& ctxt);

	private:
		float m_width;
		float m_depth;
		CVector3 m_pos;
		CVector3 m_offset;
		CTransformation m_tra;
		bool m_active;
		IRAxis m_axis;
		CVector3 m_tangent;
		IRJoint* m_prev;
		IRJoint* m_next;
		IRElemType m_type;
		IRElement* m_element;
		int m_subid;
	};
}		// ICERIG
#endif	// IR_JOINT