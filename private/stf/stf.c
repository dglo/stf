/*
 *
 *
 * copywrite info
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stf/stf.h"
#include "hal/DOM_MB_fpga.h"
#include "hal/DOM_MB_pld.h"

/* Prototypes
 */

/* Module level variables
 */
void stfInitTest(STF_DESCRIPTOR *sd) {
   if (!sd->isInit) {
      int i;
      int vermm;
      
      /* make sure dependencies are valid...
       */
      vermm = hal_FPGA_query_versions(DOM_HAL_FPGA_TYPE_ICEBOOT,
				      sd->fpgaDependencies)!=0;
      if (vermm) {
	 vermm = hal_FPGA_query_versions(DOM_HAL_FPGA_TYPE_STF_COM,
					 sd->fpgaDependencies)!=0;
	 if (vermm) {
	    vermm = hal_FPGA_query_versions(DOM_HAL_FPGA_TYPE_STF_NOCOM,
					    sd->fpgaDependencies)!=0;
	 }
      }

      sd->testRunnable = sd->initPt(sd) && !vermm;
      sd->isInit = 1;
      
      for (i=0; i<sd->nParams; i++) {
	 if (strcmp(sd->params[i].type, UINT_ARRAY_TYPE)==0) {
	    sd->params[i].value.intArrayValue = 
	       (unsigned *) calloc(sd->params[i].arrayLength,
				   sizeof(unsigned));
	 }
	 if (strcmp(sd->params[i].type, ULONG_ARRAY_TYPE)==0) {
	    sd->params[i].value.longArrayValue = 
	       (unsigned long *) calloc(sd->params[i].arrayLength,
				   sizeof(unsigned long));
	 }
      }
   }

   /* always clear pld and fpga state...
    */
   {  const int xxx = 0;
      
   /* make sure atwd mux is on led */
   halSelectAnalogMuxInput(DOM_HAL_MUX_PMT_LED_CURRENT);
   
   halDisableBarometer();
   halDisableFlasher();
   halDisableLEDPS();
   halDisablePMT_HV();
   
   halWriteDAC(DOM_HAL_DAC_ATWD0_TRIGGER_BIAS, 850 );
   halWriteDAC(DOM_HAL_DAC_ATWD0_RAMP_TOP, 2097 );
   halWriteDAC(DOM_HAL_DAC_ATWD0_RAMP_RATE , 3000 );
   halWriteDAC(DOM_HAL_DAC_ATWD_ANALOG_REF , 2048 );
   halWriteDAC(DOM_HAL_DAC_ATWD1_TRIGGER_BIAS , 850 );
   halWriteDAC(DOM_HAL_DAC_ATWD1_RAMP_TOP , 2097 );
   halWriteDAC(DOM_HAL_DAC_ATWD1_RAMP_RATE , 3000 );
   halWriteDAC(DOM_HAL_DAC_PMT_FE_PEDESTAL , 1925 );
   halWriteDAC(DOM_HAL_DAC_MULTIPLE_SPE_THRESH , xxx );
   halWriteDAC(DOM_HAL_DAC_SINGLE_SPE_THRESH , 500 );
   halWriteDAC(DOM_HAL_DAC_LED_BRIGHTNESS , xxx );
   halWriteDAC(DOM_HAL_DAC_FAST_ADC_REF , xxx );
   halWriteDAC(DOM_HAL_DAC_INTERNAL_PULSER , 500 );
   halWriteDAC(DOM_HAL_DAC_FE_AMP_LOWER_CLAMP , xxx );
   }
   
   /* Thorsten recommends we wait a bit for these things to
    * settle...
    */
   halUSleep(1000);
   
   /* fpga routines...
    */
   hal_FPGA_TEST_disable_ping_pong();
   hal_FPGA_TEST_disable_pulser();
   hal_FPGA_TEST_clear_trigger();
   hal_FPGA_TEST_init_state();
}

/*------------------------------------------------------------
 */
void stfInitAllTests()
{
    STF_DESCRIPTOR *d=NULL;

    d=findNextTest(d);
    while(d!=NULL) {
       stfInitTest(d);
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
