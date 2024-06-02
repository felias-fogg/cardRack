#!/bin/bash
loc=$1
if [ -z "$loc" ]
then
    echo "You need to specify a location as the first argument"
    echo "This should be the base name of the config file"
    exit
fi
domain=$(head -n 1 ${loc}.conf)
password=$(tail -n 1 ${loc}.conf)
echo "Writing ${loc}.h for Arduino sketch ..."
echo "#define DOMAIN \"${domain}\"" > ../../IoTRack/${loc}.h
echo "#define PASSWORD \"${password}\"" >> ../../IoTRack/${loc}.h
echo "#define LOCATION \"${loc}\"" >> ../../IoTRack/${loc}.h
echo "Writing ${loc}.sha for server folder ..."
echo -n $password | shasum -a 256 > ../server/data/${loc}.sha
