/* simple stf server -- we use xml to send test parameters and
 * test results back and forth between server (arm board) and 
 * client (PC)...
 *
 * protocol (start in state 0):
 *
 *    Server (ARM)                      Client (PC)
 *
 * 0  skip server OK\r\n in state 1
 *
 * 1  OK\r\n
 *                                      SEND nbytes\r\n
 * 2  OK\r\n
 *                                      [nbytes of 8 bit data]
 *    OK\r\n
 *                                      GO TestName\r\n
 *    (if error goto 6)
 * 3  OK\r\n
 *                                      RCV TestName\r\n
 *    (if error goto 6)
 * 4  SEND nbytes\r\n
 *                                      OK\r\n
 *
 *                                      OK\r\n
 *    (goto state 1)
 * 5  [nbytes of 8 bit data]
 *                                      OK\r\n
 *    goto state 1
 *
 *                                      OK\r\n
 *    goto state 1
 *
 * 6  ERR msg\r\n
 *    (goto 1)
 *
 *
 * FIXME: loop count input and output!!!!
 *   factor out 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <unistd.h>

#include "xmlparse/xmlparse.h"

#include "stf/stf.h"
#include "hal/DOM_MB_hal.h"
#include "stfUtils.h"

#define talloc(a) ((a) *) malloc(sizeof(a))

static STF_DESCRIPTOR *desc = NULL;
static STF_PARAM *param = NULL;
static const char *xmlTag = NULL;

/*static char *parmType = NULL;*/

/* when we get a new element...
 */
static void startElement(void *userData, const char *name, const char **atts) {
  int *depth = userData;

  xmlTag = name;

  /* printf("startElement: depth: %d, name: %s, atts: %p\n", *depth, name, atts); */

  switch (*depth) {
  case 0:
     if (strcmp(xmlTag, "stf:setup")) {
	fprintf(stdout, 
		"invalid top level object '%s', expecting 'stf:setup'\r\n",
		xmlTag);
     }
     break;
  case 1:
     if ((desc = findTestByName(xmlTag))==NULL) {
	fprintf(stderr, "can't get descriptor name '%s'\r\n", xmlTag);
     }
     break;
  case 2:
     if (strcmp(xmlTag, "version")==0) {
	const int major = atoi(atts[0]);
	const int minor = atoi(atts[1]);
	const int err = 
	   major!=desc->majorVersion || minor!=desc->minorVersion;
	
	/* FIXME: what do we do now?!?!
	 */
     } else if (strcmp(xmlTag, "parameters")==0) {
	/* printf("parameters!\n"); */
     }
     break;
  case 3:
     if ((param = getParamByName(desc, xmlTag))==NULL) {
	fprintf(stderr, "can't get param name '%s'\r\n", xmlTag);
     }
     /* printf("startElement: param: %p\n", param); */

     break;
  default:
     fprintf(stderr, "element nesting too deep.\r\n");
     printf("tag is: '%s'\r\n", name);
     break;
  }
  *depth = *depth + 1;
  return;
}

/* when an element is done...
 */
static void endElement(void *userData, const char *name) {
  int *depthPtr = userData;
  *depthPtr = *depthPtr - 1;
  /* printf("endElement: param==NULL\n"); */
  param = NULL;
}

/* s is not 0 terminated. */
static void characterData(void *userData, const XML_Char *s, int len) {
   int *depthPtr = (int *) userData;
   char str[64];

   memcpy(str, s, len);
   str[len] = 0;

   /* remove trailing whitespace...
    */
   while (len>0 && 
	  (str[len-1]==' ' || str[len-1]=='\r' || str[len-1]=='\n' ||
	   str[len-1]=='\t')) { 
      str[len-1] = 0; len--; 
   }

   if (len==0) return;

#if 0
   fprintf(stdout, "xmlTag = %s, desc = %p, param = %p, depth = %d, s = '%s'\r\n",
	  (xmlTag==NULL) ? "NULL" : xmlTag, desc, param, *depth, str);

   printf("param: %p (%s) [%s], depth: %d, str: '%s'\r\n", param, 
	  param!=NULL ? param->type : "?", 
	  param!=NULL ? param->name : "?", *depthPtr, str);
#endif

   if (NULL != param) {
      if (strcmp(param->type, CHAR_TYPE)==0) {
	 param->value.charValue = strdup(str);
      }
      else if (strcmp(param->type, UINT_TYPE)==0) {
	 param->value.intValue = atoi(str);
      }
      else if (strcmp(param->type, ULONG_TYPE)==0) {
	 param->value.longValue = strtol(str, NULL, 0);
      }
      else if (strcmp(param->type, BOOLEAN_TYPE)==0) {
	 param->value.boolValue = strcmp(str, BOOLEAN_TRUE)==0;
      }
   }
   return;
}

/* turn a directory entry into a xml file...
 *
 * returns number of bytes written to buf, or:
 *
 * -1 : buf is too small.
 *
 * FIXME: check max...
 */
static int dirToXML(char *buf, int max, STF_DESCRIPTOR *stf) {
   int idx = 0;
   int i;

   idx += sprintf(buf+idx, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\r\n");
   idx += sprintf(buf+idx, "<stf:result xmlns:stf=\"http://glacier.lbl.gov/icecube/daq/stf\"\r\n");
   idx += sprintf(buf+idx, "  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\r\n");
   idx += sprintf(buf+idx, "  xsi:schemaLocation=\"http://glacier.lbl.gov/icecube/daq/stf stf.xsd\">\r\n");
   idx += sprintf(buf+idx, " <%s>\r\n", stf->name);
   idx += sprintf(buf+idx, "  <description>\r\n%s\r\n  </description>\r\n", stf->desc);
   idx += sprintf(buf+idx, "  <version major=\"%d\" minor=\"%d\"/>\r\n", 
		  stf->majorVersion, stf->minorVersion);
   idx += sprintf(buf+idx, "  <parameters>\r\n");
   for (i=0; i<stf->nParams; i++) {
      int j;
      
      idx += sprintf(buf+idx, "   <%s>", stf->params[i].name);
      if (strcmp(stf->params[i].type, CHAR_TYPE)==0) {
	 idx += sprintf(buf+idx, "%s", stf->params[i].value.charValue);
      }
      else if (strcmp(stf->params[i].type, UINT_TYPE)==0) {
	 idx += sprintf(buf+idx, "%u", stf->params[i].value.intValue);
      }
      else if (strcmp(stf->params[i].type, ULONG_TYPE)==0) {
	 idx += sprintf(buf+idx, "%lu", stf->params[i].value.longValue);
      }
      else if (strcmp(stf->params[i].type, BOOLEAN_TYPE)==0) {
	 idx += sprintf(buf+idx, "%s", 
			stf->params[i].value.boolValue ? 
			BOOLEAN_TRUE : BOOLEAN_FALSE);
      }
      else if (strcmp(stf->params[i].type, UINT_ARRAY_TYPE)==0) {
	 for (j=0; j<stf->params[i].arrayLength; j++) {
	    idx += sprintf(buf+idx, "%u ", 
			   stf->params[i].value.intArrayValue[j]);
	 }
      }
      else if (strcmp(stf->params[i].type, ULONG_ARRAY_TYPE)==0) {
	 for (j=0; j<stf->params[i].arrayLength; j++) {
	    idx += sprintf(buf+idx, "%lu ", 
			   stf->params[i].value.longArrayValue[j]);
	 }
      }
      else {
	 idx += sprintf(buf+idx, "?");
      }
      idx += sprintf(buf+idx, "</%s>\r\n", stf->params[i].name);
   }

   idx += sprintf(buf+idx, "   <passed>%s</passed>\r\n", 
	   stf->passed ? BOOLEAN_TRUE : BOOLEAN_FALSE);
   idx += sprintf(buf+idx, "   <testRunnable>%s</testRunnable>\r\n", 
		  stf->testRunnable ? BOOLEAN_TRUE : BOOLEAN_FALSE);   
   idx += sprintf(buf+idx, "   <boardID>%s</boardID>\r\n", halGetBoardID()+4);
   
   if (stfErrorMessage()!=NULL) {
      idx+= sprintf(buf+idx, "    <errorMessage>%s</errorMessage>\r\n",
		    stfErrorMessage());
      clearError();
   }

   idx += sprintf(buf+idx, "  </parameters>\r\n");
   idx += sprintf(buf+idx, " </%s>\r\n", stf->name);
   idx += sprintf(buf+idx, "</stf:result>\r\n");
   return idx;
}

/* returns: zero OK, non-zero error, empty lines are ignored...
 */
static int getLine(char *line, int max) {
   int idx = 0;

   memset(line, 0, max);
   while (idx<max-1) {
      if ((read(0, line+idx, 1))<=0) {
	 return 1;
      }

#if 0
      {  int i;
	  
         for (i=0; i<idx; i++) fprintf(stderr, "%02x ", line[i]);
	 fprintf(stderr, "\r\n");
      }
#endif

      /* look for '\r'...
       */
      if ('\r'==line[idx]) {
         line[idx] = '\0';
         return 0;
      }
      idx++;
   }
   return 1;
}

static int readall(int fd, char *buf, int nb) {
   int idx=0;
   
   while (idx<nb) {
      const int nr = read(fd, buf+idx, nb-idx);
      if (nr<0) {
	 fprintf(stderr, "unable to read\n");
	 return -1;
      }
      else if (nr==0) {
	 fprintf(stderr, "eof!\n");
	 return -1;
      }
      idx+=nr;
   }
   return idx;
}

int main() {
   char msg[512];
   char name[64];
   char line[256];
   int  nbytes, needAck = 0;
   char *buf = NULL;
   int buflen = 0;
   int state = 0;
   int depth = 0;

   setvbuf(stdout, (char *)NULL, _IOLBF, 0);

   while (1) {
      /* fprintf(stdout, "state: %d, needAck: %d\r\n", state, needAck);*/
      
      if (needAck) {
	 if (getLine(line, sizeof(line))) {
	    strcpy(msg, "unable to read ack line!");
	    state = 6;
	 }
	 else if (strcmp(line, "OK")!=0) {
	    sprintf(msg, "invalid Ack command! '%s'", line);
	    state = 6;
	 }
	 needAck = 0;
       }

      if (state==0 || state==1) {
	 if (state==1) fprintf(stdout, "OK\r\n");

	 if (getLine(line, sizeof(line))) {
	    strcpy(msg, "unable to read line!");
	    state = 6;
	 }
	 else {
	    if (strcmp(line, "REBOOT")==0) {
	       halBoardReboot();
	    }
	    else if (sscanf(line, "SEND %d", &nbytes)==1) {
	       state = 2;
	    }
	    else if (sscanf(line, "GO %s", name)==1) {
	       state = 3;
	    }
	    else if (sscanf(line, "RCV %s", name)==1) {
	       state = 4;
	    }
	    else if (strcmp(line, "OK")==0) {
	       state = 1;
	    }
	    else {
	       sprintf(msg, "invalid state 1 command!: '%s'", line);
	       state = 6;
	    }
	 }
      }
      else if (state==2) {
	 int nr;
	 
	 /* realloc buf if necessary...
	  */
	 if (buflen<nbytes+1) {
	    void *nb = malloc(nbytes+1);
	    if (nb==NULL) {
	       sprintf(msg, "unable to alloc %d bytes\r\n", nbytes);
	       state = 6;
	       continue;
	    }
	    buf = nb;
	    buflen = nbytes+1;
	 }
	 fprintf(stdout, "OK\r\n");

	 /* read xml file
	  */
	 if ((nr=readall(0, buf, nbytes))!=nbytes) {
	    sprintf(msg, "can't read data, got: %d, expecting: %d\r\n", 
		    nr, nbytes);
	    state = 6;
	 }
	 else {
	    XML_Parser parser = XML_ParserCreate(NULL);

	    buf[nbytes] = 0;

	    /* fprintf(stdout, "parse: '%s'\r\n", buf); */
	    
	    /* now parse data...
	     */
	    XML_SetUserData(parser, &depth);
	    XML_SetElementHandler(parser, startElement, endElement);
	    XML_SetCharacterDataHandler(parser, characterData);
	    if (!XML_Parse(parser, buf, nbytes, 1)) {
	       sprintf(msg,
		       "%s at line %d\r\n",
		       XML_ErrorString(XML_GetErrorCode(parser)),
		       XML_GetCurrentLineNumber(parser));
	       state = 6;
	    }
	    else {
	       /* done parsing...
		*/
	       state = 1;
	    }
	    XML_ParserFree(parser);
	 }
      }
      else if (state==3) {
	 STF_DESCRIPTOR *sd;
	 
	 /* name now has testname to go -- so go...
	  *
	  * 1) lookup test name...
	  * 2) exec test...
	  */
	 if ((sd = findTestByName(name))==NULL) {
	    sprintf(msg, "can not find test '%s'", name);
	    state = 6;
	 }
	 else {
	    stfInitTest(sd);
	    
	    if (sd->testRunnable==0) {
	       sprintf(msg, "test '%s' is not runnable", name);
	       state = 6;
	    }
	    else {
	       clearError();
	       sd->passed = sd->entryPt(sd);
	       state = 1;
	    }
	 }
      }
      else if (state==4) {
	 STF_DESCRIPTOR *sd;
	 const int minbl = 32*1024;
	 
	 /* test is now done
	  *
	  * 1) format results to buffer...
	  * 2) request to client to send buffer...
	  */
	 if (buflen<minbl) {
	    void *nb = malloc(minbl);
	    buf = nb;
	    buflen = minbl;
	 }

	 if ((sd = findTestByName(name))==NULL) {
	    sprintf(msg, "can't find test '%s'\n", name);
	    state = 6;
	 }
	 else {
	    if ((nbytes=dirToXML(buf, buflen, sd))<0) {
	       sprintf(msg, "can't create xml file\n");
	       state = 6;
	    }
	    else {
	       fprintf(stdout, "SEND %d\r\n", nbytes);
	       needAck = 1;
	       state = 5;
	    }
	 }
      }
      else if (state==5) {
	 /* client is ready for the results...
	  *
	  * 1) write file
	  * 2) wait for client ack
	  */
	 write(1, buf, nbytes);

	 /* wait for ack from client...
	  */
         needAck = 1;
	 state = 1;
      }
      else if (state==6) {
	 fprintf(stdout, "ERR '%s'\r\n", msg);
	 strcpy(msg, "");
	 state = 1;
      }
   }

   return 0;
}
