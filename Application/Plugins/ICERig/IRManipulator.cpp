/*--------------------------------------------------------------------
 IRManipulator
---------------------------------------------------------------------*/
#include "IRRegister.h"

enum Axis
{
  kAxisX,
  kAxisY,
  kAxisZ,
  kNoAxis
};
#define kHandleDragThreshold 5 //pixels

class IRManipulator {
private: 
  LONG            _cursorX;
  LONG            _cursorY;
  LONG            _startX;
  LONG            _startY;
  X3DObject       _picked;
  CPlane          _plane;
  CTransformation _tra;
  float           _axissize;
  Axis            _activeaxis;
  CLine           _axis[3];
  CVector3        _origin;
  CVector3        _initialPosition;
  CRefArray       _object;

public: 
  IRManipulator():_axissize(3.0){}

  ~IRManipulator(){}

  CStatus Setup( ToolContext& in_ctxt )
  {
    Application().LogMessage( L"IRManipulator Setup Called" );
    return CStatus::OK;
  }

  CStatus Cleanup( ToolContext& in_ctxt )
  {
    return CStatus::OK;
  }

  CStatus Activate( ToolContext& in_ctxt )
  {
    // Called when tool becomes the active tool
    in_ctxt.SetCursor( siSelectCursor );

    // Uncomment this to enable standard tool snapping behaviour
    //in_ctxt.EnableSnapping( true );

    //Uncomment this to setup a custom tool shortcut key
    //in_ctxt.RegisterShortcutKey( 0x09/*Tab*/, L"Tab" );

    // Set custom status message
    in_ctxt.SetToolDescription( L"IRManipulator\nLeft-button\nMiddle-button\nRight-button" );

    return CStatus::OK;
  }

  CStatus Deactivate( ToolContext& in_ctxt )
  {
    // Called when tool becomes inactive (e.g. when user orbits using nav or chooses another tool)
    return CStatus::OK;
  }

  CStatus MouseDown( ToolContext& in_ctxt )
  {
    if(!_picked.IsValid())return CStatus::False;

    // Called when mouse button is pressed
    in_ctxt.GetMousePosition( _startX,_startY );

    if ( in_ctxt.IsLeftButtonDown() ) {
      // Get world cursor ray
      CLine cursorRay;
      in_ctxt.EnableSnapping( false );
      if ( in_ctxt.GetWorldRay( _startX, _startY, cursorRay ) != CStatus::OK )
      return CStatus::Fail;

      // Will cursor intersect drag axis?
      if ( cursorRay.Parallel( _axis[_activeaxis] ) )
      return CStatus::Fail;

      // Compute intersection between cursor ray and drag axis
      _initialPosition = _axis[_activeaxis].GetClosestPoint( cursorRay );

      // Reject intersection points behind the camera
      CVector3 l_cameraPos;
      if ( in_ctxt.WorldToCamera( _initialPosition, l_cameraPos ) != CStatus::OK || l_cameraPos.GetZ() > 0.0 )
      return CStatus::Fail;

      _object.Clear();
      _object.Add(_picked);
      in_ctxt.BeginTransformUpdate(_object);
      return CStatus::OK;
    } else if ( in_ctxt.IsRightButtonDown() ) {
      // Show tool context menu
      in_ctxt.ShowContextMenu( _startX, _startY );
    }
    return CStatus::False; // Ignore other mouse buttons etc.
  }

  CStatus MouseDrag( ToolContext& in_ctxt )
  {
    // Called when mouse is moved with a mouse button pressed
    LONG x, y;
    in_ctxt.GetMousePosition( x, y );

    if ( in_ctxt.IsLeftButtonDown() ) {
      // Get world cursor ray
      CLine cursorRay;
      in_ctxt.EnableSnapping( false );
      if ( in_ctxt.GetWorldRay( x, y, cursorRay ) != CStatus::OK )
        return CStatus::Fail;

      // Will cursor intersect drag axis?
      if ( cursorRay.Parallel( _axis[_activeaxis] ) )
        return CStatus::Fail;

      // Compute intersection between cursor ray and drag axis
      CVector3 l_position = _axis[_activeaxis].GetClosestPoint( cursorRay );

      // Reject intersection points behind the camera
      CVector3 l_cameraPos;
      if ( in_ctxt.WorldToCamera( l_position, l_cameraPos ) != CStatus::OK || l_cameraPos.GetZ() > 0.0 )
        return CStatus::Fail;
	
      // Compute relative translation
      CVector3 l_translation;
      l_translation.Sub( l_position, _initialPosition );

      _tra.SetTranslation(l_translation);
      // Apply the transform
      CTransformation xfo;
      xfo.SetTranslation( l_translation );
      in_ctxt.UpdateTransform( _object, xfo, siTrn, siGlobal, siRelative, siXYZ );
    }
    return CStatus::OK;
  }

  CStatus MouseUp( ToolContext& in_ctxt )
  {
    in_ctxt.EndTransformUpdate( _object );
    _object = CRefArray();
    in_ctxt.EnableSnapping( false );
    return CStatus::OK;
  }

  CStatus Draw( ToolContext& in_ctxt )
  {
    // Called when a redraw occurs to allow the tool to show feedback
    if ( in_ctxt.IsActiveView() ) {
      if ( _picked.IsValid() ) {
        in_ctxt.BeginViewDraw();
        DrawAxis(in_ctxt,kAxisX);
        DrawAxis(in_ctxt,kAxisY);
        DrawAxis(in_ctxt,kAxisZ);
      }
    }
    return CStatus::OK;
  }

  void DrawAxis(ToolContext& in_ctxt,Axis axis)
  {
  CVector3 a;
  switch(axis)
  {
    case kAxisX:
      if(_activeaxis==kAxisX)
        glColor3f(1.0,1.0,0.0);
      else
        glColor3f(1.0,0.0,0.0);
      a.Set(_axissize,0.0,0.0);
      break;
    case kAxisY:
      if(_activeaxis==kAxisY)
        glColor3f(1.0,1.0,0.0);
      else
        glColor3f(0.0,1.0,0.0);
      a.Set(0.0,_axissize,0.0);
      break;
    case kAxisZ:
      if(_activeaxis==kAxisZ)
        glColor3f(1.0,1.0,0.0);
      else
        glColor3f(0.0,0.0,1.0);
      a.Set(0.0,0.0,_axissize);
      break;
  }

  CMatrix4 vm, tm;
  in_ctxt.GetWorldToViewMatrix(vm);
  tm = _tra.GetMatrix4();
  tm.MulInPlace(vm);

  GLdouble mat[16];
  tm.Get(
    mat[0],  mat[1],  mat[2],  mat[3],
    mat[4],  mat[5],  mat[6],  mat[7],
    mat[8],  mat[9],  mat[10], mat[11],
    mat[12], mat[13], mat[14], mat[15]
  );
  glPushMatrix();
  glMultMatrixd(mat);

  glBegin( GL_LINES );
  glVertex3d(0.0,0.0,0.0);
  glVertex3d(a.GetX(),a.GetY(),a.GetZ());
  glEnd();

  glTranslated( a.GetX(),a.GetY(),a.GetZ());
  if(axis==kAxisX)
    glRotatef(90.0,0.0,1.0,0.0);
  else if(axis == kAxisY)
   glRotatef(-90.0,1.0,0.0,0.0);
  DrawCone(_axissize);
  glPopMatrix();
}

  CStatus MouseMove( ToolContext& in_ctxt )
  {
    if(! in_ctxt.IsInteracting()) {
      in_ctxt.GetMousePosition( _cursorX, _cursorY );

      CLongArray points;
      points.Add( _cursorX );
      points.Add( _cursorY );

      // First get object under the cursor
      in_ctxt.Pick( points, siPickSingleObject, siPickRaycast, L"", CRefArray(), _object );
      if(_object.GetCount()>0) {
        _picked = _object.GetItem(0);
        _tra = _picked.GetKinematics().GetGlobal().GetTransform();
        GetAxis();
        GetActiveAxis(in_ctxt);
      } else { 
        _picked = CRef();
        _tra = CTransformation();
        _activeaxis = kNoAxis;
      }
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
    // Selection has changed...
    return CStatus::OK;
  }

  CStatus KeyDown( ToolContext& in_ctxt )
  {
    // Called when a registered tool shortcut key is pressed
    switch (in_ctxt.GetShortcutKey()) 
    {
      case 0x09:/*Tab*/ 
        // Tab key pressed...
        Application().LogMessage( L"Tab shortcut key pressed" );
        break;
      default:
        break;
    }
    return CStatus::OK;
  }

  CStatus KeyUp( ToolContext& in_ctxt )
  {
    // Called when a registered tool shortcut key is release
    return CStatus::OK;
  }

  CStatus SnapValid( ToolContext& in_ctxt )
  {
    // Return CStatus::OK if we can snap to this target...
    bool canSnapToTarget = true;
    //siSnapType snapType = in_ctxt.GetSnapType();
    //CRef snapObject = in_ctxt.GetSnapObject();
    return canSnapToTarget ? CStatus::OK : CStatus::False;
  }

  CStatus MenuInit( ToolContext& in_ctxt )
  {
    // Called to initialize the context menu
    MenuItem menuItem;
    Menu menu = in_ctxt.GetSource();
    menu.AddCallbackItem( L"Menu Item 1", L"IRManipulator_DoMenuItem1", menuItem );
    menu.AddSeparatorItem();
    menu.AddCallbackItem( L"Exit Tool", L"IRManipulator_DoExitTool", menuItem );
    return CStatus::OK;
  }

  CStatus DoMenuItem1( ToolContext& in_ctxt )
  {
    // Menu item selected...
    Application().LogMessage( L"Menu item 1 selected" );
    return CStatus::OK;
  }

  CStatus DoExitTool( ToolContext& in_ctxt )
  {
    in_ctxt.ExitTool();
    return CStatus::OK;
  }

  void GetAxis()
  {
    _origin = _tra.GetTranslation();
    CMatrix3 matrix = _tra.GetRotation().GetMatrix();
    CVector3 tangent(1.0,0.0,0.0);
    tangent.MulByMatrix3InPlace(matrix);
    _axis[0].Set(_origin,tangent);
    tangent.Set(0.0,1.0,0.0);
    tangent.MulByMatrix3InPlace(matrix);
    _axis[1].Set(_origin,tangent);
    tangent.Set(0.0,0.0,1.0);
    tangent.MulByMatrix3InPlace(matrix);
    _axis[2].Set(_origin,tangent);
  }

  void GetActiveAxis(ToolContext& in_ctxt)
  {
    LONG x,y;
    in_ctxt.GetMousePosition(x,y);
    CVector3 o;
    in_ctxt.WorldToView(_origin,o);

    CVector3 mouse(x-o.GetX(),y-o.GetY(),0);
    _activeaxis = kNoAxis; // No axis
    double maxProjection = kHandleDragThreshold;

    for ( int i = 0; i < 3; i++ ) {
      CVector3 initial, axis;
      if ( in_ctxt.WorldToView( _axis[i].GetOrigin(), initial ) != CStatus::OK ||
        in_ctxt.WorldToView( _axis[i].GetPosition(1.0), axis ) != CStatus::OK )
          return;

      // Convert to a 2D vector (without depth)
      axis.SubInPlace( initial ).PutZ( 0.0 );
      double lenSquared = axis.GetLengthSquared();
      if ( lenSquared > kHandleDragThreshold * kHandleDragThreshold ) {
        double projectedLen = fabs(mouse.Dot(axis)) * axis.GetLength() / lenSquared;
        if ( projectedLen > maxProjection ) {
          _activeaxis = (Axis)i;
          maxProjection = projectedLen;
        }
      }
    }
  }

}; 

/*------------------------------------------------------------------
    Plugin Callbacks
------------------------------------------------------------------*/
SICALLBACK IRManipulator_Init( CRef& in_ctxt )
{
  IRManipulator *l_pTool = new IRManipulator;
  if ( !l_pTool ) return CStatus::Fail;
  ToolContext l_ctxt( in_ctxt );
  l_ctxt.PutUserData( CValue((CValue::siPtrType)l_pTool) );
  return CStatus::OK;
}

SICALLBACK IRManipulator_Term( CRef& in_ctxt )
{
  ToolContext l_ctxt( in_ctxt );
  IRManipulator *l_pTool = (IRManipulator *)(CValue::siPtrType)l_ctxt.GetUserData();
  if ( !l_pTool ) return CStatus::Fail;
  delete l_pTool;
  l_ctxt.PutUserData( CValue((CValue::siPtrType)NULL) ); // Clear user data
  return CStatus::OK;
}

// Use a macro to simplify callback setup and forwarding...
#define DECLARE_TOOL_CALLBACK(TOOL,CALLBACK)                          \
SICALLBACK TOOL##_##CALLBACK( ToolContext& in_ctxt ) {                \
  TOOL *l_pTool = (TOOL *)(CValue::siPtrType)in_ctxt.GetUserData();   \
  return ( l_pTool ? l_pTool->CALLBACK( in_ctxt ) : CStatus::Fail ); }

DECLARE_TOOL_CALLBACK( IRManipulator, Setup );
DECLARE_TOOL_CALLBACK( IRManipulator, Cleanup );
DECLARE_TOOL_CALLBACK( IRManipulator, Activate );
DECLARE_TOOL_CALLBACK( IRManipulator, Deactivate );
DECLARE_TOOL_CALLBACK( IRManipulator, MouseDown );
DECLARE_TOOL_CALLBACK( IRManipulator, MouseDrag );
DECLARE_TOOL_CALLBACK( IRManipulator, MouseUp );
DECLARE_TOOL_CALLBACK( IRManipulator, Draw );
DECLARE_TOOL_CALLBACK( IRManipulator, MouseMove );
DECLARE_TOOL_CALLBACK( IRManipulator, MouseEnter );
DECLARE_TOOL_CALLBACK( IRManipulator, MouseLeave );
DECLARE_TOOL_CALLBACK( IRManipulator, ModifierChanged );
DECLARE_TOOL_CALLBACK( IRManipulator, SelectionChanged );
DECLARE_TOOL_CALLBACK( IRManipulator, KeyDown );
DECLARE_TOOL_CALLBACK( IRManipulator, KeyUp );
DECLARE_TOOL_CALLBACK( IRManipulator, SnapValid );
DECLARE_TOOL_CALLBACK( IRManipulator, MenuInit );
DECLARE_TOOL_CALLBACK( IRManipulator, DoMenuItem1 );
DECLARE_TOOL_CALLBACK( IRManipulator, DoExitTool );