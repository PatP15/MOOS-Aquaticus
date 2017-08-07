#--------------------------------------------------------
# The CMakeLists.txt for:                  lib_colregs_aq
#                                      and BHV_AvdColregs
# Author(s):                                Mike Benjamin
#--------------------------------------------------------

# Set System Specific Libraries
if (${WIN32})
  # Windows Libraries
  SET(SYSTEM_LIBS
      )
else (${WIN32})
  # Linux and Apple Libraries
  SET(SYSTEM_LIBS
      m )
endif (${WIN32})

#--------------------------------------------------------
#                                     BHV_OpRegionRecover
#--------------------------------------------------------
ADD_LIBRARY(BHV_OpRegionRecover SHARED 
   BHV_OpRegionRecover.cpp
)
TARGET_LINK_LIBRARIES(BHV_OpRegionRecover
   helmivp 
   behaviors 
   ivpbuild 
   logic 
   ivpcore 
   bhvutil 
   mbutil 
   geometry 
   ${SYSTEM_LIBS} )


#--------------------------------------------------------
#                                             BHV_Defense
#--------------------------------------------------------
ADD_LIBRARY(BHV_Defense SHARED
   BHV_Defense.cpp)
TARGET_LINK_LIBRARIES(BHV_Defense
   helmivp
   behaviors
   ivpbuild
   logic
   ivpcore
   bhvutil
   mbutil
   geometry
   ${SYSTEM_LIBS} )

#--------------------------------------------------------
#                                       BHV_Defense_Collab
#--------------------------------------------------------
ADD_LIBRARY(BHV_Defense_Collab SHARED
   BHV_Defense_Collab.cpp NodeReport.cpp)
TARGET_LINK_LIBRARIES(BHV_Defense_Collab
   helmivp
   behaviors
   ivpbuild
   logic
   ivpcore
   bhvutil
   mbutil
   geometry
   ${SYSTEM_LIBS} )
#--------------------------------------------------------
#                                       BHV_Defense_Collab
#--------------------------------------------------------
ADD_LIBRARY(BHV_Cover SHARED
   BHV_Cover.cpp NodeReport.cpp)
TARGET_LINK_LIBRARIES(BHV_Cover
   helmivp
   behaviors
   ivpbuild
   logic
   ivpcore
   bhvutil
   mbutil
   geometry
   ${SYSTEM_LIBS} )
