/^\(/ {
   depth++;
   if ($1 == "(" param && depth==4) {
      inParameter = 1;
   }
}

/^\)/ { 
   depth--;
   if ($1 == ")" param ) {
      inParameter = 0;
   }
}

{ 
   if (inParameter>0) {
      if (inParameter>1) { print substr($0, 2); }
      inParameter++; 
   }
}

