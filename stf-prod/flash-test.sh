#!/bin/bash

if (( $# < 1 )); then
	echo "usage: `basename $0` all|CWD [CWD ...]"
	exit 1
fi

doms=`iceboot $* | awk '$0 ~ /^[0-7][0-3][aAbB] in iceboot$/ { print $1; }' | \
	tr '\n' ' ' | sed 's/ $//1'`

let ndoms=$(( `echo $doms | tr ' ' '\n' | grep '^[0-7][0-3][aAbB]$' | wc -l` ))

if (( $ndoms == 0 )); then
	echo "`basename $0`: error: no doms found in iceboot"
	exit 1
elif (( $ndoms == 1 )); then
	dom=${doms}
else 
	for dom in ${doms}; do $0 ${dom} & done
	wait
	exit 0
fi

#
# $1 <- dom, $2 <- value (ones|zeros)...
#
function run1() {
	local ret=0
	
	if [[ "$2" == "ones" ]]; then
		sed 's/%value%/$ffffffff/1' flash-test.exp | se $1 >& /dev/null
	else
		sed 's/%value%/0/1' flash-test.exp | se $1 >& /dev/null
	fi

	if ! printf 'send "feq1 . drop\r"\nexpect "^> "' | se $dom | \
		tr -d '\r' | grep '^1$' >& /dev/null; then
		ret=1
	fi

	if ! printf 'send "feq2 . drop\r"\nexpect "^> "' | se $dom | \
        	tr -d '\r' | grep '^1$' >& /dev/null; then
		ret=`echo "${ret} 2 + p" | dc`
	fi

	return $ret
}

#
# save current flash image...
#
se $dom < flash-test-funcs.exp >& /dev/null

printf 'send "fsave\r"\nexpect "^> "' | se $dom >& /dev/null

run1 $dom ones
r1=$?
run1 $dom zeros
r2=$?
run1 $dom ones
r3=$?

let rr=$(( $r1 | $r2 | $r3 ))
let c0=$(( $rr % 2 ))
let c1=$(( $rr / 2 ))

if (( $c0 != 0 )); then
       echo "${dom} flash chip 0 has errors"
fi

if (( $c1 != 0 )); then
       echo "${dom} flash chip 1 has errors"
fi

printf 'send "frestore\r"\nexpect "^install: all .* are you sure"' | \
	se $dom >& /dev/null
printf 'send "y"\nexpect "^> "' | se $dom >& /dev/null

exit `echo "$c0 $c1 + p" | dc`
