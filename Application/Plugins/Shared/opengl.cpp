#include "xsi_common.h"
#include "opengl.h"

namespace SOFTIMAGE
{
	#ifndef unix
	bool APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
	{
		switch(ul_reason_for_call)
		{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
		}
		return true;
	}
	#endif

	// GetWindowSize
	void GetWindowSize(int& width, int& height)
	{
		HDC hdc = NULL;
		hdc = wglGetCurrentDC();

		// Get the window dimensions
		RECT rect;
		GetWindowRect(WindowFromDC(hdc),&rect);

		// Calculate the window rectangle width and height
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;
	}

	// IsExtensionSupported
	bool IsExtensionSupported(char* extName)
	{
		/* Search for extName in the extensions string.  Use of strstr()
		is not sufficient because extension names can be prefixes of
		other extension names.  Could use strtok() but the constant
		string returned by glGetString can be in read-only memory. */
		char *p = (char *) glGetString(GL_EXTENSIONS);
		char *end;
		size_t extNameLen;

		extNameLen = strlen(extName);
		end = p + strlen(p);

		while (p < end)
		{
			size_t n = strcspn(p, " ");
			if ((extNameLen == n) && (strncmp(extName, p, n) == 0))
			{
				return true;
			}
			p += (n + 1);
		}

		return false;
	}
}