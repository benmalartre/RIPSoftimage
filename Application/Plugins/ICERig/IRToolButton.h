#ifndef IR_TOOL_BUTTON_H
#define IR_TOOL_BUTTON_H
///////////////////////////////////////////////////////////////////////////
//
// File: IRToolButton.h
//
///////////////////////////////////////////////////////////////////////////
#include <xsi_toolcontext.h>
#ifndef linux
#define WIN32_LEAN_AND_MEAN
#include <windows.h> // Needed for OpenGL on windows
#endif
#include <string.h>
#include <GL/gl.h>

using namespace XSI; 

namespace ICERIG {
  class IRToolButton {
  public:
    IRToolButton();
    ~IRToolButton();
    void Setup(const wchar_t *basename, LONG in_index, LONG in_count);
    void Cleanup();
    bool IsSelected() const { return m_selected; }
    void SetSelected(bool selected) { m_selected = selected; }
    bool IsPointInside(ToolContext &in_ctxt, LONG x, LONG y) const;
    void Draw(ToolContext &in_ctxt);
    void LoadTextures(ToolContext &in_ctxt);

  private: // Implementation
    CStatus GetButtonPosition(ToolContext &in_ctxt, LONG out_rect[]) const;

    CString m_basename;       // Base texture name
    GLuint  m_textureID[2];   // OpenGL texture IDs
    LONG    m_buttonSize[2];  // Button size
    LONG    m_buttonIndex;    // Index of button
    LONG    m_buttonCount;    // Total number of buttons
    bool    m_selected;       // Selected state
  };
} // ICERIG
#endif // IR_TOOL_BUTTON_INCLUDED
