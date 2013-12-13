#!/bin/bash
#
#to run the ATWDtester GUI...
#

if [[ ${#JAVA_HOME} == 0 ]]; then
    echo "stf-client: JAVA_HOME must be set"
    exit 1
fi

#
#
#
pwr off; echo "searching for DOMs"; pwr on
iceboot all | awk '$3 ~ /^iceboot$/ { print $1; }'
doms=`iceboot all | awk '$3 ~ /^iceboot$/ { print $1; }'`
#echo $doms
doms1=`echo $doms | tr ' ' '\n' | grep '^0'`
#echo $doms1
doms1=`echo $doms | tr ' ' '\n' | grep '^1'`
doms1=`echo $doms | tr ' ' '\n' | grep '^0'`
doms2=`echo $doms | tr ' ' '\n' | grep '^1'`
#echo $doms2
doms10=`echo $doms1 | tr ' ' ','`
doms20=`echo $doms2 | tr ' ' ','`

if (( ${#doms1} == 0 )); then
    echo "`basename $0`: no doms found on card 0..."
    exit 1
fi

if (( ${#doms2} == 0 )); then 
    echo "`basename $0`: no doms found card 1..." 
    exit 1 
fi                                                                                                                                                              
#
# run stf...
#
echo "available doms ${doms10} ${doms20}"
echo "starting GUI..."
jbin=${JAVA_HOME}/bin
cp=`/usr/bin/lessecho ./jars/*.jar | tr ' ' ':' | sed 's/:$//1'`
exec ${jbin}/java -classpath "${cp}" icecube.daq.stf.stfapp_atwd.STFAppATWDGUI ${doms10} ${doms20} $*

#
# BTW: a big thanks to Arthur Jones...
#
