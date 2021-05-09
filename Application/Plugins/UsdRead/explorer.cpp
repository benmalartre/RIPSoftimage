#include <pxr/imaging/garch/glApi.h>
#include "icons.h"
#include "scene.h"
#include "stage.h"
#include "window.h"
#include "explorer.h"
#include "utils.h"

extern ImFontAtlas* U2X_SHARED_ATLAS;
extern U2XScene* U2X_SCENE;

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
    ImGuiTreeNodeFlags_OpenOnDoubleClick;

  _backgroundColor = ImVec4(0.5, 0.5, 0.5, 1.0);
  _alternateColor = ImVec4(0.6, 0.6, 0.6, 1.0);
  _selectedColor = ImVec4(1.0, 0.7, 0.3, 1.0);
  _hoveredColor = ImVec4(0.9, 0.6, 0.2, 1.0);

  U2X_UIS.push_back(this);
}

UsdExplorerWindow::~UsdExplorerWindow()
{
  for (auto it = U2X_UIS.begin(); it < U2X_UIS.end(); ++it) {
    if (*it == this) {
      U2X_UIS.erase(it);
      break;
    }
  }
}

LRESULT	UsdExplorerWindow::Init( XSI::CRef& in_ctxt )
{
  XSI::ViewContext viewContext = in_ctxt;
  assert (viewContext.IsValid() );

  U2XGetSharedContext();
  Create((HWND)viewContext.GetParentWindowHandle(), false);
  InitGL();

  _visibleTex = U2X_ICONS["visible.png"]._tex;
  _invisibleTex = U2X_ICONS["invisible.png"]._tex;
  
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
          /*
          U2XStage* stage = U2X_SCENE->GetStage(CustomPrimitive(xPrim).GetObjectID());
          if (stage)
          {
            _stage = stage;
            RecurseStage();
          }
          */
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

    XSI::CString l_szMessage;
    l_szMessage = L"XSI_OBJECT_ADDED_CV";

    const XSI::CRef& l_pCRef = l_pObjectAdded->GetObjectAdded();
    XSI::SIObject mySIObject ( l_pCRef );
    XSI::CString Name = mySIObject.GetFullName();

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
        break;
      }
      case siWindowPaint: break;
      case siWindowSetFocus: 
      {
        SetFocus(_hWnd);
        break;
      }
      case siWindowLostFocus: 
      {
        SetFocus(GetParent(_hWnd));
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

    XSI::CString l_szMessage;
    l_szMessage = L"XSI_OBJECT_REMOVED_CV";

    const XSI::CString l_szObjectName = l_pObjectRemoved->GetObjectName();
    const XSI::siBranchFlag l_eFlag = l_pObjectRemoved->GetBranchFlag();
    l_szMessage += L"[ ";
    l_szMessage += l_szObjectName;

    switch (l_eFlag)
    {
      case siNode: l_szMessage += L" [Node] "; break;
      case siBranch: l_szMessage += L" [Branch] "; break;
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

    XSI::CString l_szMessage;
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
        break;
      }

      case siParameterID:
      {
        Parameter paramObj(cRef);
        l_szMessage += L": New value: " + paramObj.GetValue().GetAsText();
        break;
      }

      case siClusterID:
      {
        Cluster clustObj(cRef);
        CClusterElementArray elemArray = clustObj.GetElements();
        LONG nElems = elemArray.GetCount();
        l_szMessage += L": Number of elements: " + CValue(nElems).GetAsText();
        break;
      }
      
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
  GarchGLApiLoad();

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

void UsdExplorerWindow::DrawItemBackground(ImDrawList* drawList, 
  const UsdExplorerItem* item, bool& flip)
{
  ImVec2 pos = ImGui::GetCursorPos();
  if (item->_selected) {
    drawList->AddRectFilled(
      { 0, pos.y }, 
      { ImGui::GetWindowWidth(), pos.y + U2X_EXPLORER_LINE_HEIGHT }, 
      ImColor(_selectedColor));
  }
  else {
    if (flip)
      drawList->AddRectFilled(
        { 0, pos.y }, 
        { ImGui::GetWindowWidth(), pos.y + U2X_EXPLORER_LINE_HEIGHT }, 
        ImColor(_backgroundColor));
    else
      drawList->AddRectFilled(
        { 0, pos.y }, 
        { ImGui::GetWindowWidth(), pos.y + U2X_EXPLORER_LINE_HEIGHT }, 
        ImColor(_alternateColor));
  }
 
  ImGui::SetCursorPos(ImVec2(0, pos.y + U2X_EXPLORER_LINE_HEIGHT));
  if (item->_expanded) {
    for (const auto child : item->_items) {
      flip = !flip;
      DrawItemBackground(drawList, child, flip);
    }
  }
}

void UsdExplorerWindow::DrawBackground()
{
  auto* drawList = ImGui::GetBackgroundDrawList();
  const auto& style = ImGui::GetStyle();

  float scrollOffsetH = ImGui::GetScrollX();
  float scrollOffsetV = ImGui::GetScrollY();

  ImVec2 clipRectMin(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
  ImVec2 clipRectMax(clipRectMin.x + ImGui::GetWindowWidth(), 
    clipRectMin.y + ImGui::GetWindowHeight());

  if (ImGui::GetScrollMaxX() > 0)
  {
    clipRectMax.y -= style.ScrollbarSize;
  }

  drawList->PushClipRect(clipRectMin, clipRectMax);
  bool flip = false;

  drawList->AddRectFilled(
    { 0, -scrollOffsetV }, 
    { ImGui::GetWindowWidth(), -scrollOffsetV + U2X_EXPLORER_LINE_HEIGHT },
    ImColor(1,0,0,1));

  ImGui::SetCursorPos(ImVec2(0, -scrollOffsetV + U2X_EXPLORER_LINE_HEIGHT));
  DrawItemBackground(drawList, _root, flip);
  ImGui::SetCursorPos(ImVec2(0, U2X_EXPLORER_LINE_HEIGHT));

  drawList->PopClipRect();
  
}

void UsdExplorerWindow::DrawItemType(UsdExplorerItem* item)
{
  ImGui::Text(item->_prim.GetTypeName().GetText());
  ImGui::NextColumn();
}

void UsdExplorerWindow::DrawItemVisibility(UsdExplorerItem* item, bool heritedVisibility)
{
  GLuint tex = item->_visible ? _visibleTex : _invisibleTex;
  ImVec4 col = heritedVisibility ?
    ImVec4(0, 0, 0, 1) : ImVec4(0.33, 0.33, 0.33, 1);

  ImGui::ImageButton(
    (void*)tex,
    ImVec2(16, 16),
    ImVec2(0, 0),
    ImVec2(1, 1),
    0,
    ImVec4(0,0,0,0),
    col);

  if (ImGui::IsItemClicked()) {
    item->_visible = !item->_visible;
    pxr::UsdGeomImageable imageable(item->_prim);
    if (TF_VERIFY(imageable)) {
      if (item->_visible)imageable.MakeVisible();
      else imageable.MakeInvisible();
      Application app;
      XSI::ProjectItem item = app.GetObjectFromID(_stage->GetObjectID());
      if (item.IsValid())
      {
        CustomPrimitive primitive(item);
        item.PutParameterValue(L"Update", CValue(true));
        _needRefresh = true;
      }
    }
  }

  ImGui::NextColumn();
}

void UsdExplorerWindow::DrawItem(UsdExplorerItem* current, bool heritedVisibility)
{
  ImGuiTreeNodeFlags itemFlags = _selectBaseFlags;
      
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

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
      current->_selected = !current->_selected;

    ImGui::NextColumn();

    DrawItemType(current);
    DrawItemVisibility(current, heritedVisibility);

    if (currentOpen)
    {
      current->_expanded = true;
      for (const auto item : current->_items)
        DrawItem(item, current->_visible && heritedVisibility);
      ImGui::TreePop();
    }
    else current->_expanded = false;
  }
  // leaf
  else
  {
    itemFlags |= ImGuiTreeNodeFlags_Leaf | 
      ImGuiTreeNodeFlags_NoTreePushOnOpen;

    ImGui::TreeNodeEx(
      current->_prim.GetPath().GetText(), 
      itemFlags, 
      current->_prim.GetName().GetText());
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
      current->_selected = true;
    current->_expanded = false;

    ImGui::NextColumn();
    DrawItemType(current);
    DrawItemVisibility(current, heritedVisibility);
  }
}

bool UsdExplorerWindow::Draw()
{
  if(!_active)return false;
  BeginDraw();
  
  if (_stage)
  {
    // setup colors
    const size_t numColorIDs = 7;
    int colorIDs[numColorIDs] = {
      ImGuiCol_WindowBg,
      ImGuiCol_Header,
      ImGuiCol_HeaderHovered,
      ImGuiCol_HeaderActive,
      ImGuiCol_Button,
      ImGuiCol_ButtonActive,
      ImGuiCol_ButtonHovered
    };
    for(int i=0;i<numColorIDs;++i)
      ImGui::PushStyleColor(
        colorIDs[i],
        ImVec4(0, 0, 0, 0));

    ImGui::Begin("UsdExplorer", NULL, _flags);
    ImGui::SetWindowPos(ImVec2(0, 0));
    ImGui::SetWindowSize(ImVec2(GetWidth(), GetHeight()));

    // setup columns
    ImGui::Columns(3);
    ImGui::SetColumnWidth(0, GetWidth() - 120);
    ImGui::SetColumnWidth(1, 60);
    ImGui::SetColumnWidth(2, 30);

    // draw title
    ImGui::PushFont(U2X_FONT_BOLD);
    ImGui::Text("Prim name");
    ImGui::NextColumn();
    ImGui::Text("Type");
    ImGui::NextColumn();
    ImGui::Text("Vis");
    ImGui::NextColumn();
    ImGui::PopFont();

    DrawBackground();

    ImGui::PushFont(U2X_FONT_REGULAR);
    DrawItem(_root, true);
    ImGui::PopFont();

    ImGui::End();
    ImGui::PopStyleColor(numColorIDs);
  }
  else ImGui::ShowDemoWindow();
 
  EndDraw();

  if (_needRefresh) {
    Application().ExecuteCommand("Refresh", CValueArray(), CValue());
    _needRefresh = false;
  }
  return true;
}


void UsdExplorerWindow::RecurseStage()
{
  if (_root)delete _root;
  _root = new UsdExplorerItem();
  _root->_expanded = true;
  _root->_prim = _stage->Get()->GetPseudoRoot();
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
    pxr::UsdAttribute visibilityAttr =
      pxr::UsdGeomImageable(childPrim).GetVisibilityAttr();
    if (visibilityAttr.IsValid())
    {
      TfToken visible;
      visibilityAttr.Get(&visible);
      if (visible == UsdGeomTokens->invisible)currentItem->_visible = false;
      else childItem->_visible = true;
    }
    RecursePrim(childItem);
  }
}

//
// Softimage Plugin Callbacks
//
XSIPLUGINCALLBACK void	
UsdExplorer_Init(XSI::CRef in_ctxt)
{
  assert(in_ctxt.IsA(XSI::siViewContextID));
  XSI::ViewContext viewContext = in_ctxt;
  assert(viewContext.IsValid());

  UsdExplorerWindow* explorer = new UsdExplorerWindow();

  viewContext.PutUserData((void*)explorer);
  viewContext.SetFlags(XSI::siWindowNotifications);

  explorer->Init(in_ctxt);
}

XSIPLUGINCALLBACK void 
UsdExplorer_Term(XSI::CRef in_ctxt)
{
  XSI::ViewContext viewContext = in_ctxt;
  assert(viewContext.IsValid());

  UsdExplorerWindow* explorer = (UsdExplorerWindow*)((void*)viewContext.GetUserData());

  assert(explorer != NULL);

  explorer->Term(in_ctxt);

  delete explorer;
  explorer = NULL;
}


XSIPLUGINCALLBACK void 
UsdExplorer_Notify(XSI::CRef in_ctxt)
{
  XSI::ViewContext viewContext = in_ctxt;
  assert(viewContext.IsValid());

  UsdExplorerWindow* explorer = (UsdExplorerWindow*)((void*)viewContext.GetUserData());

  assert(explorer != NULL);

  explorer->Notify(in_ctxt);
}

XSIPLUGINCALLBACK void 
UsdExplorer_SetAttributeValue(XSI::CRef in_ctxt, XSI::CString name, XSI::CValue value)
{
  XSI::ViewContext viewContext = in_ctxt;
  assert(viewContext.IsValid());

  UsdExplorerWindow* explorer = (UsdExplorerWindow*)((void*)viewContext.GetUserData());
  assert(explorer != NULL);

  //explorer->SetAttributeValue(name, value);

}

XSIPLUGINCALLBACK XSI::CValue	
UsdExplorer_GetAttributeValue(XSI::CRef in_ctxt, XSI::CString name)
{

  XSI::ViewContext viewContext = in_ctxt;
  assert(viewContext.IsValid());

  UsdExplorerWindow* explorer = (UsdExplorerWindow*)((void*)viewContext.GetUserData());
  assert(explorer != NULL);

  XSI::CValue value;
  //explorer->GetAttributeValue(name, value);

  return value;
}
