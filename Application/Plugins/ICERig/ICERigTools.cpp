// ICERigCustomTools 

#include "IRRegister.h"
#include "IRElement.h"

using namespace XSI::MATH; 
using namespace XSI; 

/*Global*/ULONG _selectionID = 0;

SICALLBACK XSILoadPlugin( PluginRegistrar& in_reg )
{
	in_reg.PutAuthor(L"benmalartre");
	in_reg.PutName(L"ICERigTools Plugin");
	in_reg.PutVersion(1,0);
	in_reg.RegisterTool(L"IRSetElement");
	in_reg.RegisterEvent( L"IRSetElement_SelectionChanged", siOnSelectionChange );
	//in_reg.RegisterTool(L"IRManipulator");
	//RegistrationInsertionPoint - do not remove this line

	return CStatus::OK;
}

////////////////////////////////////////////////////////////////////////////////
//
// IRSetElement
//
////////////////////////////////////////////////////////////////////////////////
 
class IRSetElement {
private:
	IRElement*	_element;
	LONG		_x;
	LONG 		_y;
	JointAxis	_axis;
	float		_activewidth;
	float		_activedepth;
	float		_globalwidth;
	float		_globaldepth;
	float		_basewidth;
	float		_basedepth;
	float		_baselength;
	LONG		_activejoint;
	CVector3	_activepos;
	CRefArray	_picked;
	float		_l;

public: // Methods
	IRSetElement()
	{
		_element = NULL;
	}

	~IRSetElement()
	{
		if(_element !=NULL)delete _element;
	}

	CStatus Setup( ToolContext& in_ctxt )
	{
		_element  = new IRElement();
		// Called when tool is created
		Selection sel = Application().GetSelection();
		X3DObject crv = sel.GetItem(0);
		if(crv.GetType() != L"crvlist")
		{
			Application().LogMessage(L"Invalid Selection : you need to select a Rig Element Curve!!", siErrorMsg);
			return CStatus::InvalidArgument;
		}
		_element->SetCurve(crv);

		return CStatus::OK;
	}

	CStatus Cleanup( ToolContext& in_ctxt )
	{
		// Called when tool is destroyed
		return CStatus::OK;
	}

	CStatus Activate( ToolContext& in_ctxt )
	{
		// Called when tool becomes the active tool
		in_ctxt.SetCursor( siScaleCursor );
		
			//siArrowCursor	= 0,		/*!< Standard arrow cursor. */
			//siCrossCursor	= 1,		/*!< Crosshair cursor. */
			//siHandCursor	= 2,		/*!< Pointing hand cursor. */
			//siPenCursor	= 3,		/*!< Pen cursor. */
			//siDropperCursor	= 4,		/*!< Eye dropper cursor. */
			//siSelectCursor	= 5,		/*!< Selection cursor. */
			//siTranslateCursor	= 6,		/*!< Translate cursor. */
			//siRotateCursor	= 7,		/*!< Rotate cursor. */
			//siScaleCursor	= 8,		/*!< Scale cursor. */
			//siZoomCursor	= 9,		/*!< Magnifying glass cursor. */
			//siDragXYCursor	= 10,		/*!< 4-way arrow cursor. */
			//siDragXCursor	= 11,		/*!< Horizontal arrow cursor. */
			//siDragYCursor	= 12		/*!< Vertical arrow cursor. */

		//Uncomment this to setup a custom tool shortcut key
		//in_ctxt.RegisterShortcutKey( 0x09/*Tab*/, L"Tab" );


		in_ctxt.SetToolDescription( L"ICERigTools\nLeft-button\nMiddle-button\nRight-button" );
		in_ctxt.EnableSnapping(true);

		return CStatus::OK;
	}

	CStatus Deactivate( ToolContext& in_ctxt )
	{
		in_ctxt.EnableSnapping(false);
		return CStatus::OK;
	}

	CStatus MouseDown( ToolContext& in_ctxt )
	{
		CRef r = _picked.GetItem(0);
		in_ctxt.GetMousePosition( _x, _y );

		if ( in_ctxt.IsLeftButtonDown() )
		{
			IRJoint* jnt = _element->GetActiveJoint();
			if(jnt == NULL)return CStatus::Fail;
			
			_activepos = jnt->GetOffsetPosition();

			ToolMode mode = _element->GetToolMode();


			if(mode == ShapeMode)
			{
				CPlane plane(jnt->GetPosition(),jnt->GetNormal());
				in_ctxt.SetManipulationPlane(plane);
			}
			else if(mode == TranslateMode)
			{
				CVector3 normal(0,1,0);
				if(_axis==AxisY)normal.Set(1,0,0);
				CPlane plane(jnt->GetPosition(),jnt->GetNormal());
				in_ctxt.SetManipulationPlane(plane);
			}

			_basewidth = jnt->GetWidth();
			_basedepth = jnt->GetDepth();
			_axis = jnt->GetActiveAxis(in_ctxt,mode);

			CVector3 v;
			in_ctxt.GetWorldPosition( _x, _y, v );
	
			//Get Distance from Offseted centers
			CVector3 delta;
			delta.Sub(v,_activepos);
			_baselength = delta.GetLength();
			

			return CStatus::OK; // To start MouseDrag/MouseUp interaction...
		}
		else if ( in_ctxt.IsRightButtonDown() )
		{
			// Show tool context menu
			in_ctxt.ShowContextMenu( _x, _y );
		}
		return CStatus::False; // Ignore other mouse buttons etc.
	}

	CStatus MouseDrag( ToolContext& in_ctxt )
	{
		if ( in_ctxt.IsLeftButtonDown() )
		{
			IRJoint* jnt = _element->GetActiveJoint();
			if(jnt == NULL)return CStatus::InvalidArgument;

			// Called when mouse is moved with a mouse button pressed
			LONG x, y;
			in_ctxt.GetMousePosition( x, y );
			CVector3 v;
			in_ctxt.GetWorldPosition( x, y, v );

			//Get Base Length
			CVector3 delta,ax,az;
			delta.Sub(v,_activepos);
			float l = delta.GetLength() - _baselength;

			ToolMode mode = _element->GetToolMode();
			//ShapeMode
			if(mode==ShapeMode)
			{
				if(_axis == AxisX)
				{
					_activewidth = _basewidth+l;
					jnt->SetWidth(_activewidth);
				}
				else if(_axis == AxisZ)
				{
					_activedepth = _basedepth+l;
					jnt->SetDepth(_activedepth);
				}
			}

			else if(mode == TranslateMode)
			{
			
				CVector3 p;
				if(_axis == AxisX)p.Set(delta.GetX(),0,0);
				else if(_axis == AxisY)p.Set(0,delta.GetY(),0);
				else if(_axis == AxisZ)p.Set(0,0,delta.GetZ());
				//p.Add(jnt->GetPosition(),delta);
				jnt->SetOffset(p,_axis);
			}
			
		}
		return CStatus::OK;
	}

	CStatus MouseUp( ToolContext& in_ctxt )
	{
		Application().LogMessage(L"Mouse Up Called!!");
		if ( in_ctxt.IsLeftButtonDown() )
		{
			_element->SetWidth();
			_element->SetDepth();
			_element->SetOffset();
		}
		in_ctxt.Redraw( false );
		return CStatus::OK;
	}

	/*
	CStatus Draw( ToolContext& in_ctxt )
	{
		// Called when a redraw occurs to allow the tool to show feedback
		if ( in_ctxt.IsActiveView() )
		{
			// This section will only be drawn for the active view (under the cursor)...
		}
		if ( in_ctxt.IsInteracting() )
		{
			// This section will only be drawn when dragging with a mouse button pressed...
		}
		return CStatus::OK;
	}
	*/

	CStatus Draw( ToolContext& in_ctxt )
	{
		Application().LogMessage(L"Draw Called...");
		if ( in_ctxt.IsActiveView() )
		{
			if (_element->IsValid())
			{
				in_ctxt.BeginViewDraw();

				_element->DrawJoints( in_ctxt);
				
				//Display length
				if(in_ctxt.IsInteracting())
				{
					CString l_Str = (CString)_activewidth;
					LONG width = 0, height = 0, descent = 0;
					in_ctxt.GetTextSize( l_Str, width, height, descent );
					GLint x = (GLint)_x - width/2, y = (GLint)_y + 20;//pixels

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
		}
		return CStatus::OK;
	}

	CStatus MouseMove( ToolContext& in_ctxt )
	{
		if(in_ctxt.IsInteracting())return CStatus::OK;

		_activejoint = _element->GetActiveJoint(in_ctxt);
		
		if(_activejoint>-1)
		{
			Application().LogMessage(L"Active Joint : "+(CString)_activejoint);
			in_ctxt.Redraw( false );
		}
		return CStatus::OK;
	}

	CStatus MouseEnter( ToolContext& in_ctxt )
	{
		// Called when mouse enters a view
		Camera camera = in_ctxt.GetCamera();
		Application().LogMessage( L"Entering camera view: "  + camera.GetName() );
		return CStatus::OK;
	}

	CStatus MouseLeave( ToolContext& in_ctxt )
	{
		// Called when mouse leaves a view
		return CStatus::OK;
	}

	CStatus ModifierChanged( ToolContext& in_ctxt )
	{
		// Called when modifier key (Shift/Control) changes
		return CStatus::OK;
	}

	CStatus SelectionChanged( ToolContext& in_ctxt )
	{
		Application().LogMessage(L"Selection Changed...");
		Selection sel = Application().GetSelection();
		X3DObject crv = sel.GetItem(0);
		if(crv.GetType() != L"crvlist")
		{
			Application().LogMessage(L"Invalid Selection : you need to select a Rig Element Curve!!", siErrorMsg);
			return CStatus::InvalidArgument;
		}
		_element->SetCurve(crv);
		return CStatus::OK;
	}

	CStatus MenuInit( ToolContext& in_ctxt )
	{
		// Called to initialize the context menu
		MenuItem menuItem;
		Menu menu = in_ctxt.GetSource();
		menu.AddCallbackItem( L"Shape Mode", L"IRSetElement_SetShapeMode", menuItem );
		menu.AddCallbackItem( L"Translate Mode", L"IRSetElement_SetTranslateMode", menuItem );
		menu.AddSeparatorItem();
		menu.AddCallbackItem( L"Exit Tool", L"IRSetElement_DoExitTool", menuItem );
		return CStatus::OK;
	}

	CStatus SetShapeMode( ToolContext& in_ctxt )
	{
		_element->SetToolMode(ShapeMode);
		Application().LogMessage( L"Set  Shape Mode..." );
		return CStatus::OK;
	}

	CStatus SetTranslateMode( ToolContext& in_ctxt )
	{
		_element->SetToolMode(TranslateMode);
		Application().LogMessage( L"Set  Translate Mode..." );
		return CStatus::OK;
	}


	CStatus DoExitTool( ToolContext& in_ctxt )
	{
		in_ctxt.ExitTool();
		return CStatus::OK;
	}

	static ULONG IncrementGlobalSelectionID()
	{
		if ( ++_selectionID == 0 )
			++_selectionID; // Reserve 0 as an invalid ID
		return _selectionID;
	}

}; // end class

SICALLBACK IRSetElement_SelectionChanged_OnEvent( CRef& in_ctxt )
{
	// Make sure tool resets orientation
	IRSetElement::IncrementGlobalSelectionID();
	Application().LogMessage(L"Selection ID : "+(CString)_selectionID);
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
DECLARE_TOOL_CALLBACK( IRSetElement, Deactivate );
DECLARE_TOOL_CALLBACK( IRSetElement, MouseDown );
DECLARE_TOOL_CALLBACK( IRSetElement, MouseDrag );
DECLARE_TOOL_CALLBACK( IRSetElement, MouseUp );
DECLARE_TOOL_CALLBACK( IRSetElement, Draw );
DECLARE_TOOL_CALLBACK( IRSetElement, MouseMove );
DECLARE_TOOL_CALLBACK( IRSetElement, MouseEnter );
DECLARE_TOOL_CALLBACK( IRSetElement, MouseLeave );
DECLARE_TOOL_CALLBACK( IRSetElement, ModifierChanged );
DECLARE_TOOL_CALLBACK( IRSetElement, SelectionChanged );
DECLARE_TOOL_CALLBACK( IRSetElement, MenuInit );
DECLARE_TOOL_CALLBACK( IRSetElement, SetShapeMode );
DECLARE_TOOL_CALLBACK( IRSetElement, SetTranslateMode );
DECLARE_TOOL_CALLBACK( IRSetElement, DoExitTool );
