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
    # get input/output
    #
    if ( $2 == "passed" ) {
	type = "output";
    }
    else if ($2 == "testRunnable" ) {
	type = "output";
    }
    else if ( $2 == "boardID" ) {
	type = "output";
    }
    else if ( $2 == "errorMessage" ) {
	type = "output";
    }
    else {
	type = "";
	cmd =  "awk '/^" $1 "\t" $2 "\t/ { print $3; }' /var/www/stf/xml/all.tab";
	cmd | getline type;
    }

    #
    # check for array type...
    #
    isArray = "";
    cmd = "awk '/^" $1 "\t" $2 "\t/ { print $4; }' /var/www/stf/xml/all.tab";
    cmd | getline isArray;
    
    if (isArray == "unsignedIntArray") {
       value = "<a href=\"http://deimos.lbl.gov/cgi-bin/stf/plot-array" \
	  "?file=" xml "&parameter=" $2 "\">plot</a>";
    }
    else {
       value = $3;
    }

    #
    # FIXME: check to see if this is an array and
    # put link to plot here...
    #
    
    print "<tr>";
    print "<td>" $2 "</td><td>" type "</td><td>" value "</td>";
    print  "</tr>";
}

END {
   print "</tbody>";
   print "</table>";
   print "</body>";
   print "</html>";
}





