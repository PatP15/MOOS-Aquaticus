##############################################################################
# FILE:        moos-ivp-extend/src/CMakeLists.txt
# DATE:        2010/09/07
# DESCRIPTION: CMakeLists.txt file for the moos-ivp-extend source directory
##############################################################################


#============================================================================
# Add the libraries in the current directory to the include path
#============================================================================
FILE(GLOB LOCAL_LIBRARY_DIRS ./lib_*)
INCLUDE_DIRECTORIES(${LOCAL_LIBRARY_DIRS})

#============================================================================
# option for minimal and full compilation
#============================================================================
option(BUILD_ALL "Build all apps." ON)


#============================================================================
# List the subdirectories to build...
#============================================================================
IF(BUILD_ALL)
  ADD_SUBDIRECTORY(iBlink)
  ADD_SUBDIRECTORY(lib_SimpleSerial)
  ADD_SUBDIRECTORY(uSpeechRec)
  ADD_SUBDIRECTORY(uDialogManager)
  ADD_SUBDIRECTORY(iJoystick)
  ADD_SUBDIRECTORY(pMapValues)
  ADD_SUBDIRECTORY(iGPSDevice)
  ADD_SUBDIRECTORY(iOS5000)
  ADD_SUBDIRECTORY(uSimM200)
#  ADD_SUBDIRECTORY(uFleetManager)
#  ADD_SUBDIRECTORY(pShorePing)
#  ADD_SUBDIRECTORY(pPingDisplay)
#  ADD_SUBDIRECTORY(uFleetManager)
ENDIF()
ADD_SUBDIRECTORY(iGPS)
#ADD_SUBDIRECTORY(iIMU_RazorAHRS)
  ADD_SUBDIRECTORY(lib_behaviors-aq)
  ADD_SUBDIRECTORY(uFldZoneEvent)
  ADD_SUBDIRECTORY(uFldTagManager)
  ADD_SUBDIRECTORY(uFldFlagManager)
  ADD_SUBDIRECTORY(lib_NMEAParse)
  ADD_SUBDIRECTORY(lib_gpsParser)
  ADD_SUBDIRECTORY(pBotDialog)
  ADD_SUBDIRECTORY(app_catauthkeys)
  ADD_SUBDIRECTORY(pRangeEvent)
  #ADD_SUBDIRECTORY(pAttackCommander)
  ADD_SUBDIRECTORY(iMetaCompass)
  ADD_SUBDIRECTORY(iM200)
  ADD_SUBDIRECTORY(pNodeReportParse)
  ADD_SUBDIRECTORY(iButtonBox)
  #ADD_SUBDIRECTORY(pRecord)
  


#  ADD_SUBDIRECTORY(pPingResponder)
#  ADD_SUBDIRECTORY(pTimeWatch)
#  ADD_SUBDIRECTORY(iIMU_RazorAHRS)
#  ADD_SUBDIRECTORY(pShorePing)
#  ADD_SUBDIRECTORY(pPingDisplay)



##############################################################################
#                           END of CMakeLists.txt
##############################################################################
