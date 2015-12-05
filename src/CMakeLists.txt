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
ADD_SUBDIRECTORY(iJoyStick)
ADD_SUBDIRECTORY(pMapValues)
ADD_SUBDIRECTORY(iActuationMokai)

##############################################################################
#                           END of CMakeLists.txt
##############################################################################

