
/*
 *
 * STF Test -- PMT High Voltage Stability
 *
 * This test sets the PMT high voltage to a particular level 
 * and then simply monitors it for a given time.  The return
 * parameters include the mean, max, min, and rms of the collected
 * values.  Pass/fail criteria include a maximum allowed rms, and
 * a largest allowed |set-mean|, |mean-min|, and |mean-max|.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "stf/stf.h"
#include "hal/DOM_MB_hal.h"

/* Lots of information on stdout; for use with menu.c */
#define VERBOSE

/* PASS/FAIL criteria */
#define HV_MAX_RMS_MVOLT      1000  /* Maximum RMS, in millivolts */
#define HV_MEAN_ERR_MVOLT     5000  /* Maximum difference of the mean from the set value, mV */
#define HV_MAX_MIN_ERR_MVOLT  5000  /* Maximum difference of min/max values from the mean, mV */

BOOLEAN pmt_hv_stabilityInit(STF_DESCRIPTOR *desc) { return TRUE; }

BOOLEAN pmt_hv_stabilityEntry(STF_DESCRIPTOR *desc,
                              unsigned int hv_set_volt,
                              unsigned int run_time_sec,
                              unsigned int sample_time_ms,
                              unsigned int init_wait_sec,
                              unsigned int* hv_read_mean_mvolt,
                              unsigned int* hv_read_min_mvolt,
                              unsigned int* hv_read_max_mvolt,
                              unsigned int* hv_read_rms_mvolt) {

    unsigned int hv_read_level = 0;
    unsigned int hv_min_level = 4095;
    unsigned int hv_max_level = 0;

    float mean_level = 0.0;
    float rms = 0.0;

    unsigned int i, sample_cnt;
    unsigned int *sample_arr;

    /* Start with the PMT HV disabled */
    halDisablePMT_HV();

    #ifdef VERBOSE
    printf("DEBUG: Enabling PMT at %d V\r\n", hv_set_volt);
    #endif

    /* Enable the HV and set the level (in that order) */
    /* DAC value is 2xVoltage */
    halEnablePMT_HV();
    halWriteActiveBaseDAC(hv_set_volt * 2);

    /* If requested, wait a while before starting data collection */
    /* halUSleep() doesn't seem to like really large values (not sure */
    /* what max is), so break into 10-second chunks */
    #ifdef VERBOSE
    printf("DEBUG: Waiting %d seconds before starting data collection\r\n", init_wait_sec);
    #endif

    if (init_wait_sec > 0) {
        for (i = 0; i < (init_wait_sec / 10); i++)
            halUSleep(10000000);
        halUSleep((init_wait_sec % 10) * 1000000);
    }

    /* Figure out how many samples we're taking and allocate memory */
    sample_cnt = (run_time_sec * 1000) / sample_time_ms;
    
    #ifdef VERBOSE
    printf("DEBUG: Total samples = %d\r\n", sample_cnt);
    #endif

    if ((sample_arr = (unsigned int *) malloc(sizeof(unsigned int) * sample_cnt)) == NULL) {
        printf("ERROR: malloc failed\r\n");
        return FALSE;
    }
    
    /* Record the base ADC value at the requested intervals */
    for (i = 0; i < sample_cnt; i++) {
        
        /* Sleep for a given number of microseconds */
        halUSleep(sample_time_ms * 1000);

        hv_read_level = halReadBaseADC();

        #ifdef VERBOSE
        printf("%.2f %.1f\r\n", ((i*(float)sample_time_ms/1000) + init_wait_sec), (float)hv_read_level/2);
        #endif

        /* Record in the sample array */
        sample_arr[i] = hv_read_level;
        
        /* Check for min/max */
        hv_min_level = (hv_read_level < hv_min_level) ? hv_read_level : hv_min_level;
        hv_max_level = (hv_read_level > hv_max_level) ? hv_read_level : hv_max_level;
        
        /* Calculate the mean along the way */
        mean_level  += (float)hv_read_level / sample_cnt;
    }

    /* Calculate the RMS (standard deviation) */
    for (i = 0; i < sample_cnt; i++) {
        rms += (sample_arr[i] - mean_level) * (sample_arr[i] - mean_level) / (sample_cnt-1);
    }
    rms = sqrt(rms);

    #ifdef VERBOSE
    printf("DEBUG: RMS of ADC = %g\r\n", rms);
    #endif

    /* Set the output parameters (integers!) */
    *hv_read_mean_mvolt = (int)(mean_level * 1000 / 2);
    *hv_read_min_mvolt  = hv_min_level * 1000 / 2;
    *hv_read_max_mvolt  = hv_max_level * 1000 / 2;
    *hv_read_rms_mvolt  = (int)(rms * 1000 / 2);

    /* Turn the HV off */
    halDisablePMT_HV();

    /* Check for failure */
    if (*hv_read_rms_mvolt > HV_MAX_RMS_MVOLT) 
        return FALSE;
    
    if (abs(*hv_read_mean_mvolt - hv_set_volt*1000) > HV_MEAN_ERR_MVOLT)
        return FALSE;

    if ((abs(*hv_read_max_mvolt - *hv_read_mean_mvolt) > HV_MAX_MIN_ERR_MVOLT) ||
        (abs(*hv_read_min_mvolt - *hv_read_mean_mvolt) > HV_MAX_MIN_ERR_MVOLT))
        return FALSE;
                                                                          
    return TRUE;
}
