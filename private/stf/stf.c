/*
 *
 *
 * copywrite info
 *
 */
#include <stdio.h>
#include <string.h>

#include "stf/stf.h"

/* Prototypes
 */

/* Module level variables
 */

/*------------------------------------------------------------
 */
void stfInitAllTests()
{
    STF_DESCRIPTOR *d=NULL;
    STF_PARAM *newParam;
    unsigned int i;
    unsigned long l;

    d=findNextTest(d);
    while(d!=NULL) {
	d->testRunnable = (d->initPt)(d);
	
        // init all params for this test to their default values
        newParam=d->params;
        while(strcmp(newParam->name,"")) {
	    if(!strcmp(newParam->type,CHAR_TYPE)) {
	        strcpy(newParam->value.charValue,newParam->defValue);
	    }
	    if(!strcmp(newParam->type,UINT_TYPE)) {
	        sscanf(newParam->defValue,"%u",&i);
	        newParam->value.intValue=i;
	    }
	    if(!strcmp(newParam->type,ULONG_TYPE)) {
	        sscanf(newParam->defValue,"%lu",&l);
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

	    // add code to init arrayLength field from arraySize
	    // if any inconsistencies, tag test as unrunnable and don't
	    // bother calling test's init fcn.

	    newParam++;
        }
        // go to next test
        d=findNextTest(d);
    }
}

/*------------------------------------------------------------
 */
void executeTest(STF_DESCRIPTOR *d)
{
   d->passed = (d->entryPt)(d);
}

/*------------------------------------------------------------
 */
STF_DESCRIPTOR * findNextTest(STF_DESCRIPTOR *d)
{
    int i;

    if(d==NULL) {
       return stfDirectory[0];
    }
    for(i=0;stfDirectory[i]!=NULL;i++) {
	if(stfDirectory[i]==d) {
	    return stfDirectory[i+1];
	}
    }
    return NULL;
}

/*------------------------------------------------------------
 */
STF_DESCRIPTOR * findTestByName(const char *name)
{
    int i;

    for(i=0;stfDirectory[i]!=0;i++) {
	if(!strcmp(stfDirectory[i]->name,name)) {
	    return stfDirectory[i];
	}
    }
    return NULL;
}

/*------------------------------------------------------------
 */
char * getTestName(STF_DESCRIPTOR *d) 
{
    return d->name;
}

/*------------------------------------------------------------
 */
char * getTestDescription(STF_DESCRIPTOR *d) 
{
    return d->desc;
}

/*------------------------------------------------------------
 */
int getTestMajorVersion(STF_DESCRIPTOR *d) 
{
    return d->majorVersion;
}

/*------------------------------------------------------------
 */
int getTestMinorVersion(STF_DESCRIPTOR *d) 
{
    return d->minorVersion;
}

/*------------------------------------------------------------
 */
BOOLEAN testRunnable(STF_DESCRIPTOR *d)
{
    return d->testRunnable;
}

/*------------------------------------------------------------
 */
STF_PARAM * getNextParam(STF_DESCRIPTOR *d, STF_PARAM *p)
{
    STF_PARAM *newParam;

    if(p==NULL) {
	return d->params;
    }
    else {
	newParam=p;
	newParam++;
	if(!strcmp(newParam->name,"")) {
	    return NULL;
	} 
	else {
	    return newParam;
	}
    }
}

/*------------------------------------------------------------
 */
STF_PARAM * getParamByName(STF_DESCRIPTOR *d, const char *name)
{
    STF_PARAM *newParam=d->params;

    while (strcmp(newParam->name,"")) {
	if(!strcmp(newParam->name,name)) {
	    return newParam;
	}
	newParam++;
    }
    
    return NULL;
}

/*------------------------------------------------------------
 */
char * getParamName(STF_PARAM *p) 
{
    return p->name;
}

/*------------------------------------------------------------
 */
const char * getParamClass(STF_PARAM *p)
{
    return p->class;
}

/*------------------------------------------------------------
 */
const char * getParamType(STF_PARAM *p)
{
    return p->type;
}

/*------------------------------------------------------------
 */
char * getParamMinValue(STF_PARAM *p)
{
    return p->minValue;
}

/*------------------------------------------------------------
 */
char * getParamMaxValue(STF_PARAM *p)
{
    return p->maxValue;
}

/*------------------------------------------------------------
 */
char * getParamDefValue(STF_PARAM *p) 
{
    return p->defValue;
}

/*------------------------------------------------------------
 */
char * getParamValueAsString(STF_PARAM *p)
{
    return p->value.charValue;
}

/*------------------------------------------------------------
 */
unsigned int getParamValueAsInt(STF_PARAM *p)
{
    return p->value.intValue;
}

/*------------------------------------------------------------
 */
unsigned long getParamValueAsLong(STF_PARAM *p)
{
    return p->value.longValue;
}

/*------------------------------------------------------------
 */
BOOLEAN getParamValueAsBool(STF_PARAM *p) 
{
    return p->value.boolValue;
}
