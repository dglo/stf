#!/bin/bash

#
# analyze-cold-ramp.sh, summarize the results of a cold-ramp run
#
if (( $# != 1 )); then
    echo "usage: `basename $0` results-directory"
    exit 1
fi

#
# lookup tag serial from the hw serial (dom mainboard id)
#
# $1 is dom-mb id to lookup...
#
function lookupSerial() {
    local server="bergy.lbl.gov"
    local user="tester"
    local passwd="t35Tpas5"

    if [[ -f /usr/local/etc/domprodtest.properties ]]; then
	server=`grep 'database.domprodtest.url: ' /usr/local/etc/domprodtest.properties | \
	    awk '{ print $2; }' | tr '/' ':' | awk -vFS=':' '{ print $5; }'`
	user=`grep 'database.domprodtest.user: ' /usr/local/etc/domprodtest.properties | \
	    awk '{ print $2; }'`
	passwd=`grep 'database.domprodtest.password: ' /usr/local/etc/domprodtest.properties | \
	    awk '{ print $2; }'`
    fi

    stmt=`printf "select tag_serial from Product where hardware_serial='%s'" $1`
    mysql -s -h ${server} -u ${user} -p${passwd} -e "${stmt}" domprodtest
}

#
# what is the temperature of a run?
#
# $1 run
# $2 all results file...
#
function tempOfRun() {
    grep "^${run} " $2 | awk '{ print $2; }' | uniq
}

results_dir=$1
dir_list=`lessecho ${results_dir}/normal.* ${results_dir}/soak.*`
all=${results_dir}/all_results.txt

#
# unify results?
#
if [[ ! -f ${all} ]]; then
    echo "touch all"
    touch ${all}

    for dir in ${dir_list}; do
        # get temperature
        temp=`echo ${dir} | sed 's/^.*\.//1'`
	st=`cat ${dir}/start.time`

	echo "dir=${dir}, st=${st}, temp=${temp}"
    
	if [[ -f ${dir}/stf_results.txt ]]; then
	    echo "found $d stf results..."
	    awk '{ print $5, $6, $7; }' ${dir}/stf_results.txt | \
		awk -vt=${temp} -vs=${st} '{ print s, t, $1, $2, $3; }' >> \
		${all}
	fi

	if [[ -f ${dir}/tcal_results.txt ]]; then
	    echo "found $d tcal results..."
	    awk '$1 ~ /^Saved$/ { print $4; }' ${dir}/tcal_results.txt | \
		sed -e 's/^.*@//1' | tr '()' '  ' | sed 's/ :$//1' | \
		awk '{ print $1, "tcal", $2; }' | sed 's/pass/P/1' | \
		awk -vt=${temp} -vs=${st} '{ print s, t, $1, $2, $3; }' >> \
		${all}
	fi
    done
fi

#
# now, everything we need is in the all_results.txt file...
#

#
# get runs sorted by time...
#
runs=`awk '{ print $1; }' ${all} | sort | uniq`
doms=`awk '{ print $3; }' ${all} | sort | uniq`

#
# print header...
#
date

#
# print run time hh:mm:ss
#
# $1 is directory where start and end time sit...
#
function runTime() {
    local st=`cat $1/start.time`
    local et=`cat $1/end.time`
    local ts=`echo $et $st '- p' | dc`
    local hh=`echo $ts '60 60 * / p' | dc`
    ts=`echo $ts $hh '60 * 60 * - p' | dc`
    local mm=`echo $ts '60 / p' | dc`
    local ss=`echo $ts $mm '60 * - p' | dc`
    printf '%02dh %02dm %02ds\n' $hh $mm $ss
}

echo "Period length: `runTime ${results_dir}`  Boards tested: `echo $doms | tr ' ' '\n' | wc -l`"
printf '\n'

printf '%11s %12s TEST FAIL ' "BOARD TAG" "BOARD SERIAL"
for run in ${runs}; do printf 'TEST FAIL '; done
printf '\n'
printf '%23s   TOT  TOT ' " " 
for run in ${runs}; do printf '%+3dC %+3dC ' `tempOfRun ${run} ${all}` `tempOfRun ${run} ${all}`; done
printf '\n'

#
# we're interested in per-dom statistics...
#
for dom in ${doms}; do
    tf=`mktemp '/tmp/analyze-cold-ramp-XXXXXX'`
    awk -vdom=${dom} '{ if ( $3 == dom ) { print $0; } }' ${all} > ${tf}

    printf '%4s %6s ' `lookupSerial ${dom}`
    printf "${dom} "
    printf '%4d ' `cat ${tf} | wc -l`
    printf '%4d ' `grep -v 'P$' ${tf} | wc -l`
    for run in ${runs}; do
	printf '%4d ' `grep "^${run} " ${tf} | wc -l`
	printf '%4d ' `grep "^${run} " ${tf} | grep -v 'P$' | wc -l`
    done
    printf '\n'
    rm -f ${tf}
done | sort -k 2
