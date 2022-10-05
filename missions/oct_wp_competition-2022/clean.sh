#!/bin/bash

VERBOSE=""

#-------------------------------------------------------
#  Part 1: Check for and handle command-line arguments
#-------------------------------------------------------
for ARGI; do
    if [ "${ARGI}" = "--help" -o "${ARGI}" = "-h" ] ; then
      printf "%s [SWITCHES]                       \n" $0
      printf "  --verbose                         \n"
      printf "  --help, -h                        \n"
      exit 0;
    elif [ "${ARGI}" = "--verbose" -o "${ARGI}" = "-v" ] ; then
      VERBOSE="-v"
    else
      printf "Bad Argument: %s \n" $ARGI
      exit 0
    fi
done

#-------------------------------------------------------
#  Part 2: Do the cleaning!
#-------------------------------------------------------
cd surveyor

rm -rf  $VERBOSE   MOOSLog_*  C*_LOG_*
rm -f   $VERBOSE   *~  targ_* *.moos++
rm -f   $VERBOSE   .LastOpenedMOOSLogDirectory

cd ..

cd shoreside

rm -rf  $VERBOSE   MOOSLog_*  C*_LOG_*
rm -f   $VERBOSE   *~  targ_* *.moos++
rm -f   $VERBOSE   .LastOpenedMOOSLogDirectory
rm -f   $VERBOSE   murmur/murmur.log

cd ..
