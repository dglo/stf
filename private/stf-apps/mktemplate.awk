BEGIN {
    print "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>";
    print "<stf:setup xmlns:stf=\"http://glacier.lbl.gov/icecube/daq/stf\"";
    print "  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"";
    print "  xsi:schemaLocation=\"http://glacier.lbl.gov/icecube/daq/stf file:/Volumes/IceCube/WorkSpaces/jas-1/xml/daq/stf.xsd\">";
    print "";
}

$3 ~ /input/ {
    if ( $1 != testname ) {
	testname = $1;
	print " <" testname ">";
	print "  <parameters>";
    }

    print "   <" $2 ">" $5 "</" $2 ">";
}


END {
    print "  </parameters>";
    print " </" testname ">";
    print "</stf:setup>";
}













