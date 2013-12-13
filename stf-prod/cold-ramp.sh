#!/bin/bash

#
# FIXME: parameters for:
#   first temperature soak time
#   first temperature test directory
#   all other tests soak time
#   all other tests directory
#
# comm test first...
#

# source common tools
source /usr/local/share/domhub-tools/common.sh

# set the fridge here: F4 (sanmina) or W942 (lbl)
#FRIDGE=W942
FRIDGE=F4

# cold-ramp.sh, run cold ramp stf-tests
startmjb=60
#startmjb=300

#
# minutes to run mjb...
#
loopmjb=10

#
# testing mode?
#
#testing=1
testing=0

if (( testing == 1 )); then
    # set upper/lower test temperature
    max_burnin_temp=32
    min_burnin_temp=20

    # set soak time for burn-in time
    burnin_minutes=1

    # test temperature and time (in minutes)...
    test_temps="30 29 28 27 26 25 24 23"
    test_minutes=1

    # target temperature...
    target_temp="26"
else
    # set upper/lower test temperature
    max_burnin_temp=65
    min_burnin_temp="-50"

    # set soak time for burn-in time
    burnin_minutes=1140

    # test temperature and time (in minutes)...
    test_temps="-10 -15 -20 -25 -30 -35 -40 -45"
    test_minutes=10

    # target temperature...
    target_temp="-30"
fi

#
# final temperature...
#
end_temp=25

#
# run all tests...
#
# $1 is the target temperature...
# $2 is the number of minutes to run the mjb tests
# $3 is the directory where the stf_results.txt file should go
#
function runAllTests() {
    local temp=$1

    # kill results file, otherwise results are appended
    rm -f stf_results.txt

    pwr on

    #
    # FIXME: this is necessary as bad doms may get stuck in iceboot...
    #
    se.pl all r r
#   iceboot -q all
    domlist=`domstate all | awk '$2 ~ /^iceboot$/ { print $1; }' | \
	tr '\n' ' ' | sed 's/ $//1'`
    echo "doms=${domlist}" 

    echo "running tcal-stf...(temp = $temp)"
    ( ./tcal-stf.sh all ${temp} > $3/tcal_results.txt )

    echo "running stf-client...(temp = $temp)"
    if [[ ${temp} == ${max_burnin_temp} || ${temp} == ${min_burnin_temp} || ${temp} == ${target_temp} ]]; then
      ( ./stf-client --tests-dir=std-burnin-tests --dom-test=false --niters=1 \
	--temperature=${temp} --batch --screenshot=${run_directory}/screenshot${temp}.png )
      lpr ${run_directory}/screenshot${temp}.png
    else
      ( ./stf-client --tests-dir=std-tests-supershort --dom-test=false --niters=1 \
	--temperature=${temp} --batch )
    fi

    mv stf_results.txt $3
}

#
#  run all the tests, but check to make sure that:
#
# 1) the time limit is reached (if tests end prematurely...)
# 2) the time limit is not exceeded (if tests get stuck...)
#
# $1 is the time (in minutes) to run the tests...
# $2 is the number of times to run mjb...
# $3 is the directory where the stf results should go...
#
function runAllTestsCheck() {
    local temp=$1
    local sleepsecsmax=`echo $2 500 '* p' | dc`
    local sleepsecsmin=`echo $2 120 '* p' | dc`
    # mjb minutes=total minutes - stf minutes - tcal-stf minutes - overhead
    local mjbminutes=`echo "$2 30 - 10 - 10 - 10 - p" | dc`

    date '+%s' > $3/start.time
    runAllTests $temp $mjbminutes $3 &
    local testpid=$!

    if [[ ${temp} == ${max_burnin_temp} || ${temp} == ${min_burnin_temp} || ${temp} == ${target_temp} ]]; then
      sleep ${sleepsecsmax} &
    else
      sleep ${sleepsecsmin} &
    fi
    local wdpid=$!

    printf "current test pid: ${testpid}      \r"

    # make sure we wait at least timeout period...
    wait ${wdpid}

    # kill testpid if it is still running...
    massacre ${testpid}
    date '+%s' > $3/end.time
}

function getTemperatureW942() {
   local temp=`echo "? C1" | ./w942`

   # round it...
   local tdc=`echo ${temp}  | tr '-' '_'`
   local ttt=`echo "${tdc} 10 * p" | dc | sed 's/\..*$//1'`
   local val=0
   local tttdc=`echo ${ttt} | tr '-' '_'`
   echo "${tttdc} ${val} + p" | dc | sed 's/[0-9]$//1'
}

function getTemperatureF4() {
   ./f4 r 100 1 | sed 's/[0-9]$//1'
}

function getTemperature() {
   # returned in degrees C
   local val=`getTemperature${FRIDGE} $*`

   # check for funny values
   if [[ "${val}" == "" ]]; then val=0; fi
   if [[ "${val}" == "-" ]]; then val="-0"; fi
   echo ${val}
}

function setFridgeTemperatureW942() {
    if ! echo "= SP1 ${temp}" | ./w942; then
        echo "`basename $0`: unable to set temperature"
        return 1
    fi
}

function setFridgeTemperatureF4() {
    if ! ./f4 w 300 $10; then
	echo "`basename $0`: unable to set temperature"
        return 1
    fi
}

function setFridgeTemperature() {
    setFridgeTemperature${FRIDGE} $*
}

#
# set temperature -- wait for it to be reached...
#
function setTemperature() {
    local temp=$1

    echo "setting temperature to: $temp"
    setFridgeTemperature ${temp}
    
    # wait for set temperature...
    echo "waiting for temperature to reach $temp"
    while /bin/true; do
        local current=`getTemperature`

        printf "current temperature: ${current}      \r"
        local let mintemp=$(( ${temp} - 1 ))
	local let maxtemp=$(( ${temp} + 1 ))
        if (( ${current} <= ${maxtemp} && ${current} >= ${mintemp} )); then
            break
        fi
        sleep 1
    done
    printf "\n"

    # wait 30 seconds... 
    awk 'BEGIN { print "waiting 30 seconds, ready at " strftime("%r", systime() + 30) " ..."; }'
    sleep 30
}

# create new run directory structure...
if [[ ! -f run.num ]]; then
    echo 1 > run.num
fi
run_number=`cat run.num`
echo "${run_number} 1 + p" | dc > run.num

run_directory=`printf 'run-%03d' ${run_number}`

if [[ -x ${run_directory} ]]; then
    echo "`basename $0`: ${run_directory} already exists, fix run.num..."
    exit 1
fi

if ! mkdir ${run_directory}; then
    echo "`basename $0`: unable to make directory: ${run_directory}"
    exit 1
fi

# mark the start of the run...
date '+%s' > ${run_directory}/start.time

for temp in ${max_burnin_temp} ${test_temps} ${min_burnin_temp}; do
    # set temperature...
    if ! setTemperature ${temp}; then
	echo "`basename $0`: unable to set temperature"
	exit 1
    fi

    #
    # if the chamber is at one of the soak temperatures, run a full STF 
    # when the chamber gets to the target temperature
    #
    if [[ ${temp} == ${max_burnin_temp} || ${temp} == ${min_burnin_temp} ]]; then
	results_dir="${run_directory}/soak.${temp}"
	if ! mkdir ${results_dir}; then
	    echo "`basename $0`: unable to mkdir ${results_dir}"
	    exit 1
	fi
	runAllTestsCheck ${temp} ${loopmjb} ${results_dir}
    fi

    #
    # Soak the MBs with the power on until the target time is reached
    # delay soak time...
    #
    if [[ ${temp} == ${max_burnin_temp} || ${temp} == ${min_burnin_temp} ]]; then
	let seconds=$(( ${burnin_minutes} * 60 ))
    else
	let seconds=$(( ${test_minutes} * 60 ))
    fi

    echo "soaking for $seconds seconds..."
    sleep $seconds

    # turn off doms
    echo "turning off doms..."
    pwr off

    # make sure doms are off and closed...
    doms=`domstate all`
    if (( ${#doms} != 0 )); then
      echo "`basename $0`: yikes, some doms did not die..."
    fi

    results_dir="${run_directory}/normal.${temp}"
    if ! mkdir ${results_dir}; then
	echo "`basename $0`: unable to mkdir ${results_dir}"
	exit 1
    fi

    runAllTestsCheck ${temp} ${loopmjb} ${results_dir}

    # turn off doms
    echo "turning off doms..."
    pwr off

    # make sure doms are off and closed...
    doms=`domstate all`
    if (( ${#doms} != 0 )); then
	echo "`basename $0`: yikes, some doms did not die..."
    fi
done

# mark the end of the run...
date '+%s' > ${run_directory}/end.time

# back up to room temperature...
setTemperature ${end_temp}

# analyze the results files once analyze-cold-ramp is done...
./analyze-cold-ramp.sh ${run_directory} > ${run_directory}/report.txt

# print results
enscript --no-header --landscape --font=Courier7 ${run_directory}/report.txt 

cat ${run_directory}/report.txt
