#!/bin/bash
read POST_STRING
echo "$(date) $POST_STRING" > test.txt

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
done
if [ -f ../data/key.sha ]
then
   FILE="../data/key.sha"
else
   FILE="../data/testkey.sha" 
fi
echo $(date), $TICKETS, $PASSWORD, $(echo -n $PASSWORD | shasum -a 256) > test1.txt
if [ "$(echo -n $PASSWORD | shasum -a 256)" = "$(cat $FILE)" ]  &&  [ -n "$TICKETS" ]
then
echo $(date), "SUCC", $TICKETS, $PASSWORD > test2.txt
    succ="1"
    echo "function tickets() { return $TICKETS }" > ../data/newdata.js
    echo "function timestamp() { return $(date +%s) }" >> ../data/newdata.js
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

