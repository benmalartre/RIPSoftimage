/*------------------------------------------------------------------
		IROpenGL.h
------------------------------------------------------------------*/
#ifndef IR_OPENGL_H
#define IR_OPENGL_H
// OpenGL Stuff
#ifndef linux
#define WIN32_LEAN_AND_MEAN
#include <windows.h> // Needed for OpenGL on windows
#endif
#include <GL/gl.h>
#include <GL/glu.h>

// Macros for handling packed colors
#define RGBA_R(rgba)					(GLubyte)((rgba) & 0xFF)
#define RGBA_G(rgba)					(GLubyte)(((rgba) >>  8) & 0xFF)
#define RGBA_B(rgba)					(GLubyte)(((rgba) >> 16) & 0xFF)
#define RGBA_A(rgba)					(GLubyte)(((rgba) >> 24) & 0xFF)
#define RGBA_COLOR(r, g, b, a)			(ULONG)(((a) << 24) | ((b) << 16) | ((g) << 8) | (r))

inline  void SetGLColor(ULONG c)		{ glColor4ub( RGBA_R(c), RGBA_G(c), RGBA_B(c), RGBA_A(c)); }

inline void DrawCone( float in_size)
{
	GLUquadricObj *quadric = gluNewQuadric();
	gluQuadricDrawStyle( quadric, GLU_FILL );
	gluQuadricNormals( quadric, GLU_SMOOTH );
	gluCylinder (quadric,in_size/20,0.0,in_size/4,8,2);
	gluDeleteQuadric( quadric );
}

inline void DrawBone(float length,float radius)
{
	GLUquadricObj *quadric = gluNewQuadric();
	gluQuadricDrawStyle( quadric, GLU_FILL );
	gluQuadricNormals( quadric, GLU_SMOOTH );
	gluCylinder (quadric,radius,radius,length,4,2);
	gluDeleteQuadric( quadric );
}

#endif