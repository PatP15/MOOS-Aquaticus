#!/bin/bash


#-------------------------------------------------------
#  Part 1: Check for and handle command-line arguments
#-------------------------------------------------------
ADDING_ARG=""
UNDEFINED_ARG=""

for ARGI; do
    UNDEFINED_ARG=$ARGI
    if [ "${ARGI}" = "-t" ] ; then
	ADDING_ARG="-t"
	UNDEFINED_ARG=""
    fi
    if [ "${UNDEFINED_ARG}" != "" ] ; then
        BAD_ARGS=$UNDEFINED_ARG
    fi
done


#-------------------------------------------------------
#  Part 2: Handle Ill-formed command-line arguments
#-------------------------------------------------------

if [ "${BAD_ARGS}" != "" ] ; then
    printf "Bad Argument: %s \n" $BAD_ARGS
    exit 0
fi


cd ./m200
./launch_m200.sh -e -b -s $ADDING_ARG > /tmp/m200_blue &
./launch_m200.sh -f -r -s > /tmp/m200_red  &

cd ..

cd ./mokai
./launch_mokai.sh -b -ss > /tmp/mokai_blue &
./launch_mokai.sh -r -ss > /tmp/mokai_red  &

cd ..

cd ./shoreside
./launch_shoreside.sh
