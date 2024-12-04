cmake_minimum_required (VERSION 2.8.11)
project (RIPSoftimage)

# Require C++14
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# cross platform
set(LIB_PREFIX "")
set(LIB_DYNAMIC_EXT "")
set(LIB_STATIC_EXT "")

set(LIB_PREFIX "")
set(LIB_DYNAMIC_EXT ".lib")
set(LIB_STATIC_EXT ".lib")


# usd 
if(NOT DEFINED USD_DIR)
	message(FATAL_ERROR "No USD_DIR variable set! USD installation path must be passed to cmake!!!")
endif()


set(USD_DIR "C:/Users/graph/Documents/bmal/src/USD_BUILD")
set(USD_INCLUDE_DIR "${USD_DIR}/include")
set(USD_LIBRARY_DIR "${USD_DIR}/lib")
list(APPEND USD_LIB_NAMES 
  usd_vt
  usd_trace
  usd_work
  usd_arch
  usd_ar
  usd_gf
  usd_glf
  usd_tf
  usd_sdf
  usd_plug
  usd_garch
  usd_ndr
  usd_sdr
  usd_sdf
  usd_pcp
  usd_usd
  usd_hd
  usd_hdSt
  usd_hdx
  usd_hgi
  usd_hf
  usd_hio
  usd_usdGeom
  usd_usdSkel
  usd_usdLux
  usd_usdShade
  usd_usdUI
  usd_usdImaging
  usd_usdImagingGL
  usd_python
)

set(USD_LIBRARIES "")
foreach(USD_LIB_NAME IN LISTS USD_LIB_NAMES)
  set(USD_LIBRARIES 
    ${USD_LIBRARIES} 
      "${USD_LIBRARY_DIR}/${LIB_PREFIX}${USD_LIB_NAME}${LIB_DYNAMIC_EXT}"
  )
endforeach()

# python
find_package(Python3 COMPONENTS Interpreter Development)

# boost
if(UNIX)
  if(APPLE)
    set(BOOST_INCLUDE_DIR "{USD_INCLUDE_DIR}")
  else()

  endif()
else()
	set(BOOST_INCLUDE_DIR "${USD_INCLUDE_DIR}/boost-1_76")
endif()

set(BOOST_LIBRARY_DIR "{USD_LIBRARY_DIR}")
list(APPEND BOOST_LIB_NAMES 
  atomic
  regex
)

set(BOOST_LIBRARIES "")
set(BOOST_VERSION "")
if(NOT UNIX)
	set(BOOST_VERSION "-vc142-mt-x64-1_76")
endif()
foreach(BOOST_LIB_NAME IN LISTS BOOST_LIB_NAMES)
  set(BOOST_LIBRARIES 
    ${BOOST_LIBRARIES} 
      "${USD_LIBRARY_DIR}/${LIB_PREFIX}boost_${BOOST_LIB_NAME}${BOOST_VERSION}${LIB_DYNAMIC_EXT}"
  )
endforeach()

# tbb
set(TBB_INCLUDE_DIR "${USD_DIR}/include/tbb")
set(TBB_LIBRARY_DIR "${USD_DIR}/lib")

list(APPEND TBB_LIB_NAMES 
  tbb
  tbbmalloc_proxy
  tbbmalloc
)

set(TBB_LIBRARIES "")
foreach(TBB_LIB_NAME IN LISTS TBB_LIB_NAMES)
  set(TBB_LIBRARIES 
    ${TBB_LIBRARIES} 
      "${TBB_LIBRARY_DIR}/${LIB_PREFIX}${TBB_LIB_NAME}${LIB_DYNAMIC_EXT}"
  )
endforeach()

# OpenGL
find_package(OpenGL REQUIRED)

# system
IF(APPLE)
  SET(ADDITIONAL_LIBRARIES ${ADDITIONAL_LIBRARIES} 
    "-framework Cocoa -framework IOKit -framework AppKit")
ENDIF(APPLE)

# Softimage
set (SOFTIMAGE_SDK_DIR "C:/Program Files/Autodesk/Softimage 2015 SP2/XSISDK")
set (SOFTIMAGE_INCLUDE_DIR "${SOFTIMAGE_SDK_DIR}/include")
set (SOFTIMAGE_LIBRARY_DIR "${SOFTIMAGE_SDK_DIR}/lib/nt-x86-64")

list(APPEND SOFTIMAGE_LIB_NAMES 
  sicoresdk
  sicppsdk
)

set(SOFTIMAGE_LIBRARIES "")
foreach(SOFTIMAGE_LIB_NAME IN LISTS SOFTIMAGE_LIB_NAMES)
  set(SOFTIMAGE_LIBRARIES 
    ${SOFTIMAGE_LIBRARIES} 
      "${SOFTIMAGE_LIBRARY_DIR}/${LIB_PREFIX}${SOFTIMAGE_LIB_NAME}${LIB_STATIC_EXT}"
  )
endforeach()

set (ADDITIONAL_LIBRARIES Comctl32)

add_subdirectory (UsdRead)
add_subdirectory (UsdWrite)
#add_subdirectory (UsdIce)
add_subdirectory (IceRig)
add_subdirectory (SynthNode)
add_subdirectory (Triangulate)
add_subdirectory (Voronoi)