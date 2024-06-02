#!/bin/bash
read POST_STRING

IFS="&"
set -- $POST_STRING
array=($@)
for i in "${array[@]}"
do
    IFS="=" 
    set -- $i
    if [ "$1" == "KEY" ]
    then
	PASSWORD=$2
    fi
    if [ "$1" == "TICKETS" ]
    then
	TICKETS=$2
    fi
    if [ "$1" == 'LOCATION' ]
    then
	LOCATION=$2
    fi
done
FILE="../data/${LOCATION}.sha" 

if [ -f $FILE ] && [ "$(echo -n $PASSWORD | shasum -a 256)" = "$(cat $FILE)" ]  &&  [ -n "$TICKETS" ] && ! [ -z "${TICKETS##*[!0-9]*}" ]
then
    succ="1"
    echo $TICKETS > ../data/${LOCATION}.txt
    echo $(date +%s) >> ../data/${LOCATION}.txt
else
    succ="0"
fi

echo "Content-type: text/html"
echo ""
if [ "$succ" == "1" ]
then
    echo 'OK'
else
    echo 'FAIL'
fi

