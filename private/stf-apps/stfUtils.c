#include <stddef.h>
#include <stdlib.h>

#include "stfUtils.h"

static const char *errorMessage = NULL;

void clearError(void) {
   if (errorMessage!=NULL) {
      free((char *)errorMessage);
      errorMessage = NULL;
   }
}

void stfError(const char *err) {
   char *t = strdup(err);
   clearError();
   errorMessage = t;
}

const char *stfErrorMessage(void) { return errorMessage; }
