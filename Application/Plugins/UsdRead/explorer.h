#pragma once


#include <xsi_ref.h>
#include <xsi_decl.h>
#include "window.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui.h"

extern bool GL_EXTENSIONS_LOADED;
extern U2XWindow* U2X_HIDDEN_WINDOW;

class UsdExplorerWindow : public U2XWindow
{
public:
  UsdExplorerWindow();
	virtual ~UsdExplorerWindow();

	virtual char*	GetName() override{ return "UsdExplorerWindow"; };
		
	virtual LRESULT	Init( XSI::CRef& in_pViewCtx );
	virtual LRESULT	Term( XSI::CRef& in_pViewCtx );

  virtual void InitGL() override;
  virtual void TermGL() override;
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
