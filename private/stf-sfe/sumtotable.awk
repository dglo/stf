#
# convert lines of summary info to an html table...
#
BEGIN {
  print "<table border=\"1\">\n  <tbody>";
  print "    <tr><th>Date</th><th>Test</th><th>Status</th></tr>";
}

{
  print "    <tr>",
    "<th>" $1 "</th>",
    "<th>" $2 "</th>",
    "<th><a href=\"/cgi-bin/stf/view-results?file=" $5 "\">" (($4 == "1" ) ? "OK" : "FAILED" ) "</a></th>",
    "</tr>";
}

END {
  print "  </tbody>\n</table>";
}
