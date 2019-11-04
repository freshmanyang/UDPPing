#!/bin/sh
is_wireless() {
if iwconfig $1 > /dev/null 2>&1; then
 echo 0
 return 0;
fi
echo 1
return 1;
}
iw="wlan0"
ret=$(is_wireless $iw)
#echo $ret
if [ $ret -eq 1 ]; then
 #echo "$iw is not exited"
 echo -1
 echo -1
else
 echo `iwconfig $iw |grep  -oE "(\<Quality=)[0-9]+" |grep -oE "[0-9]+"`
 echo `iwconfig $iw |grep  -oE "(\<level=)-[0-9]+" |grep -oE "\-[0-9]+"`
fi