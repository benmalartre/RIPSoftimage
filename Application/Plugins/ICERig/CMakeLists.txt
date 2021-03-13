set(TARGET ICERig)

add_definitions(
  -DNOMINMAX
)

add_library(${TARGET} SHARED
  #ICERigTools.cpp
  IRBrushTool.cpp
  IRBuildElementID.cpp
  IRCubicBezier.cpp
  IRCubicBezierGenerator.cpp
  IRCubicBezierPoint.cpp
  IRDeltaMush.cpp
  IRDeltaMushHelpFunction.cpp
  IRDeltaMushInit.cpp
  IRElement.cpp
  IRFabrik.cpp
  #IRHandle.cpp
  IRJoint.cpp
  IRManipulator.cpp
  IRMoveHandle.cpp
  IROffsetHandle.cpp
  IRRegister.cpp
  IRResampleCurve.cpp
  IRRotateHandle.cpp
  IRScaleHandle.cpp
  IRSetElement.cpp
  IRShapeHandle.cpp
  IRSkeletonHandle.cpp
  IRSmooth.cpp
  IRSmoothWeights.cpp
  IRToolButton.cpp
  IRToolHandle.cpp
  #IRToolRegister.cpp
  IRUpVectorHandle.cpp
  TestICEAttribute.cpp
)

target_include_directories(${TARGET} 
  PUBLIC
	${SOFTIMAGE_INCLUDE_DIR}
)

target_link_libraries(${TARGET}
  ${SOFTIMAGE_LIBRARIES}
  ${OPENGL_LIBRARY}
  ${ADDITIONAL_LIBRARIES}
)
