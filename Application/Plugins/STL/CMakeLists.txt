set(TARGET STL)

add_definitions(
  -DNOMINMAX
)

add_library(${TARGET} SHARED
  STL_File.cpp
  STL_Export.cpp
  STL_Import.cpp
  STL_Register.cpp
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
