#include <stddef.h>
#include <stdlib.h>

#include "hal/DOM_MB_hal.h"
#include "hal/DOM_MB_fpga.h"

#include "stf-apps/atwdUtils.h"

/* reverse an atwd waveform in time...
 */
void reverseATWDWaveform(short *p) {
   int i;
   for (i=0; i<128/2; i++) {
      const short t = p[i];
      p[i] = p[127-i];
      p[127-i] = t;
   }
}

void reverseATWDIntWaveform(unsigned *p) {
   int i;
   for (i=0; i<128/2; i++) {
      const unsigned t = p[i];
      p[i] = p[127-i];
      p[127-i] = t;
   }
}

void prescanATWD(unsigned trigger_mask) {
   int i;

   /* azriel recommends to throw away a few atwd captures first...
    */
   for (i=0; i<8; i++) {
      hal_FPGA_TEST_trigger_forced(trigger_mask);
      while (!hal_FPGA_TEST_readout_done(trigger_mask)) ;
      halUSleep(1000);
   }
}

void lookupPulserRate(int repetition_rate, DOM_HAL_FPGA_PULSER_RATES *rate, 
		      unsigned *hz) {  
   const struct {
      DOM_HAL_FPGA_PULSER_RATES val;
      float rate;
   } rates[] = {
      { .val = DOM_HAL_FPGA_PULSER_RATE_78k, .rate = 78e3 },
      { .val = DOM_HAL_FPGA_PULSER_RATE_39k, .rate = 39e3 },
      { .val = DOM_HAL_FPGA_PULSER_RATE_19_5k, .rate = 19.5e3 },
      { .val = DOM_HAL_FPGA_PULSER_RATE_9_7k, .rate = 9.7e3 },
      { .val = DOM_HAL_FPGA_PULSER_RATE_4_8k, .rate = 4.8e3 },
      { .val = DOM_HAL_FPGA_PULSER_RATE_2_4k, .rate = 2.4e3 },
      { .val = DOM_HAL_FPGA_PULSER_RATE_1_2k, .rate = 1.2e3 },
      { .val = DOM_HAL_FPGA_PULSER_RATE_0_6k, .rate = 0.6e3 }
   };
   const int nrates = sizeof(rates)/sizeof(rates[0]);
   int i, idx = 0;
   float dist = 
      (rates[idx].rate-repetition_rate) * (rates[idx].rate-repetition_rate);
   
   for (i=1; i<nrates; i++) {
      const float d = 
	 (rates[i].rate-repetition_rate) * (rates[i].rate-repetition_rate);
      
      if (d<dist) {
	 idx = i;
	 dist = d;
      }
   }
   if (rate!=NULL) *rate = rates[idx].val;
   if (hz!=NULL) *hz = (int) rates[idx].rate;
}

/* calculate nominal spe dac setting...
 */
int speDACNominal(float disc_mv, int pedestal_dac) {
   return (int)
      (disc_mv*9.6*(2240+249)/249.0 + pedestal_dac*5000.0/4096.0);
}

void getSummedWaveform(int loop_count, unsigned trigger_mask, int channel,
		       unsigned *waveform) {
   int i;
   const int cnt = 128;
   short *buffer = (short *) calloc(cnt, sizeof(short));
   short *ch[4] = { NULL, NULL, NULL, NULL };
   
   /* set proper channel */
   ch[channel] = buffer;
   
   /* 1) */
   for (i=0; i<(int)loop_count; i++) {
      int j;
      
      hal_FPGA_TEST_trigger_forced(trigger_mask);
      
      hal_FPGA_TEST_readout(ch[0], ch[1], ch[2], ch[3],
			    ch[0], ch[1], ch[2], ch[3],
			    cnt, NULL, 0, trigger_mask);

      /* get summed waveform... */
      for (j=0; j<cnt; j++) waveform[j]+=buffer[j];
   }

   for (i=0; i<cnt; i++) waveform[i]/=loop_count;

   reverseATWDIntWaveform(waveform);

   free(buffer);
}
