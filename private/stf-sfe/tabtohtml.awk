#
# tabtohtml.awk, convert tabbed data output from
# restotab.awk into html...
#
BEGIN {
   FS="\t";
   print "<html>";
   print "<head>";

   first = 1;
}

{
    if (first) {
	print "<title>Results: " $1 "</title>";
	print "</head>";
	print "<body>";
	
	print "<h3>" $1 "</h3>";
	print "<table border=\"1\">";
	print "<tbody>";
	
	print "<tr>";
	print "<th>Parameter</th><th>Type</th><th>Value</th>";
	print "</tr>";

	first = false;
    }

    #
    # check for array type...
    #
    if ( $4 == 1 ) {
	if ( $5 == "unsignedIntArray") {
            #
            # check for img file, if it doesn't exist, we
	    # need to create it...
	    #
	    value = "<a href=\"/cgi-bin/stf/plot-array" \
		"?file=" xml "&parameter=" $2 "\">plot</a>";
	}
	else {
	    value = $3;
	}

	print "<tr>";
	print "<td>" $2 "</td><td>" $6 "</td><td>" value "</td>";
	print  "</tr>";
    }
}

END {
   print "</tbody>";
   print "</table>";
   print "</body>";
   print "</html>";
}





