/*
 *
 * STF Test -- Flasherboard current pulse amplitude test
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
#include <math.h>

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

/* Number of brightness points */
#define N_BRIGHTS                 10

/* Maximum brightness setting */
#define FB_MAX_BRIGHTNESS        127

/* Pass/fail defines */
/* Maximum allowed deviation of each point from linear fit */
#define MAX_ERR_PCT                5

/* Minimum current peak in ATWD units at maximum brightness */
/* About 90% of nominal value of 400 */
#define MIN_PEAK_MAX_BRIGHT      360 

/* Rounding convert to int */
#define round(x) ((x)>=0?(int)((x)+0.5):(int)((x)-0.5))

/*
 * Linear regression of arrays of integers
 *
 * Takes an array of (x,y) points and returns the 
 * slope, intercept, and R-squared value of the best-fit
 * line.
 *
 */
void linearFitInt(int *x, int *y, int pts, float *m, float *b, float *rsqr) {

    int i;
    long sum_x, sum_y;
    long sum_xy, sum_xx, sum_yy;
    float denom;

    sum_x = sum_y = 0;
    sum_xy = sum_xx = sum_yy = 0;

    for (i=0; i<pts; i++) {        
        sum_x  += x[i];
        sum_xx += x[i]*x[i];
        sum_y  += y[i];        
        sum_yy += y[i]*y[i];
        sum_xy += x[i]*y[i];     
    }

    denom = (float)(pts*sum_xx - sum_x*sum_x);
    *m = (float)(pts*sum_xy - sum_x*sum_y) / denom;
    *b = (float)(sum_xx*sum_y - sum_x*sum_xy) / denom;
    *rsqr = (float)(pts*sum_xy - sum_x*sum_y) * (*m) / 
        (pts*sum_yy - sum_y*sum_y);
}

BOOLEAN flasher_brightnessInit(STF_DESCRIPTOR *desc) { return TRUE; }

BOOLEAN flasher_brightnessEntry(STF_DESCRIPTOR *desc,
                                unsigned int atwd_chip_a_or_b,
                                unsigned int flasher_width,
                                unsigned int led_trig_cnt,
                                char ** flasher_id,
                                unsigned int * max_current_err_pct,
                                unsigned int * worst_linearity_brightness,
                                unsigned int * worst_linearity_led,
                                unsigned int * min_peak_brightness_atwd,
                                unsigned int * worst_brightness_led,
                                unsigned int * led_avg_current
                             ) {

    int i,j,trig;
    const int ch = (atwd_chip_a_or_b) ? 0 : 4;
    const int cnt = 128;
    int trigger_mask = (atwd_chip_a_or_b) ? 
        HAL_FPGA_TEST_TRIGGER_ATWD0 : HAL_FPGA_TEST_TRIGGER_ATWD1;

    /* Default return values */
    *worst_linearity_led = *worst_linearity_brightness = *max_current_err_pct = 0;
    *min_peak_brightness_atwd = *worst_brightness_led = 0;

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

    /* Peaks in current wavesforms */
    int peaks[N_BRIGHTS];
    int widths[N_BRIGHTS];

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
    /* Cycle through each brightness and  LED -- measure current as each one flashes */

    int brights[7];
    int led;
    float pred;
    int err_pct;

    #ifdef VERBOSE
    printf("Setting pulse width to %d\n", flasher_width);
    #endif

    hal_FB_set_pulse_width(flasher_width);

    for (led = 0; led < N_LEDS; led++) {
                
        #ifdef VERBOSE
        printf("Enabling LED %d\n", (led+1));
        #endif
        hal_FB_enable_LEDs(1 << led);
        
        /* Select which LED current to send from the flasherboard (encoded) */
        hal_FB_select_mux_input(DOM_FB_MUX_LED_1 + led);

        for (i = 0; i < N_BRIGHTS; i++) {
            /* Want to hit maximum on last setting always */
            brights[i] = FB_MAX_BRIGHTNESS - 
                ((FB_MAX_BRIGHTNESS / N_BRIGHTS) * (N_BRIGHTS-1-i));
            
            #ifdef VERBOSE
            printf("Setting brightness to %d\n", brights[i]);
            #endif
            
            hal_FB_set_brightness(brights[i]);

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
            
            /* Extract true maximum (including spike) of current pulse */
            int ampl;
            peaks[i] = 0;
            for (j=0; j<cnt; j++) {
                /* Invert so that current pulse is positive */
                ampl = currents[led][0] - currents[led][j];
                peaks[i] = (ampl > peaks[i]) ? ampl : peaks[i];
            }
            
            /* Extract the half-max width of the current pulse */
            /* The amplitude should be something reasonable */
            int rise = 0;
            if (peaks[i] > 10) {
                for (j=0; j<cnt; j++) {
                    ampl = currents[led][0] - currents[led][j];
                    if ((rise == 0) && (ampl > 0.5*peaks[i])) {
                        rise = j;
                    }
                    else if ((rise > 0) && (ampl < 0.5*peaks[i])) {
                        widths[i] = j - rise;
                        break;
                    }            
                }
            }
            else {
                widths[i] = 0;
            }
            
            /* Refine peak as average between width -- washes out spike */
            /* Uses the fact that rise time is very fast */
            int peak_sum = 0;
            if (widths[i] > 0) {
                for (j=0; j<cnt; j++) {
                    /* Intentionally used < instead of <= */
                    if ((j > rise) && (j < rise+widths[i])) {
                        peak_sum += currents[led][0] - currents[led][j];
                    }
                }
                peaks[i] = peak_sum / (widths[i]-1);
            }

            /* Print the waveform */
            #ifdef VERBOSE
            printf("Current peak (avg): %d\n", peaks[i]);
            printf("Current width: %d\n", widths[i]);
            for (j=0; j<cnt; j++)
                printf("%d %d %d %d\n", j, currents[led][j], channels[3][j], atwd_pedestal[3][j]);
            #endif
            
            /* Stop flashing */
            hal_FPGA_TEST_stop_FB_flashing();

        } /* End brightness loop */

        /* Fit the brightness / peak relationship */
        float slope, intercept, r_squared;
        linearFitInt(brights, peaks, N_BRIGHTS, &slope, &intercept, &r_squared);

        #ifdef VERBOSE
        printf("Fit for LED %d: slope %f, int %f, r^2 %f\n", led+1, slope, intercept, r_squared);
        #endif
        
        /* Check error of points */
        for (i = 0; i < N_BRIGHTS; i++) {
            
            pred = slope*brights[i] + intercept;
            err_pct = abs(round(((pred - peaks[i]) * 100.0 / pred)));
            
            /* Record worst error */
            if (err_pct > *max_current_err_pct) {                
                *max_current_err_pct = err_pct;
                *worst_linearity_led = led+1;
                *worst_linearity_brightness = brights[i];
                #ifdef VERBOSE
                printf("New linearity error, LED %d: %d pct at brightness %d\n",
                       led+1,*max_current_err_pct,*worst_linearity_brightness);
                #endif
            }
        }

        /* Record minimum brightness at peak setting */
        if (*min_peak_brightness_atwd == 0) {
            *min_peak_brightness_atwd = peaks[N_BRIGHTS-1];
        }
        else {
            if (peaks[N_BRIGHTS-1] < *min_peak_brightness_atwd) {
                *min_peak_brightness_atwd = peaks[N_BRIGHTS-1];
                *worst_brightness_led = led+1;
                #ifdef VERBOSE
                printf("New minimum peak brightness, LED %d: %d\n",
                       led+1, *min_peak_brightness_atwd);
                #endif
            }
        }

    } /* End LED loop */        
        
    /* Turn the flasherboard off */
    hal_FB_disable();

    /* Return waveform average of all LEDs at maximum brightness point */
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
    BOOLEAN passed = TRUE;

    passed  = (*max_current_err_pct > MAX_ERR_PCT) ? FALSE : TRUE;
    passed &= (*min_peak_brightness_atwd > MIN_PEAK_MAX_BRIGHT);

    /* TO DO: Add minimum slope/intercept check? */

    /* Free allocated structures */
    free(atwd_pedestal[3]);
    free(channels[3]);
    for (i=0; i<N_LEDS; i++)
        free(currents[i]);

    return passed;

}