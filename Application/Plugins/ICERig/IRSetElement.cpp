/*------------------------------------------------------------------
		IRSetElement.cpp
------------------------------------------------------------------*/
#include "IRSetElement.h"

using namespace ICERIG;

CStatus IRSetElement::Setup( ToolContext& in_ctxt )
{
	Application().LogMessage(L"IRSetElement Setup Called!!!");
	Selection sel = Application().GetSelection();
	m_valid = false;
	if(sel.GetCount() == 0)
	{
		Application().LogMessage(L"Nothing Selected : you need to select a ICERig Curve!!", siErrorMsg);
		CStatus status = in_ctxt.ExitTool();
		Application().LogMessage(status.GetDescription());
		return CStatus::OK;
	}

	X3DObject crv = sel.GetItem(0);
	if(crv.GetType() != L"crvlist" || GetElementType(crv) == IRNoType)
	{
		Application().LogMessage(L"Invalid Selection : you need to select a ICERig Curve!!", siErrorMsg);
		return CStatus::OK;
	}

	if(m_element!=NULL)delete m_element;
	
	m_element = new IRElement();
	m_element->SetCurve(crv);
	m_element->RandomColor();

	m_gid = g_selectionID;
	m_valid = true;

	return CStatus::OK;
}

void IRSetElement::SetupHandle()
{
	if(m_handle !=NULL)delete m_handle;
	IRToolMode mode = m_element->GetToolMode();
	
	if (mode == IRMoveMode)
		m_handle = new IRMoveHandle();
	else if (mode == IRShapeMode)
		m_handle = new IRShapeHandle();
	else if (mode == IROffsetMode)
		m_handle = new IROffsetHandle();
	else if (mode == IRUpVectorMode)
		m_handle = new IRUpVectorHandle();
	else if (mode == IRSkeletonMode)
		m_handle = new IRSkeletonHandle();

	else m_handle = NULL;

	if(m_handle != NULL)
	{
		m_handle->Setup(m_element,mode);
	}
}

bool IRSetElement::IsValid()
{
	return m_valid;
}

IRElemType IRSetElement::GetElementType(X3DObject& curve)
{
	//Application().LogMessage(L"Get Element Type Called!!");
	if(curve.GetProperties().GetItem(IR_PROP_SKELETON).IsValid())
		m_type = IRSkeletonType;

	else if(curve.GetProperties().GetItem(IR_PROP_MUSCLE).IsValid())
		m_type = IRMuscleType;

	else if (curve.GetProperties().GetItem(IR_PROP_CONTROL).IsValid())
		m_type = IRMuscleType;

	else
		m_type = IRNoType;
	//Application().LogMessage(L"Element Type : "+(CString)_type);
	return m_type;
}

CStatus IRSetElement::Cleanup( ToolContext& in_ctxt )
{
	return CStatus::OK;
}

CStatus IRSetElement::Activate( ToolContext& in_ctxt )
{
	long v;
	//UIToolkit().MsgBox(L"Nb Joints : "+(CString)_joints.size()+", Nb Points : "+(CString)points.GetCount(),0,L"Caption",v);
	UIToolkit().MsgBox(L"Activate Tool ? ",siMsgOkOnly ,L"Caption",v);
	Application().LogMessage(L"IRSetElement Activate Called!!!");
	if(IsValid())
	{
		// Called when tool becomes the active tool
		in_ctxt.SetCursor( siCrossCursor );

		//Uncomment this to setup a custom tool shortcut key
		//in_ctxt.RegisterShortcutKey( 0x09/*Tab*/, L"Tab" );

		in_ctxt.SetToolDescription( L"ICERigTools\nLeft-button\nMiddle-button\nRight-button" );
		in_ctxt.EnableSnapping(true);

		return CStatus::OK;
	}
	else
	{
		DoExitTool(in_ctxt);
		return CStatus::OK;
	}
}


CStatus IRSetElement::MouseDown( ToolContext& in_ctxt )
{
	if(!IsValid())return CStatus::Fail;

	in_ctxt.GetMousePosition( m_x, m_y );

	if ( in_ctxt.IsLeftButtonDown() )
	{
		if(m_handle!=NULL)m_handle->MouseDown(in_ctxt);
		/*
		IRJoint* jnt = _element->GetActiveJoint();
		if(jnt == NULL)return CStatus::Fail;
		
		_activepos = jnt->GetOffsetPosition();

		CPlane plane(jnt->GetPosition(),jnt->GetNormal());
		in_ctxt.SetManipulationPlane(plane);
		
		_mode = _element->GetToolMode();
		_handle->Setup(jnt, _mode);
		_handle->MouseDown(in_ctxt);
		
		if(_mode == IRShapeScaleMode || _mode == IRJointScaleMode)
		{
			in_ctxt.SetCursor( siScaleCursor );
		}
		else if(_mode == IRShapeTranslateMode || _mode == IRJointTranslateMode)
		{
			in_ctxt.SetCursor( siTranslateCursor );
		}

		_basewidth = jnt->GetWidth();
		_basedepth = jnt->GetDepth();
		_axis = jnt->GetActiveAxis(in_ctxt,_mode);

		CVector3 v;
		in_ctxt.GetWorldPosition( _x,_y, v );

		//Get Distance from Offseted centers
		CVector3 delta;
		delta.Sub(v,_activepos);
		_baselength = (float)delta.GetLength();
		*/
		return CStatus::OK; // To start MouseDrag/MouseUp interaction...
	}
	else if ( in_ctxt.IsRightButtonDown() )
	{
		// Show tool context menu
		in_ctxt.ShowContextMenu( m_x,m_y );
	}
	return CStatus::False; // Ignore other mouse buttons etc.
}

CStatus IRSetElement::MouseDrag( ToolContext& in_ctxt )
{
	if ( in_ctxt.IsLeftButtonDown() )
	{
		if(m_handle !=NULL)
		{
			//Application().LogMessage(L"Mouse Drag...");
			m_handle->MouseDrag(in_ctxt);
			//_element->GetJoints();
			//_handle->Draw(in_ctxt);
			//in_ctxt.Redraw(false);
		}
	}
	return CStatus::OK;
}

CStatus IRSetElement::MouseUp( ToolContext& in_ctxt )
{
	if(m_handle!=NULL)m_handle->MouseUp(in_ctxt);
	/*
	Application().LogMessage(L"Mouse Up Called!!");
	in_ctxt.SetCursor(siCrossCursor);
	*/
	
	//in_ctxt.Redraw( false );
	return CStatus::OK;
}

CStatus IRSetElement::Draw( ToolContext& in_ctxt )
{
	if ( IsValid() && in_ctxt.IsActiveView() )
	{
		//_element->GetActiveJoint(in_ctxt);
		in_ctxt.BeginViewDraw();
		
		m_handle->Draw(in_ctxt);
		m_element->DrawJoints(in_ctxt);
		m_element->DrawSkin(in_ctxt);


		//Display length
		if(in_ctxt.IsInteracting())
		{
			CString l_Str = (CString)m_activewidth;
			LONG width = 0, height = 0, descent = 0;
			in_ctxt.GetTextSize( l_Str, width, height, descent );
			GLint x = (GLint)m_x - width/2, y = (GLint)m_y + 20;//pixels

			// Draw background
			glColor3d(160/255.0, 1.0, 1.0);
			glBegin( GL_QUADS );
			glVertex2d( x - 2, y - descent + height + 2 );
			glVertex2d( x - 2, y - descent - 2);
			glVertex2d( x + width + 2, y - descent - 2 );
			glVertex2d( x + width + 2, y - descent + height + 2 );
			glEnd();

			// Draw outline
			glColor3d( 0, 0, 0 );
			glBegin( GL_LINE_LOOP );
			glVertex2d( x - 2, y - descent + height + 2 );
			glVertex2d( x - 2, y - descent - 2 );
			glVertex2d( x + width + 2, y - descent - 2 );
			glVertex2d( x + width + 2, y - descent + height + 2 );
			glEnd();

			// Draw text
			glRasterPos2i( x, y );
			in_ctxt.DrawTextString( L"Fuck You MotherFuckers!!" );

		}


		in_ctxt.EndViewDraw();
	}
	return CStatus::OK;
}

bool IRSetElement::CheckSelection(ToolContext& in_ctxt)
{
	if (IsValid() && m_gid != g_selectionID)
	{
		m_element->WriteData();
		Setup(in_ctxt);
		return true;
	}
	return false;
}

CStatus IRSetElement::MouseMove( ToolContext& in_ctxt )
{
	if(!IsValid() || in_ctxt.IsInteracting())return CStatus::OK;

	CheckSelection(in_ctxt);
	m_activejoint = m_element->GetActiveJoint(in_ctxt);

	if (m_activejoint>-1)
	{
		m_handle->MouseMove(in_ctxt);
		in_ctxt.Redraw(false);
	}
	
	return CStatus::OK;
}


CStatus IRSetElement::ModifierChanged( ToolContext& in_ctxt )
{
	// Called when modifier key (Shift/Control) changes
	return CStatus::OK;
}

CStatus IRSetElement::SelectionChanged( ToolContext& in_ctxt )
{
	//Application().LogMessage(L"Selection Changed...");
	return CStatus::OK;
}

CStatus IRSetElement::MenuInit( ToolContext& in_ctxt )
{
	// Called to initialize the context menu
	MenuItem menuItem;
	Menu menu = in_ctxt.GetSource();
	menu.AddCallbackItem( L"Shape Mode", L"IRSetElement_SetShapeMode", menuItem );
	menu.AddCallbackItem( L"Move Mode", L"IRSetElement_SetMoveMode", menuItem );
	menu.AddCallbackItem( L"Offset Mode", L"IRSetElement_SetOffsetMode", menuItem );
	menu.AddCallbackItem( L"UpVector Mode", L"IRSetElement_SetUpVectorMode", menuItem);
	menu.AddCallbackItem(L"Skeleton Mode", L"IRSetElement_SetSkeletonMode", menuItem);
	menu.AddSeparatorItem();
	menu.AddCallbackItem( L"Exit Tool", L"IRSetElement_DoExitTool", menuItem );
	return CStatus::OK;
}

CStatus IRSetElement::SetShapeMode(ToolContext& in_ctxt)
{
	SetToolMode(IRShapeMode);
	SetupHandle();
	return CStatus::OK;
}

CStatus IRSetElement::SetMoveMode(ToolContext& in_ctxt)
{
	SetToolMode(IRMoveMode);
	SetupHandle();
	return CStatus::OK;
}

CStatus IRSetElement::SetOffsetMode(ToolContext& in_ctxt)
{
	SetToolMode(IROffsetMode);
	SetupHandle();
	return CStatus::OK;
}

CStatus IRSetElement::SetUpVectorMode(ToolContext& in_ctxt)
{
	SetToolMode(IRUpVectorMode);
	SetupHandle();
	return CStatus::OK;
}

CStatus IRSetElement::SetSkeletonMode(ToolContext& in_ctxt)
{
	SetToolMode(IRSkeletonMode);
	SetupHandle();
	return CStatus::OK;
}

CStatus IRSetElement::SetToolMode( IRToolMode mode )
{
	m_element->SetToolMode(mode);
	SetupHandle();
	return CStatus::OK;
}

CStatus IRSetElement::DoExitTool( ToolContext& in_ctxt )
{
	if(m_element != NULL)
		m_element->WriteData();
	m_valid = false;
	in_ctxt.ExitTool();
	return CStatus::OK;
}



SICALLBACK IRSetElement_SelectionChanged_OnEvent( CRef& in_ctxt )
{
	// Make sure tool resets orientation
	IRSetElement::IncrementGlobalSelectionID();
	//Application().LogMessage(L"Selection ID : "+(CString)_selectionID);
	return CStatus::OK;
}

////////////////////////////////////////////////////////////////////////////////
//
// Plugin Callbacks
//
////////////////////////////////////////////////////////////////////////////////

SICALLBACK IRSetElement_Init( CRef& in_ctxt )
{
	IRSetElement *l_pTool = new IRSetElement;
	if ( !l_pTool ) return CStatus::Fail;
	ToolContext l_ctxt( in_ctxt );
	l_ctxt.PutUserData( CValue((CValue::siPtrType)l_pTool) );
	return CStatus::OK;
}

SICALLBACK IRSetElement_Term( CRef& in_ctxt )
{
	ToolContext l_ctxt( in_ctxt );
	IRSetElement *l_pTool = (IRSetElement *)(CValue::siPtrType)l_ctxt.GetUserData();
	if ( !l_pTool ) return CStatus::Fail;
	delete l_pTool;
	l_ctxt.PutUserData( CValue((CValue::siPtrType)NULL) ); // Clear user data
	return CStatus::OK;
}

// Use a macro to simplify callback setup and forwarding...
#define DECLARE_TOOL_CALLBACK(TOOL,CALLBACK) \
SICALLBACK TOOL##_##CALLBACK( ToolContext& in_ctxt ) { \
	TOOL *l_pTool = (TOOL *)(CValue::siPtrType)in_ctxt.GetUserData(); \
	return ( l_pTool ? l_pTool->CALLBACK( in_ctxt ) : CStatus::Fail ); }

DECLARE_TOOL_CALLBACK( IRSetElement, Setup );
DECLARE_TOOL_CALLBACK( IRSetElement, Cleanup );
DECLARE_TOOL_CALLBACK( IRSetElement, Activate );
DECLARE_TOOL_CALLBACK( IRSetElement, MouseDown );
DECLARE_TOOL_CALLBACK( IRSetElement, MouseDrag );
DECLARE_TOOL_CALLBACK( IRSetElement, MouseUp );
DECLARE_TOOL_CALLBACK( IRSetElement, Draw );
DECLARE_TOOL_CALLBACK( IRSetElement, MouseMove );
DECLARE_TOOL_CALLBACK( IRSetElement, ModifierChanged );
DECLARE_TOOL_CALLBACK( IRSetElement, SelectionChanged );
DECLARE_TOOL_CALLBACK( IRSetElement, MenuInit );
DECLARE_TOOL_CALLBACK( IRSetElement, DoExitTool );
DECLARE_TOOL_CALLBACK( IRSetElement, SetShapeMode );
DECLARE_TOOL_CALLBACK( IRSetElement, SetOffsetMode );
DECLARE_TOOL_CALLBACK( IRSetElement, SetMoveMode );
DECLARE_TOOL_CALLBACK( IRSetElement, SetUpVectorMode);
DECLARE_TOOL_CALLBACK( IRSetElement, SetSkeletonMode);
