/* fadc_fe_forced.c, skeleton file created by gendir
   code added by Eric G. Henson  eghenson@lbl.gov                                                                            */
#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#include "stf/stf.h"
#include "stf-apps/fadc_fe_forced.h"

#include "hal/DOM_MB_hal.h"
#include "hal/DOM_MB_fpga.h"


BOOLEAN fadc_fe_forcedInit(STF_DESCRIPTOR *d) {
   return TRUE;
}

BOOLEAN fadc_fe_forcedEntry(STF_DESCRIPTOR *d,
                    unsigned fadc_reference_dac,
                    unsigned atwd_pedestal_dac,
                    unsigned long pulser_amplitude_uvolt,
                    unsigned long loop_count,

                    unsigned *fadc_baseline_mean,
                    unsigned *fadc_fe_pulser_amplitude,
                    unsigned *fadc_fe_pulser_width,
                    unsigned *fadc_fe_pulser_position,
                    unsigned * fadc_fe_pulser_waveform) {

  unsigned baseline,baseline_sum,baseline_mean,waveform_mean;
  unsigned temp,pos,amp;
  unsigned lp1,lp2;
  unsigned half_max,start_width,end_width;
  int time_out;

  short *waveform = (short *) calloc(512, sizeof(short));
  int *waveform_sum = (int *) calloc(512, sizeof(int));
  int *waveform_avg = (int *) calloc(512, sizeof(int));

  /*  A.Pretest checks: */

  /*    1.If HV base is connected HV is set to 0.*/
  /* Always assume base is installed */
  halPowerDownBase();

  /*    B.Test algorithm: */
  /*    1.The two input DAC settings are programmed.*/
  /*  printf("%p,%p,%p,%p\n\r",waveform,waveform_sum,waveform_avg,fadc_fe_pulser_waveform);*/
         halWriteDAC(DOM_HAL_DAC_FAST_ADC_REF,fadc_reference_dac);
	 halUSleep(200000);
           halWriteDAC(DOM_HAL_DAC_PMT_FE_PEDESTAL,atwd_pedestal_dac);
         halUSleep(200000);
  /*    2.Set the Front-End pulser amplitude DAC to zero.*/
           halWriteDAC(DOM_HAL_DAC_INTERNAL_PULSER,0);
         halUSleep(200000);
           hal_FPGA_TEST_disable_pulser();

	   baseline_sum = 0;
       for(lp2=0;lp2<10;lp2++)
         {
  /*    3.Take one waveform for the FADC with FORCED (CPU) trigger (pulser Off)*/ 
           hal_FPGA_TEST_trigger_forced(HAL_FPGA_TEST_TRIGGER_FADC);
    time_out = hal_FPGA_TEST_readout(NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,0,waveform,512,HAL_FPGA_TEST_TRIGGER_FADC);
  /*    4.Calculate the mean of all the 512 samples values (integers ok, range [0-1023]):
	  this is the baseline for this waveform.*/ 
              waveform_mean = 0;
          for(lp1=0;lp1<512;lp1++)
            {
              waveform_mean += (unsigned int)waveform[lp1];
            }
              waveform_mean /=512;
              baseline = waveform_mean;
  /*	5.Keep a sum of all baselines.*/
           baseline_sum += baseline;
  /*    6.Repeat from step 3, 10  times.*/
	 }
       /*     printf("baseline sum = %d\n\r",baseline_sum);*/
  /*    7.Compute the baseline mean using the running sum (integer arithmetic is ok).*/
           baseline_mean = baseline_sum/10;

  /*	8.Fill the FADC_BASELINE_MEAN output variable.*/
       *fadc_baseline_mean = baseline_mean;

  /*    9.Set Front End Pulser DAC to deliver the signal level given in PULSER_AMPLITUDE_UVOLT using the following:
          Front-End-Pulser-DAC= PULSER_AMPLITUDE_UVOLT*25/5000
	  (this uses the ad-hoc knowledge that 25 DAC units produce a 5 mV signal at the front end).*/
       temp = (pulser_amplitude_uvolt*25)/5000;
       halWriteDAC(DOM_HAL_DAC_INTERNAL_PULSER,temp);
       halUSleep(100000);

       for(lp1=0;lp1<512;lp1++)
         {
           waveform_sum[lp1] = 0;
         } 
        for(lp2=0;lp2<loop_count;lp2++)
         {
 /*    10.Take an FADC waveform with a CPU trigger and at the same time
        (on the same register write operation) turn ON the pulser.  */
       /* ***************** may need new command ********************* */
                hal_FPGA_TEST_trigger_forced(HAL_FPGA_TEST_TRIGGER_FADC | HAL_FPGA_TEST_TRIGGER_FE_PULSER); 
		/*  hal_FPGA_TEST_trigger_forced(HAL_FPGA_TEST_TRIGGER_FADC);*/
	time_out = hal_FPGA_TEST_readout(NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,0,waveform,512,HAL_FPGA_TEST_TRIGGER_FADC);
  /*    11.Keep a sum-waveform to obtain the average waveform
        (that means 512 individual sums, one for each waveform sample).*/ 
       for(lp1=0;lp1<512;lp1++)
	 {
	   waveform_sum[lp1] += (unsigned)waveform[lp1];

	 }

  /*    12.Turn off the pulser and reset the CPU trigger (important in order to make synchronous waveform).*/
       hal_FPGA_TEST_disable_pulser();
       /* *****************  reset cpu trigger? *****************  */

  /*    13.Repeat from step 10 LOOP_COUNT times.  */
	 }
	/*	printf("waveform_sum = %d\n\r",waveform_sum[1]);*/

  /*    14.Calculate the average waveform:
           for each sample in the waveform offset its baseline by adding 100 and subtracting FADC_BASELINE_MEAN.
           This makes a baseline-subtracted waveform with a baseline at 100 counts
           (good for plotting and for negative going pulses).
           If a negative sample value is obtained set to zero.  */
	for(lp1=0;lp1<512;lp1++)
	  {
	    temp = waveform_sum[lp1] / loop_count;
	    if(100+temp < (*fadc_baseline_mean))  waveform_avg[lp1]=0;
	    else  waveform_avg[lp1] = (100+temp) - (*fadc_baseline_mean);
	  }
	/*printf("calculated average\n\r");*/
  /*    15.If requested, fill the FADC_FE_PULSER_WAVEFORM output array with the baseline subtracted average waveform.*/

	     for(lp1=0;lp1<512;lp1++)
	      {
		fadc_fe_pulser_waveform[lp1] = waveform_avg[lp1];
		}
	     /*  printf("output array filled\n\r");*/

  /*    16.Find the position of the largest sample in the average waveform.
           Fill FADC_FE_PULSER_POSITION with that position (a number between 1 and 512).
           If the maximum value appears in more that one sample, report and keep the first
           (smaller-and earliest- sample number). */
	amp = 0;
	pos = 0;
	for(lp1=0;lp1<512;lp1++)
	  {
	    if(waveform_avg[lp1]>amp)
	      {
		amp = waveform_avg[lp1];
		pos = lp1;
	      }
	  }
	    *fadc_fe_pulser_position = pos;

  /*    17.Compute the FADC_FE_PULSER_AMPLITUDE = Maximum sample value in average waveform -100.  */
	    if(amp>100) *fadc_fe_pulser_amplitude = amp-100;
	    else *fadc_fe_pulser_amplitude = 0;

  /*    18.Compute the half-maximum amplitude as FADC_FE_PULSER_AMPLITUDE/2 + 100.*/
	half_max = (*fadc_fe_pulser_amplitude/2) + 100;
 
  /*    19.Start at the position of the maximum sample and move up until the first sample with
           a value less than the half-maximum amplitude is found. Save that position.
           If no value if found to meet this condition save the maximum position=512.*/
	pos = 512;
	if((*fadc_fe_pulser_position) >=0 && (*fadc_fe_pulser_position)<512)
	for(lp1= (*fadc_fe_pulser_position);lp1<512;lp1++)
	  {
	    if(waveform_avg[lp1] < half_max) {pos = lp1;break;}	  
	  }
	end_width = pos;
	/*printf("step 19 done\n\r");*/
  /*    20.Start at the position of the maximum sample and move down until the first sample with
           a value less than the half-maximum amplitude is found. Save that position.
           If no value if found to meet this condition save the minimum position=1.*/
	pos = 1;
        if((*fadc_fe_pulser_position) >1 && (*fadc_fe_pulser_position)<512)
	for(lp1= (*fadc_fe_pulser_position);lp1>0;lp1--)
	  {
	    if(waveform_avg[lp1] < half_max) {pos = lp1;break;}
	  }
        start_width = pos;
	/*	printf("step 20 done\n\r");*/

  /*    21.Compute FADC_FE_PULSER_WIDTH = result from 19 - result from step 20.*/
	*fadc_fe_pulser_width = end_width - start_width;

	free(waveform);
	free(waveform_sum);
	free(waveform_avg);
	/*printf("freed mem\n\r");*/
  /*    C.Success criteria: */
  /*    1.Set TEST_PASS_NOPASS to pass value if ALL of the following are true:*/
  /*    2.5 < FADC_FE_PULSER_POSITION < 10. (need to review this limit  with a real board)*/
           if(*fadc_fe_pulser_position<6 || *fadc_fe_pulser_position >9) return FALSE;
  /*    3.FADC_FE_PULSER_AMPLITUDE is within 50% of its nominal value of: PULSER_AMPLITUDE_UVOLT * 8 / 5000*/
           temp = (pulser_amplitude_uvolt*8) /5000;
           if(abs(*fadc_fe_pulser_amplitude - temp)>(temp/2)) return FALSE;
  /*    4.2 <  FADC_FE_PULSER_WIDTH < 6*/
	   if(*fadc_fe_pulser_width<3 || *fadc_fe_pulser_width >5) return FALSE;
	   /*printf("exiting");*/
   return TRUE; 
}










