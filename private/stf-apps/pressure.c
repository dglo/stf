/* pressure.c, skeleton file created by gendir and modified by George
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "stf/stf.h"
#include "stf-apps/pressure.h"
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
  unsigned loop_n, x;
  double pressure_sum, voltage_sum;
  unsigned voltage_value;
  unsigned Adc_5v_mean_counts;
  double temp_float, temp2_float, sum_sqr_float;
  unsigned *buff = 
     (unsigned *) calloc(loop_count, sizeof(unsigned));
 
  if(buff == NULL)return FALSE;
  /*  adc_5v_mean_mvolts = &here;*/

  halEnableBarometer();
  halUSleep(2000000);
  /* add 2 sec wait */
  pressure_sum = voltage_sum = 0;

  /* throw first few data out because in cold, first few data are BAD */
  for(x=0; x<10;x++){
     halReadADC(DOM_HAL_ADC_PRESSURE);    
     halReadADC(DOM_HAL_ADC_5V_POWER_SUPPLY);
  }

  for(loop_n=0;loop_n<loop_count;loop_n++) {
     buff[loop_n] = halReadADC(DOM_HAL_ADC_PRESSURE);
     pressure_sum += buff[loop_n];
     voltage_sum += halReadADC(DOM_HAL_ADC_5V_POWER_SUPPLY);
  }
  
  *adc_max_counts = *adc_min_counts = buff[0];
  for(loop_n=1;loop_n<loop_count;loop_n++) {
      if(buff[loop_n]<*adc_min_counts) *adc_min_counts = buff[loop_n];
      if(buff[loop_n]>*adc_max_counts) *adc_max_counts = buff[loop_n];
  }
  halDisableBarometer();

  *adc_mean_counts = (unsigned) (pressure_sum/loop_count);
  Adc_5v_mean_counts = (unsigned) (voltage_sum/loop_count);
  temp_float = (double)Adc_5v_mean_counts * 0.002 * (25.0/10.0)*1000.0;
  *adc_5v_mean_mvolts = (unsigned)floor(temp_float);

  sum_sqr_float = 0;
  for(loop_n=0;loop_n<loop_count;loop_n++)
    {
       const double pressure_float = 
          (double)buff[loop_n]-(double)*adc_mean_counts;
       sum_sqr_float += pressure_float * pressure_float;
    }

  free(buff);

  temp_float = sqrt( (1.0/((double)loop_count-1.0)) * sum_sqr_float );  
  *adc_rms_counts = (unsigned)floor(temp_float);
    
  temp2_float = Adc_5v_mean_counts;

  temp_float = *adc_mean_counts;
  *adc_mean_kpascal = (unsigned)floor((temp_float/temp2_float + 0.095)/0.009);
  
  temp_float = *adc_rms_counts;
  *adc_rms_kpascal = (unsigned)floor(temp_float/(temp2_float* 0.009));
  
  temp_float = *adc_max_counts;
  *adc_max_kpascal = (unsigned)floor((temp_float/temp2_float + 0.095)/0.009);

  temp_float = *adc_min_counts;
  *adc_min_kpascal = (unsigned)floor((temp_float/temp2_float + 0.095)/0.009);

  if(*adc_5v_mean_mvolts <4850 || *adc_5v_mean_mvolts >5350)
    return FALSE;
  if(dom_sealed)
    {
      if(*adc_mean_kpascal >60 || *adc_mean_kpascal <30)
	return FALSE;
    }
  else if(!dom_sealed)
    {
        if(*adc_mean_kpascal >110 || *adc_mean_kpascal <90)
	    return FALSE;
    }

  if(*adc_rms_kpascal >5)
    return FALSE;

  if((*adc_max_kpascal - *adc_min_kpascal)>10)
    return FALSE; 
 
  return TRUE; 
}
