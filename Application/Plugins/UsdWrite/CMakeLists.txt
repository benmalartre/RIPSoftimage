set(TARGET UsdWrite)

add_definitions(
  -DTASKING_TBB
  -DNOMINMAX
)


add_library(${TARGET} SHARED
  attribute.cpp
  camera.cpp
  prim.cpp
  xform.cpp
  model.cpp
  points.cpp
  mesh.cpp
  curve.cpp
  instancer.cpp
  skeleton.cpp
  plugin.cpp
  scene.cpp
  ui.cpp
  utils.cpp
  
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
