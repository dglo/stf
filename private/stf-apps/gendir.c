/* gendir.c, generate the stfDirectory file automatically
 * from a list of xml files...
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "stf/stf.h"
#include "hal/DOM_MB_fpga.h"

struct ParamNodeStruct;
typedef struct ParamNodeStruct ParamNode;

struct ParamNodeStruct {
   STF_PARAM parm;
   ParamNode *next;
};

struct DescNodeStruct;
typedef struct DescNodeStruct DescNode;

struct DescNodeStruct {
   STF_DESCRIPTOR desc;
   ParamNode *parms;
   DescNode *next;
};

static void usage(void) {
   fprintf(stderr, "usage: gendir xmlfiles...\n");
}

static void fillParam(STF_PARAM *param, xmlDocPtr doc, 
		      struct _xmlNode *children) {
   xmlNodePtr pxn;

   param->arraySize = "1";
   param->arrayLength = 1;
   
   for (pxn = children; pxn!=NULL; pxn=pxn->next) {
      if (xmlStrcmp(pxn->name, (const xmlChar *) "name")==0) {
	 param->name = 
	    xmlNodeListGetString(doc, pxn->xmlChildrenNode, 1);
      }
      else {
	 xmlAttrPtr ap;
	 
	 param->type = pxn->name;
	 param->minValue = param->maxValue = param->defValue = NULL;

	 ap = xmlHasProp(pxn, "minValue");
	 if (ap!=NULL) 
	    param->minValue = 
	       xmlNodeListGetString(doc, ap->xmlChildrenNode, 1);
	 
	 ap = xmlHasProp(pxn, "maxValue");
	 if (ap!=NULL) 
	    param->maxValue = 
	       xmlNodeListGetString(doc, ap->xmlChildrenNode, 1);
	 
	 ap = xmlHasProp(pxn, "default");
	 if (ap!=NULL) 
	    param->defValue = 
	       xmlNodeListGetString(doc, ap->xmlChildrenNode, 1);

	 ap = xmlHasProp(pxn, "length");
	 if (ap!=NULL) {
	    param->arraySize = xmlNodeListGetString(doc, ap->xmlChildrenNode, 
						    1);
	    param->arrayLength = atoi(param->arraySize);
	 }

	 if (strcmp(param->type, CHAR_TYPE)==0) {
	    param->value.charValue = 
	       (param->defValue==NULL) ? "" : param->defValue;
	 }
	 else if (strcmp(param->type, UINT_TYPE)==0) {
	    param->value.intValue = 
	       (param->defValue==NULL) ? 0 : atoi(param->defValue);
	 }
	 else if (strcmp(param->type, ULONG_TYPE)==0) {
	    param->value.longValue = 
	       (param->defValue==NULL) ? 0 : atol(param->defValue);
	 }
	 else if (strcmp(param->type, BOOLEAN_TYPE)==0) {
	    param->value.boolValue = 
	       (param->defValue==NULL) ? 0 : 
	         ((strcmp(param->defValue, BOOLEAN_TRUE)==0) ? 1 : 0);
	 }
      }
   }
}

static unsigned fillDependencies(struct _xmlNode *children) {
   xmlNodePtr pxn;
   unsigned mask = 0;

   for (pxn = children; pxn!=NULL; pxn=pxn->next) {
      if (xmlStrcmp(pxn->name, "COM_FIFO")==0) {
	 mask |= DOM_HAL_FPGA_COMP_COM_FIFO;
      }
      else if (xmlStrcmp(pxn->name, "DAQ")==0) {
	 mask |= DOM_HAL_FPGA_COMP_DAQ;
      }
      else if (xmlStrcmp(pxn->name, "PULSERS")==0) {
	 mask |= DOM_HAL_FPGA_COMP_PULSERS;
      }
      else if (xmlStrcmp(pxn->name, "DISCRIMINATOR_RATE")==0) {
	 mask |= DOM_HAL_FPGA_COMP_DISCRIMINATOR_RATE;
      }
      else if (xmlStrcmp(pxn->name, "LOCAL_COINC")==0) {
	 mask |= DOM_HAL_FPGA_COMP_LOCAL_COINC;
      }
      else if (xmlStrcmp(pxn->name, "FLASHER_BOARD")==0) {
	 mask |= DOM_HAL_FPGA_COMP_FLASHER_BOARD;
      }
      else if (xmlStrcmp(pxn->name, "TRIGGER")==0) {
	 mask |= DOM_HAL_FPGA_COMP_TRIGGER;
      }
      else if (xmlStrcmp(pxn->name, "LOCAL_CLOCK")==0) {
	 mask |= DOM_HAL_FPGA_COMP_LOCAL_CLOCK;
      }
      else if (xmlStrcmp(pxn->name, "SUPERNOVA")==0) {
	 mask |= DOM_HAL_FPGA_COMP_SUPERNOVA;
      }
      else if (xmlStrcmp(pxn->name, "ALL")==0) {
	 mask |= DOM_HAL_FPGA_COMP_ALL;
      }
   }
   return mask;
}

static DescNode *parseFile(const char *fn, DescNode *next) {
   xmlDocPtr doc;
   xmlNodePtr cur;
   DescNode *dn = (DescNode *) malloc(sizeof(DescNode));

   if (dn==NULL) {
      fprintf(stderr, "gendir: unable to allocate memory\n");
      return NULL;
   }
   dn->desc.testRunnable = dn->desc.passed = 0;
   dn->desc.fpgaDependencies = 0;
   dn->parms = NULL;
   dn->next = NULL;

   if ((doc = xmlParseFile(fn))==NULL) {
      fprintf(stderr, "gendir: can't parse file!\n");
      return NULL;
   }

   if ((cur = xmlDocGetRootElement(doc))==NULL) {
      fprintf(stderr, "gendir: empty document\n");
      xmlFreeDoc(doc);
      return NULL;
   }

   if (xmlStrcmp(cur->name, (const xmlChar *) "test")) {
      fprintf(stderr, "gendir: not a stf test document\n");
      xmlFreeDoc(doc);
      return NULL;
   }
   
   for (cur=cur->xmlChildrenNode; cur!=NULL; cur=cur->next) {
      if (xmlStrcmp(cur->name, (const xmlChar *) "name")==0) {
	 dn->desc.name = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
      }
      else if (xmlStrcmp(cur->name, (const xmlChar *) "description")==0) {
	 dn->desc.desc = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
      }
      else if (xmlStrcmp(cur->name, (const xmlChar *) "version")==0) {
	 dn->desc.majorVersion = atoi(xmlGetProp(cur, "major"));
	 dn->desc.minorVersion = atoi(xmlGetProp(cur, "minor"));
      }
      else if (xmlStrcmp(cur->name, (const xmlChar *) "fpgaDependencies")==0) {
	 dn->desc.fpgaDependencies = fillDependencies(cur->xmlChildrenNode);
      }
      else if (xmlStrcmp(cur->name, (const xmlChar *) "inputParameter")==0 ||
	       xmlStrcmp(cur->name, (const xmlChar *) "outputParameter")==0) {
	 ParamNode *pn = (ParamNode *) malloc(sizeof(ParamNode));
	 
	 if (pn==NULL) {
	    fprintf(stderr, "gendir: unable to allocate memory\n");
	    xmlFreeDoc(doc);
	    return NULL;
	 }

	 pn->parm.class = 
	    xmlStrcmp(cur->name, 
		      (const xmlChar *) "inputParameter")==0 ? "input" : 
	    "output";

	 fillParam(&pn->parm, doc, cur->xmlChildrenNode);

	 /* link this param...
	  */
	 pn->next = NULL;
	 {  ParamNode *tmp;
	    for (tmp=dn->parms; tmp!=NULL && tmp->next!=NULL; tmp=tmp->next) ;
	    if (tmp==NULL) {
	       dn->parms = pn;
	    }
	    else {
	       tmp->next = pn;
	    }
	 }
      }
   }
   
   return dn;
}

static int nparams(const DescNode *node) {
   ParamNode *p;
   int np = 0;
   for (p=node->parms; p!=NULL; p=p->next) np++;
   return np;
}

static const char *getInputType(STF_PARAM *p) {
   if (strcmp(p->type, CHAR_TYPE)==0) {
      return "const char *";
   }
   else if (strcmp(p->type, UINT_TYPE)==0) {
      return "unsigned";
   }
   else if (strcmp(p->type, ULONG_TYPE)==0) {
      return "unsigned long";
   }
   else if (strcmp(p->type, BOOLEAN_TYPE)==0) {
      return "BOOLEAN";
   }
   else if (strcmp(p->type, UINT_ARRAY_TYPE)==0) {
      return "const unsigned *";
   }
   else if (strcmp(p->type, ULONG_ARRAY_TYPE)==0) {
      return "const unsigned long *";
   }
   else {
      fprintf(stderr, "invalid type '%s'\n", p->type);
      return NULL;
   }
}

static const char *getOutputType(STF_PARAM *p) {
   if (strcmp(p->type, CHAR_TYPE)==0) {
      return "char *";
   }
   else if (strcmp(p->type, UINT_TYPE)==0) {
      return "unsigned";
   }
   else if (strcmp(p->type, ULONG_TYPE)==0) {
      return "unsigned long";
   }
   else if (strcmp(p->type, BOOLEAN_TYPE)==0) {
      return "BOOLEAN";
   }
   else if (strcmp(p->type, UINT_ARRAY_TYPE)==0) {
      return "unsigned *";
   }
   else if (strcmp(p->type, ULONG_ARRAY_TYPE)==0) {
      return "unsigned long *";
   }
   else {
      fprintf(stderr, "invalid type '%s'\n", p->type);
      return NULL;
   }
}

static const char *getUnionField(STF_PARAM *p) {
   if (strcmp(p->type, CHAR_TYPE)==0) {
      return "charValue";
   }
   else if (strcmp(p->type, UINT_TYPE)==0) {
      return "intValue";
   }
   else if (strcmp(p->type, ULONG_TYPE)==0) {
      return "longValue";
   }
   else if (strcmp(p->type, BOOLEAN_TYPE)==0) {
      return "boolValue";
   }
   else if (strcmp(p->type, UINT_ARRAY_TYPE)==0) {
      return "intArrayValue";
   }
   else if (strcmp(p->type, ULONG_ARRAY_TYPE)==0) {
      return "longArrayValue";
   }
   else {
      fprintf(stderr, "invalid type '%s'\n", p->type);
      return NULL;
   }
}

static int isArrayType(const char *type) {
   return 
      strcmp(type, UINT_ARRAY_TYPE)==0 ||
      strcmp(type, ULONG_ARRAY_TYPE)==0;
}

int main(int argc, char *argv[]) {
   int ai, i;
   DescNode *dn = NULL, *tdn;
   FILE *fptr, *hptr;
   char *descs[1024];
   int ndescs = 0;
   int skeleton = 0;
   
   for (ai=1; ai<argc; ai++) {
      if (argv[ai][0]!='-') break;
      if (strcmp(argv[ai], "-skeleton")==0) {
	 skeleton = 1;
      }
   }

   if (ai>=argc) { 
      usage();
      return 1;
   }

   if (argc-ai>sizeof(descs)/sizeof(descs[0])) {
      fprintf(stderr, "too many files to gendir!\n");
      return 1;
   }

   LIBXML_TEST_VERSION
   xmlKeepBlanksDefault(0);

   /* now generate directory file -- and header...
    */
   if ((fptr=fopen("stfDirectory.c", "w"))==NULL) {
      fprintf(stderr, "gendir: can't open stfDirectory.c!\n");
      return 1;
   }
   
   /* now generate directory file -- and header...
    */
   if ((hptr=fopen("stfDirectory.h", "w"))==NULL) {
      fprintf(stderr, "gendir: can't open stfDirectory.h!\n");
      return 1;
   }
   
   fprintf(hptr,
	   "/* stfDirectory.h, automaticaly generated by gendir\n */\n"
	   "#ifndef STF_DIRECTORY_HEADER\n"
	   "#define STF_DIRECTORY_HEADER\n\n"
	   "#include \"stf/stf.h\"\n\n");

   fprintf(fptr, 
	   "/* stfDirectory.c, automatically generated by gendir\n */");
   fprintf(fptr, "#include \"stf/stf.h\"\n");
   fprintf(fptr, "#include \"stfDirectory.h\"\n\n");
   
   while (ai<argc) {
      if ((dn = parseFile(argv[ai], dn))==NULL) {
	 fprintf(stderr, "gendir: can't parse file: '%s'\n", argv[ai]);
	 return 1;
      }

      for (tdn = dn; tdn!=NULL; tdn = tdn->next) {
	 FILE *cptr = NULL;
	 int i;
	 const int np = nparams(tdn);
	 int nOutputs = 0, nInputs = 0;
	 ParamNode *tp = NULL;
	 char *fname;
	 
	 if (ndescs>=sizeof(descs)/sizeof(descs[0])) {
	    fprintf(stderr, "too many descriptors!\n");
	    return 1;
	 }
	    
	 descs[ndescs] = tdn->desc.name;
	 ndescs++;

	 if ((fname = (char *) malloc(strlen(tdn->desc.name) + 3))==NULL) {
	    fprintf(stderr, "gendir: can't allocate fname\n");
	    return 1;
	 }
	 sprintf(fname, "%s.c", tdn->desc.name);
	 
	 if (skeleton && access(fname, F_OK)!=0) {
	    cptr = fopen(fname, "w");

	    if (cptr!=NULL) {
	       fprintf(cptr, 
		       "/* %s, skeleton file created by gendir\n */\n"
		       "#include \"stf/stf.h\"\n"
		       "#include \"stf-apps/%s.h\"\n\n", 
		       fname, tdn->desc.name);
	    }
	 }
	 
	 fprintf(fptr, "static STF_PARAM %s_params[%d] = {\n", 
		 tdn->desc.name, np+1);
	 for (i=0, tp=tdn->parms; i<np; i++, tp=tp->next) {
	    if (strcmp(tp->parm.class, "output")==0) nOutputs++;
	    else nInputs++;

	    fprintf(fptr, "   {\n");
	    fprintf(fptr, "     .name = \"%s\",\n", tp->parm.name);
	    fprintf(fptr, "     .class = \"%s\",\n", tp->parm.class);
	    fprintf(fptr, "     .type = \"%s\",\n", tp->parm.type);
	    if (tp->parm.maxValue!=NULL) {
	       fprintf(fptr, "     .maxValue = \"%s\",\n", tp->parm.maxValue);
	    }
	    else fprintf(fptr, "     .maxValue = NULL,\n");
	    if (tp->parm.minValue!=NULL) {
	       fprintf(fptr, "     .minValue = \"%s\",\n", tp->parm.minValue);
	    }
	    else fprintf(fptr, "     .minValue = NULL,\n");
	    if (tp->parm.defValue!=NULL) {
	       fprintf(fptr, "     .defValue = \"%s\",\n", tp->parm.defValue);
	    }
	    else fprintf(fptr, "     .defValue = NULL,\n");
	    
	    fprintf(fptr, "     .arraySize = \"%s\",\n", tp->parm.arraySize);
	    fprintf(fptr, "     .value = {\n");
	    if (strcmp(tp->parm.type, CHAR_TYPE)==0) {
	       fprintf(fptr, 
		       "       .charValue = "
		       "\"%s\"\n", tp->parm.value.charValue);
	    }
	    else if (strcmp(tp->parm.type, UINT_TYPE)==0) {
	       fprintf(fptr, "       .intValue = %d\n",
		       tp->parm.value.intValue);
	    }
	    else if (strcmp(tp->parm.type, ULONG_TYPE)==0) {
	       fprintf(fptr, 
		       "       .longValue = %ld\n",
		       tp->parm.value.longValue);
	    }
	    else if (strcmp(tp->parm.type, BOOLEAN_TYPE)==0) {
	       fprintf(fptr, 
		       "       .boolValue = %d\n",
		       tp->parm.value.boolValue);
	    }
	    else if (strcmp(tp->parm.type, UINT_ARRAY_TYPE)==0) {
	       fprintf(fptr, 
		       "       .intArrayValue = NULL "
		       "/* initialized at run time */\n");
	    }
	    else if (strcmp(tp->parm.type, ULONG_ARRAY_TYPE)==0) {
	       fprintf(fptr, 
		       "       .longArrayValue = NULL "
		       "/* initialized at run time */\n");
	    }
	    else {
	       fprintf(stderr, "invalid type '%s'\n", tp->parm.type);
	       return 1;
	    }
	    fprintf(fptr, "     },\n");
	    fprintf(fptr, "     .arrayLength = %d\n", tp->parm.arrayLength);
	    fprintf(fptr, "   },\n");
	 }
	 fprintf(fptr, "   { .name = \"\" }\n");
	 fprintf(fptr, "};\n");
	 
	 fprintf(fptr, "\n");

	 fprintf(hptr, "BOOLEAN %sInit(STF_DESCRIPTOR *);\n", 
		 tdn->desc.name);

	 if (cptr!=NULL) {
	    fprintf(cptr, 
		    "BOOLEAN %sInit(STF_DESCRIPTOR *d) {\n"
		    "   return FALSE;\n}\n\n",
		    tdn->desc.name);
	 }

	 if (np>0) {
	    /* if we have any params, put them into the
	     * extern descriptor...
	     */
	    fprintf(hptr, 
		    "BOOLEAN %sEntry(STF_DESCRIPTOR *",
		    tdn->desc.name);

	    if (cptr!=NULL) {
	       fprintf(cptr, 
		       "BOOLEAN %sEntry(STF_DESCRIPTOR *d",
		       tdn->desc.name);
	    }
	    
	    /* first inputs...
	     */
	    for (i=0, tp=tdn->parms; i<np; i++, tp=tp->next) {
	       if (strcmp(tp->parm.class, "input")==0) {
		  fprintf(hptr,
			  ",\n"
			  "                    %s %s",
			  getInputType(&tp->parm), tp->parm.name);

		  if (cptr!=NULL) {
		     fprintf(cptr,
			     ",\n"
			     "                    %s %s",
			     getInputType(&tp->parm), tp->parm.name);
		  }
	       }
	    }
	    
	    /* now do outputs...
	     */
	    for (i=0, tp=tdn->parms; i<np; i++, tp=tp->next) {
	       if (strcmp(tp->parm.class, "output")==0) {
		  fprintf(hptr,
			  ",\n"
			  "                    %s %s%s", 
			  getOutputType(&tp->parm),
			  isArrayType(tp->parm.type) ? "" : "*",
			  tp->parm.name);
		  
		  if (cptr!=NULL) {
		     fprintf(cptr,
			     ",\n"
			     "                    %s %s%s", 
			     getOutputType(&tp->parm), 
			     isArrayType(tp->parm.type) ? "" : "*",
			     tp->parm.name);
		  }
	       }
	    }
	    
	    fprintf(hptr,");\n");

	    if (cptr!=NULL) {
	       fprintf(cptr, ") {\n   return FALSE; \n}\n");
	    }

	    /* add callout function...
	     */
	    fprintf(fptr, 
		    "\nstatic BOOLEAN %sEntryLocal(STF_DESCRIPTOR *d) {\n",
		    tdn->desc.name);

	    fprintf(fptr, "   return %sEntry(d", tdn->desc.name);

	    /* first inputs...
	     */
	    for (i=0, tp=tdn->parms; i<np; i++, tp=tp->next) {
	       if (strcmp(tp->parm.class, "input")==0) {
		  fprintf(fptr,
			  ",\n"
			  "                     "
			  "getParamByName(d, \"%s\")->value.%s",
			  tp->parm.name, getUnionField(&tp->parm));
	       }
	    }
	    
	    /* now do outputs...
	     */
	    for (i=0, tp=tdn->parms; i<np; i++, tp=tp->next) {
	       if (strcmp(tp->parm.class, "output")==0) {
		  fprintf(fptr,
			  ",\n"
			  "                     "
			  "%sgetParamByName(d,  \"%s\")->value.%s",
			  isArrayType(tp->parm.type) ? "" : "&",
			  tp->parm.name, getUnionField(&tp->parm));
	       }
	    }
	    
	    fprintf(fptr,");\n}\n");
	 }
	 else 
	    fprintf(hptr, "BOOLEAN %sEntry(STF_DESCRIPTOR *);\n",
		    tdn->desc.name);

	 fprintf(fptr, "\n");
	 fprintf(fptr, "static STF_DESCRIPTOR %s_descriptor = {\n", 
		 tdn->desc.name);
	 fprintf(fptr, "  .name = \"%s\",\n", tdn->desc.name);
	 fprintf(fptr, "  .desc = \"%s\",\n", tdn->desc.desc);
	 fprintf(fptr, "  .majorVersion = %d,\n", tdn->desc.majorVersion);
	 fprintf(fptr, "  .minorVersion = %d,\n", tdn->desc.minorVersion);
	 fprintf(fptr, "  .testRunnable = %d,\n", tdn->desc.testRunnable);
	 fprintf(fptr, "  .passed = %d,\n", tdn->desc.passed);
	 fprintf(fptr, "  .fpgaDependencies = 0x%08x,\n", 
		 tdn->desc.fpgaDependencies);
	 fprintf(fptr, "  .nParams = %d,\n", np);
	 fprintf(fptr, "  .params = %s_params,\n", tdn->desc.name);
	 fprintf(fptr, "  .initPt = %sInit,\n", tdn->desc.name);
	 fprintf(fptr, "  .entryPt = %sEntry%s,\n",
		 tdn->desc.name, np>0 ? "Local" : "");
	 fprintf(fptr, "  .isInit = 0\n");
	 fprintf(fptr, "};\n");

	 if (cptr!=NULL) fclose(cptr);
      }

      ai++;
   }
   
   xmlCleanupParser();
   
   /* write the actual directory...
    */
   fprintf(fptr, "\nSTF_DESCRIPTOR *stfDirectory[]={\n");
   for (i=0; i<ndescs; i++) {
      fprintf(fptr, "   &%s_descriptor,\n", descs[i]);
   }
   fprintf(fptr, "   NULL\n};\n");
   fclose(fptr);
   
   fprintf(hptr, "#endif\n");
   fclose(hptr);

   return 0;
}
