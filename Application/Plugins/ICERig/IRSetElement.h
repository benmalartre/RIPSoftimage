/*------------------------------------------------------------------
		IRSetElement.h
------------------------------------------------------------------*/
#ifndef IR_SET_ELEMENT_H
#define IR_SET_ELEMENT_H

#include "IRRegister.h"
#include "IRElement.h"
#include "IRToolHandle.h"
#include "IRShapeHandle.h"
#include "IRMoveHandle.h"
#include "IROffsetHandle.h"
#include "IRUpVectorHandle.h"
#include "IRSkeletonHandle.h"


using namespace XSI::MATH; 
using namespace XSI; 

namespace ICERIG {
	/*Global*/ULONG g_selectionID = 0;

	class IRSetElement
	{
	private:
		IRElement * m_element;
		IRToolHandle* m_handle;
		IRElemType	m_type;
		LONG		m_x;
		LONG 		m_y;
		IRAxis		m_axis;
		float		m_activewidth;
		float		m_activedepth;
		float		m_basewidth;
		float		m_basedepth;
		float		m_baselength;
		LONG		m_activejoint;
		CVector3	m_activepos;
		float		m_l;
		ULONG		m_gid;
		IRToolMode	m_mode;
		bool		m_valid;

		bool		IsValid();

	public: // Methods
		IRSetElement() {
			m_element = NULL;
			m_handle = new IRToolHandle();
		};
		~IRSetElement() {
			if (m_handle != NULL)delete m_handle;
			if (m_element != NULL)delete m_element;
		}

		IRElemType GetElementType(X3DObject& curve);
		void SetupHandle();

		CStatus Setup(ToolContext& in_ctxt);
		CStatus Cleanup(ToolContext& in_ctxt);
		CStatus Activate(ToolContext& in_ctxt);
		CStatus MouseDown(ToolContext& in_ctxt);
		CStatus MouseDrag(ToolContext& in_ctxt);
		CStatus MouseUp(ToolContext& in_ctxt);
		CStatus Draw(ToolContext& in_ctxt);
		CStatus MouseMove(ToolContext& in_ctxt);
		CStatus ModifierChanged(ToolContext& in_ctxt);
		CStatus SelectionChanged(ToolContext& in_ctxt);
		CStatus MenuInit(ToolContext& in_ctxt);
		CStatus SetToolMode(IRToolMode mode);
		CStatus SetShapeMode(ToolContext& in_ctxt);
		CStatus SetOffsetMode(ToolContext& in_ctxt);
		CStatus SetMoveMode(ToolContext& in_ctxt);
		CStatus SetUpVectorMode(ToolContext& in_ctxt);
		CStatus SetSkeletonMode(ToolContext& in_ctxt);
		CStatus DoExitTool(ToolContext& in_ctxt);
		bool CheckSelection(ToolContext& in_ctxt);
		static ULONG IncrementGlobalSelectionID()
		{
			if (++g_selectionID == 0)
				++g_selectionID; // Reserve 0 as an invalid ID
			return g_selectionID;
		}

	};
} // ICERIG
#endif