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
    
    print "<tr>";
    print "<td>" $2 "</td><td>" type "</td><td>" $3 "</td";
    print  "</tr";
}

END {
   print "</tbody>";
   print "</table>";
   print "</body>";
   print "</html>";
}

