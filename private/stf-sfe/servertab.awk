BEGIN {
   print "<html>";
   print "<head>";
   print "<title>Pick STF Server</title>";
   print "</head>";
   print "<h3>STF Servers</h3>";
   print "<table border=\"1\">";
   print "<tbody>";
   print "<tr><th>Current</th><th>Host</th><th>Port</th>";
   print "<th>Status</th><th>Action</th></tr>";

   nready = 0;
}

{
   if (NF==2) {
      print "<tr>";
      print "<td>";
      if ( $1 == stfserver  && $2 == stfport ) printf "*";
      print "</td>";
      
      print "<td>";
      print $1;
      print "</td>";
      
      print "<td>";
      print $2;
      print "</td>";
      
      #
      # get the current status...
      #
      nm = xmlpath "/" $1 ":" $2;
      if ( ! system("[[ -e " nm " ]]") ) status = "Test Running";
      else {
         #
         # run status program
         #
	 if (system("chkpt 4 stftcp " $1 " " ($2+3000) " -ready") ) {
	    status = "Not ready";
	 }
	 else {
	    status = "Ready";
	    nready++;
	 }
      }
      
      print "<td>";
      print status;
      print "</td>";
      
      print "<td>";
      if ( status == "Ready" ) {
	 print "<a href=\"http://deimos.lbl.gov/cgi-bin/stf/pick-server?stfserver=" $1 "&stfport=" $2 "\">Pick</a>"
      }
      print "</td>";
      
      print "</tr>"
   }
}

END {
   print "</tbody>";
   print "</table>";

   if (nready>0) {
      print "<p>";
      print "<a href=\"http://deimos.lbl.gov/stf/run-tests.html\">";
      print "Configure and Run Tests</a>";
   }
   else {
      print "<p>No stfservers seem to be ready...";
   }
   
   print "</body>";
   print "</html>";
}
