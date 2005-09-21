/* atwd_baseline_shift.c, skeleton file created by gendir
 */
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "stf/stf.h"
#include "stf-apps/atwd_baseline_shift.h"

#include "hal/DOM_MB_hal.h"
#include "hal/DOM_MB_fpga.h"

#include "stf-apps/atwdUtils.h"

BOOLEAN atwd_baseline_shiftInit(STF_DESCRIPTOR *d) {
   return TRUE;
}

static void captureAll(short **waveform, int n, int us, int channel,
                       int trigger_mask) {
   const int cnt = 128;
   int i;
   short *ch[4] = { NULL, NULL, NULL, NULL };

   /* 1) */
   for (i=0; i<n; i++) {
      ch[channel] = waveform[i];
      
      halUSleep(us);
      
      hal_FPGA_TEST_trigger_forced(trigger_mask);
      
      hal_FPGA_TEST_readout(ch[0], ch[1], ch[2], ch[3],
			    ch[0], ch[1], ch[2], ch[3],
			    cnt, NULL, 0, trigger_mask);
   }
}

static void analyzeAll(double *pedestal, short **waveform, int n,
                       unsigned *mn, unsigned *rms, unsigned *min,
                       unsigned *max, unsigned *m) {
   int i, j;
   double sumy = 0, sum2 = 0;
   double maxdw = 0, mindw = 0;
   double sumx = 0, sum2x = 0, sumxy = 0;
   
   for (i=0; i<n; i++) 
      for (j=0; j<128; j++) {
         double dw = waveform[i][j] - pedestal[j];
         sumy += waveform[i][j];
         sum2 += dw*dw;
         sumx += j;
         sum2x += j*j;
         sumxy += j*waveform[i][j];
         if (dw>maxdw) maxdw = dw;
         else if (dw<mindw) mindw = dw;
      }
   
   *m = (unsigned)
      ( 1000 + 10 * (n*128*sumxy - sumx*sumy) / (n*128*sum2x - sumx*sumx));
   *mn = (unsigned) (1000*sumy/(n*128));
   *rms = (unsigned) (1000 * sqrt( (1.0/(n*128-1)) * sum2 ));
   *min = (unsigned) (-mindw*1000);
   *max = (unsigned) (1000*maxdw);
}

BOOLEAN atwd_baseline_shiftEntry(STF_DESCRIPTOR *d,
                    unsigned atwd_sampling_speed_dac,
                    unsigned atwd_ramp_top_dac,
                    unsigned atwd_ramp_bias_dac,
                    unsigned atwd_analog_ref_dac,
                    unsigned atwd_pedestal_dac,
                    unsigned atwd_chip_a_or_b,
                    unsigned atwd_channel,
                    unsigned spe_discriminator_uvolt,
                    unsigned loop_count,
                    unsigned skip,
                    unsigned *atwd_baseline_mean,
                    unsigned *atwd_baseline_rms,
                    unsigned *atwd_baseline_min,
                    unsigned *atwd_baseline_max,
                    unsigned *atwd_baseline_slope) {
   const int ch = 
      (atwd_chip_a_or_b) ? 
      DOM_HAL_DAC_ATWD0_TRIGGER_BIAS : DOM_HAL_DAC_ATWD1_TRIGGER_BIAS;
   int i, j;
   double *meanv = (double *) calloc(128, sizeof(double));
   const int trigger_mask = (atwd_chip_a_or_b) ?
      HAL_FPGA_TEST_TRIGGER_ATWD0 : HAL_FPGA_TEST_TRIGGER_ATWD1;
   short **wv = (short **) calloc(loop_count, sizeof(short *));

   /* A. all five atwd dac settings are programmed...
    */
   halWriteDAC(ch, atwd_sampling_speed_dac);
   halWriteDAC(ch+1, atwd_ramp_top_dac);
   halWriteDAC(ch+2, atwd_ramp_bias_dac);
   halWriteDAC(DOM_HAL_DAC_ATWD_ANALOG_REF, atwd_analog_ref_dac);
   halWriteDAC(DOM_HAL_DAC_PMT_FE_PEDESTAL, atwd_pedestal_dac);

   /* allocate waveforms... */
   for (i=0; i<loop_count; i++) wv[i] = (short *) calloc(128, sizeof(short));

   /* C. if the SPE trigger was requested, calculate the SPE DAC that
    * corresponds to SPE_DISCRIMINATOR_UVOLT and program it...
    */
   prescanATWD(trigger_mask);

   captureAll(wv, loop_count, 0, atwd_channel, trigger_mask);
   
   /* get mean waveform... */
   for (i=0; i<128; i++) meanv[i] = 0;
   for (j=0; j<loop_count; j++) 
      for (i=0; i<128; i++) 
         meanv[i] += wv[j][i];
   for (i=0; i<128; i++) meanv[i]/=loop_count;

   analyzeAll(meanv, wv, loop_count, 
              atwd_baseline_mean,
              atwd_baseline_rms,
              atwd_baseline_min,
              atwd_baseline_max,
              atwd_baseline_slope);
   
   for (i=1; i<100; i++) {
      prescanATWD(trigger_mask);

      captureAll(wv, loop_count, i*skip, atwd_channel, trigger_mask);
      
      analyzeAll(meanv, wv, loop_count, 
                 atwd_baseline_mean + i,
                 atwd_baseline_rms + i,
                 atwd_baseline_min + i,
                 atwd_baseline_max + i,
                 atwd_baseline_slope + i);
   }

   for (i=0; i<loop_count; i++) free(wv[i]);
   free(wv);
   free(meanv);
   
   return TRUE; 
}




