BEGIN {
   printf("<html>\n<head>\n  <title>Run STF tests</title>\n</head>\n");
   printf("<body>\n");
   printf("<h2>Configure and run an STF test</h2>\n");
   printf("<ul>\n");
}

{  
   if ( tests[ $1 ] == 0 ) {
      printf("  <li><a href=\"%s.html\">%s</a></li>\n", 
	     $1, $1);
      tests [ $1 ] = 1;
   }
}

END {
   printf("</ul>\n");
   printf("</body>\n</html>\n");
}

 
