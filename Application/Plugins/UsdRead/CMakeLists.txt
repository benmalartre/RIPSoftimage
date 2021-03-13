set(TARGET UsdRead)

add_definitions(
  -DTASKING_TBB
  -DNOMINMAX
  -DIMGUI_IMPL_OPENGL_LOADER_CUSTOM="${USD_INCLUDE_DIR}/pxr/imaging/garch/glApi.h"
)

add_library(${TARGET} SHARED
  imgui/imgui.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_impl_win32.cpp
  imgui/imgui_impl_opengl3.cpp
  imgui/imgui_demo.cpp
  imgui/imgui_widgets.cpp
  attribute.cpp
  event.cpp
  explorer.cpp
  mesh.cpp
  plugin.cpp
  prim.cpp
  scene.cpp
  shader.cpp
  stage.cpp
  topology.cpp
  utils.cpp
  vertexArray.cpp
  vertexBuffer.cpp
  window.cpp
  xform.cpp
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
