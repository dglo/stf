/*
 *
 *
 * copywrite info
 *
 */

#include <stf/stf.h>
/* Prototypes
 */

/*
 * Module level variables
 */

/*------------------------------------------------------------
 */
int  main() 
{
    char selectCmd[128];
    char testCmd[128];
    STF_DESCRIPTOR *d=NULL;
    STF_PARAM *p=NULL;
    BOOLEAN b;

    stfInitAllTests();

    for (;;) {
        printf("\n\nTests present:\n");

        d=NULL;
        d=findNextTest(d);
        while(d!=NULL) {
  	    printf("Test name: %s\n",getTestName(d));
 	    printf("\tDesc: %s\n",getTestDescription(d));
	    d=findNextTest(d);
        }

        printf("\nSelect test by name (<cr> exits): ");

        gets(selectCmd);
        if(!strcmp(selectCmd,"")) {
  	    printf("exiting.......\n");
	    return 1;
        }

   
        for (;;) {
    	    d=findTestByName(selectCmd);
    	    if(d!=NULL) {
		printf("\n\nTest: %s\n",getTestName(d));
		printf("Description: %s\n",getTestDescription(d));
		printf("Version Info: %d,%d\n",getTestMajorVersion(d),
	    	    getTestMinorVersion(d));
		if(testRunnable(d)) {
	    	    printf("Runnable State: true\n");
		}
		else {
	    	    printf("Runnable State: false\n");
		}
		if(testInProgress(d)) {
	    	    printf("Current State: running\n");
		}
		else {
	            printf("Current State: Not running\n");
		}	

		p=NULL;
		p=getNextParam(d,p);
		while(p!=0) {
	    	    printf("\tParam Name: %s\n",getParamName(p));
	    	    printf("\t\tClass: %s\n",getParamClass(p));
	    	    printf("\t\tType: %s\n",getParamType(p));
	    	    printf("\t\tMin/Max Value: %s / %s\n",
			getParamMinValue(p),getParamMaxValue(p));
	    	    printf("\t\tDefault Value: %s\n",getParamDefValue(p));
	    	    if(!strcmp(getParamType(p),CHAR_TYPE)) {
	        	printf("\t\tValue: %s\n",getParamValueAsString(p));
	    	    }
	    	    if(!strcmp(getParamType(p),UINT_TYPE)) {
	        	printf("\t\tValue: %u\n",getParamValueAsInt(p));
	    	    }
	    	    if(!strcmp(getParamType(p),ULONG_TYPE)) {
	        	printf("\t\tValue: %u\n",getParamValueAsLong(p));
	    	    }
	    	    if(!strcmp(getParamType(p),BOOLEAN_TYPE)) {
			if(getParamValueAsBool(p)) {
	            	    printf("\t\tValue: true\n");
			}
	 		else {
	            	    printf("\t\tValue: true\n");
			}
	    	    }
	    	    p=getNextParam(d,p);
		}

    		printf("\nExexute test (y) or escape to test list (<cr>): ");
    		gets(testCmd);
    		if(!strcmp(testCmd,"")) {
		    break;
		}
		else {
		    printf("\n\ntesting\n");
		    executeTest(d);
		    printf("done!\n\n");
		}
	    }
	    else {
		printf("\n\nTest not found!\n");
		break;
	    }
	}


    }
}

