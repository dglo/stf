/* atwd_led_nopmt.c, skeleton file created by George */

#include <stdlib.h>
#include <math.h>

#include "stf/stf.h"
#include "stf-apps/atwd_led_nopmt.h"

#include "hal/DOM_MB_hal.h"
#include "hal/DOM_MB_fpga.h"

#include "stf-apps/atwdUtils.h"

BOOLEAN atwd_led_nopmtInit(STF_DESCRIPTOR *d) {
   return TRUE;
}

BOOLEAN atwd_led_nopmtEntry(STF_DESCRIPTOR *d,
                    unsigned atwd_sampling_speed_dac,
                    unsigned atwd_ramp_top_dac,
                    unsigned atwd_ramp_bias_dac,
                    unsigned atwd_analog_ref_dac,
                    unsigned atwd_pedestal_dac,
                    unsigned atwd_ch0_clamp,
                    unsigned atwd_chip_a_or_b,
		    unsigned LED_dac,
                    unsigned atwd_mux_bias_dac,
                    unsigned delay_in_ns,
                    unsigned loop_count,
                    unsigned n_pedestal_waveforms,
                    BOOLEAN pedestal_subtraction,
                    unsigned *real_LED_voltage,
                    unsigned *LED_baseline_waveform,
                    unsigned *LED_waveform_width,
                    unsigned *LED_waveform_amplitude,
                    unsigned *LED_waveform_position,
                    unsigned * LED_waveform_pmtLED) {
   const int ch = (atwd_chip_a_or_b) ? 0 : 4;
   int i;
   const int cnt = 128;
   short *channels[4] = { NULL, NULL, NULL, NULL };
   short *buffer1 = (short *) calloc(128, sizeof(short));
   int *sum_waveform_LED = (int *) calloc(128, sizeof(int));
   int trigger_mask = (atwd_chip_a_or_b) ? 
      HAL_FPGA_TEST_TRIGGER_ATWD0 : HAL_FPGA_TEST_TRIGGER_ATWD1;
   float real_delay = delay_in_ns/25.0 - 2;

   /* pretest 1) all five atwd dac settings are programmed... */
   halWriteDAC(ch, atwd_sampling_speed_dac);
   halWriteDAC(ch+1, atwd_ramp_top_dac);
   halWriteDAC(ch+2, atwd_ramp_bias_dac);
   halWriteDAC(DOM_HAL_DAC_ATWD_ANALOG_REF, atwd_analog_ref_dac);
   halWriteDAC(DOM_HAL_DAC_PMT_FE_PEDESTAL, atwd_pedestal_dac);
   halWriteDAC(DOM_HAL_DAC_LED_BRIGHTNESS, LED_dac);
   halWriteDAC(DOM_HAL_DAC_MUX_BIAS, atwd_mux_bias_dac);
   /* FIXME: clamp? */

   /* pretest 2) LED is off */
   halDisableLEDPS();

   /* pretest 3) Enable LED to get the noise */
   hal_FPGA_TEST_enable_LED();
   halSelectAnalogMuxInput(DOM_HAL_MUX_PMT_LED_CURRENT);
   hal_FPGA_TEST_set_atwd_LED_delay((int) real_delay);

   /* clear atwd */
   prescanATWD(trigger_mask);

   /* acquire pedestal baseline for LED*/
   for (i=0; i<cnt; i++) sum_waveform_LED[i] = 0;
   for (i=0; i<n_pedestal_waveforms; i++) {
      int j;
      hal_FPGA_TEST_trigger_LED(trigger_mask);      
      channels[3] = buffer1;
      hal_FPGA_TEST_readout(channels[0], channels[1], channels[2], 
		            channels[3], 
			    channels[0], channels[1], channels[2], 
			    channels[3],
			    cnt, NULL, 0, trigger_mask);
      for (j=0; j<cnt; j++) {
	sum_waveform_LED[j] += buffer1[j];
      }
   }
   for (i=0; i<cnt; i++) {
     sum_waveform_LED[i] /= n_pedestal_waveforms;
   }

   /* 2) set LED dac... */
   halEnableLEDPS();
   halUSleep(1000*2000);
   *real_LED_voltage = (halReadADC(DOM_HAL_ADC_SINGLELED_HV)*17/1000);    /*LED voltage = ADC*2*(150+20)/20*(1/1000) */

   /* wait for dacs, et al... */
   halUSleep(1000*100);

   /* 3) take loop_count waveforms...
    */
   for (i=0; i<cnt; i++) LED_waveform_pmtLED[i] = 0;
   for (i=0; i<loop_count; i++) {
      int j;
      
      hal_FPGA_TEST_trigger_LED(trigger_mask);      
      channels[3] = buffer1;
      hal_FPGA_TEST_readout(channels[0], channels[1], channels[2], 
			    channels[3], 
			    channels[0], channels[1], channels[2], 
			    channels[3],
			    cnt, NULL, 0, trigger_mask);
      for (j=0; j<cnt; j++) {
	LED_waveform_pmtLED[j] += buffer1[j];
      }
   }
      
      for (i=0; i<cnt; i++) {
	LED_waveform_pmtLED[i] /= loop_count;
      }

   /* 6), 7) */
   if (pedestal_subtraction) {
      *LED_baseline_waveform = 100;
      
      for (i=0; i<cnt; i++) {
	 const int v1 = (int) LED_waveform_pmtLED[i];
	 const int v2 = (int) sum_waveform_LED[i];
	 const int v_LED = v1 - v2 + 100;
	 LED_waveform_pmtLED[i] = (v_LED<0) ? 0 : v_LED;
      }
   }
   else {
      *LED_baseline_waveform = 0;
      for (i=0; i<cnt; i++) {
	*LED_baseline_waveform += sum_waveform_LED[i];
      }
      *LED_baseline_waveform /= cnt;
   }
   
   free(sum_waveform_LED);

   /*turn off LED*/
   halDisableLEDPS();

   /* 8) reverse waveform */
   reverseATWDIntWaveform(LED_waveform_pmtLED);
     
   /* 9) find max index */
   {   int LED_maxIdx =0;
       unsigned LED_maxValue = LED_waveform_pmtLED[LED_maxIdx];
       int LED_half_max, LED_hmxIdx = 127, LED_hmnIdx = 0;
       
       for (i=1; i<128; i++) {
	  if (LED_maxValue < LED_waveform_pmtLED[i]) {
	     LED_maxIdx = i;
	     LED_maxValue = LED_waveform_pmtLED[i];
	  }
       }

       /* 10) */
       *LED_waveform_amplitude = LED_maxValue - *LED_baseline_waveform;

       /* 11) */
       LED_half_max = 
	 (*LED_waveform_amplitude)/2 + *LED_baseline_waveform;

       for (i=LED_maxIdx; i<cnt; i++) {
	  if (LED_waveform_pmtLED[i]<LED_half_max) {
	     LED_hmxIdx = i;
	     break;
	  }
       }
       for (i=LED_maxIdx; i>=0; i--) {
	  if (LED_waveform_pmtLED[i]<LED_half_max) {
	     LED_hmnIdx = i;
	     break;
	  }
       }

       /* 14 */
       *LED_waveform_width = LED_hmxIdx - LED_hmnIdx;
       *LED_waveform_position = LED_maxIdx + 1;
   }

   free(buffer1);
   return       
     ((LED_dac==0 && *real_LED_voltage>=16 && *real_LED_voltage<=20) || (LED_dac==1023 && *real_LED_voltage>=0 && *real_LED_voltage<=3));
}
