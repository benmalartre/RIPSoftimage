#include "window.h"
#include "explorer.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UsdExplorerWindow::UsdExplorerWindow()
{

}

UsdExplorerWindow::~UsdExplorerWindow()
{

}


//********************************************************************
//
// @mfunc	X2UCustomGLWindow::Init | Creates the dialog has a child of the
//								window handle specified by the view
//								context.
//
//********************************************************************
LRESULT	UsdExplorerWindow::Init( XSI::CRef& in_pViewCtx )
{
	XSI::ViewContext l_vViewContext = in_pViewCtx;
	assert ( l_vViewContext.IsValid() );

  CreateU2XWindow((HWND)l_vViewContext.GetParentWindowHandle());
  
	return S_OK;
}

//********************************************************************
//
// @mfunc	CCustomUI::Term | Destroys the dialog
//
//********************************************************************
LRESULT	UsdExplorerWindow::Term( XSI::CRef& in_pViewCtx )
{
  //DestroyU2XWindow();
  /*
  DestroyWindow(__gHandle);
  __gHandle = NULL;
  UnregisterClass("UsdExplorer", __gInstance);
  */
  
	return S_OK;
}

//********************************************************************
//
// @mfunc	CCustomUI::Notify | Handles Softimage notifications
//
//********************************************************************
LRESULT UsdExplorerWindow::Notify ( XSI::CRef& in_pViewCtx )
{
	using namespace XSI;

	//
	// Convert the CRef into a ViewContext
	//

	XSI::ViewContext l_vViewContext = in_pViewCtx;
	assert ( l_vViewContext.IsValid() );

	//
	// Retrieve the notification information from the view context
	//

	XSI::siEventID in_eNotifcation;
	void*	in_pData;

	l_vViewContext.GetNotificationData ( in_eNotifcation, &in_pData );

	switch ( in_eNotifcation )
	{
	case siOnSelectionChange:
		{
			//
			// The selection list has changed. Cast the notification data
			// into a CSelectionChangeNotification pointer to get at the
			// details.
			//

			XSI::CSelectionChangeNotification* l_pSelection = (XSI::CSelectionChangeNotification*)in_pData;

			XSI::CString	l_szMessage;
			l_szMessage = L"XSI_SELECTION_CHANGED_CV";
			l_szMessage += L" [ ";

			//
			// Build a string of all objects that are in the selection list
			//

			for (int c=0;c<l_pSelection->GetSelectionList().GetCount();c++)
			{
				XSI::CRef&	l_pCRef = l_pSelection->GetSelectionList().GetItem(c);

				XSI::SIObject mySIObject ( l_pCRef );
				
				XSI::CString	Name = mySIObject.GetFullName();

				if ( c != 0 )
					l_szMessage += L",";

				l_szMessage += Name;;

			}

			l_szMessage += L" ]";

			//
			// And print it
			//

			char *l_szMessChar = new char [ l_szMessage.Length() + 1 ];
			//W2AHelper ( l_szMessChar, l_szMessage.GetWideString() );
			//PrintNotification ( l_szMessChar );
			delete [] l_szMessChar;

			break;
			
		}

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

	}


	return S_OK;
}

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



//********************************************************************
//
// @mfunc	CCustomUI::SetWindowSize | It is important to handle this 
//										message or else the window
//										will end up in the wrong place
//										in the Softimage UI.
//
//********************************************************************
LRESULT UsdExplorerWindow::SetWindowSize(int ox, int oy, int cx, int cy)
{

	return S_OK;
}

LRESULT UsdExplorerWindow::Paint( WPARAM, LPARAM )
{
  //Draw();
	return S_OK;
}

LRESULT UsdExplorerWindow::MouseMove( WPARAM, LPARAM )
{

	return S_OK;
}

void	UsdExplorerWindow::PrintNotification ( char*	in_szMessage )
{
  /*
	// Get the number of items in the list box.
	int count = (int)SendDlgItemMessage ( l_hWnd, IDC_OUTPUT2,LB_GETCOUNT, 0, 0);

	if ( count > 500 )
	{
		SendDlgItemMessage ( l_hWnd, IDC_OUTPUT2,LB_RESETCONTENT,0,0 );
		count = 0;
	}


	SendDlgItemMessage ( l_hWnd, IDC_OUTPUT2,LB_ADDSTRING, 0, (LPARAM)in_szMessage);

	SendDlgItemMessage ( l_hWnd, IDC_OUTPUT2,LB_SETCURSEL, count, 0);
  */
}








//********************************************************************
//
// @mfunc	NotificationTest_Init | Called by Softimage when the plugin
//									is first initialized
//
//********************************************************************
XSIPLUGINCALLBACK void	UsdExplorer_Init(XSI::CRef in_pViewCtx)
{
  //
  // Make sure the CRef passed to us is in fact a ViewContext object
  //

  assert(in_pViewCtx.IsA(XSI::siViewContextID));
  XSI::ViewContext l_vViewContext = in_pViewCtx;
  assert(l_vViewContext.IsValid());

  U2XWindow* l_pCustomUI = new U2XWindow();

  l_vViewContext.PutUserData((void*)l_pCustomUI);
  l_vViewContext.SetFlags(XSI::siWindowNotifications | XSI::siWindowSize | XSI::siWindowPaint);

  //
  // Initialize our custom view
  //

  l_pCustomUI->Init(in_pViewCtx);


}

//********************************************************************
//
// @mfunc	NotificationTest_Init | Called by Softimage when the plugin
//									is terminated (when the parent 
//									window is destroyed.
//
//********************************************************************
XSIPLUGINCALLBACK void	UsdExplorer_Term(XSI::CRef in_pViewCtx)
{
  //
  // Terminate our custom view
  //

  XSI::ViewContext l_vViewContext = in_pViewCtx;
  assert(l_vViewContext.IsValid());

  U2XWindow* l_pCustomUI = (U2XWindow*)((void*)l_vViewContext.GetUserData());

  assert(l_pCustomUI != NULL);

  l_pCustomUI->Term(in_pViewCtx);

  delete l_pCustomUI;
  l_pCustomUI = NULL;

}


//********************************************************************
//
// @mfunc	NotificationTest_Notify | Called by Softimage when something
//									occurs in the scene. Like a
//									parameter change or selection  
//									change.
//
//********************************************************************
XSIPLUGINCALLBACK void	UsdExplorer_Notify(XSI::CRef in_pViewCtx)
{

  //
  // We are being notified by XSI that something has 
  // changed. Pass this info to the custom view
  //

  XSI::ViewContext l_vViewContext = in_pViewCtx;
  assert(l_vViewContext.IsValid());

  U2XWindow* l_pCustomUI = (U2XWindow*)((void*)l_vViewContext.GetUserData());

  assert(l_pCustomUI != NULL);


  l_pCustomUI->Notify(in_pViewCtx);
}

XSIPLUGINCALLBACK void			UsdExplorer_SetAttributeValue(XSI::CRef in_pViewCtx, XSI::CString in_sAttribName, XSI::CValue in_vValue)
{
  XSI::ViewContext l_vViewContext = in_pViewCtx;
  assert(l_vViewContext.IsValid());

  U2XWindow* l_pCustomUI = (U2XWindow*)((void*)l_vViewContext.GetUserData());
  assert(l_pCustomUI != NULL);

  l_pCustomUI->SetAttributeValue(in_sAttribName, in_vValue);

}

XSIPLUGINCALLBACK XSI::CValue	UsdExplorer_GetAttributeValue(XSI::CRef in_pViewCtx, XSI::CString in_sAttribName)
{

  XSI::ViewContext l_vViewContext = in_pViewCtx;
  assert(l_vViewContext.IsValid());

  U2XWindow* l_pCustomUI = (U2XWindow*)((void*)l_vViewContext.GetUserData());
  assert(l_pCustomUI != NULL);

  XSI::CValue l_val;
  l_pCustomUI->GetAttributeValue(in_sAttribName, l_val);

  return l_val;
}