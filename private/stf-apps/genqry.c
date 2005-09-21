/* genqry.c, generate the stfDirectory file automatically
 * from a list of xml files...
 */
#include <stdio.h>
#include <stdlib.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

static void usage(void) {
   fprintf(stderr, "usage: genqry xmlfiles...\n");
}

int main(int argc, char *argv[]) {
   int ai, i;
   FILE *fptr = NULL;
   const char *descs[1024];
   int ndescs = 0;
   
   for (ai=1; ai<argc; ai++) {
      if (argv[ai][0]!='-') break;
   }

   if (ai>=argc) { 
      usage();
      return 1;
   }

   if (argc-ai>sizeof(descs)/sizeof(descs[0])) {
      fprintf(stderr, "too many files to genqry!\n");
      return 1;
   }

   LIBXML_TEST_VERSION
   xmlKeepBlanksDefault(0);

   while (ai<argc) {
      xmlDocPtr doc;
      xmlNodePtr cur;
      const char *fn = argv[ai];
      const char *tn = NULL;

      if ((doc = xmlParseFile(fn))==NULL) {
	 fprintf(stderr, "genqry: can't parse file: '%s'\n", fn);
	 ai++;
	 continue;
      }

      if ((cur = xmlDocGetRootElement(doc))==NULL) {
	 fprintf(stderr, "genqry: empty document: '%s'\n", fn);
	 xmlFreeDoc(doc);
	 ai++;
	 continue;
      }

      if (xmlStrcmp(cur->name, (const xmlChar *) "test")) {
	 fprintf(stderr, "genqry: not a stf test document: '%s'\n", fn);
	 xmlFreeDoc(doc);
	 ai++;
	 continue;
      }
   
      for (cur=cur->xmlChildrenNode; cur!=NULL; cur=cur->next) {
	 if (xmlStrcmp(cur->name, (const xmlChar *) "name")==0) {
	    char fname[128];

	    tn = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
	    
	    /* create a new file, put header...
	     */
	    sprintf(fname, "%s.html", tn);
	    if ((fptr=fopen(fname, "w"))==NULL) {
	       fprintf(stderr, "can't open file: '%s'\n", fname);
	       return 1;
	    }

	    /* write the header...
	     */
	    fprintf(fptr, "<html>\n<head>\n");
	    fprintf(fptr, "  <title>Configure and Run %s</title>\n</head>\n",
		    tn);
	    fprintf(fptr, "<body>\n");
	    fprintf(fptr, "<h2>Configure and Run %s Test</h2>\n", tn);
	    fprintf(fptr,
		    "<form action="
		    "\"http://icecube-1.lbl.gov/cgi-bin/run-test\""
                    " method=\"get\">\n");
	    fprintf(fptr, "<table border=\"1\">\n  <tbody>\n");
	    fprintf(fptr, 
		    "    "
		    "<tr><th>Name</th>"
		    "<th>Type</th>"
		    "<th>Value</th>"
		    "<th>Min</th>"
		    "<th>Max</th>"
		    "</tr>\n");
	    descs[ndescs] = tn;
	    ndescs++;
	 }
	 else if (xmlStrcmp(cur->name, 
			    (const xmlChar *) "inputParameter")==0) {
	    xmlNodePtr pxn;
	    const char *cn = NULL;
	    
	    fprintf(fptr, "    <tr>");
	    for (pxn = cur->children; pxn!=NULL; pxn=pxn->next) {
	       if (xmlStrcmp(pxn->name, (const xmlChar *) "name")==0) {
		  cn = xmlNodeListGetString(doc, pxn->xmlChildrenNode, 1);
		  fprintf(fptr, "<td>%s</td>",  cn);
	       }
	       else {
		  xmlAttrPtr ap;
		  const char *minv = NULL, *maxv = NULL, *defv = NULL;

		  fprintf(fptr, "<td>%s</td>", pxn->name);

		  if (xmlStrcmp(pxn->name, "boolean")==0) {
		     fprintf(fptr, "<td><select name=\"%s\"><option value=\"true\">True</option><option value=\"false\">False</option>", cn);
		     fprintf(fptr, "<td></td><td></td>");
		  }
		  else {
		     ap = xmlHasProp(pxn, "minValue");
		     if (ap!=NULL) 
			minv = xmlNodeListGetString(doc, 
						    ap->xmlChildrenNode, 1);
		     
		     ap = xmlHasProp(pxn, "maxValue");
		     if (ap!=NULL) 
		     maxv = xmlNodeListGetString(doc, 
						 ap->xmlChildrenNode, 1);
		  
		     ap = xmlHasProp(pxn, "default");
		     if (ap!=NULL) 
			defv = xmlNodeListGetString(doc, 
						    ap->xmlChildrenNode, 1);
		     
		     fprintf(fptr, "<td><input name=\"%s\" value=\"%s\"></input>", cn, (defv!=NULL) ? defv : "");
		     /* if (defv!=NULL) fprintf(fptr, "%s", defv);*/
		     fprintf(fptr, "</td>");
		     
		     fprintf(fptr, "<td>");
		     if (minv!=NULL) fprintf(fptr, "%s", minv);
		     fprintf(fptr, "</td>");
		     
		     fprintf(fptr, "<td>");
		     if (maxv!=NULL) fprintf(fptr, "%s", maxv);
		     fprintf(fptr, "</td>");
		  }
	       }
	    }
	    fprintf(fptr, "</tr>\n");
	 }
      }
      fprintf(fptr, "  </tbody>\n</table>\n");
      fprintf(fptr, 
	      "<button name=\"test\" value=\"%s\" type=\"submit\">"
	      "Run Test"
	      "</button>\n",
	      tn);
      fprintf(fptr, "</body>\n</html>\n");
      fclose(fptr);
      
      ai++;
   }
      
   xmlCleanupParser();

   /* generate index page...
    */
   if ((fptr = fopen("run-tests.html", "w"))==NULL) {
      fprintf(stderr, "can't open run-tests.html\n");
      return 1;
   }
   
   fprintf(fptr, "<html>\n<head>\n  <title>Run STF tests</title>\n</head>\n");
   fprintf(fptr, "<body>\n");
   fprintf(fptr, "<h2>Configure and run an STF test</h2>\n");
   fprintf(fptr, "<ul>\n");
   for (i=0; i<ndescs; i++) {
      fprintf(fptr, "  <li><a href=\"%s.html\">%s</a></li>\n", 
	      descs[i], descs[i]);
   }
   fprintf(fptr, "</ul>\n");
   fprintf(fptr, "</body>\n</html>\n");
   fclose(fptr);
   
   return 0;
}
