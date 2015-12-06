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
# List the subdirectories to build...
#============================================================================
ADD_SUBDIRECTORY(lib_behaviors-test)
ADD_SUBDIRECTORY(lib_NMEAParse)
ADD_SUBDIRECTORY(lib_SimpleSerial)
ADD_SUBDIRECTORY(lib_gpsParser)

ADD_SUBDIRECTORY(uSpeechRec)
ADD_SUBDIRECTORY(uDialogManager)
ADD_SUBDIRECTORY(pXRelayTest)
ADD_SUBDIRECTORY(pExampleApp)
ADD_SUBDIRECTORY(uFldFlagManager)
ADD_SUBDIRECTORY(uFldTagManager)
ADD_SUBDIRECTORY(pFlagStrategy)
ADD_SUBDIRECTORY(iJoystick)
ADD_SUBDIRECTORY(pMapValues)
ADD_SUBDIRECTORY(iActuationMokai)
ADD_SUBDIRECTORY(iGPSDevice)
ADD_SUBDIRECTORY(iGPSSim)
ADD_SUBDIRECTORY(iM200)
ADD_SUBDIRECTORY(iOS5000AC)
ADD_SUBDIRECTORY(iuModemMP)
ADD_SUBDIRECTORY(uJSON)
ADD_SUBDIRECTORY(uSimM200)


##############################################################################
#                           END of CMakeLists.txt
##############################################################################

