#include <GL/glew.h>
#include "icons.h"
#include "scene.h"
#include "stage.h"
#include "window.h"
#include "explorer.h"
#include "utils.h"

extern ImFontAtlas* U2X_SHARED_ATLAS;


UsdExplorerWindow::UsdExplorerWindow() 
  : U2XWindow()
  , _locked(false)
  , _root(NULL)
{
  _flags = 
    ImGuiWindowFlags_NoCollapse |
    ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoDecoration|
    ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoTitleBar |
    ImGuiWindowFlags_AlwaysVerticalScrollbar;

  _selectBaseFlags =
    ImGuiTreeNodeFlags_OpenOnArrow |
    ImGuiTreeNodeFlags_OpenOnDoubleClick |
    ImGuiTreeNodeFlags_SpanFullWidth;
}

UsdExplorerWindow::~UsdExplorerWindow()
{

}


LRESULT	UsdExplorerWindow::Init( XSI::CRef& in_ctxt )
{
	XSI::ViewContext viewContext = in_ctxt;
	assert (viewContext.IsValid() );

  U2XGetSharedContext();
  Create((HWND)viewContext.GetParentWindowHandle(), false);
  InitGL();
  
	return S_OK;
}

LRESULT	UsdExplorerWindow::Term( XSI::CRef& in_ctxt)
{
  TermGL();
  
	return S_OK;
}


LRESULT UsdExplorerWindow::Notify ( XSI::CRef& in_ctxt)
{
	using namespace XSI;
  
	//
	// Convert the CRef into a ViewContext
	//

	XSI::ViewContext viewContext = in_ctxt;
	assert ( viewContext.IsValid() );

	//
	// Retrieve the notification information from the view context
	//

	XSI::siEventID notification;
	void*	data;

	viewContext.GetNotificationData (notification, &data );

	switch (notification)
  {
	  case siOnSelectionChange:
    {
    if (!_locked)
    {
      XSI::CSelectionChangeNotification* selection = (XSI::CSelectionChangeNotification*)data;

      for (int c = 0; c<selection->GetSelectionList().GetCount(); c++)
      {
        XSI::CRef&	cRef = selection->GetSelectionList().GetItem(c);
        XSI::SIObject object(cRef);
        XSI::CString	name = object.GetFullName();
        if (CString(object.GetType().GetAsciiString()) == CString("UsdPrimitive"))
        {
          X3DObject xObj(cRef);
          Primitive xPrim = xObj.GetActivePrimitive();
          U2XStage* stage = U2X_PRIMITIVES.Get(CustomPrimitive(xPrim));
          if (stage)
          {
            _stage = stage->Get();
            RecurseStage();
          }
        }
      }
    }
    break;
    }

    /*
	case siOnTimeChange:
		{
			//
			// The current time has changed (the timeline has been scrubbed).
			// Cast the data into a CTimeChangeNotification pointer to get at the
			// details and print it.
			//

			XSI::CTimeChangeNotification* l_pTimeChange = (XSI::CTimeChangeNotification*)in_pData;
			
			XSI::CString	l_szMessage;
			l_szMessage = L"XSI_TIME_CHANGE_CV";
		
			char *l_szMessChar = new char [ l_szMessage.Length() + 1 ];
			//W2AHelper ( l_szMessChar, l_szMessage.GetWideString() );
			//PrintNotification ( l_szMessChar );
			delete [] l_szMessChar;
			
			break;
		}

	case siOnObjectAdded:
		{
			//
			// A new object is being added to the scene
			// Print it's name 
			//
            
			XSI::CObjectAddedNotification* l_pObjectAdded = (XSI::CObjectAddedNotification*)in_pData;
			
			XSI::CString	l_szMessage;
			l_szMessage = L"XSI_OBJECT_ADDED_CV";

			const XSI::CRef&	l_pCRef = l_pObjectAdded->GetObjectAdded();
			XSI::SIObject mySIObject ( l_pCRef );
			XSI::CString	Name = mySIObject.GetFullName();

			l_szMessage += L"[ ";
			l_szMessage += Name;
			l_szMessage += L" ]";
		
			char *l_szMessChar = new char [ l_szMessage.Length() + 1 ];
			//W2AHelper ( l_szMessChar, l_szMessage.GetWideString() );
			//PrintNotification ( l_szMessChar );
			delete [] l_szMessChar;
			
			break;
		}
    */
	case siOnWindowEvent:
		{

			XSI::CWindowNotification* lpWindowEvent = (XSI::CWindowNotification*)data;

			switch (lpWindowEvent->GetWindowState())
			{
			case siWindowSize: 
				{
					int x,y,w,h;

					lpWindowEvent->GetPosition (x,y,w,h);
					char l_szMessage[MAX_PATH];
					sprintf ( l_szMessage, "XSI_WINDOW_SIZE: (%d,%d) (%d, %d)",x,y,w,h);
					LOG(l_szMessage);
                    break;
				}
			case siWindowPaint: LOG ( "XSI_WINDOW_PAINT"); break;
			case siWindowSetFocus: 
      {
        SetFocus(_hWnd);
        LOG("XSI_WINDOW_SETFOCUS");
        break;
      }
			case siWindowLostFocus: 
      {
        SetFocus(GetParent(_hWnd));
        LOG("XSI_WINDOW_LOSTFOCUS");
        break;
      }
			}
			break;
		}
    /*
	case siOnObjectRemoved:
		{
			//
			// An existing object is being removed from the scene (deleted)
			// Print it's name 
			//
            
			XSI::CObjectRemovedNotification* l_pObjectRemoved = (XSI::CObjectRemovedNotification*)in_pData;
			
			XSI::CString	l_szMessage;
			l_szMessage = L"XSI_OBJECT_REMOVED_CV";

			const XSI::CString	l_szObjectName = l_pObjectRemoved->GetObjectName();
			const XSI::siBranchFlag l_eFlag = l_pObjectRemoved->GetBranchFlag();
			l_szMessage += L"[ ";
			l_szMessage += l_szObjectName;

			switch (l_eFlag)
			{
			case siNode:	l_szMessage += L" [Node] "; break;
			case siBranch:	l_szMessage += L" [Branch] "; break;
			case siUnspecified:	l_szMessage += L" [Unspecified] "; break;
			}

			l_szMessage += L" ]";
		
			char *l_szMessChar = new char [ l_szMessage.Length() + 1 ];
			//W2AHelper ( l_szMessChar, l_szMessage.GetWideString() );
			//PrintNotification ( l_szMessChar );
			delete [] l_szMessChar;
			
			break;
		}


	case siOnValueChange:
		{
			//
			// Something has changed in the scene.
			//

			XSI::CString	l_szMessage;
			l_szMessage = L"XSI_SET_VALUE_CV ";
		
			XSI::CValueChangeNotification* l_pData = (XSI::CValueChangeNotification*) in_pData;

			//
			// Get the object that generated the notification
			//
			XSI::CRef cRef = l_pData->GetOwner();

			XSI::CString l_csCIDName = cRef.GetClassIDName();

			//
			// Get the full name of the parameter that has changed
			//

			l_szMessage += l_csCIDName;
			l_szMessage += L" [ ";
			l_szMessage += l_pData->GetComponentName();

			// note: GetOwner returns the Softimage object that initiates the 
			// notification. The returned object is actually a CRef object 
			// which refers to the underlying Softimage object which can be accessed
			// through an API class.
			//			
			// For instance, if a parameter value is changed on a primitive 
			// object then GetOwner returns a reference (i.e. CRef) to the 
			// primitive object. The CRef can then be used to access the 
			// primitive's geometry with the Primitive API class.
			// 
			// Please refer to the CRef documentation page and the section
			// entitled "Understanding the C++ library" in the on-line 
			// documentation for more details.
			//

			LONG objClassID = cRef.GetClassID();
			switch (objClassID)
			{
				case siPrimitiveID:
				{
					Primitive primObj(cRef);
					Geometry geomObj = primObj.GetGeometry();
					LONG nPoints = geomObj.GetPoints().GetCount();

					l_szMessage += L": Number of points: " + CValue(nPoints).GetAsText();
				}
				break;

				case siParameterID:
				{
					Parameter paramObj(cRef);
					l_szMessage += L": New value: " + paramObj.GetValue().GetAsText();
				}
				break;

				case siClusterID:
				{
					Cluster clustObj(cRef);
					CClusterElementArray elemArray = clustObj.GetElements();
					LONG nElems = elemArray.GetCount();
					l_szMessage += L": Number of elements: " + CValue(nElems).GetAsText();
				}
				break;
			}

			l_szMessage += L" ] ";

			char *l_szMessChar = new char [ l_szMessage.Length() + 1 ];
			//W2AHelper ( l_szMessChar, l_szMessage.GetWideString() );
			//PrintNotification ( l_szMessChar );
			delete [] l_szMessChar;
			
			break;

		}
    */
	}

	return S_OK;
}


void UsdExplorerWindow::InitGL()
{
  glewInit();

  _ctxt = ImGui::CreateContext(U2X_SHARED_ATLAS);
  ImGui::SetCurrentContext(_ctxt);
  ImGuiIO& io = ImGui::GetIO();

  // init Win32
  ImGui_ImplWin32_Init(_hWnd);

  // init OpenGL Imgui Implementation
  const char* glsl_version = "#version 130";
  ImGui_ImplOpenGL3_Init(glsl_version);

  // style
  U2XSetWindowStyle();

}

void UsdExplorerWindow::TermGL()
{
  //ImGui::PopID();
  ImGui::SetCurrentContext(_ctxt);
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplWin32_Shutdown(_hWnd);
  if (_ctxt)ImGui::DestroyContext(_ctxt);
}

void UsdExplorerWindow::DrawBackground(const ImColor& color)
{
  auto* drawList = ImGui::GetWindowDrawList();
  const auto& style = ImGui::GetStyle();

  float scrollOffsetH = ImGui::GetScrollX();
  float scrollOffsetV = ImGui::GetScrollY();
  float scrolledOutLines = floorf(scrollOffsetV / U2X_EXPLORER_LINE_HEIGHT);
  scrollOffsetV -= U2X_EXPLORER_LINE_HEIGHT * scrolledOutLines;

  ImVec2 clipRectMin(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
  ImVec2 clipRectMax(clipRectMin.x + ImGui::GetWindowWidth(), clipRectMin.y + ImGui::GetWindowHeight());

  if (ImGui::GetScrollMaxX() > 0)
  {
    clipRectMax.y -= style.ScrollbarSize;
  }

  drawList->PushClipRect(clipRectMin, clipRectMax);

  bool isOdd = (static_cast<int>(scrolledOutLines) % 2) == 0;

  float yMin = clipRectMin.y - scrollOffsetV + ImGui::GetCursorPosY();
  float yMax = clipRectMax.y - scrollOffsetV + U2X_EXPLORER_LINE_HEIGHT;
  float xMin = clipRectMin.x + scrollOffsetH + ImGui::GetWindowContentRegionMin().x;
  float xMax = clipRectMin.x + scrollOffsetH + ImGui::GetWindowContentRegionMax().x;

  for (float y = yMin; y < yMax; y += U2X_EXPLORER_LINE_HEIGHT, isOdd = !isOdd)
  {
    if (isOdd)
    {
      drawList->AddRectFilled({ xMin, y }, { xMax, y + U2X_EXPLORER_LINE_HEIGHT }, color);
    }
  }

  drawList->PopClipRect();

}

void UsdExplorerWindow::DrawItemType(UsdExplorerItem* item)
{
  ImGui::Text(item->_prim.GetTypeName().GetText());
  ImGui::NextColumn();
}

void UsdExplorerWindow::DrawItemVisibility(UsdExplorerItem* item)
{
  GLuint tex = item->_visible ?
    U2X_ICONS["visible.png"]._tex :
    U2X_ICONS["invisible.png"]._tex;

  ImGui::ImageButton(
    (void*)tex,
    ImVec2(16, 16),
    ImVec2(0, 0),
    ImVec2(1, 1),
    0,
    ImVec4(1, 1, 1, 1),
    ImVec4(0, 0, 0, 1));
  ImGui::NextColumn();
}

void UsdExplorerWindow::DrawItem(UsdExplorerItem* current)
{
  ImGuiTreeNodeFlags itemFlags = _selectBaseFlags;
  ImGui::PushStyleColor(
    ImGuiCol_WindowBg,
    ImVec4(
    (float)rand() / (float)RAND_MAX,
      (float)rand() / (float)RAND_MAX,
      (float)rand() / (float)RAND_MAX,
      1));

  if (current->_selected) {
    itemFlags |= ImGuiTreeNodeFlags_Selected;
  }

  // parent
  if (current->_items.size())
  {
    bool currentOpen =
      ImGui::TreeNodeEx(
        current->_prim.GetPath().GetText(),
        itemFlags,
        current->_prim.GetName().GetText());
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())current->_selected = true;

    ImGui::NextColumn();

    DrawItemType(current);
    DrawItemVisibility(current);

    
    
    if (currentOpen)
    {
      for (const auto item : current->_items)
        DrawItem(item);
      ImGui::TreePop();
    }
  }
  // leaf
  else
  {
    itemFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    ImGui::TreeNodeEx(
      current->_prim.GetPath().GetText(), 
      itemFlags, 
      current->_prim.GetName().GetText());
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())current->_selected = true;

    ImGui::NextColumn();
    DrawItemType(current);
    DrawItemVisibility(current);
  }
  ImGui::PopStyleColor();
}

bool UsdExplorerWindow::Draw()
{
  if(!_active)return false;
  BeginDraw();
  
  
  //show Main Window
  //
  if (_stage)
  {
    ImGui::Begin("UsdExplorer", NULL, _flags);
    ImGui::SetWindowPos(ImVec2(0, 0));
    ImGui::SetWindowSize(ImVec2(GetWidth(), GetHeight()));
   // DrawBackground();

    // setup columns
    ImGui::Columns(3);
    ImGui::SetColumnWidth(0, GetWidth() - 120);
    ImGui::SetColumnWidth(1, 60);
    ImGui::SetColumnWidth(2, 30);

    // draw title
    ImGui::PushFont(U2X_FONT_BOLD);
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(1, 0, 0, 1));
    ImGui::Text("Prim name");
    ImGui::NextColumn();
    ImGui::Text("Type");
    ImGui::NextColumn();
    ImGui::Text("Vis");
    ImGui::NextColumn();
    ImGui::PopStyleColor();
    ImGui::PopFont();
 
    ImGui::PushFont(U2X_FONT_REGULAR);
    DrawItem(_root);
    ImGui::PopFont();
    ImGui::End();
  }
  else ImGui::ShowDemoWindow();
 
  //bool opened;
  //int flags = 0;
  //flags |= ImGuiWindowFlags_NoResize;
  //flags |= ImGuiWindowFlags_NoTitleBar;
  //flags |= ImGuiWindowFlags_NoMove;
  //
  //ImGui::PushID((int)_hWnd);
  //ImGui::Begin(_className.c_str(), &opened, flags);
  //
  //ImGui::SetWindowSize(ImVec2(rect.right - rect.left, rect.bottom - rect.top), ImGuiCond_Always);
  //ImGui::SetWindowPos(ImVec2(0,0), ImGuiCond_Always);
  //
  //FillBackground();
  //
  //ImGui::End();
  //ImGui::PopID();
  EndDraw();
  return true;
}


void UsdExplorerWindow::RecurseStage()
{
  if (_root)delete _root;
  _root = new UsdExplorerItem();
  _root->_expanded = true;
  _root->_prim = _stage->GetPseudoRoot();
  _root->_visible = true;
  RecursePrim(_root);

}

void UsdExplorerWindow::RecursePrim(UsdExplorerItem* currentItem)
{
  for (const auto& childPrim: currentItem->_prim.GetChildren())
  {
    UsdExplorerItem* childItem = currentItem->AddItem();
    childItem->_expanded = true;
    childItem->_prim = childPrim;
    childItem->_visible = ((float)rand()/(float)RAND_MAX) >0.5;
    RecursePrim(childItem);
  }
}

/*
LRESULT UsdExplorerWindow::SetAttributeValue ( XSI::CString& in_cString, XSI::CValue& in_vValue )
{

	XSI::CString l_mess;
	l_mess = L"SetAttributeValue: ";
	l_mess += in_cString;
	PrintNotification ( (char*)l_mess.GetAsciiString());

	return S_OK;
}

LRESULT UsdExplorerWindow::GetAttributeValue ( XSI::CString& in_cString, XSI::CValue& out_vValue )
{

	XSI::CString l_mess;
	l_mess = L"GetAttributeValue: ";
	l_mess += in_cString;
	PrintNotification ( (char*)l_mess.GetAsciiString());

	return S_OK;
}
*/

//
// Softimage Plugin Callbacks
//
XSIPLUGINCALLBACK void	UsdExplorer_Init(XSI::CRef in_ctxt)
{
  assert(in_ctxt.IsA(XSI::siViewContextID));
  XSI::ViewContext viewContext = in_ctxt;
  assert(viewContext.IsValid());

  UsdExplorerWindow* explorer = new UsdExplorerWindow();

  viewContext.PutUserData((void*)explorer);
  viewContext.SetFlags(XSI::siWindowNotifications | XSI::siWindowSize | XSI::siWindowPaint );

  explorer->Init(in_ctxt);
}

XSIPLUGINCALLBACK void UsdExplorer_Term(XSI::CRef in_ctxt)
{
  XSI::ViewContext viewContext = in_ctxt;
  assert(viewContext.IsValid());

  UsdExplorerWindow* explorer = (UsdExplorerWindow*)((void*)viewContext.GetUserData());

  assert(explorer != NULL);

  explorer->Term(in_ctxt);

  delete explorer;
  explorer = NULL;
}


XSIPLUGINCALLBACK void UsdExplorer_Notify(XSI::CRef in_ctxt)
{
  XSI::ViewContext viewContext = in_ctxt;
  assert(viewContext.IsValid());

  UsdExplorerWindow* explorer = (UsdExplorerWindow*)((void*)viewContext.GetUserData());

  assert(explorer != NULL);

  explorer->Notify(in_ctxt);
}

XSIPLUGINCALLBACK void UsdExplorer_SetAttributeValue(XSI::CRef in_ctxt, XSI::CString name, XSI::CValue value)
{
  XSI::ViewContext viewContext = in_ctxt;
  assert(viewContext.IsValid());

  UsdExplorerWindow* explorer = (UsdExplorerWindow*)((void*)viewContext.GetUserData());
  assert(explorer != NULL);

  //explorer->SetAttributeValue(name, value);

}

XSIPLUGINCALLBACK XSI::CValue	UsdExplorer_GetAttributeValue(XSI::CRef in_ctxt, XSI::CString name)
{

  XSI::ViewContext viewContext = in_ctxt;
  assert(viewContext.IsValid());

  UsdExplorerWindow* explorer = (UsdExplorerWindow*)((void*)viewContext.GetUserData());
  assert(explorer != NULL);

  XSI::CValue value;
  //explorer->GetAttributeValue(name, value);

  return value;
}
