BEGIN {
    FS = "\t";
}

{
   printf " $%s$ & %s & %s", $2, $3, $5;

   if (input=="y") {
       if ( NF >= 6) printf " & %s", $6;
       if ( NF >= 7 ) printf " [%d..", $7;
       if ( NF == 8 ) printf "%d]", $8;
   }
   printf " \\\\ \n";
}
