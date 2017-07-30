#--------------------------------------------------------
# The CMakeLists.txt for:             uDialogManager_4_0
# Author(s): Michael "Misha" Novitzky                             
#--------------------------------------------------------

SET(SRC
  DialogManager_4_0.cpp
  DialogManager_Info_4_0.cpp
  main_4_0.cpp
)

ADD_EXECUTABLE(uDialogManager ${SRC})

TARGET_LINK_LIBRARIES(uDialogManager
   ${MOOS_LIBRARIES}
   apputil
   mbutil
   m
   pthread)

