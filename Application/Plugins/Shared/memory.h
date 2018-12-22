
#ifndef FLAG_MEMORY
#define FLAG_MEMORY

#if _DEBUG	// Debug
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
#else		// Release
	#ifdef WIN32
		#include <malloc.h>
	#endif
#endif

namespace SOFTIMAGE {

#if _DEBUG	// Debug
	class DetectMemoryLeaks
	{
	public:
		DetectMemoryLeaks(int line)	{ _crtBreakAlloc = line; }
		~DetectMemoryLeaks()		{ _CrtDumpMemoryLeaks(); }
	};
	#define FIND_MEMORY_LEAKS(val) static DetectMemoryLeaks detect(val);
	#define ENABLE_SMALL_MEMORY_BLOCK_HEAP(val)
#else		// Release
	#ifdef WIN32
		// setup small block memory heap
		class b_cEnable_small_memory_heap_block
		{
		public:
			b_cEnable_small_memory_heap_block(int val)		{ _set_sbh_threshold(val); }
		};
		#define FIND_MEMORY_LEAKS(val)
		#define ENABLE_SMALL_MEMORY_BLOCK_HEAP(val) b_cEnable_small_memory_heap_block EnableSmhb(val)
	#else
		#define FIND_MEMORY_LEAKS(val)
		#define ENABLE_SMALL_MEMORY_BLOCK_HEAP(val)
	#endif
#endif

} // namespace b_xsi

using namespace b_xsi;

#endif // B_FLAG_MEMORY


//
// _CRTDBG_ALLOC_MEM_DF (Default ON)
//		ON: Enable debug heap allocations and use of memory block type identifiers, such as _CLIENT_BLOCK.
//		OFF: Add new allocations to heap's linked list, but set block type to _IGNORE_BLOCK.
//
// _CRTDBG_LEAK (Default OFF)
//		ON: Perform automatic leak checking at program exit through a call to _CrtDumpMemoryLeaks and generate an error report if the application failed to free all the memory it allocated.
//		OFF: Do not automatically perform leak checking at program exit.
//
//	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
//
