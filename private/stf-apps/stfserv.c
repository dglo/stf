/* simple stf server -- we use xml to send test parameters and
 * test results back and forth between server (arm board) and 
 * client (PC)...
 *
 * protocol:
 *
 *    Server (ARM)                      Client (PC)
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
 * 6  ERR msg\r\n
 *    (goto 1)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xmlparse.h"

#include "stf.h"

extern int read(int , void *, int);
extern int write(int , void *, int);

/* when we get a new element...
 */
static void startElement(void *userData, const char *name, const char **atts) {
  int i;
  int *depthPtr = userData;
  for (i=0; i<*depthPtr; i++) printf(" ");
  printf("%s\r\n", name);
  *depthPtr = *depthPtr + 1;
}

/* when an element is done...
 */
static void endElement(void *userData, const char *name) {
  int *depthPtr = userData;
  *depthPtr = *depthPtr - 1;
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

   idx += sprintf(buf+idx, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
   idx += sprintf(buf+idx, "<test>\n");
   idx += sprintf(buf+idx, " <name>%s</name>\n", stf->name);
   idx += sprintf(buf+idx, " <desc>%s</desc>\n", stf->desc);
   idx += sprintf(buf+idx, " <majorVersion>%d</majorVersion>\n", 
		  stf->majorVersion);
   idx += sprintf(buf+idx, " <minorVersion>%d</minorVersion>\n",
		  stf->minorVersion);
   idx += sprintf(buf+idx, " <testRunnable>%s</testRunnable>\n",
		  stf->testRunnable ? "TRUE" : "FALSE");

   for (i=0; i<stf->nParams; i++) {
      idx += sprintf(buf+idx, "  <param>\n");
      idx += sprintf(buf+idx, "   <name>%s</name>\n", stf->params[i].name);
      idx += sprintf(buf+idx, "   <class>%s</class>\n", stf->params[i].class);
      idx += sprintf(buf+idx, "   <type>%s</type>\n", stf->params[i].type);
      idx += sprintf(buf+idx, "   <maxValue>%s</maxValue>\n", 
		     stf->params[i].maxValue);
      idx += sprintf(buf+idx, "   <minValue>%s</minValue>\n", 
		     stf->params[i].minValue);
      idx += sprintf(buf+idx, "   <defValue>%s</defValue>\n", 
		     stf->params[i].defValue);
      idx += sprintf(buf+idx, "   <arraySize>%s</arraySize>\n",
		     stf->params[i].arraySize);
      idx += sprintf(buf+idx, "  </param>\n");
   }
   idx += sprintf(buf+idx, "</test>\n");
   return idx;
}

/* returns: zero OK, non-zero error, empty lines are ignored...
 */
static int getLine(char *line, int max) {
   int idx = 0;

   memset(line, 0, max);
   while (idx<max-1) {
      /* int i;*/
      int nr;
      char *t;

      if ((nr=read(0, line+idx, max-1-idx))<=0) {
	 return 1;
      }

#if 0
      for (i=0; i<idx+nr; i++) printf("%02x ", line[i]);
      printf("\r\n");
#endif

      /* look for '\r'...
       */
      if ((t=strchr(line, '\r'))!=NULL) {
	 *t = 0;
	 if (t!=line) return 0;
      }
      else {
	 idx += nr;
      }
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
   int state = 1;
   XML_Parser parser = XML_ParserCreate(NULL);
   int depth = 0;

   XML_SetUserData(parser, &depth);
   XML_SetElementHandler(parser, startElement, endElement);

   while (1) {
      printf("state: %d, needAck: %d\r\n", state, needAck);
      
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

      if (state==1) {
	 printf("OK\r\n");

	 if (getLine(line, sizeof(line))) {
	    strcpy(msg, "unable to read line!");
	    state = 6;
	 }
	 else {
	    if (strcmp(line, "EXIT")==0) {
	       printf("exiting...\r\n");
	       return 0;
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
	 if (buflen<nbytes) {
	    void *nb = malloc(nbytes);
	    if (nb==NULL) {
	       sprintf(msg, "unable to alloc %d bytes\r\n", nbytes);
	       state = 6;
	    }
	    buf = nb;
	    buflen = nbytes;
	 }
	 printf("OK\r\n");

	 /* read xml file
	  */
	 if ((nr=readall(0, buf, nbytes))!=nbytes) {
	    sprintf(msg, "can't read data, got: %d, expecting: %d\r\n", 
		    nr, nbytes);
	    state = 6;
	 }
	 else {
	    /* now parse data...
	     */
	    if (!XML_Parse(parser, buf, nbytes, 1)) {
	       sprintf(msg,
		       "%s at line %d\r\n",
		       XML_ErrorString(XML_GetErrorCode(parser)),
		       XML_GetCurrentLineNumber(parser));
	       state = 6;
	    }
	    else {
	       state = 1;
	    }
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
	    if (!sd->isInit) {
	       sd->initPt(sd);
	       sd->isInit = 1;
	    }
	    
	    if (!sd->testRunnable) {
	       sprintf(msg, "test '%s' is not runnable", name);
	       state = 6;
	    }
	    else {
	       sd->entryPt(sd);
	       state = 1;
	    }
	 }
      }
      else if (state==4) {
	 STF_DESCRIPTOR *sd;
	 
	 const int minbl = 32*1024;
	 int nb;
	 
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
	       printf("SEND %d\r\n", nb);
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
	 printf("ERR '%s'\r\n", msg);
	 strcpy(msg, "");
	 state = 1;
      }
   }

   return 0;
}











