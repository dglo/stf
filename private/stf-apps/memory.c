/* memory tests.
 *
 * FIXME:
 *  test sdram, sram, spsram separately...
 *
 * copywrite info
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stf/stf.h"
#include "stf/memtests.h"

#include "stf-apps/memory.h"

#include "stfUtils.h"

BOOLEAN memoryInit(STF_DESCRIPTOR *d) {
   return TRUE;
}

BOOLEAN memoryEntry(STF_DESCRIPTOR *d,
                    unsigned loopCount,
                    unsigned bsz,
		    unsigned addr,
		    const char *testName) {
   ui32 *buf = (ui32 *) addr;
   int i;
   int ret = ERROR;
   int needFree = 0;

   if (buf==NULL) {
      buf = malloc(bsz*sizeof(ui32));
      needFree = 1;
   }

   for (i=0; i<loopCount; i++) {
      if (strcmp(testName, "stuck-address")==0) {
	 ret = test_stuck_address(buf, buf, bsz);
      }
      else if (strcmp(testName, "thorsten0f")==0) {
	 ret = test_thorsten0f(buf, buf, bsz);
      }
      else if (strcmp(testName, "thorsten16")==0) {
	 ret = test_thorsten16(buf, buf, bsz);
      }
      else {
	 char msg[80];
	 sprintf(msg, "unknown memory test: '%s'", testName);
	 stfError(msg);
	 ret = ERROR;
      }
      
      if (ret==ERROR) break;
   }
   
   if (needFree) free(buf);
   return (ret==OK) ? TRUE : FALSE;
}
