#ifndef STF_H
#define STF_H

/*
 * header and copywrite
 *
 */

/* Summary
 *	This header contains structure and constant definitions
 * used for accessing, manipulating and executing modular test
 * routines contained on the DOM main board.
 */

/* Synopsis
 *
 *
 */

/* Usage Example
 */

/* Todo List
 */

/* Defines
 */

/** Definition of boolean (This can be removed if we 
 * have a general xxxTypes.h file)
 */
#include "hal/DOM_MB_types.h"

#if 0
#define BOOLEAN int
#endif

/** See BOOLEAN */
#define TRUE 1
/** See BOOLEAN */
#define FALSE 0

/** define tags for primitive types */
#define CHAR_TYPE "string"
#define UINT_TYPE "unsignedInt"
#define ULONG_TYPE "unsignedLong"
#define BOOLEAN_TYPE "boolean"
#define BOOLEAN_TRUE "true"
#define BOOLEAN_FALSE "false"
#define UINT_ARRAY_TYPE "unsignedIntArray"
#define ULONG_ARRAY_TYPE "unsignedLongArray"

#ifndef NULL
/** define NULL, if not already available */
#define NULL 0
#endif

/** Longest possible character representation of a test parameter value*/
#define MAX_VALUE_CHAR_LEN 32

/** Parameter class constant */
#define CONTROL_PARAM "control_status"

/** Parameter class constant */
#define INPUT_PARAM "input"

/** Parameter class constant */
#define OUTPUT_PARAM "output"

/* Structures
 */
typedef struct {
	/** name of test parameter (must be unique to individual test) */
	char	*name;
	/** parameter class (see xxx_PARAM defines) */
   const char	*class;
	/** conversion format to be applied to char *value field 
	 * (see xxx_FORMAT defines) */
   const char	*type;
	/** char representstion of minimum param value */
	char	*maxValue;
	/** char representation of maximum param value */
	char	*minValue;
	/** char representation of default param value */
	char	*defValue;
	/** char representation of array length required for
	    this param. Required if param is an array */
	char	*arraySize;
	/** representation of value (see type above).  Note this
	* field is writable */
	union {
	   char *charValue; /* [MAX_VALUE_CHAR_LEN]; */
	    unsigned int intValue;
	    unsigned long longValue;
	    BOOLEAN boolValue;
	    unsigned int *intArrayValue;
	    unsigned long *longArrayValue;
	} value;
	/** number of elements in the array...unused for non array params */
	unsigned int arrayLength;
} STF_PARAM;

struct STF_DESCRIPTOR_STRUCT;
typedef struct STF_DESCRIPTOR_STRUCT STF_DESCRIPTOR;

struct STF_DESCRIPTOR_STRUCT {
	/** name of this modular test */
	char 	*name;
	/** brief description of this test */
	char	*desc;
	/** integer value for this test's major version number */
	int	majorVersion;
	/** integer value for this test's minor version number */
	int	minorVersion;

        /** common parameters...
	 */

	/** BOOLEAN to indicate that this test is runnable (determined
	 * at initialization time) */
	BOOLEAN	testRunnable;
        BOOLEAN passed; /* valid after entryPt called... */
        const char *boardID; /* board id is read before test is started */

	/** The 'comps_mask' used to check FPGA dependencies */
	unsigned int fpgaDependencies;

         /** number of parameters */
        int nParams;
	/** pointer to this test's STF_PARAM structure */
	STF_PARAM *params;
	/** pointer to this test's initialization entry point 
	 *
	 * returns true if test is runnable...
	 */
	BOOLEAN	(*initPt)(STF_DESCRIPTOR *);
        /** pointer to this test's execution entry point 
	 *
	 * returns true if test passed...
	 */
        BOOLEAN	(*entryPt)(STF_DESCRIPTOR *);

   /** used internally...
    */
   int isInit;
};

/* Prototypes
 */

/** initialize a test */
void stfInitTest(STF_DESCRIPTOR *d);

/** initialize all tests */
void stfInitAllTests(void);

/** call test entry point */
void executeTest(STF_DESCRIPTOR *d);

/** find first and next test */
STF_DESCRIPTOR * findNextTest(STF_DESCRIPTOR *d);

/** find test by name */
STF_DESCRIPTOR * findTestByName(const char *name);

/** get test name */
char * getTestName(STF_DESCRIPTOR *d);

/** get test description */
char * getTestDescription(STF_DESCRIPTOR *d);

/** get test major version number */
int getTestMajorVersion(STF_DESCRIPTOR *d);

/** get test minor version number */
int getTestMinorVersion(STF_DESCRIPTOR *d);

/** get next test param */
STF_PARAM * getNextParam(STF_DESCRIPTOR *d, STF_PARAM *p);

/** get param by name */
STF_PARAM * getParamByName(STF_DESCRIPTOR *p, const char *name);

/** get param name */
char * getParamName(STF_PARAM *p);

/** get param class */
const char * getParamClass(STF_PARAM *p);

/** get param type */
const char * getParamType(STF_PARAM *p);

/** get param minimum value */
char * getParamMinValue(STF_PARAM *p);

/** get param maximum value */
char * getParamMaxValue(STF_PARAM *p);

/** get param default value */
char * getParamDefValue(STF_PARAM *p);

/** get param value as string */
char * getParamValueAsString(STF_PARAM *p);

/** get param value as int */
unsigned int getParamValueAsInt(STF_PARAM *p);

/** get param value as long */
unsigned long getParamValueAsLong(STF_PARAM *p);

/** get param value as BOOLEAN */
BOOLEAN getParamValueAsBool(STF_PARAM *p);

extern STF_DESCRIPTOR *stfDirectory[];

#endif /* STF_H */
