/*
 *
 *
 * copywrite info
 *
 */
#include <stdio.h>

#include "stf.h"

void memoryInit(STF_DESCRIPTOR *d) {
   printf("memoryInit\r\n");
    d->testRunnable=TRUE;
}

void memoryEntry(STF_DESCRIPTOR *d) {
   printf("memoryEntry\r\n");
}

