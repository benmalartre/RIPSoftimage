#ifndef FLAG_OPENGL
#define FLAG_OPENGL

#ifdef unix
#define LPVOID void*
#endif

#ifndef unix
#if _MSC_VER>1000
#pragma once
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
//#include <GL/glaux.h>
//#include <GL/glut.h>
//#include <GL/glext.h>

#ifdef _MSC_VER
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"user32.lib")
#endif

namespace SOFTIMAGE{

	void GetWindowSize(int& width, int& height);
	bool IsExtensionSupported(char* extensionName);
}


#endif //FLAG_OPENGL
