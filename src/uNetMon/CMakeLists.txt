#--------------------------------------------------------
# The CMakeLists.txt for:                       uFleetMon
# Author(s):                              
#--------------------------------------------------------

SET(SRC
  NetMon.cpp
  NetMon_Info.cpp
  main.cpp
)

ADD_EXECUTABLE(uNetMon ${SRC})

TARGET_LINK_LIBRARIES(uNetMon
   ${MOOS_LIBRARIES}
   mbutil
   m
   pthread
   ncurses)

