#
# take the xml results file which has been converted
# to tab format from the restotab.awk script and
# extract the summary info from it...
/^[^ ]*\tpassed\t/ {
   test=$1;
   passed=$3;
}

/^[^ ]*\tboardID\t/ {
   boardID = $3;
}

END { print test " " boardID " " passed; }

   
