/* atwd_pedestal_noise.c, skeleton file created by gendir
 */
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "hal/DOM_MB_hal.h"
#include "hal/DOM_MB_fpga.h"

#include "stf/stf.h"
#include "stf-apps/atwd_pedestal_noise.h"
#include "stf-apps/atwdUtils.h"

BOOLEAN atwd_pedestal_noiseInit(STF_DESCRIPTOR *d) {
   return TRUE;
}

BOOLEAN atwd_pedestal_noiseEntry(STF_DESCRIPTOR *d,
                    unsigned atwd_sampling_speed_dac,
                    unsigned atwd_ramp_top_dac,
                    unsigned atwd_ramp_bias_dac,
                    unsigned atwd_analog_ref_dac,
                    unsigned atwd_pedestal_dac,
                    unsigned atwd_chip_a_or_b,
                    unsigned atwd_channel,
                    unsigned loop_count,
                    unsigned *noise_positive_max,
                    unsigned *noise_negative_max,
                    unsigned *noise_rms,
                    unsigned *noise_mean,
                    unsigned *error_histogram) {
   
   const int ch = (atwd_chip_a_or_b) ? 0 : 4;
   int i;
   const int cnt = 128;
   short *buffer = (short *) calloc(cnt, sizeof(short));
   unsigned *pattern = (unsigned *) calloc(cnt, sizeof(unsigned));
   short *chs[4] = { NULL, NULL, NULL, NULL };
   int trigger_mask = (atwd_chip_a_or_b) ? 
      HAL_FPGA_TEST_TRIGGER_ATWD0 : HAL_FPGA_TEST_TRIGGER_ATWD1;
   double rms, avg;

   *noise_positive_max = *noise_negative_max = 0;

   /* clear the pattern... */
   memset(error_histogram, 0, sizeof(unsigned)*201);

   /* A. all five atwd dac settings are programmed...
    */
   halWriteDAC(ch, atwd_sampling_speed_dac);
   halWriteDAC(ch+1, atwd_ramp_top_dac);
   halWriteDAC(ch+2, atwd_ramp_bias_dac);
   halWriteDAC(DOM_HAL_DAC_ATWD_ANALOG_REF, atwd_analog_ref_dac);
   halWriteDAC(DOM_HAL_DAC_PMT_FE_PEDESTAL, atwd_pedestal_dac);

   /* warm up the atwd... */
   prescanATWD(trigger_mask);

   /* get average pedestal waveform... */
   for (i=0; i<(int)loop_count; i++) {
      int j;
      
      /* force a cpu trigger... */
      hal_FPGA_TEST_trigger_forced(trigger_mask);

      chs[atwd_channel] = buffer;
      hal_FPGA_TEST_readout(chs[0], chs[1], chs[2], chs[3], 
			    chs[0], chs[1], chs[2], chs[3],
			    cnt, NULL, 0, trigger_mask);
      
      /* get summed waveform... */
      for (j=0; j<cnt; j++) pattern[j]+=buffer[j];
   }
   for (i=0; i<cnt; i++) pattern[i]/=loop_count;

   /* get differences... */
   for (i=0; i<(int)loop_count; i++) {
      int j;
      
      /* force a cpu trigger... */
      hal_FPGA_TEST_trigger_forced(trigger_mask);

      chs[atwd_channel] = buffer;
      hal_FPGA_TEST_readout(chs[0], chs[1], chs[2], chs[3], 
			    chs[0], chs[1], chs[2], chs[3],
			    cnt, NULL, 0, trigger_mask);
      
      /* get differences... */
      for (j=0; j<cnt; j++) {
         const int diff = buffer[j] - pattern[j];

         if (diff > (int) *noise_positive_max) {
            *noise_positive_max = diff;
         }
         else if (-diff > (int) *noise_negative_max) {
            *noise_negative_max = -diff;
         }

         if (diff >= -100 && diff<=200) error_histogram[diff + 100]++;
      }
   }

   /* don't need these anymore... */
   free(buffer);
   free(pattern);

   /* compute statistics... */
   {  double sum=0, sum2=0;
      int n = 0;
   
      for (i=0; i<201; i++) {
         sum += i*error_histogram[i];
         n += error_histogram[i];
      }

      avg = sum/n;
      *noise_mean = (unsigned) (avg*1000);

      for (i=0; i<201; i++) sum2 += error_histogram[i]*((i-avg)*(i-avg));
      rms = sqrt( (1.0/(n-1.0)) * sum2 );
      *noise_rms = (unsigned) (rms*1000);
   }

   return 
      avg > 95 && avg < 105 &&
      rms < 1.5 &&
      *noise_negative_max < 10 &&
      *noise_positive_max < 10;
}
