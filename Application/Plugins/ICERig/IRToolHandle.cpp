/*--------------------------------------------------------------------
	IRToolHandle.cpp
---------------------------------------------------------------------*/
#include "IRToolHandle.h"
namespace ICERIG {
	IRToolHandle::IRToolHandle()
		: m_mode(IRInvalidMode)
		, m_valid(false)
	{}

	IRToolHandle::~IRToolHandle()
	{
	}


	void IRToolHandle::Setup(IRElement* elem, IRToolMode mode)
	{
		if (elem == NULL)m_valid = false;
		else
		{
			m_elem = elem;
			m_mode = mode;
			m_valid = true;
		}
	}

	X3DObject& IRToolHandle::GetCurve()
	{
		return m_elem->GetCurve();
	}


	double IRToolHandle::GetViewScaleFactor(ToolContext &in_ctxt, const CVector3 &pos, double viewSize) const
	{
		CVector3 offset;
		if (in_ctxt.WorldToView(pos, offset) == CStatus::OK)
		{
			// Displace point 1-pixel in view-space
			offset.PutX(offset.GetX() + 1.0/*pixels*/);
			if (in_ctxt.ViewToWorld(offset, offset) == CStatus::OK)
			{
				offset.SubInPlace(pos);
				return offset.GetLength() * viewSize;
			}
		}
		return 0.0; // Projection failed
	}

	LONG IRToolHandle::GetClosestDragAxis(ToolContext &in_ctxt, const CVector3 &mouse, CLine axes[], double &out_projection) const
	{
		// Calculate axis that best matches the mouse drag direction in screen-space
		LONG result = -1; // No axis
		double maxProjection = IRHandleDragThreshold;
		for (int i = 0; i < 3; ++i)
		{
			CVector3 initial, axis;
			if (in_ctxt.WorldToView(axes[i].GetOrigin(), initial) != CStatus::OK ||
				in_ctxt.WorldToView(axes[i].GetPosition(1.0), axis) != CStatus::OK)
				return result;

			// Convert to a 2D vector (without depth)
			axis.SubInPlace(initial).PutZ(0.0);
			double lenSquared = axis.GetLengthSquared();
			if (lenSquared > IRHandleDragThreshold*IRHandleDragThreshold)
			{
				double projectedLen = fabs(mouse.Dot(axis)) * axis.GetLength() / lenSquared;
				if (projectedLen > maxProjection)
				{
					result = i;
					out_projection = maxProjection = projectedLen;
				}
			}
		}
		return result;
	}

	double IRToolHandle::GetClosestIncrement(double in_value, double in_increment) const
	{
		if (in_increment > 0.0)
		{
			double dInteger;
			double dFraction = modf(in_value / in_increment, &dInteger);
			in_value = (dInteger + (LONG)(dFraction + (dFraction < 0 ? -0.5 : 0.5))) * in_increment;
		}
		return in_value;
	}

	void IRToolHandle::GetAxis(bool global)
	{
		m_origin = m_xform.GetTranslation();
		CMatrix3 matrix = m_xform.GetRotation().GetMatrix();
		if (global)
			matrix.SetIdentity();
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

	void IRToolHandle::GetActiveAxis(ToolContext& in_ctxt)
	{
		LONG x, y;
		in_ctxt.GetMousePosition(x, y);
		CVector3 o;
		in_ctxt.WorldToView(m_origin, o);

		CVector3 mouse(x - o.GetX(), y - o.GetY(), 0);
		m_activeaxis = IRNoAxis; // No axis
		double maxProjection = IRHandleDragThreshold;

		for (int i = 0; i < 3; i++)
		{
			CVector3 initial, axis;
			if (in_ctxt.WorldToView(m_axis[i].GetOrigin(), initial) != CStatus::OK ||
				in_ctxt.WorldToView(m_axis[i].GetPosition(1.0), axis) != CStatus::OK)
				return;

			// Convert to a 2D vector (without depth)
			axis.SubInPlace(initial).PutZ(0.0);
			double lenSquared = axis.GetLengthSquared();
			if (lenSquared > IRHandleDragThreshold*IRHandleDragThreshold)
			{
				double projectedLen = fabs(mouse.Dot(axis)) * axis.GetLength() / lenSquared;
				if (projectedLen > maxProjection)
				{
					m_activeaxis = (IRAxis)i;
					maxProjection = projectedLen;
				}
			}
		}
	}

	CStatus IRToolHandle::GetClosestPositionOnAxis(ToolContext& in_ctxt, LONG x, LONG y, CVector3& io_pos)
	{
		// Get world cursor ray
		CLine cursorRay;
		in_ctxt.EnableSnapping(false);
		if (in_ctxt.GetWorldRay(x, y, cursorRay) != CStatus::OK)
			return CStatus::False;

		// Will cursor intersect drag axis?
		if (cursorRay.Parallel(m_axis[m_activeaxis]))
			return CStatus::False;

		// Compute intersection between cursor ray and drag axis
		io_pos = m_axis[m_activeaxis].GetClosestPoint(cursorRay);

		// Reject intersection points behind the camera
		CVector3 camera_pos;
		if (in_ctxt.WorldToCamera(io_pos, camera_pos) != CStatus::OK || camera_pos.GetZ() > 0.0)
			return CStatus::False;

		else return CStatus::OK;
	}
}