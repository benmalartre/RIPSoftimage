
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


extern HGLRC U2X_SHARED_CONTEXT;
extern ImFontAtlas* _gAtlas;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void SetupPixelFormat(HDC hDC);
void CreateFontAtlas();
void DeleteFontAtlas();

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
// Standard Win32 dialog callback
//
LRESULT CALLBACK U2XWindowOpenProc(HWND hOgl, UINT Message, WPARAM wParam, LPARAM lParam);

class U2XWindow
{
public:
  U2XWindow();
	virtual ~U2XWindow();

	virtual char*	GetName() { return "U2XWindow"; };
		
	//
	// window
	//

  virtual LRESULT SetWindowSize(int ox, int oy, int cx, int cy);

  virtual void FillBackground();
 
  virtual void Create(HWND hwnd, bool shared=false);
  virtual void CreateContext(HWND hwnd);
  virtual void DestroyContext(HWND hwnd);
  virtual void InitGL();
  virtual void TermGL();
  virtual void Draw();
  virtual void Reshape(int width, int height);

protected:
  std::string       _className;
	HWND              _hWnd;
  HINSTANCE         _hInstance;
  HDC               _hDC;
  HGLRC             _hRC;
  ImGuiWindowFlags  _flags;
  ImGuiContext*     _ctxt;
  bool              _shared;
};
