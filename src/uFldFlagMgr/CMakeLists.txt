#--------------------------------------------------------
# The CMakeLists.txt for:                     uFldFlagMgr
# Author(s):                                Mike Benjamin
#--------------------------------------------------------

SET(SRC
  FlagManager.cpp
  FlagManager_Info.cpp
  main.cpp
)

ADD_EXECUTABLE(uFldFlagMgr ${SRC})

TARGET_LINK_LIBRARIES(uFldFlagMgr
   ${MOOS_LIBRARIES}
   apputil
   contacts
   geometry
   mbutil
   m
   pthread)

