set(TARGET UsdICE)

add_definitions(
  -DTASKING_TBB
  -DNOMINMAX
  -DIMGUI_IMPL_OPENGL_LOADER_CUSTOM="${USD_INCLUDE_DIR}/pxr/imaging/garch/glApi.h"
  -DUSDHYDRACALLBACK_EXPORTS
)

add_library(${TARGET} SHARED
  callback.cpp
  data.cpp
  event.cpp
  nodes.cpp
  plugin.cpp
  scene.cpp
  stage.cpp
  utils.cpp
  engine.cpp
)

target_include_directories(${TARGET} 
  PUBLIC
	${SOFTIMAGE_INCLUDE_DIR}
    ${USD_INCLUDE_DIR}
    ${Python2_INCLUDE_DIRS}
    ${TBB_INCLUDE_DIR}
	${BOOST_INCLUDE_DIR}
)

target_link_libraries(${TARGET}
  ${SOFTIMAGE_LIBRARIES}
  ${OPENGL_LIBRARY}
  ${Python2_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${USD_LIBRARIES}
  ${TBB_LIBRARIES}
  #${GLEW_LIBRARY}
  ${ADDITIONAL_LIBRARIES}
)
