#pragma once


#include <xsi_ref.h>
#include <xsi_decl.h>
#include "window.h"


class UsdExplorerWindow : public U2XWindow
{
public:
  UsdExplorerWindow();
	virtual ~UsdExplorerWindow();

	virtual char*	GetName() override{ return "UsdExplorerWindow"; };
		
  
	virtual LRESULT	Init( XSI::CRef& in_pViewCtx ) override;
	virtual LRESULT	Term( XSI::CRef& in_pViewCtx ) override;
	virtual LRESULT Notify ( XSI::CRef& in_pViewCtx ) override;

  virtual void InitGL() override;
  virtual void Draw() override;
  
  /*
	virtual LRESULT SetAttributeValue ( XSI::CString& in_cString, XSI::CValue& in_vValue );
	virtual LRESULT GetAttributeValue ( XSI::CString& in_cString, XSI::CValue& out_vValue );

	//
	// window
	//

	virtual LRESULT SetWindowSize(int ox, int oy, int cx, int cy);
	
	virtual LRESULT Paint( WPARAM, LPARAM );
	virtual LRESULT MouseMove( WPARAM, LPARAM );
  */


};
