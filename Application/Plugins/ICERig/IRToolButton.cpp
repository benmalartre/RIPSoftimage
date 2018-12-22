///////////////////////////////////////////////////////////////////////////
//
// File: IRToolButton.cpp
//
///////////////////////////////////////////////////////////////////////////
#include "IRToolButton.h"
#include <xsi_application.h>

namespace ICERIG {
	IRToolButton::IRToolButton()
		: m_buttonIndex(0)
		, m_buttonCount(0)
		, m_selected(false)
	{
		m_textureID[0] = m_textureID[1] = 0;
		m_buttonSize[0] = m_buttonSize[1] = 0;
	}

	IRToolButton::~IRToolButton()
	{
	}

	void IRToolButton::Setup(const wchar_t *basename, LONG index, LONG count)
	{
		m_basename = basename;
		m_buttonIndex = index;
		m_buttonCount = count;
	}

	void IRToolButton::Cleanup()
	{
		if (m_textureID[0]) {
			glDeleteTextures(1, &m_textureID[0]);
			m_textureID[0] = 0;
		}
		if (m_textureID[1]) {
			glDeleteTextures(1, &m_textureID[1]);
			m_textureID[1] = 0;
		}
	}

	bool IRToolButton::IsPointInside(ToolContext &in_ctxt, LONG x, LONG y) const
	{
		LONG rect[4];
		if (GetButtonPosition(in_ctxt, rect) != CStatus::OK) return false;
		return (x > rect[0] && x < rect[2] && y > rect[1] && y < rect[3]);
	}

	void PremultiplyAlpha(GLubyte *pixel)
	{
		pixel[0] = (GLubyte)(((LONG)(pixel[0]) * pixel[3]) / 255);
		pixel[1] = (GLubyte)(((LONG)(pixel[1]) * pixel[3]) / 255);
		pixel[2] = (GLubyte)(((LONG)(pixel[2]) * pixel[3]) / 255);
	}

	void IRToolButton::Draw(ToolContext &in_ctxt)
	{
		LoadTextures(in_ctxt);
		if (m_textureID[m_selected] == 0) return; // Missing texture

		// Draw textured button
		LONG rect[4];
		GetButtonPosition(in_ctxt, rect);
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glBindTexture(GL_TEXTURE_2D, m_textureID[m_selected]);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // Pre-multiplied alpha
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glColor4ub(0, 0, 0, 255);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 1.0);
		glVertex2i(rect[0], rect[3]);
		glTexCoord2f(1.0, 1.0);
		glVertex2i(rect[2], rect[3]);
		glTexCoord2f(1.0, 0.0);
		glVertex2i(rect[2], rect[1]);
		glTexCoord2f(0.0, 0.0);
		glVertex2i(rect[0], rect[1]);
		glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	void IRToolButton::LoadTextures(ToolContext &in_ctxt)
	{
		if (m_textureID[0] != 0) return; // Already loaded

		for (int i = 0; i < 2; ++i)
		{
			CString l_Filename = m_basename + (i == 0 ? L"U.png" : L"D.png");
			Image image = in_ctxt.LoadImageFromFile(l_Filename);
			if (!image.IsValid()) {
				Application().LogMessage(L"IRToolButton: Unable to load texture: " + l_Filename, siWarningMsg);
				continue;
			}

			if (image.GetNumChannels() != 4 || image.GetChannelSize() != 1)
			{
				Application().LogMessage(L"IRToolButton: Wrong image format (8-bit RGBA expected): " + l_Filename, siWarningMsg);
				continue;
			}


			glGenTextures(1, &m_textureID[i]);
			glBindTexture(GL_TEXTURE_2D, m_textureID[i]);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // No pixel averaging
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // No pixel averaging

			m_buttonSize[0] = image.GetResX();
			m_buttonSize[1] = image.GetResY();
			LONG numPixels = m_buttonSize[0] * m_buttonSize[1];
			GLubyte *bits = (GLubyte *)malloc(numPixels * sizeof(GLubyte[4]));
			memcpy(bits, image.GetPixelArray(), numPixels * sizeof(GLubyte[4]));

			// convert image to premultiplied alpha
			for (int i = 0; i < numPixels; ++i)
				PremultiplyAlpha(bits + 4 * i);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_buttonSize[0], m_buttonSize[1], 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, bits);
			free(bits);
		}
	}

	CStatus IRToolButton::GetButtonPosition(ToolContext &in_ctxt, LONG out_rect[]) const
	{
		out_rect[0] = out_rect[1] = out_rect[2] = out_rect[3] = 0;
		if (m_buttonSize[0] == 0) return CStatus::Fail; // texture has not been loaded

		LONG viewWidth, viewHeight;
		if (in_ctxt.GetViewSize(viewWidth, viewHeight) != CStatus::OK) return CStatus::Fail; // No view information

		LONG spacing = 4/*pixels*/;
		LONG x = viewWidth / 2 - m_buttonSize[0] * m_buttonCount / 2 - spacing * (m_buttonCount - 1) / 2 + m_buttonSize[0] * m_buttonIndex + spacing * m_buttonIndex;
		LONG y = m_buttonSize[1];
		out_rect[0] = x; out_rect[1] = y; out_rect[2] = x + m_buttonSize[0]; out_rect[3] = y + m_buttonSize[1];
		return CStatus::OK;
	}
}
