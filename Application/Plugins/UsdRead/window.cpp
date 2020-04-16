#include <GL/glew.h>
#include "window.h"
#include "utils.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_opengl3.h"


HGLRC U2X_SHARED_CONTEXT = (HGLRC)0;
U2XWindow* U2X_HIDDEN_WINDOW = NULL;
ImFontAtlas* U2X_SHARED_ATLAS = NULL;

//
// Softimage Main Window
//
BOOL U2XIsMainWindow(HWND hWnd)
{
  return GetWindow(hWnd, GW_OWNER) == (HWND)0 && IsWindowVisible(hWnd);
}

bool CALLBACK U2XEnumWindowsCallback(HWND hWnd, LPARAM lParam)
{
  U2XHandleData& data = *(U2XHandleData*)lParam;
  unsigned long processId = 0;
  GetWindowThreadProcessId(hWnd, &processId);
  if (data.processId != processId || !U2XIsMainWindow(hWnd))
    return true;
  data.hWnd = hWnd;
  return false;
}

HWND U2XGetSoftimageWindow()
{
  U2XHandleData data;
  data.processId = GetCurrentProcessId();
  data.hWnd = 0;
  EnumWindows((WNDENUMPROC)U2XEnumWindowsCallback, (LPARAM)&data);
  return data.hWnd;
}

//
// Shared Font Atlas
//
void CreateFontAtlas()
{
  U2X_SHARED_ATLAS = new ImFontAtlas();
  U2X_SHARED_ATLAS->AddFontDefault();
}

void DeleteFontAtlas()
{
  if (U2X_SHARED_ATLAS)delete U2X_SHARED_ATLAS;
}

void GetSharedContext()
{
  if (U2X_HIDDEN_WINDOW == NULL)
  {
    U2X_HIDDEN_WINDOW = new U2XWindow();
    U2X_HIDDEN_WINDOW->Create(U2XGetSoftimageWindow(), true);
    U2X_HIDDEN_WINDOW->InitGL();
  }
}

void U2XWindow::InitGL()
{
  wglMakeCurrent(_hDC, _hRC);
  glewInit();

  // Setup Dear ImGui binding
  IMGUI_CHECKVERSION();
  CreateFontAtlas();

  // hidden context
  _ctxt = ImGui::CreateContext(U2X_SHARED_ATLAS);
  ImGui::SetCurrentContext(_ctxt);

  // init Win32
  ImGui_ImplWin32_Init(_hWnd);

  // init OpenGL Imgui Implementation
  const char* glsl_version = "#version 130";
  ImGui_ImplOpenGL3_Init(glsl_version);

  Draw();

}

void U2XWindow::TermGL()
{
  DeleteFontAtlas();
  ImGui::SetCurrentContext(_ctxt);
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplWin32_Shutdown(_hWnd);
  if (_ctxt)ImGui::DestroyContext(_ctxt);
}


void U2XWindow::Draw()
{
  wglMakeCurrent(_hDC, _hRC);
  // fake render to initialize shared data
  ImGuiIO& io = ImGui::GetIO();
  io.DisplaySize.x = 32;
  io.DisplaySize.y = 32;
  
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplWin32_NewFrame(_hWnd);

  ImGui::NewFrame();

  ImGui::EndFrame();
  ImGui::Render();

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  SwapBuffers(_hDC);
}

void U2XWindow::Reshape(int width, int height)
{
}

void U2XWindow::FillBackground()
{
 
  ImVec2 vMin = ImGui::GetWindowContentRegionMin();
  ImVec2 vMax = ImGui::GetWindowContentRegionMax();

  vMin.x += ImGui::GetWindowPos().x;
  vMin.y += ImGui::GetWindowPos().y;
  vMax.x += ImGui::GetWindowPos().x;
  vMax.y += ImGui::GetWindowPos().y;

  ImGui::GetForegroundDrawList()->AddRect(vMin, vMax, IM_COL32(rand(), rand(), 0, 255));
 
}


U2XWindow::U2XWindow()
{
}

U2XWindow::~U2XWindow()
{
  DestroyWindow(_hWnd);
  _hWnd = NULL;
  UnregisterClass(_className.c_str(), _hInstance);
}

void U2XWindow::Create(HWND hParent, bool shared)
{
  _className = "U2XWindow" +std::to_string((int)hParent);
  _shared = shared;
  WNDCLASS WindowClass = {
    0,                            // style
    U2XWindowCallback,            // window proc
    0,                            // additional class memory in bytes
    0,                            // additional window memory in bytes
    _hInstance,                   // instance
    0,                            // icon
    0,                            // cursor
    0,                            // bg color
    NULL,                         // menu
    _className.c_str()            // name window class
  };
  if (!RegisterClass(&WindowClass))
  {
    MessageBox(NULL, "Can't register OpenGl window", "exit(1)", NULL);
    exit(1);
  }

  DWORD style;
  if (_shared)style = WS_CHILD | WS_DISABLED;
  else style = WS_CHILD | WS_VISIBLE | WS_BORDER;

  _hWnd = CreateWindow(
    _className.c_str(),
    "U2X OpenGL Window",
    style,
    0,                // x
    0,                // y
    400,              // width
    800,              // height
    hParent,          // parent
    0,                // child id
    _hInstance,       // instance
    this              // user data
  );
  if (!_hWnd)
  {
    MessageBox(NULL, "OpenGl handle is NULL", "exit(1)", NULL);
    exit(1);
  }

  if(!_shared)ShowWindow(_hWnd, SW_SHOW);
  
}

void U2XWindow::CreateContext(HWND hwnd)
{
  _hDC = GetDC(hwnd);
  SetupPixelFormat(_hDC);
  _hRC = wglCreateContext(_hDC);
  if (!_hRC) SendMessage(hwnd, WM_CLOSE, 0, 0);
  if (_shared)U2X_SHARED_CONTEXT = _hRC;
  else wglShareLists(U2X_SHARED_CONTEXT, _hRC);
  wglMakeCurrent(_hDC, _hRC);
}

void U2XWindow::DestroyContext(HWND hwnd)
{
  wglMakeCurrent(NULL, NULL);
  if (_hRC) wglDeleteContext(_hRC);
  ReleaseDC(hwnd, _hDC);
  PostQuitMessage(0);
}

void SetupPixelFormat(HDC hDC)
{
  PIXELFORMATDESCRIPTOR pfd =
  {
    sizeof(PIXELFORMATDESCRIPTOR), 	// format descriptor size
    1,	                            // version
    PFD_SUPPORT_OPENGL |
    PFD_DRAW_TO_WINDOW |
    PFD_DOUBLEBUFFER,	              // properties
    PFD_TYPE_RGBA, 	                // color mode
    16, 	                          // color bits
    0, 0, 0, 0, 0, 0,	              // color parameters
    0,0, 	                          // alpha parameters
    0,0, 0, 0, 0,	                  // accumulation buffer parameters
    32,	                            // depth buffer bits
    0, 	                            // scencil buffer bits
    0, 	                            // num secondary buffer
    0, 	                            // unused (deprecated)
    0, 	                            // resereved
    0, 	                            // unused (deprecated)
    0,	                            // transparency color
    0 	                            // unused (deprecated)
  };

  int pixelFormat;
  pixelFormat = ChoosePixelFormat(hDC, &pfd);
  if (!pixelFormat)
  {
    MessageBox(
      WindowFromDC(hDC), 
      "Unsupported Graphics Mode",
      "Problem",
      MB_ICONERROR | MB_OK
    );
    exit(1);	// check pixel format descriptor
  }
  if (!SetPixelFormat(hDC, pixelFormat, &pfd))
  {

    MessageBox
    (
      WindowFromDC(hDC),
      "Unsupported Graphics Mode",
      "Problem",
      MB_ICONERROR | MB_OK
    );
    exit(1);	// apply pixel format
  }
}

LRESULT CALLBACK U2XWindowCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    return true;

  U2XWindow* window = (U2XWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
  switch (msg)
  {
  case WM_DESTROY:
    window->DestroyContext(hWnd);
    break;

  case WM_CREATE:
    window = (U2XWindow *)((CREATESTRUCT *)lParam)->lpCreateParams;
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)window);
    window->CreateContext(hWnd);
    break;

  case WM_SIZE:
    window->Reshape( LOWORD(lParam), HIWORD(lParam) );
    break;

  case WM_PAINT:
    window->Draw();
    break;

  default:
    return DefWindowProc(hWnd, msg, wParam, lParam);
    break;
  }
  return FALSE;
}