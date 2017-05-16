#--------------------------------------------------------
# The CMakeLists.txt for:                   uFleetManager
# Author(s):                                Raphael Segal
#--------------------------------------------------------

# Set System Specific Libraries
if (${WIN32})
  # Windows Libraries
  SET(SYSTEM_LIBS
     wsock32 )
else (${WIN32})
  # Linux and Apple Libraries
  SET(SYSTEM_LIBS
      m
      pthread )
endif (${WIN32})


SET(SRC
   src/system_call.cpp
   src/ManagedMoosMachine.cpp
   src/Configuration.cpp
   src/utils.cpp
   src/ui.cpp
   src/main.cpp
)

ADD_EXECUTABLE(uFleetManager ${SRC})

TARGET_LINK_LIBRARIES(uFleetManager
   ${MOOS_LIBRARIES}
   mbutil
   ncurses
   apputil
   ${SYSTEM_LIBS} )
