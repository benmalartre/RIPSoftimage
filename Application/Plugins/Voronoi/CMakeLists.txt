set(TARGET Voronoi)

add_definitions(
  -DNOMINMAX
)

add_library(${TARGET} SHARED
  ApplyVoronoi.cpp
  CVoronoi.cpp
  HEMesh.cpp
  IMVoronoi.cpp
  Voronoi.cpp
)

target_include_directories(${TARGET} 
  PUBLIC
	${SOFTIMAGE_INCLUDE_DIR}
)

target_link_libraries(${TARGET}
  ${SOFTIMAGE_LIBRARIES}
  ${ADDITIONAL_LIBRARIES}
  winmm
)
