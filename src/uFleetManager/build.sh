#!/bin/bash
# MOOS Manager build script
# Raphael Segal
# 03/08/17

if [[ $# -ne 0 ]]; then
	ARTIFACT=$1
else
	ARTIFACT=MOOSFleetManager
fi

OLDDIR=`pwd`
cd src

# g++ system_call.cpp ManagedMoosMachine.cpp GetConfiguration.cpp dm_utils.cpp katm_exec.cpp -o katm
# SUCCESS=$( echo $? )

# if [[ SUCCESS -eq 0 ]]; then
# 	mv katm ../bin/
# else
# 	exit $SUCCESS
# fi

# g++ system_call.cpp ManagedMoosMachine.cpp GetConfiguration.cpp dm_utils.cpp restart_exec.cpp -o restart
# SUCCESS=$( echo $? )

# if [[ SUCCESS -eq 0 ]]; then
# 	mv restart ../bin/
# else
# 	exit $SUCCESS
# fi

g++  system_call.cpp ManagedMoosMachine.cpp GetConfiguration.cpp dm_utils.cpp \
	 -I$HOME/moos-ivp/ $HOME/moos-ivp/ivp/src/lib_apputil/ACTable.cpp ui.cpp \
	 -lncurses -o $ARTIFACT
SUCCESS=$( echo $? )

if [[ SUCCESS -eq 0 ]]; then
	mv $ARTIFACT ../bin/
else
	exit $SUCCESS
fi

cd $OLDDIR
# I think that $SUCCESS has to be 0 at this point...
exit $SUCCESS