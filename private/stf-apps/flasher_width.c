/*
 *
 * STF Test -- Flasherboard current pulse width test
 *
 * - Cycles through all LEDs on the flasherboard,
 *   capturing their current waveforms with ATWD
 *   channel 3.
 *
 * - Measures pulse widths at every setting and 
 *   makes sure every width within a given range,
 *   about 1ns/bin, is covered.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "stf/stf.h"
#include "hal/DOM_MB_hal.h"
#include "stf-apps/atwdUtils.h"

/* ATWD DAC settings */
#define ATWD_SAMPLING_SPEED_DAC 4095 /* Non-standard! */
#define ATWD_RAMP_TOP_DAC       2097
#define ATWD_RAMP_BIAS_DAC      2800 /* Non-standard! */
#define ATWD_ANALOG_REF_DAC     2048
#define ATWD_PEDESTAL_DAC       1925

/* Offset for current measurement */
#define ATWD_FLASHER_REF         450

/* ATWD-LED trigger offset delay */
#define ATWD_LED_DELAY            7

/* Number of pedestals to average */
#define PEDESTAL_TRIG_CNT        100

/* Number of LEDs */
#define N_LEDS                    12

/* Maximum pulse width setting */
#define FB_MAX_WIDTH             127

/* Pass/fail defines */
/* Width in ATWD units here is approximately same in ns */
#define FB_MIN_ATWD_WIDTH          7
#define FB_MAX_ATWD_WIDTH         50

/* Rounding convert to int */
#define round(x) ((x)>=0?(int)((x)+0.5):(int)((x)-0.5))

BOOLEAN flasher_widthInit(STF_DESCRIPTOR *desc) { return TRUE; }

BOOLEAN flasher_widthEntry(STF_DESCRIPTOR *desc,
                           unsigned int atwd_chip_a_or_b,
                           unsigned int flasher_brightness,
                           unsigned int led_trig_cnt,
                           char ** flasher_id,
                           unsigned int * missing_width,
                           unsigned int * failing_led,
                           unsigned int * led_avg_current
                           ) {
    
    int i,j,trig;
    const int ch = (atwd_chip_a_or_b) ? 0 : 4;
    const int cnt = 128;
    int trigger_mask = (atwd_chip_a_or_b) ? 
        HAL_FPGA_TEST_TRIGGER_ATWD0 : HAL_FPGA_TEST_TRIGGER_ATWD1;

    /* Default return values */
    *failing_led = *missing_width = 0;

    /* Pedestal buffers -- only use channel 3 in this test */
    int *atwd_pedestal[4] = {NULL,
                             NULL,
                             NULL,
                             (int *) malloc(cnt*sizeof(int))};

    /* We only use channel 3 in this test */
    short *channels[4] = {NULL,
                          NULL,
                          NULL,
                          (short *) calloc(cnt, sizeof(short))};

    /* Average current waveform for each LED */
    int *currents[N_LEDS];
    for (i=0; i<N_LEDS; i++) {
        currents[i] = (int *) malloc(cnt*sizeof(int));
        if (currents[i] == NULL)
            return FALSE;
    }

    /* Check mallocs */
    if ((atwd_pedestal[3] == NULL) || (channels[3] == NULL))
        return FALSE;

    /*---------------------------------------------------------------------------------*/
    /* Make sure PMT is off */
    halPowerDownBase();

    /* Initialize the flasherboard and power up */
    hal_FB_enable();

    /* Read the flasherboard ID */
    /* Not malloc'ed by STF */
    static char id[20];
    strcpy(id, hal_FB_get_serial());
    *flasher_id = id;

    #ifdef VERBOSE
    printf("Flasher board ID = %s\n", *flasher_id);
    #endif

    /*---------------------------------------------------------------------------------*/
    /* Record an average pedestal for this ATWD */

    /* Set up the ATWD DAC values */
    halWriteDAC(ch, ATWD_SAMPLING_SPEED_DAC);
    halWriteDAC(ch+1, ATWD_RAMP_TOP_DAC);
    halWriteDAC(ch+2, ATWD_RAMP_BIAS_DAC);
    halWriteDAC(DOM_HAL_DAC_ATWD_ANALOG_REF, ATWD_ANALOG_REF_DAC);
    halWriteDAC(DOM_HAL_DAC_PMT_FE_PEDESTAL, ATWD_PEDESTAL_DAC);   
    halWriteDAC(DOM_HAL_DAC_FL_REF, ATWD_FLASHER_REF);

    /* Set the trigger offset delay */
    hal_FPGA_TEST_set_atwd_LED_delay(ATWD_LED_DELAY);

    /* Select the LED current as the ATWD analog mux input */
    halSelectAnalogMuxInput(DOM_HAL_MUX_FLASHER_LED_CURRENT);

    #ifdef VERBOSE
    printf("DEBUG: Taking pedestal patterns...\r\n");
    #endif

    /* Initialize the atwd_pedestal array */
    for(j=0; j<cnt; j++)
        atwd_pedestal[3][j] = 0;

    /* Warm up the ATWD */
    prescanATWD(trigger_mask);
    
    for (trig=0; trig<(int)PEDESTAL_TRIG_CNT; trig++) {

        /* CPU-trigger the ATWD */
        hal_FPGA_TEST_trigger_forced(trigger_mask);

        /* Wait for done */
        while (!hal_FPGA_TEST_readout_done(trigger_mask));
        
        /* Read out one waveform for channel 3 */        
        hal_FPGA_TEST_readout(channels[0], channels[1], channels[2], channels[3], 
                              channels[0], channels[1], channels[2], channels[3],
                              cnt, NULL, 0, trigger_mask);
        
        /* Sum the waveform */
        for (j=0; j<cnt; j++)
            atwd_pedestal[3][j]+=channels[3][j];        
    }
    
    /* 
     * Divide the resulting sum waveform by PEDESTAL_TRIG_CNT to get an average
     * waveform.
     */
    for (j=0; j<cnt; j++) 
        atwd_pedestal[3][j]/=PEDESTAL_TRIG_CNT;

    /*---------------------------------------------------------------------------------*/
    /* Cycle through each width and LED -- measure current as each one flashes */

    float widths[FB_MAX_WIDTH+1];
    float peak;
    int led, w;

    #ifdef VERBOSE
    printf("Setting brightness to %d\n", flasher_brightness);
    #endif            
    hal_FB_set_brightness(flasher_brightness);

    for (led = 0; led < N_LEDS; led++) {
                
        #ifdef VERBOSE
        printf("Enabling LED %d\n", (led+1));
        #endif
        hal_FB_enable_LEDs(1 << led);
        
        /* Select which LED current to send from the flasherboard (encoded) */
        hal_FB_select_mux_input(DOM_FB_MUX_LED_1 + led);

        for (w = 0; w <= FB_MAX_WIDTH; w++) {

            #ifdef VERBOSE
            printf("Setting pulse width to %d\n", w);
            #endif

            hal_FB_set_pulse_width(w);
            
            /* Initialize the current waveform array */
            for(j=0; j<cnt; j++)
                currents[led][j] = 0;
            
            #ifdef VERBOSE
            printf("DEBUG: Taking %d flasherboard triggers using LED %d\r\n",led_trig_cnt, led+1);
            #endif

            /* Start flashing */
            hal_FPGA_TEST_start_FB_flashing();
            
            /* Warm up the ATWD */
            prescanATWD(trigger_mask);
            
            /* Now take the real data */
            for (trig=0; trig<(int)led_trig_cnt; trig++) {
                
                /* LED-trigger the ATWD */
                hal_FPGA_TEST_trigger_LED(trigger_mask);

                /* Wait for done */
                while (!hal_FPGA_TEST_readout_done(trigger_mask));                

                /* Read out one waveform of channel 3 */
                hal_FPGA_TEST_readout(channels[0], channels[1], channels[2], channels[3], 
                                      channels[0], channels[1], channels[2], channels[3],
                                      cnt, NULL, 0, trigger_mask);
                
                /* Subtract pedestals and add to average waveform*/
                for (j=0; j<cnt; j++)
                    currents[led][j] += (channels[3][j] - atwd_pedestal[3][j]);            
            }            
            
            /* Average the current waveform */
            for (j=0; j<cnt; j++)
                currents[led][j] /= (int)led_trig_cnt;

            /* Multi-stage pulse width determination: */
            /*   - find spike amplitude               */
            /*   - find first-pass pulse width        */
            /*   - find average peak amplitude        */
            /*   - find final, interpolated width     */

            /* Admittedly, this multi-pass business is cumbersome, but this */
            /* gives us sub-ns resolution of pulse width */
            
            /* Extract true maximum (including spike) of current pulse */
            int ampl;
            peak = 0;
            for (j=0; j<cnt; j++) {
                /* Invert so that current pulse is positive */
                ampl = currents[led][127] - currents[led][j];
                peak = (ampl > peak) ? ampl : peak;
            }
            
            /* Extract the half-max width of the current pulse */
            /* The amplitude should be something reasonable */
            int rise = 0;
            if (peak > 10) {
                for (j=0; j<cnt; j++) {
                    ampl = currents[led][127] - currents[led][j];
                    if ((rise == 0) && (ampl > 0.5*peak)) {
                        rise = j;
                    }
                    else if ((rise > 0) && (ampl < 0.5*peak)) {
                        widths[w] = j - rise;
                        break;
                    }            
                }
            }
            else {
                widths[w] = 0;
            }
            
            /* Refine, if we have a real pulse -- find average amplitude */
            /* Instead of spike peak, by averaging between pulse width */
            /* Uses the fact that rise time is reasonably fast */
            float peak_sum = 0;
            if (widths[w] > 0) {
                for (j=0; j<cnt; j++) {
                    /* Intentionally used < instead of <= */
                    if ((j > rise) && (j < rise+widths[w])) {
                        peak_sum += currents[led][127] - currents[led][j];
                    }
                }
                peak = (float)peak_sum / (round(widths[w] - 1));
            }

            int last_ampl = 0;
            float rise_f, fall_f;
            rise = 0;
            rise_f = fall_f = 0;

            /* Interpolate for final width measurement */
            if (peak > 10) {
                for (j=0; j<cnt; j++) {
                    ampl = currents[led][127] - currents[led][j];
                    if ((rise == 0) && (ampl > 0.5*peak) && (j > 0)) {
                        rise = j;
                        rise_f = (float)(0.5*peak - last_ampl) / (ampl - last_ampl) + (j-1);
                    }
                    else if ((rise > 0) && (ampl < 0.5*peak)) {
                        fall_f = (float)(0.5*peak - last_ampl) / (ampl - last_ampl) + (j-1);
                        break;
                    }
                    last_ampl = ampl;
                }
                widths[w] = fall_f - rise_f;

            }
            else {
                widths[w] = 0;
            }
            

            #ifdef VERBOSE
            printf("Current peak (average): %.1f\n", peak);
            printf("Current width: %.2f\n", widths[w]);
            for (j=0; j<cnt; j++)
                printf("%d %d %d %d\n", j, currents[led][j], channels[3][j], atwd_pedestal[3][j]);
            #endif
            
            /* Stop flashing */
            hal_FPGA_TEST_stop_FB_flashing();

        } /* End width loop */

        /* Check to make sure all widths are covered by some setting */
        int w_ref, found = 0;
        for (w_ref = FB_MIN_ATWD_WIDTH; w_ref <= FB_MAX_ATWD_WIDTH; w_ref++) {
            for (w = 0; w <= FB_MAX_WIDTH; w++) {
                if ((int)widths[w] == w_ref) {
                    found = 1;
                    break;
                }
            }
            if (found == 0) {
                #ifdef VERBOSE
                printf("Found missing width: led %d, width %d\n",led+1,w_ref);
                #endif
                *failing_led = led+1;
                *missing_width = w_ref;
            }
        }
        
    } /* End LED loop */        
        
    /* Turn the flasherboard off */
    hal_FB_disable();

    /* Return waveform average of all LEDs at maximum width point */
    /* This is merely for reference -- is not used for pass/fail */
    /* Offset by 1024 since STF doesn't support arrays of signed ints */
    for(j=0; j<cnt; j++)            
        led_avg_current[j] = 0;
   
    for (led=0; led<N_LEDS; led++) {
        for(j=0; j<cnt; j++) {
            led_avg_current[j] += (unsigned int)(currents[led][j] + 1024);
        }
    }
    for(j=0; j<cnt; j++)            
        led_avg_current[j] /= (unsigned int)N_LEDS;

    #ifdef VERBOSE
    printf("Averaged current of all LEDs (offset +1024)\n");
    for(j=0; j<cnt; j++)            
        printf("%d %d\n",j, led_avg_current[j]);
    #endif

    /* Check pass/fail conditions */
    BOOLEAN passed = (*missing_width > 0) ? FALSE : TRUE;

    /* Free allocated structures */
    free(atwd_pedestal[3]);
    free(channels[3]);
    for (i=0; i<N_LEDS; i++)
        free(currents[i]);

    return passed;

}
