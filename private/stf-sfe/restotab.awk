#
# restotab.awk a simple script to convert the
# xml stf results file to a tab delimited file
# of parameter information...
#
# the xml file is preprocessed with xmlv, a
# program to convert xml to line oriented data...
#
# new format should be:
#   test parameter value value_index type input/output
#
BEGIN {
    depth = 0;
}

/^\(/ { 
    depth++; 

    if ( depth == 2 ) {
	toplevel = substr($0, 2);
	
	#
	# we have the test name, now go out and
	# get the type info for the parameters...
	#
	# format should be:
	#   test parameter
	#
    }
    else if (depth == 4) {
	param = substr($0, 2);
	idx = 1;
	cmd = "awk ' $1 ~ /" \
	    toplevel "/ { if ( $2 == \"" param  "\") print $3, $4; }' " \
            "/var/www/stf/xml/all.tab";
	var="";
	cmd | getline var;
	close(cmd);

	if ( var == "" ) {
	    if ( param == "passed" ) { 
		directionType = "output"; 
		paramType = "boolean";
	    }
	    else if (param == "testRunnable" ) { 
		directionType = "output"; 
		paramType = "boolean";
	    }
	    else if ( param == "boardID" ) { 
		directionType = "output"; 
		paramType = "string";
	    }
	    else if ( param == "errorMessage" ) {
		directionType = "output";
		paramType = "string";
	    }
	    else {
		directionType = "?";
		paramType = "?";
	    }
	}
	else {
	    split(var, avar);
	    paramType=avar[2];
	    directionType=avar[1];
	}
    }
}

/^\)/ { 
    if ( depth == 4 ) {
       for (i=1; i<idx; i++) {
	   printf "%s\t%s\t%s\t%d\t%s\t%s\n",
	       toplevel, param, value[i], i, paramType, directionType;
       }
    }
    depth--; 
}

/^-/ {
    if ( depth == 4 ) {
       if ( paramType == "unsignedIntArray" ) {
           # parse into elements...
	   n = split(substr($0, 2), avar);
           # index each element...
	   for (i = 1; i <= n; i++) { value[idx] = avar[i]; idx++; }
       }
       else { value[idx] = substr($0, 2); idx++; }
    }
}

















