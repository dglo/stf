#
# common routines/variables for all shell scripts...
#

#
# set proper paths...
#
PATH=/bin:/usr/bin/:/usr/lib/cgi-bin/stf:/usr/lib/cgi-bin/stf/xml/bin
PATH=${PATH}:/usr/local/bin
export PATH

xmlpath=/var/www/stf/xml

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




