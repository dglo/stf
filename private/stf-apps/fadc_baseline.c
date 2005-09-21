/* fadc_baseline.c, skeleton file created by gendir
   code added by Eric G. Henson eghenson@lbl.gov                                
 */
#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#include "stf/stf.h"
#include "stf-apps/fadc_baseline.h"

#include "hal/DOM_MB_hal.h"
#include "hal/DOM_MB_fpga.h"

BOOLEAN fadc_baselineInit(STF_DESCRIPTOR *d) {
   return FALSE;
}

BOOLEAN fadc_baselineEntry(STF_DESCRIPTOR *d,
                    unsigned fadc_reference_dac,
                    unsigned atwd_pedestal_dac,
                    unsigned long loop_count,
                    BOOLEAN fill_output_arrays,
                    unsigned *fadc_baseline_mean,
                    unsigned *fadc_baseline_rms,
                    unsigned *fadc_baseline_min,
                    unsigned *fadc_baseline_max,
                    unsigned * fadc_baseline_histogram) {

  unsigned baseline;
  unsigned baseline_max,baseline_min;
  unsigned baseline_sum,baseline_sqr;
  unsigned baseline_mean,baseline_rms;
  unsigned count;
  unsigned lp1,lp2;
  int temp;
  float tmp_float;
  unsigned waveform[256];
  unsigned histogram[1024];

  /*  A.Pretest checks:*/ 
  /*    1.The two input DAC settings are programmed.*/
  halWriteDAC(DOM_HAL_DAC_FAST_ADC_REF,fadc_reference_dac);
  halWriteDAC(DOM_HAL_DAC_PMT_FE_PEDESTAL,atwd_pedestal_dac);
  /*    2.If HV base is connected HV is set to 0.  */
  /* Always assume base is installed */
  halSetPMT_HV(0);


  /*    B.Test algorithm: */
  /*    1.Take one waveform for the FADC with FORCED (CPU) trigger */
  /*begining loop as per step 4, all sums set to zero*/
  baseline_sum=0;
  baseline_sqr=0;
  baseline_max=0;
  baseline_min=1024;
  for(lp1=0;lp1<loop_count;lp1++)
    {
      histogram[lp1]=0;
    }
  for(lp2=0;lp2<loop_count;lp2++)
    {
           hal_FPGA_TEST_trigger_forced(HAL_FPGA_TEST_TRIGGER_FADC);
           hal_FPGA_TEST_readout(NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,256,waveform,0,HAL_FPGA_TEST_TRIGGER_FADC);
  /*    2.Calculate the mean of all the 256 samples values (integers ok, range [0-1023]):
          this is the baseline for this waveform.*/
	   count=0;	   
	   for(lp1=0;lp1<256;lp1++)
	   {
	     count+=waveform[lp1];
           } 
	   baseline=count/256;
  /*    3.Keep a sum of all baselines, and a sum of the squares for RMS calculation.
          Also keep the minimum and maximum baselines.*/
           baseline_sum+=baseline;
           baseline_sqr+=(baseline * baseline);
	   if(baseline>baseline_max) baseline_max = baseline;
           if(baseline<baseline_min) baseline_min = baseline;
  /*    4.Repeat from step 1, LOOP_COUNT times, keeping a histogram
	(it is a 1024-bins histogram) of the baselines obtained.*/
	   if(baseline>=0 && baseline<1024) histogram[baseline]+=1;
    }
  /*    5.If FILL_OUTPUT_ARRAYS=1, fill the output array FADC_BASELINE_HISTOGRAM with the baseline distribution.*/
  if(fill_output_arrays)
    {
      for(lp1=0;lp1<256;lp1++)
      {
	fadc_baseline_histogram[lp1] = histogram[lp1];
      }
    }
  /*    6.Compute the mean and the RMS using the running sums (integer arithmetic is ok).*/
  baseline_mean = baseline_sum / loop_count;
  baseline_rms = sqrt(baseline_sqr);
  /*    7.Fill output variables FADC_BASELINE_MEAN, FADC_BASELINE_RMS, FADC_BASELINE_MIN, FADC_BASELINE_MAX*/
  *fadc_baseline_mean = baseline_mean; 
  *fadc_baseline_rms = baseline_rms;
  *fadc_baseline_min = baseline_min;
  *fadc_baseline_max = baseline_max;


  /*    C.Success criteria: */
  /*    1.Set TEST_PASS_NOPASS to pass value if ALL of the following are true:*/
  /*	2.Baseline mean value is within 80 counts of its nominal value (* See Note).
          Here's how to calculate the nominal value (need floating point or workarounds to get the full precision):
          ((FADC_REFERENCE_DAC*2.5/1023- ATWD_PEDESTAL_DAC*5.0/4095)+1)/2*1023 */
  tmp_float = (((fadc_reference_dac*2.5)/1023.0 - (atwd_pedestal_dac*5.0)/4095.0)+1.0)/(2046.0);
  temp = (int)(floor(tmp_float));
  if(abs((int)baseline_mean-temp)>80) return FALSE;

  /*    3.Baseline mean value > 100*/
  if(baseline_mean<100) return FALSE;
  /*    4.Baseline mean value < 250*/
  if(baseline_mean>250) return FALSE;
  /*    5.Maximum-Minimum < 5*/
  if(baseline_max-baseline_min>5) return FALSE;
  /*    6.Baseline RMS < 3*/
  if(baseline_rms>3) return FALSE;

  return TRUE;
}


  /*Notes: 
    1. The value of the parts is good to ~1% and naively the expected deviation of the baseline value from the nominal should be much smaller than 80 counts. However tests mshow that for an expected baseline of 184 counts we get around 130 (for V2 boards). This was explained as being due to a synchronous clock pickup in the inputs that produces a fixed offset at the output.*/
