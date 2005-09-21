BEGIN {
    print "/* STFParameterLookup.java, autogenerated by genspl.awk */";
    print "";
    print "package icecube.daq.stf;";
    print "";
    print "import icecube.daq.db.stftest.STFParameter;";
    print "import icecube.daq.db.domprodtest.DOMProdTestException;";
    print "import java.util.HashMap;";
    print "import java.io.IOException;";
    print "";
    print "public class STFParameterLookup {";
    print "  private static HashMap lookup;";
    print "  public static final STFParameter STFParameterLookup(String test,";
    print "                                                      String name)";
    print "    throws DOMProdTestException, IOException {";
    print "    if (lookup==null) lookup = mkLookup(); /* FIXME: lock here? above? */";
    print "    HashMap pm = (HashMap) lookup.get(test);";
    print "    if (pm==null) return null;";
    print "    return (STFParameter) pm.get(name);";
    print "  }";
    print "";
    print "  private static final HashMap mkLookup() ";
    print "    throws DOMProdTestException, IOException {";
    print "    HashMap ret = new HashMap(), hm;";
    print "    STFParameter p;";
}

END {
    print "";
    print "    return ret;";
    print "  } /* mkLookup */";
    print "} /* STFParameterLookup */";
}

#
# parameter...
#
{
    if ( NF > 3 ) {
	#
	# if new test, add to hashmap
	#
	if ( $1 != testname ) {
	    print "";
	    print "    ret.put(hm=new HashMap(), \"" $1 "\");";
	}
	testname = $1;
	
	#
	# now create and add parameter...
	#
	printf "    p = new STFParameter("; 
	if ( $3 == "input" ) print " true );";
	else print " false );";

	if ( NF > 4 && $5 != "" ) {
	    print "    p.setDefaultValue(\"" $5 "\");";
	}

	if ( NF > 5 && $6 != "" ) {
	    print "    p.setMinValue(\"" $6 "\");";
	}

	if ( NF > 6 && $7 != "" ) {
	    print "    p.setMaxValue(\"" $7 "\");";
	}

	print "    hm.put(p, \"" $2 "\");";
    }
}







