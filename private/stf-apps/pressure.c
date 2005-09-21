/* pressure.c, skeleton file created by gendir
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "stf/stf.h"
#include "stfDirectory.h"
#include "hal/DOM_MB_hal.h"


BOOLEAN pressureInit(STF_DESCRIPTOR *d) {
  return TRUE;
}

BOOLEAN pressureEntry(STF_DESCRIPTOR *d,
                    BOOLEAN dom_sealed,
                    BOOLEAN use_temperature,
                    unsigned loop_count,
		    unsigned *adc_5v_mean_mvolts,
		    unsigned *adc_mean_counts,
                    unsigned *adc_rms_counts,
                    unsigned *adc_max_counts,
                    unsigned *adc_min_counts,
                    unsigned *adc_mean_kpascal,
                    unsigned *adc_rms_kpascal,
                    unsigned *adc_max_kpascal,
                    unsigned *adc_min_kpascal) {

  /*unsigned here;*/
  /*  unsigned *adc_5v_mean_mvolts;*/
  unsigned long loop_n,pressure_sum,voltage_sum;
  unsigned short pressure_value,voltage_value;
  unsigned Adc_5v_mean_counts;
  double pressure_float,temp_float,temp2_float,sum_sqr_float;
  unsigned short *buff;

buff = (unsigned short *) calloc(loop_count+2, sizeof(short));
 
  if(buff == NULL)return FALSE;
  /*  adc_5v_mean_mvolts = &here;*/

  halEnableBarometer();
  if(loop_count<2) loop_count=2;
  pressure_sum = 0;
  *adc_max_counts = 0;
  *adc_min_counts = 0xffff;
  for(loop_n=0;loop_n<loop_count;loop_n++)
    {
      buff[loop_n] = halReadADC(DOM_HAL_ADC_PRESSURE);
      pressure_sum +=buff[loop_n];
      voltage_value = halReadADC(DOM_HAL_ADC_5V_POWER_SUPPLY);
      voltage_sum += voltage_value;

      if(buff[loop_n]<*adc_min_counts) *adc_min_counts = buff[loop_n];
      if(buff[loop_n]>*adc_max_counts) *adc_max_counts = buff[loop_n];

    }
  halDisableBarometer();

  *adc_mean_counts = pressure_sum / loop_n;
  Adc_5v_mean_counts = voltage_sum / loop_n;
  temp_float = (double)Adc_5v_mean_counts * 2.5/4095 * (25.0/10.0)*1000.0;
  *adc_5v_mean_mvolts = (unsigned short)floor(temp_float);

  sum_sqr_float = 0;
  for(loop_n=0;loop_n<loop_count;loop_n++)
    {
      pressure_value = buff[loop_n];
      pressure_float = (double)(pressure_value-*adc_mean_counts);
      pressure_float = (double)pressure_value;
      pressure_float -= (double)*adc_mean_counts;
      sum_sqr_float += pressure_float * pressure_float;
    }
  temp_float = sqrt( (1.0/((double)loop_n-1.0)) * sum_sqr_float );  
  *adc_rms_counts = (unsigned short)floor(temp_float);
    
  temp2_float = Adc_5v_mean_counts;

  temp_float = *adc_mean_counts;
  *adc_mean_kpascal = (unsigned short)floor((temp_float/temp2_float + 0.095)/0.009);
  
  temp_float = *adc_rms_counts;
  *adc_rms_kpascal = (unsigned short)floor(temp_float/(temp2_float* 0.009));
  
  temp_float = *adc_max_counts;
  *adc_max_kpascal = (unsigned short)floor((temp_float/temp2_float + 0.095)/0.009);

  temp_float = *adc_min_counts;
  *adc_min_kpascal = (unsigned short)floor((temp_float/temp2_float + 0.095)/0.009);

  if(*adc_5v_mean_mvolts <4850 || *adc_5v_mean_mvolts >5350)
    return FALSE;
  if(dom_sealed)
    {
      if(*adc_mean_kpascal <90 ||*adc_mean_kpascal >110)
	return FALSE;
    }
  else if(!dom_sealed)
    {
        if(*adc_mean_kpascal <30 ||*adc_mean_kpascal >60)
	    return FALSE;
    }

  if(*adc_rms_kpascal >5)
    return FALSE;

  if((*adc_max_kpascal - *adc_min_kpascal)>10)
    return FALSE; 
 
  return TRUE; 
}
