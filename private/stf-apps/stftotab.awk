#
#
# stftotab.awk a simple script to convert the
# xml definition file to a tab delimited file
# of parameter information...
#
# the xml file is preprocessed with xmlv, a
# program to convert xml to line oriented data...
#
BEGIN {
    depth = 0;
    inName = 0;
    inDesc = 0;
}

/^\(/ { depth++; }
/^\)/ { depth--; }

/^\(name$/ { inName++; }
/^\)name$/ { inName--; }

/^\(inputParameter$/  { 
    default = "";
    minValue = "";
    maxValue = "";
}

/^\(description$/ { inDesc++; }
/^\)description$/ { inDesc--; }

/^\)inputParameter$/  { 
    printf "%s\t%s\t%s\tinput\t%s\t%s\t%s\t%s\n", 
	toplevel, name, desc, type, 
	default, minValue, maxValue;
}

/^\(outputParameter$/ { }
/^\)outputParameter$/  { 
    printf "%s\t%s\t%s\toutput\t%s\n", 
	toplevel, name, desc, type;
}

/^\(unsignedInt$/ { type = "unsignedInt" }
/^\(unsignedLong$/ { type = "unsignedLong" }
/^\(string$/ { type = "string" }
/^\(boolean$/ { type = "boolean" }

/^Adefault/  { default = substr($0, 10); }
/^AminValue/ { minValue = substr($0, 10); }
/^AmaxValue/ { maxValue = substr($0, 10); }

/^-/ {
    if ( inName ) {
	if ( depth == 2 ) {
	    toplevel = substr($0, 2)
	}
	else if ( depth == 3 ) {
	    name = substr($0, 2)
	}
    }
    else if ( description && inDesc ) {
	desc = substr($0, 2);
    }
}
