#pragma once

#include "window.h"
#include <xsi_ref.h>
#include <xsi_decl.h>
#include <vector>

extern bool GL_EXTENSIONS_LOADED;

struct UsdExplorerItem {
  pxr::UsdPrim                  _prim;
  bool                          _visible;
  bool                          _selected;
  bool                          _expanded;
  std::vector<UsdExplorerItem*>  _items;

  UsdExplorerItem* AddItem() {
    _items.push_back(new UsdExplorerItem());
    return _items.back();
  }
  ~UsdExplorerItem() { 
    for (auto item : _items)delete item;
  };
};

class UsdExplorerWindow : public U2XWindow
{
public:
  UsdExplorerWindow();
	virtual ~UsdExplorerWindow();

	virtual char*	GetName() override{ return "UsdExplorerWindow"; };
		
	virtual LRESULT	Init( XSI::CRef& in_ctxt );
	virtual LRESULT	Term( XSI::CRef& in_ctxt);
  virtual LRESULT Notify(XSI::CRef& in_ctxt);

  virtual void InitGL() override;
  virtual void TermGL() override;
  virtual bool Draw() override;

  void RecurseStage();
  void RecursePrim(UsdExplorerItem* current);
  void DrawItem(UsdExplorerItem* item);
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

private:
  pxr::UsdStageRefPtr           _stage;
  bool                          _locked;
  UsdExplorerItem*              _root;
  ImGuiTreeNodeFlags            _selectBaseFlags;
};
