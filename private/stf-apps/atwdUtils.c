#include <stddef.h>
#include <stdlib.h>
#include <string.h>

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

   /* clear waveform... */
   memset(waveform, 0, cnt*sizeof(unsigned));

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

int speUVoltToDAC(float uv, int pedestal_dac) {
   return (int) 
     (( uv*9.6*(2200+249)/249.0 + pedestal_dac*5e6/4096) * 1024/5e6);
}

int mpeUVoltToDAC(float uv, int pedestal_dac) {
   return  (int)
     ((uv*9.6 + (pedestal_dac*5e6/4096.0)) * 1024/5e6);
}

float speDACToUVolt(int dac, int pedestal_dac) {
   return 249/(9.6*(2200+249)) * (5e6*dac/1024 - pedestal_dac*5e6/4096);
}

float mpeDACToUVolt(int dac, int pedestal_dac) {
   return 1/9.6 * (5e6*dac/1024 - pedestal_dac*5e6/4096);
}

int upper_extreme_rate(int spe_dac_nominal) {
      /* set spe threshold dac */
      halWriteDAC(DOM_HAL_DAC_SINGLE_SPE_THRESH, spe_dac_nominal*1.05+1);
      halUSleep(100000); /* let dac settle */

      /* wait for counts to show up */
      halUSleep(2 * 100 * 1000);
      /* readout the counts... */
      return (int)
        (hal_FPGA_TEST_get_spe_rate());
}

int lower_extreme_rate(int spe_dac_nominal) {
      /* set spe threshold dac */
      halWriteDAC(DOM_HAL_DAC_SINGLE_SPE_THRESH, spe_dac_nominal*0.95-1);
      halUSleep(100000); /* let dac settle */

      /* wait for counts to show up */
      halUSleep(2 * 100 * 1000);
      /* readout the counts... */
      return (int)
        (hal_FPGA_TEST_get_spe_rate());
}


/* find a reasonable value of the spe threshold dac for pulse ... */
int scanSPE(int atwd_pedestal_dac, unsigned *ret, int use_pulser) {
   DOM_HAL_FPGA_PULSER_RATES rate;
   int retv=0, i;
   const int spe_dac_nominal = (int) 
      (atwd_pedestal_dac*(5000.0/4096.0)*(1024.0/5000.0));

  if (use_pulser) {
   /* set internal pulser amplitude to zero... */
   halWriteDAC(DOM_HAL_DAC_INTERNAL_PULSER, 0);

   /* turn it on... */
   lookupPulserRate(78e3, &rate, NULL);
   hal_FPGA_TEST_set_pulser_rate(rate);
   hal_FPGA_TEST_set_scalar_period(DOM_HAL_FPGA_SCALAR_10MS);
   hal_FPGA_TEST_enable_pulser();
   halUSleep(10000);
  }
  else hal_FPGA_TEST_set_scalar_period(DOM_HAL_FPGA_SCALAR_10MS);

#if 0   
   printf("atwd_pedestal_dac: %d, spe_dac_nominal: %d\r\n", 
          atwd_pedestal_dac, spe_dac_nominal);
   
   {
      int ii;
      for (ii=470; ii<570; ii++) {
         int dac = ii;
         halWriteDAC(DOM_HAL_DAC_SINGLE_SPE_THRESH, dac);
         /* let dac and counts settle */
         halUSleep(1000 + 2 * 1000 * 10);
         printf("yikes: dac=%d, counts=%d\r\n", dac, hal_FPGA_TEST_get_spe_rate());
      }
   }
#endif

   *ret = spe_dac_nominal*1.05;  /* assume all zeros... */
   if (upper_extreme_rate(spe_dac_nominal)==0 && lower_extreme_rate(spe_dac_nominal)==0) {
      for (i=(int) (spe_dac_nominal*1.05); i>=(int) (spe_dac_nominal*0.95); i--) {
      unsigned rates;
      
      /* set spe threshold dac */
      halWriteDAC(DOM_HAL_DAC_SINGLE_SPE_THRESH, i);
      halUSleep(1000); /* let dac settle */

      /* wait for counts to show up */
      halUSleep(2 * 10 * 1000);

      /* readout the counts... */
      if ((rates=hal_FPGA_TEST_get_spe_rate())>0) {
         if (i== (int) (spe_dac_nominal*1.05)) {
            /* no zero transition found... */
            /* printf("scanSPE: err: dac=%d, rate=%u\r\n", i, rates); */
            *ret = spe_dac_nominal*1.05;
            retv=1;
            break;
         }
         else if (i== (int) (spe_dac_nominal*0.95)) {
            *ret = spe_dac_nominal*0.95;
            retv=1;
            break;
         }
         else {
            /* printf("scanSPE: ok: dac=%d, rate=%u\r\n", i, rates); */
            *ret = i + 5;
            break;
         }
      }
      /* printf("scanSPE: dac=%d, rate=%u\r\n", i, rates); */
      }
   }
   else if (lower_extreme_rate(spe_dac_nominal)>=0) {
     *ret = spe_dac_nominal*0.95-1;
     retv=1;
   }

   if (use_pulser) hal_FPGA_TEST_disable_pulser();

   return retv;
}

/* find the period of a signal given
 * the (positive only) autocorrelegram...
 *
 * 1) go till we have a zero
 * 2) go while we're increasing...
 * 3) return peak value...
 *
 * return index or -1 on error...
 */
static float findFirstPeak(unsigned *w) {
   int i, prev=0;

   for (i=0; i<128; i++) {
      if (w[i]==0) break;
   }

   if (i==0 || i==128) return -1;
   
   while (i<128 && w[i]>=prev) {
      prev = w[i];
      i++;
   }

   if (i>=127) return -1;

   /* fit a parabola here...
    */
   {  const float y1 = w[i-2];
      const float y2 = w[i-1];
      const float y3 = w[i];
      const float b = (4*y2 - y3 - 3*y1)/2;
      const float a = (-2*y2 + y3 + y1)/2;
      return (i-2) - b/(2*a);
   }
}

/* find period in an atwd waveform
 */
static unsigned findPeriod(unsigned *w) {
   int *mn = (unsigned *) calloc(128, sizeof(unsigned));
   unsigned *cg = (unsigned *) calloc(128, sizeof(unsigned));
   int i, j;
   double sum = 0;
   unsigned period;

   /* first subtract mean...
    */
   for (i=0; i<128; i++) sum += w[i];
   sum/=128;

   for (i=0; i<128; i++) mn[i] = (int)w[i] - (int) sum;

   /* now compute autocorrelagram */
   for (i=0; i<128; i++) {
      int val = 0;
      for (j=0; j<128; j++)
	 /* FIXME: this is wrong, corr can be negative! */
	 val += mn[j]*mn[(i+j)&0x7f];
      cg[i] = (val<0) ? 0 : val;
   }


   period = (unsigned) (findFirstPeak(cg)*20e6);

   free(mn);
   free(cg);
   return period;
}

unsigned calcATWDSamplingSpeed(unsigned *wf, 
                               unsigned trigger_mask,
                               unsigned loop_count) {
   halSelectAnalogMuxInput(DOM_HAL_MUX_OSC_OUTPUT);
   halUSleep(100);
   prescanATWD(trigger_mask);
   getSummedWaveform(loop_count, trigger_mask, 3, wf);
   halSelectAnalogMuxInput(DOM_HAL_MUX_PMT_LED_CURRENT);
   halUSleep(100);
   return findPeriod(wf);
}

unsigned calcATWDSamplingSpeedDAC(int dac_ch, 
                                  unsigned trigger_mask,
                                  int atwd_sampling_speed) {
   unsigned *wf = calloc(128, sizeof(unsigned));
   int d1 = 4096/4, d2 = 4096/2; /* start values... */
   int s1, s2;
   int i;

   halWriteDAC(dac_ch, d1);
   s1 = calcATWDSamplingSpeed(wf, trigger_mask, 100)/1000000;
   halWriteDAC(dac_ch, d2);
   s2 = calcATWDSamplingSpeed(wf, trigger_mask, 100)/1000000;

   for (i=0; i<10; i++) {
      if (s1 == atwd_sampling_speed) { free(wf); return d1; }
      if (s2 == atwd_sampling_speed) { free(wf); return d2; }
      
      if (d1 == d2) { free(wf); return d1; }
      if (s1 == s2) { free(wf); return d1; }
      
      {  const float m = (float)(s2 - s1) / (float)(d2 - d1);
         const int next = (atwd_sampling_speed - s1)/m + d1;

         if (abs(next - d1) < abs(next - d2)) {
            d2 = next;
            halWriteDAC(dac_ch, d2);
            s2 = calcATWDSamplingSpeed(wf, trigger_mask, 100)/1000000;
         }
         else {
            d1 = next;
            halWriteDAC(dac_ch, d1);
            s1 = calcATWDSamplingSpeed(wf, trigger_mask, 100)/1000000;
         }
      }
   }

   /* not converging, return "random" data... */
   free(wf);
   return (d1 + d2) / 2;
}
