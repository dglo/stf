BEGIN {
   print "<html>";
   print "<head>";
   print "<title>Pick DOMHub STF Server</title>";
   print "</head>";
   print "<h3>DOMHub STF Servers</h3>";
   print "<table border=\"1\">";
   print "<tbody>";
   print "<tr><th>Current</th><th>Host</th><th>Port</th>";
   print "<th>Card</th><th>Pair</th><th>DOM</th><th>Action</th></tr>";

   nready = 0;
}

{
   if (NF==5) {
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

      print "<td>";
      print $3;
      print "</td>";

      print "<td>";
      print $4;
      print "</td>";

      print "<td>";
      print $5;
      print "</td>";
      
      print "<td>";
      print "<a href=\"http://deimos.lbl.gov/cgi-bin/stf/pick-dhserver?stfserver=" $1 "&stfport=" $2 "&tabfile=" tabfile "\">Pick</a>"
      print "</td>";
      
      print "</tr>"


      nready++;
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
      print "<p>No DOMHub STF servers seem to be ready...";
   }
   
   print "</body>";
   print "</html>";
}



