#
# restotab.awk a simple script to convert the
# xml stf results file to a tab delimited file
# of parameter information...
#
# the xml file is preprocessed with xmlv, a
# program to convert xml to line oriented data...
#
BEGIN { depth = 0; }

/^\(/ { 
    depth++; 

    if ( depth == 2 ) {
	toplevel = substr($0, 2);
    }
    else if (depth == 4) {
	param = substr($0, 2);
    }
}
/^\)/ { depth--; }

/^-/ {
    if ( depth == 4 ) {
	printf "%s\t%s\t%s\n", toplevel, param, substr($0, 2);
    }
}
