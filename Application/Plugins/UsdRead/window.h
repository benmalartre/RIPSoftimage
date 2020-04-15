
#pragma once

#ifdef _WIN32
#include <windows.h>
#define WIN32_LEAN_AND_MEAN
#endif
#include <GL\gl.h>
#include <stdio.h>
#include <assert.h>

#include <xsi_ref.h>
#include <xsi_decl.h>
#include <xsi_ref.h>
#include <xsi_viewnotification.h>
#include <xsi_x3dobject.h>
#include <xsi_siobject.h>
#include <xsi_viewcontext.h>
#include <xsi_primitive.h>
#include <xsi_geometry.h>
#include <xsi_point.h>
#include <xsi_parameter.h>
#include <xsi_cluster.h>
#include <xsi_decl.h>
#include <xsi_viewcontext.h>

#include <commctrl.h>
#include <commdlg.h>


//
// Standard Win32 dialog callback
//
LRESULT CALLBACK U2XWindowOpenProc(HWND hOgl, UINT Message, WPARAM wParam, LPARAM lParam);
void CreateU2XWindow(HWND hParent);

class U2XWindow
{
public:
  U2XWindow();
	virtual ~U2XWindow();

	virtual char*	GetName() { return "U2XWindow"; };
		
	virtual LRESULT	Init( XSI::CRef& in_pViewCtx );
	virtual LRESULT	Term( XSI::CRef& in_pViewCtx );
	virtual LRESULT Notify ( XSI::CRef& in_pViewCtx );

	virtual LRESULT SetAttributeValue ( XSI::CString& in_cString, XSI::CValue& in_vValue );
	virtual LRESULT GetAttributeValue ( XSI::CString& in_cString, XSI::CValue& out_vValue );

	//
	// window
	//

	virtual LRESULT SetWindowSize(int ox, int oy, int cx, int cy);
	
	virtual LRESULT Paint( WPARAM, LPARAM );
	virtual LRESULT MouseMove( WPARAM, LPARAM );


private:

	void	PrintNotification ( char*	in_szMessage );


private:
	HWND	l_hWnd;
};

typedef struct tagMsg2Name {
	char*	m_szName;
	int		m_iID;
} Msg2Name;

