#pragma once


#include <xsi_ref.h>
#include <xsi_decl.h>


class UsdExplorerWindow
{
public:
  UsdExplorerWindow();
	virtual ~UsdExplorerWindow();

	virtual char*	GetName() { return "UsdExplorerWindow"; };
		
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
