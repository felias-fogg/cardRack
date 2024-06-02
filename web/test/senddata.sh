#!/bin/bash
loc="$1"
if [ -z "$loc" ]
then
    echo "You need to specify a location as the first argument"
    exit
fi
tcks="$2" # argument is number of tickets available
if [ -z "$tcks" ]
then
    echo "You need to specify a number as the second argument"
    exit    
fi

if [ -f ../config/${loc}.conf ]
then
    server=$(head -n 1 ../config/${loc}.conf)
else
    echo "'$loc' is not a defined location!"
    exit
fi

key=$(tail -n 1 ../config/${loc}.conf)

curl -X POST https://${server}/cgi-bin/storedata.cgi -d "LOCATION=${loc}&KEY=${key}&TICKETS=${tcks}"
