#!/bin/bash

#-----------------------------------------------
# Launch Evan and check for results
#-----------------------------------------------
./launch_m200.sh --vname=evan          \
		 --vteam=red           \
		 --startpos=-30,-60,70 \
		 --sim >& /dev/null

if [ $? -ne 0 ]; then
    echo launch of evan failed. Exiting.
    exit 1
fi

#-----------------------------------------------
# Launch Felix and check for results
#-----------------------------------------------
./launch_m200.sh --vname=felix         \
		 --vteam=blue          \
		 --startpos=30,-30,230 \
		 --sim >& /dev/null

if [ $? -ne 0 ]; then
    echo launch of felix failed. Exiting.
    exit 1
fi


#-----------------------------------------------
# Launch the Shoreside
#-----------------------------------------------
./launch_shoreside.sh
