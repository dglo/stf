#
# convert lines of summary info to an html table...
#
BEGIN {
  print "<table border=\"1\">\n  <tbody>";
  print "    <tr><th>Date</th><th>Test</th><th>Status</th></tr>";
}

{
  print "    <tr>",
    "<th>" strftime("%a %b %d %Y, %H:%M:%S", $1) "</th>",
    "<th>" $2 "</th>",
    "<th><a href=\"http://deimos.lbl.gov/cgi-bin/stf/view-results?file=" $5 "\">" (($4 == "true" ) ? "OK" : "FAILED" ) "</a></th>",
    "</tr>";
}

END {
  print "  </tbody>\n</table>";
}
