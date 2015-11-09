#--------------------------------------------------------
# The CMakeLists.txt for:                       uSpeechRec
# Author(s):                              
#--------------------------------------------------------

SET(SRC
  SpeechRec.cpp
  SpeechRec_Info.cpp
  main.cpp
)

if (NOT ${APPLE})
  SET(SPEECH_LIBS  asound pulse pulse-simple)
#else
#  SET(SPEECH_LIBS )
endif (NOT ${APPLE})

ADD_EXECUTABLE(uSpeechRec ${SRC})

include_directories(/opt/local/include/)
#link_directories(~/Documents/git-repo/julius/julius-4.3.1/libjulius/src ~Documents/git-repo/julius/julius-4.3.1/libsent/src)

TARGET_LINK_LIBRARIES(uSpeechRec
   ${MOOS_LIBRARIES}

   apputil
   mbutil
   m
   pthread
   portaudio
   sent
   ${SPEECH_LIBS}   
   dl
   julius
   sent
   z)


