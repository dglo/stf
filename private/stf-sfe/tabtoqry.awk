#
# convert a test's tab file into a html qry...
#
BEGIN {
   FS = "\t";
   printf("<html>\n<head>\n");
   printf("  <title>Configure and Run %s</title>\n</head>\n",
	   test);
   printf("<body>\n");
   printf("<h2>Configure and Run %s Test</h2>\n", test);
   printf("<form action="  "\"/cgi-bin/stf/run-test\"" " method=\"get\">\n");
   printf("<table border=\"1\">\n  <tbody>\n");
   printf( "    " "<tr><th>Name</th>" "<th>Type</th>" "<th>Value</th>" "<th>Min</th>" "<th>Max</th>" "</tr>\n");
}

{
   if ( $1 == test && $4 == "input" ) {
      printf("    <tr>");
      printf("<td>%s</td>", $2);
      printf("<td>%s</td>", $5);
      if ( $5 == "boolean" ) {
	 printf("<td><select name=\"%s\"><option value=\"true\">True</option><option value=\"false\">False</option>", $2);
	 printf("<td></td><td></td>");
      }
      else {
	 printf("<td><input name=\"%s\" value=\"%s\"></input></td>", $2, $6);
	 printf("<td>" $7 "</td>");
	 printf("<td>" $8 "</td>");
      }
      printf("</tr>\n");
   }
}

END {
   printf("  </tbody>\n</table>\n");
   printf("<button name=\"test\" value=\"Run %s\" type=\"submit\">" "Run %s" "</button>\n",
	  test, test);
   printf("</body>\n</html>\n");
}
