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

