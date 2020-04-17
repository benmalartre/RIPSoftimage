#include <GL/glew.h>
#include "window.h"
#include "explorer.h"
#include "utils.h"

extern ImFontAtlas* U2X_SHARED_ATLAS;


UsdExplorerWindow::UsdExplorerWindow()
{

}

UsdExplorerWindow::~UsdExplorerWindow()
{

}


LRESULT	UsdExplorerWindow::Init( XSI::CRef& in_ctxt )
{
	XSI::ViewContext viewContext = in_ctxt;
	assert (viewContext.IsValid() );

  GetSharedContext();
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
    //
    // The selection list has changed. Cast the notification data
    // into a CSelectionChangeNotification pointer to get at the
    // details.
    //

    XSI::CSelectionChangeNotification* selection = (XSI::CSelectionChangeNotification*)data;


    //
    // Build a string of all objects that are in the selection list
    //

    for (int c=0;c<selection->GetSelectionList().GetCount();c++)
    {
      XSI::CRef&	cref = selection->GetSelectionList().GetItem(c);
      XSI::SIObject object (cref);
      XSI::CString	name = object.GetFullName();
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
	case siOnWindowEvent:
		{

			XSI::CWindowNotification* lpWindowEvent = (XSI::CWindowNotification*)in_pData;

			switch (lpWindowEvent->GetWindowState())
			{
			case siWindowSize: 
				{
					int x,y,w,h;

					lpWindowEvent->GetPosition (x,y,w,h);
					char l_szMessage[MAX_PATH];
					sprintf ( l_szMessage, "XSI_WINDOW_SIZE: (%d,%d) (%d, %d)",x,y,w,h);
					PrintNotification(l_szMessage);
                    break;
				}
			case siWindowPaint: PrintNotification ( "XSI_WINDOW_PAINT"); break;
			case siWindowSetFocus: PrintNotification ( "XSI_WINDOW_SETFOCUS"); break;
			case siWindowLostFocus: PrintNotification ( "XSI_WINDOW_LOSTFOCUS");break;
			}
			break;
		}

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

  //Init Win32
  ImGui_ImplWin32_Init(_hWnd);

  //Init OpenGL Imgui Implementation
  const char* glsl_version = "#version 130";
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Setup style
  ImGui::StyleColorsClassic();

}

void UsdExplorerWindow::TermGL()
{
  //ImGui::PopID();
  ImGui::SetCurrentContext(_ctxt);
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplWin32_Shutdown(_hWnd);
  if (_ctxt)ImGui::DestroyContext(_ctxt);
  
}

bool UsdExplorerWindow::Draw()
{
  if(!_active)return false;
  BeginDraw();
  
  //show Main Window
  ImGui::ShowDemoWindow();

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

LRESULT UsdExplorerWindow::SetWindowSize(int ox, int oy, int cx, int cy)
{

  SetWindowPos(_hWnd, NULL, ox, oy, cx, cy, SWP_NOZORDER);

  return S_OK;
}


LRESULT UsdExplorerWindow::Paint(WPARAM, LPARAM)
{
  LOG("PAAAAAAAAAAAAAAAAAAAAAAAAIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII!!!");
  Draw();
  return S_OK;
}

LRESULT UsdExplorerWindow::MouseMove(WPARAM, LPARAM)
{

  return S_OK;
}


XSIPLUGINCALLBACK void	UsdExplorer_Init(XSI::CRef in_ctxt)
{
  assert(in_ctxt.IsA(XSI::siViewContextID));
  XSI::ViewContext viewContext = in_ctxt;
  assert(viewContext.IsValid());

  UsdExplorerWindow* explorer = new UsdExplorerWindow();

  viewContext.PutUserData((void*)explorer);
  viewContext.SetFlags(XSI::siWindowNotifications | XSI::siWindowSize | XSI::siWindowPaint);

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
