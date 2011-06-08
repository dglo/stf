/* atwd_pedestal.c, skeleton file created by gendir
 */
#include <stdlib.h>
#include <string.h>

#include "stf/stf.h"
#include "stf-apps/atwd_pedestal.h"
#include "stf-apps/atwdUtils.h"

#include "hal/DOM_MB_hal.h"
#include "hal/DOM_MB_fpga.h"

BOOLEAN atwd_pedestalInit(STF_DESCRIPTOR *d) {
   return TRUE;
}

BOOLEAN atwd_pedestalEntry(STF_DESCRIPTOR *d,
			   unsigned atwd_sampling_speed_dac,
			   unsigned atwd_ramp_top_dac,
			   unsigned atwd_ramp_bias_dac,
			   unsigned atwd_analog_ref_dac,
			   unsigned atwd_pedestal_dac,
			   unsigned atwd_chip_a_or_b,
			   unsigned atwd_channel,
			   unsigned atwd_trig_forced_or_spe,
			   unsigned spe_descriminator_uvolt,
			   unsigned loop_count,
			   unsigned *atwd_pedestal_amplitude,
			   unsigned *atwd_pedestal_pattern,
			   unsigned *atwd_disc_threshold_dac) {
   const int ch = (atwd_chip_a_or_b) ? 0 : 4;
   int i;
   unsigned minv, maxv;
   const int cnt = 128;
   short *buffer = (short *) calloc(cnt, sizeof(short));
   short *channels[4] = { NULL, NULL, NULL, NULL };
   int trigger_mask = (atwd_chip_a_or_b) ? 
      HAL_FPGA_TEST_TRIGGER_ATWD0 : HAL_FPGA_TEST_TRIGGER_ATWD1;

   /* clear the pattern... */
   memset(atwd_pedestal_pattern, 0, sizeof(unsigned)*128);

   /* A. all five atwd dac settings are programmed...
    */
   halWriteDAC(ch, atwd_sampling_speed_dac);
   halWriteDAC(ch+1, atwd_ramp_top_dac);
   halWriteDAC(ch+2, atwd_ramp_bias_dac);
   halWriteDAC(DOM_HAL_DAC_ATWD_ANALOG_REF, atwd_analog_ref_dac);
   halWriteDAC(DOM_HAL_DAC_PMT_FE_PEDESTAL, atwd_pedestal_dac);

   /* C. if the SPE trigger was requested, calculate the SPE DAC that
    * corresponds to SPE_DISCRIMINATOR_UVOLT and program it...
    */
   if (atwd_trig_forced_or_spe==1) {
      *atwd_disc_threshold_dac = speUVoltToDAC(spe_descriminator_uvolt,
                                               atwd_pedestal_dac);
      halWriteDAC(DOM_HAL_DAC_SINGLE_SPE_THRESH, *atwd_disc_threshold_dac);
   }

   /* warm up the atwd... */
   prescanATWD(trigger_mask);

   for (i=0; i<(int)loop_count; i++) {
      int j;
      
      /* B. The ATWD trigger mask is written according to 
       * ATWD_TRIG_FORCED_OR_SPE
       */
      if (atwd_trig_forced_or_spe==0) {
	 /* forced ... */
	 hal_FPGA_TEST_trigger_forced(trigger_mask);
      }
      else {
	 /* discriminator... */
	 hal_FPGA_TEST_trigger_disc(trigger_mask);
      }

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










