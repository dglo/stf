/* atwd_pulser_spe.c, skeleton file created by gendir
 */
#include <stdlib.h>
#include <math.h>

#include "stf/stf.h"
#include "stf-apps/atwd_pulser_spe.h"

#include "hal/DOM_MB_hal.h"
#include "hal/DOM_MB_fpga.h"

#include "stf-apps/atwdUtils.h"

BOOLEAN atwd_pulser_speInit(STF_DESCRIPTOR *d) {
   return TRUE;
}

BOOLEAN atwd_pulser_speEntry(STF_DESCRIPTOR *d,
                    unsigned atwd_sampling_speed_dac,
                    unsigned atwd_ramp_top_dac,
                    unsigned atwd_ramp_bias_dac,
                    unsigned atwd_analog_ref_dac,
                    unsigned atwd_pedestal_dac,
                    unsigned atwd_ch0_clamp,
                    unsigned atwd_chip_a_or_b,
                    unsigned atwd_channel,
                    unsigned pulser_amplitude_uvolt,
                    BOOLEAN pedestal_subtraction,
                    unsigned n_pedestal_waveforms,
                    unsigned loop_count,
                    unsigned *triggerable_spe_dac,
                    unsigned *atwd_baseline_waveform,
                    unsigned *atwd_waveform_width,
                    unsigned *atwd_waveform_amplitude,
                    unsigned *atwd_waveform_position,
                    unsigned *atwd_expected_amplitude,
                    unsigned *atwd_waveform_pulser_spe) {
   const int ch = (atwd_chip_a_or_b) ? 0 : 4;
   int i;
   DOM_HAL_FPGA_PULSER_RATES rate;
   const int cnt = 128;
   short *channels[4] = { NULL, NULL, NULL, NULL };
   short *buffer = (short *) calloc(128, sizeof(short));
   int *sum_waveform = (int *) calloc(128, sizeof(int));
   int trigger_mask = (atwd_chip_a_or_b) ? 
      HAL_FPGA_TEST_TRIGGER_ATWD0 : HAL_FPGA_TEST_TRIGGER_ATWD1;
   int spe_dac_nominal, fe_pulser_dac;

   /* pretest 1) all five atwd dac settings are programmed... */
   halWriteDAC(ch, atwd_sampling_speed_dac);
   halWriteDAC(ch+1, atwd_ramp_top_dac);
   halWriteDAC(ch+2, atwd_ramp_bias_dac);
   halWriteDAC(DOM_HAL_DAC_ATWD_ANALOG_REF, atwd_analog_ref_dac);
   halWriteDAC(DOM_HAL_DAC_PMT_FE_PEDESTAL, atwd_pedestal_dac);
   /* FIXME: clamp? */

   /* pretest 2) hv is off */
   halDisablePMT_HV();

   /* pretest 3) fe pulser dac gets zero */
   halWriteDAC(DOM_HAL_DAC_INTERNAL_PULSER, 0);
   
   /* pretest 4) turn on fe pulser */
   lookupPulserRate(78e3, &rate, NULL);
   hal_FPGA_TEST_set_pulser_rate(rate);
   hal_FPGA_TEST_enable_pulser();

   /* pretest 5) find reasonable value of spe disc level... */
   *triggerable_spe_dac = 0;
   spe_dac_nominal = (int) 
      (atwd_pedestal_dac*(5000.0/4096.0)*(1024.0/5000.0));
   for (i=(int) (spe_dac_nominal*1.05); i>=(int) (spe_dac_nominal*0.95); i--) {
   /* for (i=0; i<4096; i++) { */
      /* set spe threshold dac */
      halWriteDAC(DOM_HAL_DAC_SINGLE_SPE_THRESH, i);

      /* wait for counts to show up */
      halUSleep(200*1000);

      /* readout the counts... */
      if (hal_FPGA_TEST_get_spe_rate()>0) {
	 *triggerable_spe_dac = i + 2;
	 break;
      }
   }

   if (*triggerable_spe_dac == 0 ) {
      /* no triggerable value found... */
      *triggerable_spe_dac = (int) spe_dac_nominal*1.05;
      hal_FPGA_TEST_disable_pulser();
      free(buffer);
      /* return FALSE; */
   }
   
   /* pretest 9) set spe_dac */
   halWriteDAC(DOM_HAL_DAC_SINGLE_SPE_THRESH, *triggerable_spe_dac);

   /* pretest 10) turn off fe pulser */
   hal_FPGA_TEST_disable_pulser();

   /* clear atwd */
   prescanATWD(trigger_mask);

   /* acquire pedestal baseline */
   for (i=0; i<cnt; i++) sum_waveform[i] = 0;
   for (i=0; i<n_pedestal_waveforms; i++) {
      int j;
      
      hal_FPGA_TEST_trigger_forced(trigger_mask);
      channels[atwd_channel] = buffer;
      hal_FPGA_TEST_readout(channels[0], channels[1], channels[2], 
			    channels[3], 
			    channels[0], channels[1], channels[2], 
			    channels[3],
			    cnt, NULL, 0, trigger_mask);
      for (j=0; j<cnt; j++) sum_waveform[j] += buffer[j];
   }
   for (i=0; i<cnt; i++) sum_waveform[i] /= n_pedestal_waveforms;

   /* 2) set fe pulser dac... */
   fe_pulser_dac = (int) (pulser_amplitude_uvolt * (25.0/5000.0));
   if (fe_pulser_dac>1023) fe_pulser_dac = 1023;
   halWriteDAC(DOM_HAL_DAC_INTERNAL_PULSER, fe_pulser_dac);
   hal_FPGA_TEST_set_pulser_rate(rate);
   hal_FPGA_TEST_enable_pulser();

   /* wait for dacs, et al... */
   halUSleep(1000*100);

   /* 3) take loop_count waveforms...
    */
   for (i=0; i<cnt; i++) atwd_waveform_pulser_spe[i] = 0;
   for (i=0; i<loop_count; i++) {
      int j;
      
      hal_FPGA_TEST_trigger_disc(trigger_mask);
      channels[atwd_channel] = buffer;
      hal_FPGA_TEST_readout(channels[0], channels[1], channels[2], 
			    channels[3], 
			    channels[0], channels[1], channels[2], 
			    channels[3],
			    cnt, NULL, 0, trigger_mask);
      for (j=0; j<cnt; j++) atwd_waveform_pulser_spe[j] += buffer[j];
   }
   for (i=0; i<cnt; i++) atwd_waveform_pulser_spe[i] /= loop_count;

   /* 6), 7) */
   if (pedestal_subtraction) {
      *atwd_baseline_waveform = 100;
      
      for (i=0; i<cnt; i++) {
	 const int v1 = (int) atwd_waveform_pulser_spe[i];
	 const int v2 = (int) sum_waveform[i];
	 const int v = v1 - v2 + 100;
	 atwd_waveform_pulser_spe[i] = (v<0) ? 0 : v;
      }
   }
   else {
      *atwd_baseline_waveform = 0;
      for (i=0; i<cnt; i++) *atwd_baseline_waveform += sum_waveform[i];
      *atwd_baseline_waveform /= cnt;
   }
   
   free(sum_waveform);

   /* 8) reverse waveform */
   reverseATWDIntWaveform(atwd_waveform_pulser_spe);

   /* 9) find max index */
   {   int maxIdx = 0;
       unsigned maxValue = atwd_waveform_pulser_spe[maxIdx];
       int half_max, hmxIdx = 127, hmnIdx = 0;
       
       for (i=1; i<128; i++) {
	  if (maxValue < atwd_waveform_pulser_spe[i]) {
	     maxIdx = i;
	     maxValue = atwd_waveform_pulser_spe[i];
	  }
       }

       /* 10) */
       *atwd_waveform_amplitude = maxValue - *atwd_baseline_waveform;

       /* 11) */
       half_max = 
	  (*atwd_waveform_amplitude)/2 + *atwd_baseline_waveform;

       for (i=maxIdx; i<cnt; i++) {
	  if (atwd_waveform_pulser_spe[i]<half_max) {
	     hmxIdx = i;
	     break;
	  }
       }
       for (i=maxIdx; i>=0; i--) {
	  if (atwd_waveform_pulser_spe[i]<half_max) {
	     hmnIdx = i;
	     break;
	  }
       }

       /* 14 */
       *atwd_waveform_width = hmxIdx - hmnIdx;

       *atwd_waveform_position = maxIdx + 1;
   }
   
   *atwd_expected_amplitude = (int)
      (pulser_amplitude_uvolt * 40.0/5000.0/pow(8, atwd_channel));

   free(buffer);

   return 
      *atwd_waveform_position > 2 &&
      *atwd_waveform_position < 9 &&
      *atwd_waveform_width > 2 &&
      *atwd_waveform_width < 6;
}










