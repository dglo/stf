{
   FS="\t";

   if ( $1 != "test" && $1 != "" ) {
       cmd = "validate-param " test " " $1 " " $2;
       cmd | getline msg;

       if ( msg != "" ) print msg;
       msg="";
   }
}

