BEGIN {
   printf("<html>\n<head>\n  <title>Run STF tests</title>\n</head>\n");
   printf("<body>\n");
   printf("<h2>Configure and run an STF test</h2>\n");
   printf("<ol>\n");
}

{  
   if ( tests[ $1 ] == 0 ) {
      printf("  <li><a href=\"/cgi-bin/stf/conf-test?testname=%s\">%s</a></li>\n", 
	     $1, $1);
      tests [ $1 ] = 1;
   }
}

END {
   printf("</ol>\n");
   printf("</body>\n</html>\n");
}

