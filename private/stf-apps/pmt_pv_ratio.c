/*
 *
 * STF Test -- PMT Peak/Valley Ratio
 *
 *  This test measures the PMT peak/valley ratio for SPE
 *  triggers at the 10^7 gain point.
 *
 *   1. Find 10^7 gain point by varying high voltage and searching
 *      for appropriate SPE peak location.
 *
 *   2. Measure the P/V at this gain and return the P/V, peak
 *      location, HV used, and discriminator DAC used.
 *
 *  The SPE histograms themselves require a number of steps to 
 *  produce:
 *
 *   a. Measure average pedestal with HV on but set to a low value.
 *   b. Find discriminator DAC setting by measuring SPE trigger rate
 *      as DAC value is gradually raised.
 *   c. Take SPE triggers and save maxima.
 *   d. Perform heuristic histogram search to find peak and valley.
 *
 *  TO DO -- search doesn't always converge.  Can get confused by one
 *  wrong peak.  Need to provide some kind of way out.
 *
 *  J. Kelley, Oct 2003
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "stf/stf.h"
#include "hal/DOM_MB_hal.h"
#include "stf-apps/atwdUtils.h"

/* HV setting for pedestal measurements */
#define HV_START_VOLT 10

/* Loop count for average pedestal measurement */
#define PEDESTAL_TRIG_CNT  500

/* SPE HEIGHT, IN mV */
#define SPE_HEIGHT_MV      120

/* ATWD DAC settings */
#define ATWD_SAMPLING_SPEED_DAC 850
#define ATWD_RAMP_TOP_DAC       2097
#define ATWD_RAMP_BIAS_DAC      3000
#define ATWD_ANALOG_REF_DAC     2048
#define ATWD_PEDESTAL_DAC       1925

/* Target SPE rate for discriminator setting */
#define TARGET_SPE_RATE         400

/* Min discriminator level */
#define SPE_DISC_DAC_MIN        500

/* Max allowed discriminator level */
#define SPE_DISC_DAC_MAX        550

/* First guess range for 10^7 gain search */
#define HV_FIRST_MIN           1150
#define HV_FIRST_MAX           1650

/* Histogram limits, ATWD units */
#define HIST_MIN       0

/* Histogram bin count */
#define HIST_BIN_CNT  60

/* Pass/fail defines */
#define MIN_PV_RATIO 2.0

BOOLEAN pmt_pv_ratioInit(STF_DESCRIPTOR *desc) { return TRUE; }

BOOLEAN pmt_pv_ratioEntry(STF_DESCRIPTOR *desc,
                          unsigned int atwd_chip_a_or_b,
                          unsigned int atwd_channel,
                          unsigned int atwd_spe_height,
                          unsigned int pv_trig_cnt,
                          unsigned int * pv_ratio_10,
                          unsigned int * hist_spe_peak,
                          unsigned int * hist_array,
                          unsigned int * spe_disc_dac,
                          unsigned int * hv_set_volt) {

    int i,j;
    const int ch = (atwd_chip_a_or_b) ? 0 : 4;
    const int cnt = 128;
    int trigger_mask = (atwd_chip_a_or_b) ? 
        HAL_FPGA_TEST_TRIGGER_ATWD0 : HAL_FPGA_TEST_TRIGGER_ATWD1;

    /* Allocate various buffers */
    short *buffer = (short *) calloc(cnt, sizeof(short));
    int   *atwd_pedestal = (int *) malloc(cnt*sizeof(int));
    short *channels[4] = { NULL, NULL, NULL, NULL };
    short *maxima = (short *) calloc(pv_trig_cnt, sizeof(short));
    
    #ifdef VERBOSE
    printf("DEBUG: Disabling PMT HV\r\n");
    #endif

    /*---------------------------------------------------------------------------------*/

    /* Set up the ATWD DAC values */
    halWriteDAC(ch, ATWD_SAMPLING_SPEED_DAC);
    halWriteDAC(ch+1, ATWD_RAMP_TOP_DAC);
    halWriteDAC(ch+2, ATWD_RAMP_BIAS_DAC);
    halWriteDAC(DOM_HAL_DAC_ATWD_ANALOG_REF, ATWD_ANALOG_REF_DAC);
    halWriteDAC(DOM_HAL_DAC_PMT_FE_PEDESTAL, ATWD_PEDESTAL_DAC);   
    halWriteDAC(DOM_HAL_DAC_SINGLE_SPE_THRESH, SPE_DISC_DAC_MIN);

    #ifdef VERBOSE
    printf("DEBUG: Enabling PMT HV at %d V\r\n", HV_START_VOLT);
    #endif

    /* Start with the PMT HV on, but set at a very low voltage */
    /* Pedestal measurements perhaps shift with HV off/on */
    halEnablePMT_HV();
    halWriteActiveBaseDAC(HV_START_VOLT * 2);

    /* Sleep a bit (5s) */
    halUSleep(5000000);

    /*---------------------------------------------------------------------------------*/
    /* Record an average pedestal for this ATWD */
    /* Warm up the ATWD */

    #ifdef VERBOSE
    printf("DEBUG: Taking pedestal patterns...\r\n");
    #endif

    /* Initialize the atwd_pedestal array */
    for(j=0; j<cnt; j++)
        atwd_pedestal[j] = 0;

    prescanATWD(trigger_mask);
    
    for (i=0; i<(int)PEDESTAL_TRIG_CNT; i++) {

        /* CPU-trigger the ATWD */
        hal_FPGA_TEST_trigger_forced(trigger_mask);
        
        /* Read out one waveform for the channel requested */
        channels[atwd_channel] = buffer;
        hal_FPGA_TEST_readout(channels[0], channels[1], channels[2], channels[3], 
                              channels[0], channels[1], channels[2], channels[3],
                              cnt, NULL, 0, trigger_mask);
        
        /* Sum the waveform */
        for (j=0; j<cnt; j++)
            atwd_pedestal[j]+=buffer[j];        
    }
    
    /* 
     * Divide the resulting sum waveform by PEDESTAL_TRIG_CNT to get an average
     * waveform.
     */
    for (i=0; i<cnt; i++) atwd_pedestal[i]/=PEDESTAL_TRIG_CNT;
    
    /*---------------------------------------------------------------------------------*/
    /* Take SPE waveforms, searching for 10^7 gain point.  Binary search with help     */
    /* from a rough slope estimate. */

    int gain_found = 0;
    int tries = 0;

    int hv_range_max = HV_FIRST_MAX;
    int hv_range_min = HV_FIRST_MIN;
    *hv_set_volt = (hv_range_max - hv_range_min) / 2 + hv_range_min;
      
    /* Set the histogram max as 5x the target spe_height */
    int hist_max = atwd_spe_height * 5;

    /* Figure out which bin the SPE max should be */
    int target_spe_bin = (atwd_spe_height - HIST_MIN) * HIST_BIN_CNT / (hist_max - HIST_MIN);;

    while ((!gain_found) && (tries < 20)) {

        #ifdef VERBOSE
        printf("DEBUG: Setting PMT HV to %d V\r\n",*hv_set_volt);
        #endif

        /* Change HV to real value and wait 5s */
        halWriteActiveBaseDAC(*hv_set_volt*2);
        halUSleep(5000000);

        /* Find a decent discriminator setting by monitoring the SPE rate while gradually */
        /* increasing the dac setting */
        *spe_disc_dac = SPE_DISC_DAC_MIN;
        int spe_rate;
        
        do {
            halWriteDAC(DOM_HAL_DAC_SINGLE_SPE_THRESH, *spe_disc_dac);
            halUSleep(500000);
            spe_rate = hal_FPGA_TEST_get_spe_rate();
            
            #ifdef VERBOSE
            printf("Set descriminator level to %d -- rate %d\n",*spe_disc_dac,spe_rate);
            #endif
            
            *spe_disc_dac += 1;
            
        } while ((spe_rate > TARGET_SPE_RATE) && !(*spe_disc_dac > SPE_DISC_DAC_MAX));
        
        
        #ifdef VERBOSE
        printf("DEBUG: Taking %d SPE triggers\r\n",pv_trig_cnt);
        #endif

        /* Warm up the ATWD */
        prescanATWD(trigger_mask);
        
        /* Now take the real data */
        for (i=0; i<(int)pv_trig_cnt; i++) {
            
            /* Discriminator-trigger the ATWD */
            hal_FPGA_TEST_trigger_disc(trigger_mask);
            
            /* Read out one waveform for the channel requested */
            channels[atwd_channel] = buffer;
            hal_FPGA_TEST_readout(channels[0], channels[1], channels[2], channels[3], 
                                  channels[0], channels[1], channels[2], channels[3],
                                  cnt, NULL, 0, trigger_mask);
            
            /* Find the peak (after subtracting out pedestal) */
            maxima[i] = buffer[0] - atwd_pedestal[0];
            for (j=0; j<cnt; j++) {
                maxima[i] = ((buffer[j] - atwd_pedestal[j]) > maxima[i]) ?
                    (buffer[j] - atwd_pedestal[j]) : maxima[i];
            }
            
        }

        /*---------------------------------------------------------------------------------*/
        /* Histogram the maxima of each waveform */
        
        /* Initialize histogram */
        for(i=0; i < HIST_BIN_CNT; i++)
            hist_array[i] = 0;
        
        /* Construct histogram */
        int bin;
        for(i=0; i < pv_trig_cnt; i++) {
            
            if (maxima[i] < HIST_MIN)
                bin = 0;
            else if (maxima[i] > hist_max)
                bin = HIST_BIN_CNT-1;
            else
                bin = (maxima[i] - HIST_MIN) * HIST_BIN_CNT / (hist_max - HIST_MIN);
            
            hist_array[bin] += 1;                
        }

        #ifdef VERBOSE
        printf("Histogram: \r\n");
        for(i=0; i < HIST_BIN_CNT; i++) 
            printf("%d %d\r\n",i,hist_array[i]);
        #endif    
        
        /*---------------------------------------------------------------------------------*/
        /* Extract the P/V ratio and the location of the second peak */
        
        int noise_max,valley,spe_max;
        noise_max = spe_max = valley = 0;
        
        int found_noise_max = 0;
        int found_valley = 0;
        int found_spe_max = 0;

        for(i=0; i < HIST_BIN_CNT; i++) {
            
            if (!found_noise_max) {
                if (hist_array[i] > hist_array[noise_max]) {
                    noise_max = i;
                }
                /* Consider the first maximum found if we've */
                /* descended to 1/2 the max value, AND the max */
                /* is over some minimum threshold as a % of the */
                /* total samples taken. */               
                if ((hist_array[i] < 0.5*hist_array[noise_max]) &&
                    (hist_array[noise_max] > 0.05*pv_trig_cnt)) {
                    found_noise_max = 1;
                    valley = noise_max;
                }
            }
            
            if (found_noise_max && !found_valley) {
                if (hist_array[i] < hist_array[valley]) {
                    valley = i;
                }
                /* Consider the first minimum found if we've */
                /* ascended to 1.25 the min value, AND the minimum */
                /* is over some threshold */
                if ((hist_array[i] > 1.25*hist_array[valley]) &&
                    (hist_array[valley] > 0.01*pv_trig_cnt)) {
                    found_valley = 1;
                }
            }

            if (found_valley && !found_spe_max) {
                if (hist_array[i] > hist_array[spe_max]) {
                    spe_max = i;
                }
                /* Consider the second maximum found if we've */
                /* descended to 0.75 the min value, AND the maximum */
                /* is over some threshold. */
                if ((hist_array[i] < 0.75*hist_array[spe_max]) &&
                    (hist_array[spe_max] > 0.0125*pv_trig_cnt)) {
                    found_spe_max = 1;
                }
            }
        }

        *pv_ratio_10 = 0;
        *hist_spe_peak = 0;        

        #ifdef VERBOSE
        printf("DEBUG: noise_max %d valley %d spe_max %d\r\n",noise_max,valley,spe_max);
        #endif
        
        if ((found_noise_max*found_valley*found_spe_max) != 0) {


            *pv_ratio_10 = (hist_array[spe_max]*10)/hist_array[valley];
            *hist_spe_peak = (spe_max * (hist_max - HIST_MIN) / HIST_BIN_CNT) + HIST_MIN;

            /* Adjust HV based on this peak location */
            if (spe_max == target_spe_bin) {
                /* We're done! */
                gain_found = 1;
            }
            else if (spe_max < target_spe_bin) {
                hv_range_min = *hv_set_volt;
            }
            else {
                hv_range_max = *hv_set_volt;
            }

            #ifdef VERBOSE
            printf("DEBUG: new range %d-%d\r\n",hv_range_min,hv_range_max);
            #endif

            *hv_set_volt = (hv_range_max - hv_range_min) / 2 + hv_range_min;
        }   
        else {
            #ifdef VERBOSE
            printf("Error extracting max/min!\r\n");
            #endif

            /* Adjust HV, but we didn't get a peak */
            /* Try somewhere else in range */
            *hv_set_volt = (hv_range_max - hv_range_min) * ((float) rand() / RAND_MAX)
                + hv_range_min;
        }
        
        tries++;

    } /* End HV loop */

    /*---------------------------------------------------------------------------------*/
    
    /* Turn the HV off */
    halDisablePMT_HV();

    /* Free allocated structures */
    free(buffer);
    free(maxima);
    free(atwd_pedestal);

    /* Is the P/V ratio too low? */
    if ((*pv_ratio_10 / 10.0) < MIN_PV_RATIO) {
        return FALSE;
    }
    else {
        return TRUE;
    }
}
