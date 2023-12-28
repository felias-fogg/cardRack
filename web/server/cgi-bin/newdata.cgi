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
done
if [ -f ../data/key.sha ]
then
   FILE="../data/key.sha"
else
   FILE="../data/test.sha" 
fi
if [ "$(echo -n $PASSWORD | shasum -a 256)" = "$(cat $FILE)" ]  &&  [ -n "$TICKETS" ]
then
    succ="1"
    echo "function tickets() { return $TICKETS }" > ../data/newdata.js
    echo "function timestamp() { return $(date +%s) }" >> ../data/newdata.js
else
    succ="0"
fi

echo "Content-type: text/html"
echo ""

echo '<html>'
echo '<head>'
echo '<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">'
echo '</head>'
if [ "$succ" == "1" ]
then
    echo 'OK'
else
    echo 'FAIL'
fi

echo '</html>'

exit 0
