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
IF(BUILD_ALL)
 ADD_SUBDIRECTORY(uFldTagManager)
ADD_SUBDIRECTORY(uFldFlagManager)
ADD_SUBDIRECTORY(iBlink)
  ADD_SUBDIRECTORY(pMapValues)
  ADD_SUBDIRECTORY(uSimM200)
#  ADD_SUBDIRECTORY(pComms_client)
#  ADD_SUBDIRECTORY(pComms_server)
 ADD_SUBDIRECTORY(app_aq_analysis)
  #  ADD_SUBDIRECTORY(uFleetManager)
  #  ADD_SUBDIRECTORY(pShorePing)
  #  ADD_SUBDIRECTORY(pPingDisplay)
#  ADD_SUBDIRECTORY(uFleetManager)
  ADD_SUBDIRECTORY(uNetMon)
ENDIF()
IF(BUILD_MOKAI)
    ADD_SUBDIRECTORY(iGPSDevice)
  ADD_SUBDIRECTORY(iOS5000)
  ADD_SUBDIRECTORY(uSpeechRec)
  ADD_SUBDIRECTORY(uDialogManager)
   ADD_SUBDIRECTORY(iLEDInfoBar)
 ADD_SUBDIRECTORY(iButtonBox)
ADD_SUBDIRECTORY(pRecord)
  ADD_SUBDIRECTORY(iLEDInterpreter)
  ADD_SUBDIRECTORY(iJoystick)
ENDIF()
 
ADD_SUBDIRECTORY(lib_SimpleSerial)
ADD_SUBDIRECTORY(lib_behaviors-aq)
ADD_SUBDIRECTORY(lib_NMEAParse)
ADD_SUBDIRECTORY(lib_gpsParser)
ADD_SUBDIRECTORY(iGPS)
#ADD_SUBDIRECTORY(iIMU_RazorAHRS)
ADD_SUBDIRECTORY(uFldZoneEvent)
ADD_SUBDIRECTORY(pBotDialog)
ADD_SUBDIRECTORY(app_catauthkeys)
ADD_SUBDIRECTORY(pRangeEvent)
#ADD_SUBDIRECTORY(pAttackCommander)
ADD_SUBDIRECTORY(iMetaCompass)
ADD_SUBDIRECTORY(iM200)
ADD_SUBDIRECTORY(pNodeReportParse)
#  ADD_SUBDIRECTORY(pPingResponder)
#  ADD_SUBDIRECTORY(pTimeWatch)
#  ADD_SUBDIRECTORY(iIMU_RazorAHRS)
#  ADD_SUBDIRECTORY(pShorePing)
#  ADD_SUBDIRECTORY(pPingDisplay)
ADD_SUBDIRECTORY(app_keyprog)

IF(BUILD_WEBCAM)
  ADD_SUBDIRECTORY(iWebCam)
  ADD_SUBDIRECTORY(pCamViewer)
ENDIF()

IF(BUILD_VOIP)
  ADD_SUBDIRECTORY(iMumbleClient)
ENDIF()

IF(BUILD_HRM)
  ADD_SUBDIRECTORY(iZephyrHRM)
ENDIF()

##############################################################################
#                           END of CMakeLists.txt
##############################################################################
