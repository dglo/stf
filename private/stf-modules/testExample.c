/*
 *
 *
 * copywrite info
 *
 */

#include <stf/stf.h>
/* Prototypes
 */
/** test initialization entry point */
void testExampleInit();

/** test entry point */
void testExample();

/*
 * Module level variables
 */
STF_PARAM testExample_params[4]={
	{"loopCount",
	"control",
	"unsigned int",
	"1000",
	"1",
	"10",
	""},

	{"abcd",
	"control",
	"unsigned int",
	"1000",
	"1",
	"10",
	""},

	{"efgh",
	"control",
	"unsigned int",
	"1000",
	"1",
	"10",
	""},

	{"",
	"",
	"",
	"",
	"",
	"",
	""}
};

STF_DESCRIPTOR testExample_descriptor = {
	"testExample",
	"this is my first stf test",
	1,
	0,
	FALSE,
	FALSE,
	(STF_PARAM *)testExample_params,
	&testExampleInit,
	&testExample
	};


/*------------------------------------------------------------
 */
void testExampleInit()
{
    STF_PARAM *newParam;
    unsigned int i;
    unsigned long l;

    newParam=testExample_params;
    while(strcmp(newParam->name,"")) {
	if(!strcmp(newParam->type,CHAR_TYPE)) {
	    strcpy(newParam->value.charValue,newParam->defValue);
	}
	if(!strcmp(newParam->type,UINT_TYPE)) {
	    sscanf(newParam->defValue,"%u",&i);
	    newParam->value.intValue=i;
	}
	if(!strcmp(newParam->type,ULONG_TYPE)) {
	    sscanf(newParam->defValue,"%u",&l);
	    newParam->value.intValue=l;
	}
	if(!strcmp(newParam->type,BOOLEAN_TYPE)) {
	    if(!strcmp(newParam->defValue,BOOLEAN_TRUE)) {
		newParam->value.boolValue=TRUE;
	    }
	    else {
		newParam->value.boolValue=FALSE;
	    }
	}
	newParam++;
    }

    testExample_descriptor.testRunnable=TRUE;
}

/*------------------------------------------------------------
 */
void testExample()
{
}

