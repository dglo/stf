/* fadc_fe_pulser.c, skeleton file created by gendir
   code added by Eric G. Henson eghenson@lbl.gov 
*/
#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#include "stf/stf.h"
#include "stf-apps/fadc_fe_pulser.h"

#include "hal/DOM_MB_hal.h"
#include "hal/DOM_MB_fpga.h"

BOOLEAN fadc_fe_pulserInit(STF_DESCRIPTOR *d) {
   return TRUE;
}

BOOLEAN fadc_fe_pulserEntry(STF_DESCRIPTOR *d,
                    unsigned fadc_reference_dac,
                    unsigned atwd_pedestal_dac,
                    unsigned long pulser_amplitude_uvolt,
                    unsigned long loop_count,

                    unsigned *triggerable_spe_dac,
                    unsigned *fadc_baseline_mean,
                    unsigned *fadc_fe_pulser_amplitude,
                    unsigned *fadc_fe_pulser_width,
                    unsigned *fadc_fe_pulser_position,
                    unsigned *fadc_fe_pulser_waveform) {

  DOM_HAL_FPGA_PULSER_RATES rate;
  unsigned  spe_dac_nominal,scan_start,scan_stop;
  unsigned read_zero,read_nonzero,read_fail;
  unsigned baseline,baseline_sum,baseline_mean,waveform_mean;
  unsigned temp,pos,amp;
  unsigned lp1,lp2,actual_loops;
  unsigned scan_value,spe_rate;
  unsigned half_max,start_width,end_width;
  unsigned discrim_in_mv = 0;
  int time_out,time_out_error=0;
  short *waveform = (short *) calloc(512, sizeof(short));
  int *waveform_sum = (int *) calloc(512, sizeof(int));                                                                             
  int *waveform_avg = (int *) calloc(512, sizeof(int));
 
  /*  short *waveform_sum = (short *) calloc(512, sizeof(short));
      short *waveform_avg = (short *) calloc(512, sizeof(short));*/
 
  /*  unsigned waveform[512];
  unsigned waveform_sum[512];
  unsigned waveform_avg[512];*/

  /*  A.Pretest checks:*/
  /*  1.The two input DAC settings are programmed.*/
  halWriteDAC(DOM_HAL_DAC_FAST_ADC_REF,fadc_reference_dac);
  halUSleep(500000);
  halWriteDAC(DOM_HAL_DAC_PMT_FE_PEDESTAL,atwd_pedestal_dac);
  halUSleep(500000);

  /*  2.If HV base is connected HV is set to 0.*/
  /* Always assume base is installed */
  halSetPMT_HV(0);

  /*  3.Set the Front-End pulser amplitude DAC to zero.*/
  halWriteDAC(DOM_HAL_DAC_INTERNAL_PULSER,0);
  halUSleep(500000);

  /*  4.Turn ON  the front end pulser.*/
   /* pretest 4) turn on fe pulser */
  lookupPulserRate(78e3, &rate, NULL);
  hal_FPGA_TEST_set_pulser_rate(rate);
  hal_FPGA_TEST_enable_pulser();

  /*  5.Find if there is a reasonable value for the SPE discriminator level such that will not trigger on noise.
      (Because part values vary from board to board and the noise band is narrow
      the discriminator value must be scanned). The nominal value is obtained from:
      SPE DAC_NOMINAL = (Discrim_in_milivolts * 9.6 * (2240+249)/249 + ATWD_PEDESTAL_DAC * 5,000 /4096) *1024 / 5,000. */
  spe_dac_nominal = (discrim_in_mv * 95.961445783 + atwd_pedestal_dac * 1.220703125) * 0.2048;

  /*  6.Scan SPE_DAC downwards from SPE DAC_NOMINAL+5% to SPE DAC_NOMINAL-5% in 1-count steps (integer values),
      at each value set the SPE_DAC and obtain the SPE rate meter value
      (this counts discriminator crossings in 100 msec, so this step will be relatively time-consuming).
      Continue the scan until a Non-zero value is found.*/
  read_fail = 0;
  read_zero = 0;
  read_nonzero = 0;
  scan_start = (unsigned)floor((double)spe_dac_nominal * 1.05);
  scan_stop  = (unsigned)floor((double)spe_dac_nominal * 0.95); /*0.95);*/
  for(scan_value=scan_start;scan_value>scan_stop;scan_value--)
  {
       halWriteDAC(DOM_HAL_DAC_SINGLE_SPE_THRESH,scan_value);
       halUSleep(110000); /*wait 110ms */
       spe_rate = hal_FPGA_TEST_get_spe_rate();
  
       if(read_zero && read_nonzero)
	 break;
       if(read_zero && !read_nonzero)
	 *triggerable_spe_dac = scan_value;
       if(!read_zero && read_nonzero)
       {
           read_fail = 1;
           break;
       }
       if(!spe_rate)
	 read_zero = 1;
       if(spe_rate)
	 read_nonzero = 1;
  }

/*  7.Save the first SPE_DAC setting for which there is a Non-Zero value of the SPE rate meter.                        
    If there is NO transition from 0 rate to non-zero rate the test FAILS and QUITS and                                 
    returns in TRIGGERABLE_SPE_DAC the largest SPE_DAC tried if there were no zeros found and  
    smallest SPE_DAC tried if they were all zeros.*/

  if(!read_zero)       /*never read zero*/
  {
    *triggerable_spe_dac = scan_start;
    read_fail = 1;
  }
  if(read_zero && !read_nonzero)     /*read all zeros*/
  {
    *triggerable_spe_dac = scan_stop;
    read_fail = 1;
  }
  if(read_fail)
  {
    /*printf("read failed\n\r");*/
    *fadc_baseline_mean = 0;
    *fadc_fe_pulser_amplitude = 0;
    *fadc_fe_pulser_width = 0;
    *fadc_fe_pulser_position = 0;
    for(lp1=0;lp1<512;lp1++)
    {
      fadc_fe_pulser_waveform[lp1] = 0;
    }

       free(waveform); 
       free(waveform_sum); 
       free(waveform_avg); 
       return FALSE;
  }

  /*  8.Set the TRIGGERABLE_SPE_DAC output variable to
      2 + the first SPE_DAC setting that gave a Non-Zero value of the SPE rate.*/
  *triggerable_spe_dac+=2;

  /*  9.Set the SPE_DAC to the TRIGGERABLE_SPE_DAC value.*/
  halWriteDAC(DOM_HAL_DAC_SINGLE_SPE_THRESH,*triggerable_spe_dac); 
  halUSleep(100000);
       
  /*  B.Test algorithm: */
  baseline_sum = 0;
  for(lp2=0;lp2<10;lp2++)
  {

  /*  1.Take one waveform for the FADC with FORCED (CPU) trigger */
    hal_FPGA_TEST_trigger_forced(HAL_FPGA_TEST_TRIGGER_FADC);
    hal_FPGA_TEST_readout(NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,0,waveform,512,HAL_FPGA_TEST_TRIGGER_FADC);

  /*  2.Calculate the mean of all the 512 samples values (integers ok, range [0-1023]):
	this is the baseline for this waveform.*/
    waveform_mean = 0;
    for(lp1=0;lp1<512;lp1++)
    {
      waveform_mean += waveform[lp1];
    }
    waveform_mean /=512;
    baseline = waveform_mean;

  /*  3.Keep a sum of all baselines.*/
    baseline_sum += baseline;
  
  /*  4.Repeat from step 1, 10  times.*/
  }

  /*  5.Compute the baseline mean using the running sum (integer arithmetic is ok).*/
  baseline_mean = baseline_sum/10;

  /*  6.Fill the FADC_BASELINE_MEAN  output variable.*/
  *fadc_baseline_mean = baseline_mean;

  /*  7.Set Front End Pulser DAC to deliver the signal level given in 
       PULSER_AMPLITUDE_UVOLT using the following: 
       Front-End-Pulser-DAC= PULSER_AMPLITUDE_UVOLT*25/5000 
       (this uses the ad-hoc knowledge that 25 DAC units produce a 5 mV signal at the front end).*/
  temp = (pulser_amplitude_uvolt*25)/5000;
  halWriteDAC(DOM_HAL_DAC_INTERNAL_PULSER,temp);
  halUSleep(100000);

  /*   8.Take LOOP_COUNT waveforms for the FADC with the SPE- trigger,
       keeping a sum-waveform to obtain the average waveform
       (that means 512 individual sums, one for each waveform sample).
       (If a timeout occurs while trying to get a trigger,
       the test fails and reports back zeros for the amplitude, width, position
       but it fills the FADC_FE_PULSER_WAVEFORM with the average waveform so far collected.)*/

  for(lp1=0;lp1<512;lp1++)
  {
    waveform_sum[lp1] = 0;
  }
  for(lp2=0;lp2<loop_count;lp2++)
  {
    hal_FPGA_TEST_trigger_disc(HAL_FPGA_TEST_TRIGGER_FADC);
    time_out= hal_FPGA_TEST_readout(NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,0,waveform,512,HAL_FPGA_TEST_TRIGGER_FADC);
    if(time_out)
    {
	loop_count=lp2;
	time_out_error=1;
	break;
    }   
    for(lp1=0;lp1<512;lp1++)
    {
      waveform_sum[lp1] += waveform[lp1];
    }
  }
  actual_loops = lp2;
       
  /*   9.Calculate the average waveform:
       for each sample in the waveform offset its baseline by adding 100 and subtracting FADC_BASELINE_MEAN.
       This makes a baseline-subtracted waveform with a baseline at 100 counts
       (good for plotting and for negative going pulses).
       If a negative sample value is obtained set to zero.  */
  for(lp1=0;lp1<512;lp1++) 
  {
    temp = waveform_sum[lp1] / actual_loops; /*loop_count;*/
    if((100+temp) < *fadc_baseline_mean)  waveform_avg[lp1]=0;
    else  waveform_avg[lp1] = (100+temp) - *fadc_baseline_mean;
  }
	
  /*   10.Fill the FADC_FE_PULSER_WAVEFORM output array with the baseline subtracted average waveform.*/
  for(lp1=0;lp1<512;lp1++)  
    {
	fadc_fe_pulser_waveform[lp1] = waveform_avg[lp1];
    }

  /*   11.Find the position of the largest sample in the average waveform.
       Fill FADC_FE_PULSER_POSITION with that position (a number between 1 and 512).
       If the maximum value appears in more that one sample,
       report and keep the first (smaller-and earliest- sample number). */
	   amp = 0;
           pos = 0;
	   for(lp1=0;lp1<512;lp1++)
	     {
	      if(waveform_avg[lp1]>amp)
		{
	       amp = waveform_avg[lp1];
	       pos = lp1;
		}
	   *fadc_fe_pulser_position = pos;
	     }
  /*   12.Compute the FADC_FE_PULSER_AMPLITUDE = Maximum sample value in average waveform -100.*/
	     *fadc_fe_pulser_amplitude = amp-100;

  /*   13.Compute the half-maximum amplitude as FADC_FE_PULSER_AMPLITUDE/2 + 100.*/
	   half_max = (*fadc_fe_pulser_amplitude/2) + 100;
  /*   14.Start at the position of the maximum sample and move up until
       the first sample with a value less than the half-maximum amplitude is found. Save that position.
       If no value if found to meet this condition save the maximum position=512.*/
	   pos = 512;
	   for(lp1=*fadc_fe_pulser_position;lp1<512;lp1++)
	     {
	      if(waveform_avg[lp1] < half_max) { pos = lp1; break; }
             }
	       end_width = pos;
  /*   15. Start at the position of the maximum sample and
       move down until the first sample with a value less than the half-maximum amplitude is found.
       Save that position. If no value if found to meet this condition save the minimum position=1.*/
           pos = 1;
           for(lp1=*fadc_fe_pulser_position;lp1>0;lp1--)
             {
              if(waveform_avg[lp1] < half_max) { pos = lp1; break; }
             }
               start_width = pos;
  /*   16. Compute FADC_FE_PULSER_WIDTH =  result from 14 - result from step 15.*/
	   *fadc_fe_pulser_width = end_width - start_width;

	   free(waveform);
           free(waveform_sum);
           free(waveform_avg);

	   if(time_out_error)
	   {
	     *fadc_fe_pulser_amplitude=0;
	     *fadc_fe_pulser_width=0;
	     *fadc_fe_pulser_position=0;
	     return(FALSE);
	   }

  /*C.Success criteria:*/
  /*  1.Set TEST_PASS_NOPASS to pass value if ALL of the following are true:*/
  /*  2.  5 < FADC_FE_PULSER_POSITION < 10.*/
	   if(*fadc_fe_pulser_position<6 || *fadc_fe_pulser_position >9) return FALSE;

  /*  3.  FADC_FE_PULSER_AMPLITUDE is within 50% of its nominal value of: PULSER_AMPLITUDE_UVOLT * 8 / 5000*/
	   temp = (pulser_amplitude_uvolt*8) /5000;
	   if(abs(*fadc_fe_pulser_amplitude - temp)>(temp/2)) return FALSE;
  /*  4.  2 <  FADC_FE_PULSER_WIDTH < 6*/
	      if(*fadc_fe_pulser_width<3 || *fadc_fe_pulser_width >5) return FALSE;

 return TRUE; 
}
