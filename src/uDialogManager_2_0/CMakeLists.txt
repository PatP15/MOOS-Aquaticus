#--------------------------------------------------------
# The CMakeLists.txt for:                       uDialogManager_2_0
# Author(s):                              
#--------------------------------------------------------

SET(SRC
  DialogManager.cpp
  DialogManager_Info.cpp
  main.cpp
)

ADD_EXECUTABLE(uDialogManager_2_0 ${SRC})

TARGET_LINK_LIBRARIES(uDialogManager_2_0
   ${MOOS_LIBRARIES}
   apputil
   mbutil
   m
   pthread)

