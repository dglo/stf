/*
 *
 * Does this description belong somewhere else?
 *
 * STF Test -- PMT High Voltage Ramp
 *
 *  This test uses the HV base DAC to set the HV level
 *  and then the base ADC to read it back out.  The voltage 
 *  is stepped up from a minimum to a maximum value and then 
 *  back down again.  Each HV level is allowed to settle for
 *  10s before the ADC is read.
 *
 *  The output parameters indicate the HV level that had
 *  the greatest deviation when read, and the value that
 *  was read out.  The pass/fail criterion is the largest allowed
 *  deviation between the DAC and ADC voltages.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stf/stf.h"
#include "hal/DOM_MB_hal.h"

/* Pass/fail defines */
#define MAX_ERR_VOLT  10

BOOLEAN pmt_hv_rampInit(STF_DESCRIPTOR *desc) { return TRUE; }

BOOLEAN pmt_hv_rampEntry(STF_DESCRIPTOR *desc,
                         unsigned int hv_min_volt,
                         unsigned int hv_max_volt,
                         unsigned int hv_step_volt,
                         char **hv_id,
                         unsigned int * hv_worst_set_mvolt,
                         unsigned int * hv_worst_read_mvolt) {

    unsigned int hv_set_level = 0;
    unsigned int hv_read_level = 0;
    unsigned int hv_step_level = 0;
    unsigned int hv_err_level = 0;
    unsigned int hv_min_level, hv_max_level;

    unsigned int done = 0;
    unsigned int descending = 0;
    unsigned int hv_this_err = 0;

    #ifdef VERBOSE
    printf("DEBUG: Disabling PMT HV\r\n");
    #endif

    /* Start with the PMT HV disabled */
    halPowerDownBase();
    
    /* Check the min/max parameters for sanity */
    if (hv_min_volt > hv_max_volt) {
        hv_min_volt = 0;
        hv_max_volt = 2047;
    }

    /* Convert the voltages to DAC levels */
    hv_step_level = hv_step_volt * 2;
    hv_max_level  = hv_max_volt * 2;
    hv_min_level  = hv_min_volt * 2;

    /* Set the starting level and enable the HV */
    hv_set_level = hv_min_level;
    halPowerUpBase();
    halEnableBaseHV();
    halWriteActiveBaseDAC(hv_set_level);

    #ifdef VERBOSE
    printf("DEBUG: Enabling PMT at %d\r\n", hv_set_level);
    #endif

    /* Read the HV base ID */
    *hv_id = halHVSerial();
    #ifdef VERBOSE
    printf("DEBUG: HV ID is %s\r\n", *hv_id);
    #endif

    /* Ramp the HV up and check the value each time */
    while (!done) {
        
        halWriteActiveBaseDAC(hv_set_level);

        /* Currently let settle for 5s */
        halUSleep(5000000);

        hv_read_level = halReadBaseADC();
        
        #ifdef VERBOSE
        printf("%.1f %.1f\r\n", (float)hv_set_level/2, (float)hv_read_level/2);
        #endif

        /* Check the read vs. set level */
        hv_this_err = abs(hv_read_level - hv_set_level);
        if (hv_this_err > hv_err_level) {

            hv_err_level = hv_this_err;
            *hv_worst_set_mvolt  = hv_set_level  * 1000 / 2;
            *hv_worst_read_mvolt = hv_read_level * 1000 / 2;
        }

        /* Check to see if we've reached the max or min value */
        if (hv_set_level >= hv_max_level) {
            descending = 1;
        }
        if (descending && (hv_set_level <= hv_min_level)) {
            done = 1;
        }
        
        /* Ramping up */
        if (!descending) {
            /* Don't accidentally step over the max */            
            hv_set_level  = (hv_max_level - hv_set_level < hv_step_level) ? 
                hv_max_level : (hv_set_level + hv_step_level);
        }
        /* Going down */
        else {
            /* Don't accidentally step below the min */
            hv_set_level  = (hv_set_level - hv_min_level < hv_step_level) ? 
                hv_min_level : (hv_set_level - hv_step_level);           
        }
    }

    /* Turn the HV off */
    halPowerDownBase();

    /* Did we exceed the allowed error? */
    if (hv_err_level > MAX_ERR_VOLT * 2) {
        return FALSE;
    }
    else {
        return TRUE;
    }
}
