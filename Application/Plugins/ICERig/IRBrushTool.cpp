// IRSimpleBrush Plugin

#include "IRRegister.h"
#include <xsi_polygonface.h>
#include <xsi_polygonnode.h>
#include <xsi_plugin.h>

bool LogOpenGLErrors(const CString &extraInfo = CString())
{
   bool l_bError = false;
   for (GLenum error; (error = glGetError()) != GL_NO_ERROR;)
   {
      const char *msg = (const char *)gluErrorString(error);
      CString errorMsg = L"SimpleBrush: OpenGL error - ";
      errorMsg += (msg ? msg : "unknown error");
      if (!extraInfo.IsEmpty())
      {
         errorMsg += " (";
         errorMsg += extraInfo;
         errorMsg += ")";
      }
      Application().LogMessage(errorMsg, siWarningMsg);
      l_bError = true;
   }
   return l_bError;
}

class IRSimpleBrush {
private:

   CVector3          m_pos;            // drawing position
   CPlane            m_plane;          // active manipulation plane
   CLine             m_ray;            // current ray/line
   CRefArray         m_targets;        // selection targets to use in the pickbuffer
   //CRef              m_obj;            // object under mouse
   LONG              m_compIdx;        // component under mouse
   CVector3          m_normal;         // current brush alignment
   bool              m_hit;            // hit flag
   LONG              m_numSegments;    // number of segments used to draw the brush
   vector<CVector3>  m_brushPnts;      // cached brush points
   float             m_initRadius;     // initial radius
   float             m_radius;         // brush radius
   PickBuffer        m_pBuffer;        // cached pick buffer
   CLongArray        m_coords;         // view coordinates used to resize brush
   POINT             lp_mouse;         // saved mouse position
   bool              m_isCamDirty;     // is camera dirty?
   CTransformation   m_camXForm;       // saved camera transform
   CRotation         m_brushOri;       // brush orientation

public: // Methods
   IRSimpleBrush()
      : m_radius(3.0f)
      , m_initRadius(3.0f)
      , m_isCamDirty(true)
   {}

   ~IRSimpleBrush()
   {}

   CStatus Setup(ToolContext& in_ctxt)
   {
      m_camXForm.SetIdentity();
      SetRadius(m_radius);
      return CStatus::OK;
   }

   CStatus Cleanup(ToolContext& in_ctxt)
   {
      // Called when tool is destroyed
      return CStatus::OK;
   }

   CStatus Activate(ToolContext& in_ctxt)
   {
      // Called when tool becomes the active tool
      in_ctxt.SetCursor(siSelectCursor);
      Application().LogMessage(L"IRSimpleBrush Activate");

      // get a list of selected meshes
      const CString empty;
      CStringArray families;
      families.Add(siMeshFamily);
      Application().GetSelection().GetArray().Filter(empty, families, empty, m_targets);

      // Uncomment this to enable standard tool snapping behaviour
      //in_ctxt.EnableSnapping( true );

      //Uncomment this to setup a custom tool shortcut key
      in_ctxt.RegisterShortcutKey(0x52/*R*/, L"Adjust brush radius");

      // Set custom status message
      in_ctxt.SetToolDescription(L"SimpleBrush\nLeft-button\nMiddle-button\nRight-button");

      return CStatus::OK;
   }

   CStatus Deactivate(ToolContext& in_ctxt)
   {
      // Called when tool becomes inactive (e.g. when user orbits using nav or chooses another tool)
      Application().LogMessage(L"SimpleBrush Deactivate");
      m_targets.Clear();

      return CStatus::OK;
   }

   CStatus MouseDown( ToolContext& in_ctxt )
   {
      // Called when mouse button is pressed
      LONG x, y;
      in_ctxt.GetMousePosition(x, y);

      if (in_ctxt.IsLeftButtonDown())
      {
         // Left-button handling...
         //Application().LogMessage(L"Mouse Down: <" + CString(x) + L", " + CString(y) + ">");
         if (in_ctxt.IsShortcutKeyDown(0x52))
         {
            // some windows functions for hiding the cursor and storing the
            // absolute screen coordinates. linx compatibility?
            ShowCursor(FALSE);
            GetCursorPos(&lp_mouse);

            // store these screen coords for later use
            m_coords.Clear();
            m_coords.Add(x);
            m_coords.Add(y);
         }
         return CStatus::OK; // To start MouseDrag/MouseUp interaction...
      }
      else if ( in_ctxt.IsRightButtonDown() )
      {
         // Show tool context menu
         in_ctxt.ShowContextMenu(x, y);
      }
      return CStatus::False; // Ignore other mouse buttons etc.
   }

   CStatus MouseDrag( ToolContext& in_ctxt )
   {
      // Called when mouse is moved with a mouse button pressed
      LONG x, y;
      in_ctxt.GetMousePosition(x, y);

      if ( in_ctxt.IsLeftButtonDown() )
      {
         // is r key is down...
         if (in_ctxt.IsShortcutKeyDown(0x52))
         {
            float l_radiusOffset = (float)(x-m_coords[0]);
            float increment = GetInitRadius()/100.0f;
            SetRadius(GetInitRadius() + l_radiusOffset * increment);
         }
         // Left button drag...
         //Application().LogMessage( L"Mouse Drag: <" + CString(x) + L", " + CString(y) + ">");
      }
      return CStatus::OK;
   }

   CStatus MouseUp( ToolContext& in_ctxt )
   {
      // Called when mouse button is released
      LONG x, y;
      in_ctxt.GetMousePosition(x, y);
      
      if (in_ctxt.IsLeftButtonDown())
      {
         // Left button release...
         //Application().LogMessage( L"Mouse Up: <" + CString(x) + L", " + CString(y) + ">");

         // windows function to restore cursor position and visibility
         if (in_ctxt.IsShortcutKeyDown(0x52))
         {
            SetCursorPos(lp_mouse.x, lp_mouse.y);
            ShowCursor(TRUE);

            SetInitRadius(GetRadius());
         }
      }
      return CStatus::OK;
   }

   CStatus Draw(ToolContext& in_ctxt)
   {
      if ( in_ctxt.IsNavigating() )
      {
         // This section will only be drawn when dragging with a mouse button pressed...
      }
      else
      {
         // called when a redraw occurs to allow the tool to show feedback
         if (in_ctxt.IsActiveView())
         {
            glColor4d(0.0, 1.0, 1.0, 1.0);
            glTranslated(m_pos.GetX(), m_pos.GetY(), m_pos.GetZ());
            double angle;
            CVector3 l_axis = m_brushOri.GetAxisAngle(angle);
            glRotated(RadiansToDegrees(angle),l_axis.GetX(),l_axis.GetY(),l_axis.GetZ());
            DrawBrush(m_radius);
         }
      }
      
      LogOpenGLErrors();
      return CStatus::OK;
   }

   CStatus MouseMove( ToolContext& in_ctxt )
   {
      // Called when mouse is moved without any buttons pressed
      LONG x,y;
      in_ctxt.GetMousePosition(x, y);
      CVector3 l_pos;
      if (in_ctxt.GetWorldPosition(x, y, l_pos) == CStatus::OK)
      {
         // reject points behind the camera
         CVector3 l_cameraPos;
         if (in_ctxt.WorldToCamera(l_pos, l_cameraPos) == CStatus::OK && l_cameraPos.GetZ() < 0.0)
         {
            // check if the camera is dirty, cache the pick buffer if it is
            Camera l_camera = in_ctxt.GetCamera();
            if (IsCameraDirty(l_camera) && m_targets.GetCount() > (LONG)0)
            {
               m_pBuffer = in_ctxt.GetPickBuffer(0, 0, 0, 0, siPolygonFilter, m_targets, siShaded);
            }

            m_pos = l_pos;
            in_ctxt.GetGridPlane(m_plane);
            in_ctxt.GetWorldRay(x, y, m_ray);

            if (m_targets.GetCount() > (LONG)0)
            {
               CRef l_obj = m_pBuffer.GetObjectAtPosition(x, y);

               m_hit = l_obj.IsValid();
               if (m_hit)
               {
                  LONG l_compIdx = m_pBuffer.GetComponentIndexAtPosition(x, y);

                  // access components of hit object
                  X3DObject l_xObj(l_obj);
                  PolygonMesh l_polymesh = l_xObj.GetActivePrimitive().GetGeometry();
                  PolygonFace l_face = l_polymesh.GetPolygons().GetItem(l_compIdx);
                  
                  LONG l_vertCount = l_face.GetVertices().GetCount();
                  CVector3Array l_vertPositions = l_face.GetVertices().GetPositionArray();
                  CVector3Array l_nodeNormals = l_face.GetNodes().GetNormalArray();

                  // average position and normals
                  CVector3 p, n;
                  for (LONG i=0; i < l_vertCount; i++)
                  {
                     p += l_vertPositions[i];
                     n += l_nodeNormals[i];
                  }
                  p.PutX(p.GetX()/l_vertCount);
                  p.PutY(p.GetY()/l_vertCount);
                  p.PutZ(p.GetZ()/l_vertCount);
                  n.PutX(n.GetX()/l_vertCount);
                  n.PutY(n.GetY()/l_vertCount);
                  n.PutZ(n.GetZ()/l_vertCount);
                  
                  // setup plane aligned with polygon face
                  m_plane.SetOrigin(p);
                  m_plane.SetNormal(n);
                  m_plane.MulByTransformationInPlace(l_xObj.GetKinematics().GetGlobal().GetTransform());
               }
            }
            
            m_plane.Intersect(m_ray, m_pos);

            CVector3 zVec(0.0, 0.0, 1.0);
            CVector3 planeN = m_plane.GetNormal();
            CVector3 planeU = m_plane.GetTangentU();
            CVector3 planeV = m_plane.GetTangentV();

            m_brushOri.SetFromXYZAxes(planeU, planeV, planeN);

            in_ctxt.Redraw(false);
         }
      }
      // Must call redraw manually if tool state changes (i.e. highlighting)
      //in_ctxt.Redraw();
      return CStatus::OK;
   }

   CStatus MouseEnter( ToolContext& in_ctxt )
   {
      // Called when mouse enters a view
      Camera camera = in_ctxt.GetCamera();
      Application().LogMessage( L"Entering camera view: "  + camera.GetName() );

      //m_camXForm = camera.GetKinematics().GetGlobal().GetTransform();

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
      m_targets.Clear();
      return CStatus::OK;
   }

   CStatus KeyDown( ToolContext& in_ctxt )
   {
      // Called when a registered tool shortcut key is pressed
      switch (in_ctxt.GetShortcutKey()) {
      case 0x52:/*R*/ {
         // r key pressed...
         in_ctxt.SetToolDescription( L"SimpleBrush\nAdjust brush size" );

         break;
      }
      default:
         break;
      }
      return CStatus::OK;
   }

   CStatus KeyUp( ToolContext& in_ctxt )
   {
      // Called when a registered tool shortcut key is release
      in_ctxt.SetToolDescription(L"SimpleBrush\nLeft-button\nMiddle-button\nRight-button");

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
      menu.AddCallbackItem( L"Exit Tool", L"SimpleBrush_DoExitTool", menuItem );
      return CStatus::OK;
   }

   CStatus DoExitTool( ToolContext& in_ctxt )
   {
      in_ctxt.ExitTool();
      return CStatus::OK;
   }

   CStatus SetRadius(float l_radius)
   {
      m_radius = l_radius < 0.001f ? 0.001f : l_radius > 1000.0f ? 1000.0f : l_radius;

      LONG num_segments = 20;
      m_brushPnts.clear();
      m_brushPnts.resize(num_segments);
      for (LONG i=0; i<num_segments; i++)
      {
         float theta = 2.0f * (float)PI * float(i)/ float(num_segments);

         float x = m_radius * cosf(theta);
         float y = m_radius * sinf(theta);
         m_brushPnts[i] = CVector3(x, y, 0.0);
      }
      return CStatus::OK;
   }

   float GetRadius()
   {
      return m_radius;
   }

   CStatus SetInitRadius(float l_radius)
   {
      m_initRadius = l_radius;
      return CStatus::OK;
   }

   float GetInitRadius()
   {
      return m_initRadius;
   }

   CStatus DrawBrush(float radius)
   {
      //glPushMatrix();
      // draw the circle
      glBegin(GL_LINE_LOOP);
      for (size_t i=0; i<m_brushPnts.size(); ++i)
      {
         glVertex3d(m_brushPnts[i].GetX(), m_brushPnts[i].GetY(), m_brushPnts[i].GetZ());
      }
      glEnd();
      
      // draw the brush orientation line
      glBegin(GL_LINE_STRIP);
      glVertex3d(0.0, 0.0, 0.0);
      glVertex3d(0.0, 0.0, 3.0);
      glVertex3d(-0.3, 0.0, 2.4);
      glVertex3d(0.3, 0.0, 2.4);
      glVertex3d(0.0, 0.0, 3.0);
      glVertex3d(0.0, 0.3, 2.4);
      glVertex3d(0.0, -0.3, 2.4);
      glVertex3d(0.0, 0.0, 3.0);
      glEnd();
      //glPopMatrix();

      return CStatus::OK;
   }

   CVector3 GetPolygonNormal(ToolContext& in_ctxt, PolygonFace &in_poly, bool in_mulmatrix)
   {
      X3DObject			in_obj = in_poly.GetParent();
      PolygonMesh			mesh( in_obj.GetActivePrimitive().GetGeometry() );
      CVertexRefArray	verts( in_poly.GetVertices());
      CLongArray			tri_ind( in_poly.GetTriangleSubIndexArray());
      CVector3Array		n,p;	
      				
      UINT vertCnt = verts.GetCount();
      double x=0,y=0,z=0;
      for(UINT i=0;i<vertCnt;i++)
      {
         Vertex current = verts[i];			
         Vertex next = verts[ (i+1) % vertCnt];
         CVector3 c(current.GetPosition() );
         CVector3 n(next.GetPosition() );
         x= x+ ( (c[1]-n[1])* (c[2]+n[2]) );
         y= y+ ( (c[2]-n[2])* (c[0]+n[0]) );
         z= z+ ( (c[0]-n[0])* (c[1]+n[1]) );
      }

      CVector3 normal(x,y,z);		
      if(in_mulmatrix)
      {
         //normal.MulByMatrix3InPlace(in_obj.GetKinematics().GetGlobal().GetTransform().GetRotationMatrix3());
         normal.MulByTransformationInPlace(in_obj.GetKinematics().GetGlobal().GetTransform());
      }
      normal.NormalizeInPlace();
      return normal;
   }

   bool IsCameraDirty(Camera& in_camera)
   {
      CTransformation l_camXForm = in_camera.GetKinematics().GetGlobal().GetTransform();
      
      if (l_camXForm != m_camXForm)
      {
         m_camXForm = l_camXForm;
         return true;
      }
      return false;
   }
}; // end class

SICALLBACK IRSimpleBrush_Init(CRef& in_ctxt)
{
    // check if there is existing user data to reuse
    CRefArray plugins = Application().GetPlugins();
    Plugin plugin = plugins.GetItem(L"ICERigTools");
    CValue l_prevData = plugin.GetUserData();
    if (l_prevData.IsEmpty())
    {
        IRSimpleBrush *l_pTool = new IRSimpleBrush;
        if (!l_pTool) return CStatus::Fail;
        ToolContext l_ctxt(in_ctxt);
        l_ctxt.PutUserData(CValue((CValue::siPtrType)l_pTool));
    }
    else
    {
        ToolContext l_ctxt(in_ctxt);
        l_ctxt.PutUserData(CValue((CValue::siPtrType)l_prevData));
    }

    return CStatus::OK;
}

SICALLBACK IRSimpleBrush_Term(CRef& in_ctxt)
{
   ToolContext l_ctxt(in_ctxt);
   IRSimpleBrush *l_pTool = (IRSimpleBrush *)(CValue::siPtrType)l_ctxt.GetUserData();
   if (!l_pTool) return CStatus::Fail;

   // store the user data for the next tool use
   CRefArray plugins = Application().GetPlugins();
   Plugin plugin = plugins.GetItem(L"ICERigTools");
   if (plugin.IsValid())
   {
      plugin.PutUserData(CValue((CValue::siPtrType)l_pTool));
   }

   l_ctxt.PutUserData(CValue((CValue::siPtrType)NULL)); // Clear user data
   return CStatus::OK;
}

// Use a macro to simplify callback setup and forwarding...
#define DECLARE_TOOL_CALLBACK(TOOL,CALLBACK) \
SICALLBACK TOOL##_##CALLBACK( ToolContext& in_ctxt ) { \
    TOOL *l_pTool = (TOOL *)(CValue::siPtrType)in_ctxt.GetUserData(); \
    return ( l_pTool ? l_pTool->CALLBACK( in_ctxt ) : CStatus::Fail ); }

DECLARE_TOOL_CALLBACK( IRSimpleBrush, Setup );
DECLARE_TOOL_CALLBACK( IRSimpleBrush, Cleanup );
DECLARE_TOOL_CALLBACK( IRSimpleBrush, Activate );
DECLARE_TOOL_CALLBACK( IRSimpleBrush, Deactivate );
DECLARE_TOOL_CALLBACK( IRSimpleBrush, MouseDown );
DECLARE_TOOL_CALLBACK( IRSimpleBrush, MouseDrag );
DECLARE_TOOL_CALLBACK( IRSimpleBrush, MouseUp );
DECLARE_TOOL_CALLBACK( IRSimpleBrush, Draw );
DECLARE_TOOL_CALLBACK( IRSimpleBrush, MouseMove );
DECLARE_TOOL_CALLBACK( IRSimpleBrush, MouseEnter );
DECLARE_TOOL_CALLBACK( IRSimpleBrush, MouseLeave );
DECLARE_TOOL_CALLBACK( IRSimpleBrush, ModifierChanged );
DECLARE_TOOL_CALLBACK( IRSimpleBrush, SelectionChanged );
DECLARE_TOOL_CALLBACK( IRSimpleBrush, KeyDown );
DECLARE_TOOL_CALLBACK( IRSimpleBrush, KeyUp );
DECLARE_TOOL_CALLBACK( IRSimpleBrush, SnapValid );
DECLARE_TOOL_CALLBACK( IRSimpleBrush, MenuInit );
DECLARE_TOOL_CALLBACK( IRSimpleBrush, DoExitTool );
