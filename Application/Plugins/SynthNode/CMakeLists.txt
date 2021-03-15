set(TARGET SynthNode)

set(STK_SOURCES_DIR "C:/Users/graph/Documents/bmal/src/stk/src")
set(STK_INCLUDE_DIR "C:/Users/graph/Documents/bmal/src/stk/include")

add_definitions(
  -DNOMINMAX
  -D__WINDOWS_DS__
)

add_library(${TARGET} SHARED
	${STK_SOURCES_DIR}/Stk.cpp
	${STK_SOURCES_DIR}/FileWvIn.cpp
	${STK_SOURCES_DIR}/FileRead.cpp
	${STK_SOURCES_DIR}/FileLoop.cpp
	${STK_SOURCES_DIR}/FM.cpp
	${STK_SOURCES_DIR}/FMVoices.cpp
	${STK_SOURCES_DIR}/FormSwep.cpp
	${STK_SOURCES_DIR}/Modal.cpp
	${STK_SOURCES_DIR}/Fir.cpp
	${STK_SOURCES_DIR}/Twang.cpp
	${STK_SOURCES_DIR}/RtAudio.cpp
	${STK_SOURCES_DIR}/Voicer.cpp
	${STK_SOURCES_DIR}/DelayA.cpp
	${STK_SOURCES_DIR}/DelayL.cpp
	${STK_SOURCES_DIR}/BiQUad.cpp
	${STK_SOURCES_DIR}/OneZero.cpp
	${STK_SOURCES_DIR}/TwoZero.cpp
	${STK_SOURCES_DIR}/PoleZero.cpp
	${STK_SOURCES_DIR}/OnePole.cpp
	${STK_SOURCES_DIR}/Envelope.cpp
	${STK_SOURCES_DIR}/ADSR.cpp
	${STK_SOURCES_DIR}/Noise.cpp
	${STK_SOURCES_DIR}/SineWave.cpp
	${STK_SOURCES_DIR}/SingWave.cpp
	${STK_SOURCES_DIR}/Clarinet.cpp
	${STK_SOURCES_DIR}/BlowHole.cpp
	${STK_SOURCES_DIR}/Saxofony.cpp
	${STK_SOURCES_DIR}/Flute.cpp
	${STK_SOURCES_DIR}/Brass.cpp
	${STK_SOURCES_DIR}/BlowBotl.cpp
	${STK_SOURCES_DIR}/Bowed.cpp
	${STK_SOURCES_DIR}/Plucked.cpp
	${STK_SOURCES_DIR}/StifKarp.cpp
	${STK_SOURCES_DIR}/Sitar.cpp
	${STK_SOURCES_DIR}/Mandolin.cpp
	${STK_SOURCES_DIR}/Rhodey.cpp
	${STK_SOURCES_DIR}/Wurley.cpp
	${STK_SOURCES_DIR}/TubeBell.cpp
	${STK_SOURCES_DIR}/HevyMetl.cpp
	${STK_SOURCES_DIR}/PercFlut.cpp
	${STK_SOURCES_DIR}/BeeThree.cpp
	${STK_SOURCES_DIR}/FMVoices.cpp
	${STK_SOURCES_DIR}/VoicForm.cpp
	${STK_SOURCES_DIR}/Moog.cpp
	${STK_SOURCES_DIR}/Simple.cpp
	${STK_SOURCES_DIR}/Drummer.cpp
	${STK_SOURCES_DIR}/BandedWG.cpp
	${STK_SOURCES_DIR}/Shakers.cpp
	${STK_SOURCES_DIR}/ModalBar.cpp
	${STK_SOURCES_DIR}/Mesh2D.cpp
	${STK_SOURCES_DIR}/Resonate.cpp
	${STK_SOURCES_DIR}/Whistle.cpp
	${STK_SOURCES_DIR}/Phonemes.cpp
	${STK_SOURCES_DIR}/Modulate.cpp
	${STK_SOURCES_DIR}/Sampler.cpp
	${STK_SOURCES_DIR}/Sphere.cpp
	SynthoxData.cpp
	SynthoxInstrument.cpp
	SynthoxInstruments.cpp
	SynthoxPlayer.cpp
	SynthoxReader.cpp
	SynthoxRegister.cpp
	SynthoxVoicer.cpp
)

target_include_directories(${TARGET} 
  PUBLIC
	${STK_INCLUDE_DIR}
	${SOFTIMAGE_INCLUDE_DIR}
)

target_link_libraries(${TARGET}
  ${SOFTIMAGE_LIBRARIES}
  ${ADDITIONAL_LIBRARIES}
  Dsound
)
