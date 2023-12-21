#ifndef U2I_CALLBACK_H
#define U2I_CALLBACK_H

#ifdef _WIN32
// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
#endif
#include <xsi_pluginregistrar.h>
#include <xsi_decl.h>
#include <xsi_status.h>

#ifndef unix
#ifdef USDHYDRACALLBACK_EXPORTS
#define USDHYDRACALLBACK_API __declspec(dllexport)
#else
#define USDHYDRACALLBACK_API __declspec(dllimport)
#endif
#else
#define USDHYDRACALLBACK_API
#define LPVOID void*
#endif

// we can safely ignore this warning, since we know that the 
// implementation of these functions will be done in C++
#ifdef _WIN32
#pragma warning(disable:4190)
#pragma warning(disable:4244)
#pragma warning(disable:4305)
#endif

#ifdef __cplusplus
extern "C" 
{
  // Display callbacks
  USDHYDRACALLBACK_API void	UsdHydra_Execute( XSI::CRef, LPVOID * );
  USDHYDRACALLBACK_API void	UsdHydra_Init( XSI::CRef, LPVOID * );
  USDHYDRACALLBACK_API void	UsdHydra_Term( XSI::CRef, LPVOID * );
  USDHYDRACALLBACK_API void	UsdHydra_InitInstance( XSI::CRef, LPVOID * );
  USDHYDRACALLBACK_API void	UsdHydra_TermInstance( XSI::CRef, LPVOID * );

};
#endif

#endif