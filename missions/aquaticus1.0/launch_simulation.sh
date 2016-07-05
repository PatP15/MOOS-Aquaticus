#!/bin/bash

cd ./m200
./launch_m200.sh -e -b -s -t > /tmp/m200_blue &
./launch_m200.sh -f -r -s > /tmp/m200_red  &

cd ..

cd ./mokai
./launch_mokai.sh -b -ss > /tmp/mokai_blue &
./launch_mokai.sh -r -ss > /tmp/mokai_red  &

cd ..

cd ./shoreside
./launch_shoreside.sh
