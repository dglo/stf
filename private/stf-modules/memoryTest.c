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
void memoryTestInit();

/** test entry point */
void memoryTest();

/*
 * Module level variables
 */
STF_PARAM memoryTest_params[3]={
	{"loopCount",
	"control",
	"unsigned int",
	"1000",
	"1",
	"10",
	""},

	{"fluffy",
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

STF_DESCRIPTOR memoryTest_descriptor = {
	"memory test",
	"DOM MB memory test routine",
	1,
	0,
	FALSE,
	FALSE,
	(STF_PARAM *)memoryTest_params,
	&memoryTestInit,
	&memoryTest
	};


/*------------------------------------------------------------
 */
void memoryTestInit()
{
    STF_PARAM *newParam;
    unsigned int i;
    unsigned long l;

    newParam=memoryTest_params;
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

    memoryTest_descriptor.testRunnable=TRUE;
}

/*------------------------------------------------------------
 */
void memoryTest()
{
}

