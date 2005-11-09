/* atwd_pedestal_spe.c, skeleton file created by gendir
 */
#include <stdlib.h>

#include "stf/stf.h"
#include "stf-apps/atwd_pedestal_spe.h"

#include "stf-apps/atwdUtils.h"

#include "hal/DOM_MB_hal.h"
#include "hal/DOM_MB_fpga.h"

BOOLEAN atwd_pedestal_speInit(STF_DESCRIPTOR *d) {
   return TRUE;
}

BOOLEAN atwd_pedestal_speEntry(STF_DESCRIPTOR *d,
                    unsigned atwd_sampling_speed_dac,
                    unsigned atwd_ramp_top_dac,
                    unsigned atwd_ramp_bias_dac,
                    unsigned atwd_analog_ref_dac,
                    unsigned atwd_pedestal_dac,
                    unsigned atwd_ch0_clamp,
                    unsigned atwd_chip_a_or_b,
                    unsigned atwd_channel,
                    unsigned loop_count,
                    unsigned *atwd_pedestal_amplitude,
                    unsigned *atwd_pedestal_pattern,
                    unsigned *atwd_spe_disc_threshold_dac) {
   const int ch = (atwd_chip_a_or_b) ? 0 : 4;
   int i;
   int spe_dac_nominal;
   int step;
   unsigned minv, maxv;
   const int cnt = 128;
   short *buffer = (short *) calloc(cnt, sizeof(short));
   short *channels[4] = { NULL, NULL, NULL, NULL };
   int trigger_mask = (atwd_chip_a_or_b) ? 
      HAL_FPGA_TEST_TRIGGER_ATWD0 : HAL_FPGA_TEST_TRIGGER_ATWD1;

   /* pretest checks: */

   /* 1) all five atwd dac settings are programmed...
    */
   halWriteDAC(ch, atwd_sampling_speed_dac);
   halWriteDAC(ch+1, atwd_ramp_top_dac);
   halWriteDAC(ch+2, atwd_ramp_bias_dac);
   halWriteDAC(DOM_HAL_DAC_ATWD_ANALOG_REF, atwd_analog_ref_dac);
   halWriteDAC(DOM_HAL_DAC_PMT_FE_PEDESTAL, atwd_pedestal_dac);
   /* FIXME: clamping dac? */

   prescanATWD(trigger_mask);

   /* 2) disable HV */
   halPowerDownBase();

   /* 3) get spe_dac_nominal */
   spe_dac_nominal = speDACNominal(0, atwd_pedestal_dac);

   /* 4) scan spe_dac... */
   step = 0.005*spe_dac_nominal;
   if (step<=0) step = 1;
   *atwd_spe_disc_threshold_dac = 0;
   for (i=(int)(spe_dac_nominal*0.95); 
	i<= (int)(spe_dac_nominal*1.05); i+=step) {
      halWriteDAC(DOM_HAL_DAC_SINGLE_SPE_THRESH, i);
      hal_FPGA_TEST_trigger_disc(trigger_mask);
      halUSleep(1000);
      if (hal_FPGA_TEST_readout_done(trigger_mask)) {
	 hal_FPGA_TEST_clear_trigger();
	 *atwd_spe_disc_threshold_dac = i;
	 break;
      }
      hal_FPGA_TEST_clear_trigger();
   }

   /* return error if we couldn't trigger... */
   if (*atwd_spe_disc_threshold_dac == 0) return FALSE;
   
   for (i=0; i<(int)loop_count; i++) {
      int j;
      
      /* B. The ATWD trigger mask is written according to 
       * ATWD_TRIG_FORCED_OR_SPE
       */
      hal_FPGA_TEST_trigger_disc(trigger_mask);

      /* D.  Take one waveform for the channel requested...
       */
      channels[atwd_channel] = buffer;
      hal_FPGA_TEST_readout(channels[0], channels[1], channels[2], channels[3], 
			    channels[0], channels[1], channels[2], channels[3],
			    cnt, NULL, 0, trigger_mask);

      /* get summed waveform... */
      for (j=0; j<cnt; j++) atwd_pedestal_pattern[j]+=buffer[j];

      /* E. repeat...
       */
   }

   reverseATWDIntWaveform(atwd_pedestal_pattern);

   /* F. divide the resulting sum waveform by LOOP_COUNT to get an average
    * waveform.
    */
   for (i=0; i<cnt; i++) atwd_pedestal_pattern[i]/=loop_count;
   
   /* H. Analyze pedestal waveform:
    *
    *  - obtain maximum and minimum value of the average pedestal waveform...
    *  - set pass to:
    *      -> minimum value of average pedestal waveform > 0
    *      -> maximum value of average pedestal waveform < 1023
    *      -> maximum-minimum < 20
    */
   minv = maxv = atwd_pedestal_pattern[0];
   for (i=1; i<cnt; i++) {
      if (atwd_pedestal_pattern[i]<minv) minv=atwd_pedestal_pattern[i];
      else if (atwd_pedestal_pattern[i]>maxv) maxv=atwd_pedestal_pattern[i];
   }

   *atwd_pedestal_amplitude = maxv-minv;

   free(buffer);
   
   return minv>0 && maxv<1023 && (maxv-minv)<60;
}




