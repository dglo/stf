/* atwd_pmt_led.c, skeleton file created by George
 */

#include <stdlib.h>
#include <math.h>

#include "stf/stf.h"
#include "stf-apps/atwd_pmt_led.h"

#include "hal/DOM_MB_hal.h"
#include "hal/DOM_MB_fpga.h"

#include "stf-apps/atwdUtils.h"

BOOLEAN atwd_pmt_ledInit(STF_DESCRIPTOR *d) {
   return TRUE;
}

BOOLEAN atwd_pmt_ledEntry(STF_DESCRIPTOR *d,
                    unsigned atwd_sampling_speed_dac,
                    unsigned atwd_ramp_top_dac,
                    unsigned atwd_ramp_bias_dac,
                    unsigned atwd_analog_ref_dac,
                    unsigned atwd_pedestal_dac,
                    unsigned atwd_ch0_clamp,
                    unsigned atwd_chip_a_or_b,
                    unsigned atwd_channel,
                    unsigned pmt_hv_high_volt,
                    unsigned LED_dac,
                    unsigned atwd_mux_bias_dac,
                    unsigned delay_in_ns,
                    unsigned loop_count,
                    unsigned n_pedestal_waveforms,
                    BOOLEAN pedestal_subtraction,
                    unsigned *real_hv_output,
                    unsigned *real_LED_voltage,
                    unsigned *atwd_baseline_waveform,
                    unsigned *atwd_waveform_width,
                    unsigned *atwd_waveform_amplitude,
                    unsigned *atwd_waveform_position,
                    unsigned *atwd_waveform_pmtLED,
                    unsigned *LED_baseline_waveform,
                    unsigned *LED_waveform_width,
                    unsigned *LED_waveform_amplitude,
                    unsigned *LED_waveform_position,
                    unsigned *LED_waveform_pmtLED,
                    unsigned *light_pulse_count) {
   const int ch = (atwd_chip_a_or_b) ? 0 : 4;
   int i, pmt_dac, count=0;
   const int cnt = 128;
   short *channels[4] = { NULL, NULL, NULL, NULL };
   short *buffer1 = (short *) calloc(128, sizeof(short));
   short *buffer2 = (short *) calloc(128, sizeof(short));
   short *light_pulse_waveform = (short *) calloc(128, sizeof(short));
   int *sum_waveform_LED = (int *) calloc(128, sizeof(int));
   int *sum_waveform_PMT = (int *) calloc(128, sizeof(int));
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

   /* pretest 2) pmt and LED is off */
   halPowerDownBase();
   halDisableLEDPS();

   /* pretest 3) Enable LED to get the noise */
   halPowerUpBase();
   hal_FPGA_TEST_enable_LED();
   halSelectAnalogMuxInput(DOM_HAL_MUX_PMT_LED_CURRENT);
   hal_FPGA_TEST_set_atwd_LED_delay((int) real_delay);

   /* clear atwd */
   prescanATWD(trigger_mask);

   /* acquire pedestal baseline for LED and pmt */
   for (i=0; i<cnt; i++) sum_waveform_LED[i] = 0;
   for (i=0; i<cnt; i++) sum_waveform_PMT[i] = 0;
   for (i=0; i<n_pedestal_waveforms; i++) {
      int j;
      hal_FPGA_TEST_trigger_LED(trigger_mask);      
      channels[3] = buffer1;
      channels[atwd_channel] = buffer2;      
      hal_FPGA_TEST_readout(channels[0], channels[1], channels[2], 
			    channels[3], 
			    channels[0], channels[1], channels[2], 
			    channels[3],
			    cnt, NULL, 0, trigger_mask);
      for (j=0; j<cnt; j++) {
	sum_waveform_LED[j] += buffer1[j];
	sum_waveform_PMT[j] += buffer2[j];
      }
   }
   for (i=0; i<cnt; i++) {
     sum_waveform_LED[i] /= n_pedestal_waveforms;
     sum_waveform_PMT[i] /= n_pedestal_waveforms;
   }

   /* 2) set LED and pmt dac... */
   halEnableLEDPS();
   halUSleep(1000*2000);
   *real_LED_voltage = (halReadADC(DOM_HAL_ADC_SINGLELED_HV)*17/1000);    /*LED voltage = ADC*2*(150+20)/20*(1/1000) */
   pmt_dac = pmt_hv_high_volt*2;
   halEnableBaseHV();
   halWriteActiveBaseDAC(pmt_dac);
   halUSleep(1000*2000);
   *real_hv_output = halReadBaseADC()/2;

   /* wait for dacs, et al... */
   halUSleep(1000*100);

   /* 3) take loop_count waveforms...
    */
   for (i=0; i<cnt; i++) atwd_waveform_pmtLED[i] = 0;
   for (i=0; i<cnt; i++) LED_waveform_pmtLED[i] = 0;
   for (i=0; i<cnt; i++) light_pulse_waveform[i] = 0;
   for (i=0; i<loop_count; i++) {
      int j, k, l;
      
      hal_FPGA_TEST_trigger_LED(trigger_mask);      
      channels[3] = buffer1;
      channels[atwd_channel] = buffer2;      
      hal_FPGA_TEST_readout(channels[0], channels[1], channels[2], 
			    channels[3], 
			    channels[0], channels[1], channels[2], 
			    channels[3],
			    cnt, NULL, 0, trigger_mask);
      for (j=0; j<cnt; j++) {
	LED_waveform_pmtLED[j] += buffer1[j];
	atwd_waveform_pmtLED[j] += buffer2[j];
      }
      /*Count light pulse*/
      if (pedestal_subtraction) {
	*atwd_baseline_waveform = 100;
	
	for (k=0; k<cnt; k++) {
	  const int a = (int) buffer2[k];
	  const int b = (int) sum_waveform_PMT[k];
	  const int c = a-b+100;
	  light_pulse_waveform[k] = (c<0) ? 0 : c;
	}
	
	for (l=2; l<cnt; l++) {
	  if ((light_pulse_waveform[l]-light_pulse_waveform[l-2])<-10 && 
	      (light_pulse_waveform[l-1]-light_pulse_waveform[l-2])<-10)
	    {
	      count++;
	      break;
	    }
	}
      }
   }
      free(light_pulse_waveform);
      *light_pulse_count = count;
      
      for (i=0; i<cnt; i++) {
	LED_waveform_pmtLED[i] /= loop_count;
	atwd_waveform_pmtLED[i] /= loop_count;
      }

   /* 6), 7) */
   if (pedestal_subtraction) {
      *atwd_baseline_waveform = 100;
      *LED_baseline_waveform = 100;
      
      for (i=0; i<cnt; i++) {
	 const int v1 = (int) LED_waveform_pmtLED[i];
	 const int v2 = (int) sum_waveform_LED[i];
	 const int v_LED = v1 - v2 + 100;
	 LED_waveform_pmtLED[i] = (v_LED<0) ? 0 : v_LED;
	 const int v3 = (int) atwd_waveform_pmtLED[i];
	 const int v4 = (int) sum_waveform_PMT[i];
	 const int v_PMT = v3 - v4 + 100;
	 atwd_waveform_pmtLED[i] = (v_PMT<0) ? 0 : v_PMT;
      }
   }
   else {
      *atwd_baseline_waveform = 0;
      *LED_baseline_waveform = 0;
      for (i=0; i<cnt; i++) {
	*LED_baseline_waveform += sum_waveform_LED[i];
	*atwd_baseline_waveform += sum_waveform_PMT[i];
      }
      *LED_baseline_waveform /= cnt;
      *atwd_baseline_waveform /= cnt;
   }
   
   free(sum_waveform_LED);
   free(sum_waveform_PMT);

   /*turn off PMT and LED*/
   halPowerDownBase();
   halDisableLEDPS();

   /* 8) reverse waveform */
   reverseATWDIntWaveform(atwd_waveform_pmtLED);
   reverseATWDIntWaveform(LED_waveform_pmtLED);

   /* 9) find max index */
   {   int LED_maxIdx =0, pmt_maxIdx =0;
       unsigned LED_maxValue = LED_waveform_pmtLED[LED_maxIdx];
       unsigned pmt_maxValue = atwd_waveform_pmtLED[pmt_maxIdx];
       int LED_half_max, LED_hmxIdx = 127, LED_hmnIdx = 0, pmt_half_max, pmt_hmxIdx = 127, pmt_hmnIdx = 0;
       
       for (i=1; i<128; i++) {
	  if (LED_maxValue < LED_waveform_pmtLED[i]) {
	     LED_maxIdx = i;
	     LED_maxValue = LED_waveform_pmtLED[i];
	  }
	  if (pmt_maxValue < atwd_waveform_pmtLED[i]) {
	     pmt_maxIdx = i;
	     pmt_maxValue = atwd_waveform_pmtLED[i];
	  }
       }

       /* 10) */
       *LED_waveform_amplitude = LED_maxValue - *LED_baseline_waveform;
       *atwd_waveform_amplitude = pmt_maxValue - *atwd_baseline_waveform;

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

       pmt_half_max = 
	 (*atwd_waveform_amplitude)/2 + *atwd_baseline_waveform;

       for (i=pmt_maxIdx; i<cnt; i++) {
	  if (atwd_waveform_pmtLED[i]<pmt_half_max) {
	     pmt_hmxIdx = i;
	     break;
	  }
       }
       for (i=pmt_maxIdx; i>=0; i--) {
	  if (atwd_waveform_pmtLED[i]<pmt_half_max) {
	     pmt_hmnIdx = i;
	     break;
	  }
       }

       /* 14 */
       *LED_waveform_width = LED_hmxIdx - LED_hmnIdx;
       *atwd_waveform_width = pmt_hmxIdx - pmt_hmnIdx;

       *LED_waveform_position = LED_maxIdx + 1;
       *atwd_waveform_position = pmt_maxIdx + 1;
   }

   free(buffer1);
   free(buffer2);
   return       
     *real_hv_output > 0.95*pmt_hv_high_volt &&
     *real_hv_output < 1.05*pmt_hv_high_volt &&
     ((LED_dac==0 && *light_pulse_count>=(loop_count*0.95)) || (LED_dac==1023 && *light_pulse_count<=(loop_count*0.05))) &&
     ((LED_dac==0 && *atwd_waveform_amplitude>880) || (LED_dac==1023 && *atwd_waveform_amplitude<2)) &&
     ((LED_dac==0 && *LED_waveform_amplitude>60) || (LED_dac==1023 && *LED_waveform_amplitude<16)) &&
     ((LED_dac==1023) || (LED_dac==0 && *LED_waveform_position>=38 && *LED_waveform_position<=44));
}
