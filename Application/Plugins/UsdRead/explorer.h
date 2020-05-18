#pragma once

#include "window.h"
#include <xsi_ref.h>
#include <xsi_decl.h>
#include <vector>

extern bool GL_EXTENSIONS_LOADED;

static size_t U2X_EXPLORER_LINE_HEIGHT = 20;

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
  void DrawItemType(UsdExplorerItem* item);
  void DrawItemVisibility(UsdExplorerItem* item);
  void DrawBackground( const ImColor& color = ImColor(20, 20, 20, 64));

private:
  pxr::UsdStageRefPtr           _stage;
  bool                          _locked;
  UsdExplorerItem*              _root;
  ImGuiTreeNodeFlags            _selectBaseFlags;
  GLuint                        _visibleTex;
  GLuint                        _invisibleTex;
};
