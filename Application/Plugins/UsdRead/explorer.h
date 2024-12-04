#pragma once

#include "window.h"
#include <xsi_ref.h>
#include <xsi_decl.h>
#include <xsi_viewnotification.h>
#include <vector>

static size_t U2X_EXPLORER_LINE_HEIGHT = 20;

class U2XStage;
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
  void DrawItem(UsdExplorerItem* item, bool heritedVisibility);
  void DrawItemType(UsdExplorerItem* item);
  void DrawItemVisibility(UsdExplorerItem* item, bool heritedVisibility);
  void DrawBackground();
  void DrawItemBackground(ImDrawList* drawList, const UsdExplorerItem* item, bool& flip);

private:
  bool                          _locked;
  UsdExplorerItem*              _root;
  ImGuiTreeNodeFlags            _selectBaseFlags;
  GLuint                        _visibleTex;
  GLuint                        _invisibleTex;
  ImVec4                        _backgroundColor;
  ImVec4                        _alternateColor;
  ImVec4                        _selectedColor;
  ImVec4                        _hoveredColor;
  bool                          _needRefresh;
};
