#
# stfdesc.awk, get the description from a
# stf definition xml file that has been
# preprocessed with xmlv...
#
BEGIN { inDesc = 0; }

/^\(description$/ { desc = ""; inDesc++; }

/^-/ {
    if ( inDesc>0 ) {
	desc = (desc substr($0, 2));
    }
}

/^\)description$/ {
    if ( desc != "" ) {
	gsub( /\\n/, " ", desc);
	printf "%s\n", desc;
    }
    inDesc--;
}
