/*
 *
 * STF Test -- Flasherboard current pulse measurement
 *
 * - Cycles through all LEDs on the flasherboard,
 *   capturing their current waveforms with ATWD
 *   channel 3.
 *
 * - The present pass/fail scheme compares the 
 *   pulse amplitude to a reference.  This will be
 *   refined to include pulse width measurements as well.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stf/stf.h"
#include "hal/DOM_MB_hal.h"
#include "stf-apps/atwdUtils.h"

/* ATWD DAC settings */
#define ATWD_SAMPLING_SPEED_DAC 850
#define ATWD_RAMP_TOP_DAC       2097
#define ATWD_RAMP_BIAS_DAC      2800 /* Non-standard! */
#define ATWD_ANALOG_REF_DAC     2048
#define ATWD_PEDESTAL_DAC       1925

/* Offset for current measurement */
#define ATWD_FLASHER_REF         450

/* ATWD-LED trigger offset delay */
#define ATWD_LED_DELAY            4

/* Number of pedestals to average */
#define PEDESTAL_TRIG_CNT        100

/* Number of LEDs */
#define N_LEDS                    12

/* Pass/fail defines */

/* Reference linear relationship between ATWD reading */
/* of current and brightness DAC -- to be refined */
#define CURRENT_SLOPE           3.17
#define CURRENT_OFFSET          32.8

/* Maximum allowed deviation from reference */
/* Will be refined */
#define MAX_ERR_PCT               25

BOOLEAN flasher_currentInit(STF_DESCRIPTOR *desc) { return TRUE; }

BOOLEAN flasher_currentEntry(STF_DESCRIPTOR *desc,
                             unsigned int atwd_chip_a_or_b,
                             unsigned int flasher_brightness,
                             unsigned int flasher_width,
                             unsigned int led_trig_cnt,
                             unsigned int * predicted_current,
                             unsigned int * max_current_err,
                             unsigned int * worst_led,
                             unsigned int * led_avg_current
                             ) {

    int i,j,trig;
    const int ch = (atwd_chip_a_or_b) ? 0 : 4;
    const int cnt = 128;
    int trigger_mask = (atwd_chip_a_or_b) ? 
        HAL_FPGA_TEST_TRIGGER_ATWD0 : HAL_FPGA_TEST_TRIGGER_ATWD1;

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

    /* Minima in current wavesforms */
    int *peaks = (int *) malloc(N_LEDS*sizeof(int));

    /* Check mallocs */
    if ((atwd_pedestal[3] == NULL) || (channels[3] == NULL) || (peaks == NULL))
        return FALSE;

    /*---------------------------------------------------------------------------------*/
    /* Make sure PMT is off */
    halPowerDownBase();

    /* Initialize the flasherboard and power up */
    hal_FB_enable();

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
    /* Cycle through each LED -- measure the current as each one flashes */

    #ifdef VERBOSE
    printf("Setting pulse width to %d\n", flasher_width);
    printf("Setting brightness to %d\n", flasher_brightness);
    #endif

    hal_FB_set_pulse_width(flasher_width);
    hal_FB_set_brightness(flasher_brightness);

    int led;
    for (led = 0; led < N_LEDS; led++) {

        /* Initialize the current waveform array */
        for(j=0; j<cnt; j++)
            currents[led][j] = 0;

        #ifdef VERBOSE
        printf("Enabling LED %d\n", (led+1));
        #endif
        hal_FB_enable_LEDs(1 << led);
        
        /* Select which LED current to send from the flasherboard (encoded) */
        hal_FB_select_mux_input(DOM_FB_MUX_LED_1 + led);

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

        /* Extract height of current pulse */
        unsigned int ampl;
        peaks[led] = 0;
        for (j=0; j<cnt; j++) {
            ampl = abs(currents[led][j] - currents[led][0]);
            peaks[led] = (ampl > peaks[led]) ? ampl : peaks[led];
        }

        /* Print the waveform */
        #ifdef VERBOSE
        printf("Current peak amplitude: %d\n", peaks[led]);
        for (j=0; j<cnt; j++)
            printf("%d %d %d %d\n", j, currents[led][j], channels[3][j], atwd_pedestal[3][j]);
        #endif

        /* Stop flashing */
        hal_FPGA_TEST_stop_FB_flashing();
                
    }

    /* Turn the flasherboard off */
    hal_FB_disable();

    /* Return waveform average of all LEDs */
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
    int err;
    BOOLEAN passed = TRUE;
    *max_current_err = 0;
    *predicted_current = CURRENT_SLOPE*flasher_brightness + CURRENT_OFFSET;

    for(led=0; led<N_LEDS; led++) {

        err = *predicted_current - peaks[led];

        if (abs(err) > *max_current_err) {
            *max_current_err = abs(err);
            *worst_led = led+1;
        }
        
        if (abs(err) > ((float)MAX_ERR_PCT / 100 * *predicted_current))
            passed = FALSE;
    }

    /* Free allocated structures */
    free(atwd_pedestal[3]);
    free(channels[3]);
    free(peaks);
    for (i=0; i<N_LEDS; i++)
        free(currents[i]);

    return passed;

}
