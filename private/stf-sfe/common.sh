#
# common routines/variables for all shell scripts...
#

#
# set proper paths...
#
PATH=/bin:/usr/bin/:/usr/lib/cgi-bin/stf:/usr/lib/cgi-bin/stf/xml/bin
PATH=${PATH}:/usr/local/bin:/usr/local/j2sdk/bin
export PATH

awkpath=/usr/lib/cgi-bin/stf/xml/bin

xmlpath=/var/www/stf/xml

function qryCookie() {
    awk 'BEGIN { FS="="; RS=";[ ]*"; } { printf "%s\t%s\n", $1, $2; }' | \
     grep -e "^$1	" | sed -n "s/^$1	//1p"
}

#
# splitParams, split the paramString into lines, translate
#  as we go...
#
function splitParams() {
    tr '+' ' ' | awk 'BEGIN { FS="="; RS="&" } { printf "%s\t%s\n", $1, $2; }'
}

#
# qryParam param
#
#  get a parameter from a qry string (in stdin)
#
# format: v=val&v2=val2&v3=val3+val3
#
# + is replaced by ' '
#
# test is a special case parameter, strip off the "Run " in
# the beginning...
#
function qryParam() {
    local parm=`splitParams | \
	grep -e "^$1	" | \
	sed -n "s/^$1	//1p"`

    if [ $1 == "test" ]; then
	echo $parm | sed -n 's/^Run //1p'
    else
	echo $parm
    fi
}

#
# print the current time
#
function prtTime() { 
    awk 'BEGIN { printf "%s", systime() }' 
}

#
# set the mysql command...
#
mysqlargs="mysql -h glacier.lbl.gov -pt35Tpas5 -u tester domprodtest -s"
mysqlcmd="${mysqlargs} -e"
mysqlbatch="${mysqlargs} -B"

#
# java command
#
javacmd="java -classpath /usr/lib/cgi-bin/stf/xml/bin"

#
# view results from a result id...
#
# FIXME: this should be _much_ better, at least get rid of xmlv step...
#
function viewResults() {
    local tabth=${awkpath}/tabtohtml.awk
    local restt=${awkpath}/restotab.awk
    local qry="select text from STFResultXML where stf_result_id=$1"
    ${mysqlcmd} "${qry}" | xmln | awk -f ${restt} | \
	awk -v xml=$1 -f ${tabth}
}

function getSession() {
    echo $HTTP_COOKIE | qryCookie stfsession
}

function closeSession() {
    local session=`getSession`
    if [[ -d /tmp/stf/${session} ]]; then
	rm -rf /tmp/stf/${session}
    fi
}

function newSession() {
    local session=`mk-name`

    #
    # are we initialized...
    #
    if [[ ! -d /tmp/stf ]]; then
	mkdir /tmp/stf
    fi

    #
    # clear old session
    #
    if [[ ! -d /tmp/stf/${session} ]]; then
	rm -rf /tmp/stf/${session}
    fi

    #
    # make the session directory
    #
    mkdir /tmp/stf/${session}

    echo ${session}
}

#
# print the currently selected dhtab info
#
function currentPick() {
    local session=`echo $HTTP_COOKIE | qryCookie stfsession`
    local pick=`getPick`

    if [[ ${pick} == 0 ]]; then
	head -1 ${xmlpath}/servers
    else
	sed -n `printf "%dp" ${pick}` /tmp/stf/${session}/dhtab
    fi
}

#
# print the currently selected server...
#
function currentServer() {
    currentPick | awk '{print $1; }'
}

#
# print the currently selected port...
#
function currentPort() {
    currentPick | awk '{print $2; }'
}

#
# select a server, given line number of dhtab file
#
function setPick() {
    local session=`echo $HTTP_COOKIE | qryCookie stfsession`
    echo $1 > /tmp/stf/${session}/pick
}

#
# get the current pick or 0
#
function getPick() {
    local session=`echo $HTTP_COOKIE | qryCookie stfsession`
    local pick=`cat /tmp/stf/${session}/pick`
    if [[ ${#pick} == 0 ]]; then
	echo 0
    else
	echo ${pick}
    fi
}

#
# add results xml to database...
#
function addResults() {
    local restt=${awkpath}/restotab.awk
    
    # tab the xml results...
    cat $1 | xmlv | awk -f ${restt} > /tmp/results-tab.$$

    # get the test name...
    local testname=`head -1 /tmp/results-tab.$$ | awk '{ print $1; }'`
    local domid=`awk '$2 ~ /boardID/ { print $3; }' /tmp/results-tab.$$`
    local prodid=`${mysqlcmd} \
	"select prod_id from Product \
	 where prodtype_id=6 and lab_id=2 and hardware_serial='${domid}';"`

    # add hw id if necessary...
    if [[ ${#prodid} == 0 ]]; then
	local q="insert into Product (prod_id, prodtype_id, lab_id, \
	  hardware_serial) \
	  values(0, 6, 2, '${domid}'); select LAST_INSERT_ID();"
	local prodid=`${mysqlcmd} "${q}"`
    fi

    # get testtypeid, testid...
    local testid=`${mysqlcmd} \
	"select t.stf_test_id from \
	 STFTestType tt, STFTest t \
         where tt.name='${testname}' and \
	 t.stf_testtype_id=tt.stf_testtype_id and \
	 t.version='1.0'"`

    # get date
    local dt=`date '+%Y-%m-%d %H:%M:%S'`
    
    # get passed
    local passed=`awk '$2 ~ /passed/ { print ($3=="true") ? 1 : 0; }' \
	/tmp/results-tab.$$`

    # get runnable
    local runnable=`awk '$2 ~ /testRunnable/ { print ($3=="true") ? 1 : 0; }' \
	/tmp/results-tab.$$`

    # get result id...
    local q="insert into STFResult \
		(stf_result_id, prod_id, tech_id, stf_test_id, date_tested, \
		 passed, runnable) values(0, ${prodid}, 2, ${testid}, \
		 '${dt}', ${passed}, ${runnable}); select LAST_INSERT_ID();"
    local resid=`${mysqlcmd} "${q}"`

    # insert resultxml...
    echo "insert into STFResultXML (stf_result_id, text)" > /tmp/q.$$
    echo "  values(${resid}, " >> /tmp/q.$$
    sed "s/'/\\\'/g" $1 | sed "1s/^/'/1" >> /tmp/q.$$
    echo "');" >> /tmp/q.$$
    cat /tmp/q.$$ | tr '\n' ' ' | tr -d '\r' | ${mysqlbatch}
    rm -f /tmp/q.$$
    rm -f /tmp/results-tab.$$

    # return result id...
    echo ${resid}
}










