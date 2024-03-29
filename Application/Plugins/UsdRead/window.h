
#pragma once

#ifdef _WIN32
#include <windows.h>
#define WIN32_LEAN_AND_MEAN
#endif
#include <GL\gl.h>
#include <stdio.h>
#include <assert.h>
#include <string>

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

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_opengl3.h"
#include "utils.h"
#include <vector>

class U2XWindow;
class U2XStage;

extern HGLRC U2X_SHARED_CONTEXT;
extern U2XWindow* U2X_HIDDEN_WINDOW;
extern ImFontAtlas* U2X_SHARED_ATLAS;
extern ImFont* U2X_FONT_BOLD;
extern ImFont* U2X_FONT_MEDIUM;
extern ImFont* U2X_FONT_REGULAR;

static float U2X_FONT_SIZE = 16;

void U2XSetupPixelFormat(HDC hDC);
void U2XSetWindowStyle();
void U2XCreateFontAtlas();
void U2XDeleteFontAtlas();
void U2XGetSharedContext();

// Get Softimage Main Window
//
extern HWND U2X_SOFTIMAGE_WINDOW;
struct U2XHandleData {
  unsigned long processId;
  HWND hWnd;
};

BOOL U2XIsMainWindow(HWND hWnd);
bool CALLBACK U2XEnumWindowsCallback(HWND hWnd, LPARAM lParam);
HWND U2XGetSoftimageWindow();

//
// Window Callback
//
LRESULT CALLBACK U2XWindowCallback(HWND hOgl, UINT Message, WPARAM wParam, LPARAM lParam);

class U2XWindow
{
public:
  U2XWindow();
  virtual ~U2XWindow();

  virtual char*	GetName() { return "U2XWindow"; };
  virtual HWND Get() { return _hWnd; };
  virtual size_t GetWidth() {
    return _rect.right - _rect.left;
  };
  virtual size_t GetHeight() {
    return _rect.bottom - _rect.top;
  };
  virtual bool IsActive() { return _active; };
  virtual bool IsShared() { return _shared; };
  virtual void Activate(bool state);
  virtual void FillBackground();
  virtual void ClearStage() { _stage = NULL; };
 
  virtual void Create(HWND hwnd, bool shared=false);
  virtual void CreateContext(HWND hwnd);
  virtual void DestroyContext(HWND hwnd);
  virtual void InitGL();
  virtual void TermGL();
  virtual void BeginDraw();
  virtual bool Draw();
  virtual void EndDraw();
  virtual void Reshape(int width, int height);

protected:
  std::string           _className;
	HWND                  _hWnd;
  HINSTANCE             _hInstance;
  HDC                   _hDC;
  HGLRC                 _hRC;
  ImGuiWindowFlags      _flags;
  ImGuiContext*         _ctxt;
  bool                  _shared;
  RECT                  _rect;
  bool                  _active;
  bool                  _initialized;
  pxr::UsdStageWeakPtr  _stage;
};

extern std::vector<U2XWindow*> U2X_UIS;
