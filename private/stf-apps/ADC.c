#include <stdio.h>
#include <stdlib.h>

#include <math.h>

#include "stf/stf.h"
#include "hal/DOM_MB_hal.h"

BOOLEAN ADCInit(STF_DESCRIPTOR *desc) { return TRUE; }
BOOLEAN ADCEntry(STF_DESCRIPTOR *desc,
		 unsigned loop_count,
                 unsigned *adc_value,
		 unsigned *pass_or_fail) {

   const int total_channel = 24;
   unsigned i, j, adc_count, dac_count, sum;
   float dac_to_adc;
   memset(adc_value,0,total_channel*sizeof(unsigned));
   memset(pass_or_fail,0,total_channel*sizeof(unsigned));

   /*Azriel suggests to throw the first ADC data of each channel*/
   for (i=0; i<total_channel; i++) halReadADC(i);

   for (i=0; i<loop_count; i++) {
     for (j=0; j<total_channel; j++) {
         adc_value[j] += halReadADC(j);
     }
   }

   for (j=0; j<total_channel; j++) adc_value[j]/=loop_count;


   for (i=0; i<total_channel; i++) {
     unsigned adc_count = adc_value[i];

     if (i == DOM_HAL_ADC_VOLTAGE_SUM) {
       pass_or_fail[i]=(adc_count<=50) ? 1:0;            /* 65 counts = [-5V + (3.3V+5V)*162K/(100K+162K)]/0.002V */
     }
     else if (i == DOM_HAL_ADC_5V_POWER_SUPPLY ) {
       pass_or_fail[i]=(adc_count<=1020 && adc_count>=980) ? 1:0;  /* 1000 counts = [0V + 5V*10K/(10K+15K)]/0.002V */
     }
     else if (i == DOM_HAL_ADC_PRESSURE ) {
       pass_or_fail[i]=1; /* was: (dc_count<=870 && adc_count>=830) ? 1:0; default = 860, Azriel suggested to make this test pass all the time!*/        
     }
     else if (i == DOM_HAL_ADC_5V_CURRENT) {
       pass_or_fail[i]=(adc_count<=600 && adc_count>=400) ? 1:0;   /* default = 500 */                
     }
     else if (i == DOM_HAL_ADC_3_3V_CURRENT) {
       pass_or_fail[i]=(adc_count<=115 && adc_count>=75) ? 1:0;    /* default = 96 */               
     }
     else if (i == DOM_HAL_ADC_2_5V_CURRENT) {
       pass_or_fail[i]=(adc_count<=60 && adc_count>=0) ? 1:0;     /* default = 31 */              
     }
     else if (i == DOM_HAL_ADC_1_8V_CURRENT) {
       pass_or_fail[i]=(adc_count<=125 && adc_count>=75) ? 1:0;     /* default = 90 */              
     }
     else if (i == DOM_HAL_ADC_MINUS_5V_CURRENT) {
       pass_or_fail[i]=(adc_count<=220 && adc_count>=100) ? 1:0;    /* default = 115 */               
     }
     else if (i == DOM_HAL_ADC_DISC_ONESPE) {
       dac_count=halReadDAC(DOM_HAL_DAC_SINGLE_SPE_THRESH);
       dac_to_adc=dac_count*2.5/1023*500;            /* SPE_ADC_count = [SPE_DAC_count * 2.5V/1023 *2* 10K/(10K+10K)]/2mV = 610 */
       pass_or_fail[i]=(adc_count<=(dac_to_adc+10) && adc_count>=(dac_to_adc-10)) ? 1:0;           
     }
     else if (i == DOM_HAL_ADC_1_8V_POWER_SUPPLY) {
       pass_or_fail[i]=(adc_count<=900 && adc_count>=850) ? 1:0;    /* 900 = 1.8V/2mV = 900 */       
     }
     else if (i == DOM_HAL_ADC_2_5V_POWER_SUPPLY) {
       pass_or_fail[i]=(adc_count<=640 && adc_count>=610) ? 1:0;    /* 625 = 2.5V/2mV*10K/(10K+10K) = 625 */       
     }
     else if (i == DOM_HAL_ADC_3_3V_POWER_SUPPLY) {
       pass_or_fail[i]=(adc_count<=835 && adc_count>=805) ? 1:0;    /* 825 = 3.3V/2mV*100K/(100K+100K) = 825 */       
     }
     else if (i == DOM_HAL_ADC_DISC_MULTISPE) {
       dac_count=halReadDAC(DOM_HAL_DAC_MULTIPLE_SPE_THRESH);
       dac_to_adc=dac_count*2.5/1023*500;            /* SPE_ADC_count = [SPE_DAC_count * 2.5V/1023 *2* 10K/(10K+10K)]/2mV */
       pass_or_fail[i]=(adc_count<=(dac_to_adc+10) && adc_count>=(dac_to_adc-5)) ? 1:0;           
     }
     else if (i == DOM_HAL_ADC_FADC_0_REF) {
       dac_count=halReadDAC(DOM_HAL_DAC_FAST_ADC_REF);
       dac_to_adc=dac_count*2.5/1023*500;            /* SPE_ADC_count = [SPE_DAC_count * 2.5V/1023]/2mV */
       pass_or_fail[i]=(adc_count<=(dac_to_adc+70) && adc_count>=(dac_to_adc-20)) ? 1:0;           
     }
     else if (i == DOM_HAL_ADC_SINGLELED_HV) {
       pass_or_fail[i]=(adc_count<=25 && adc_count>=0) ? 1:0;   /* default = 18*/
     }
     else if (i == DOM_HAL_ADC_ATWDA_TRIGGER_BIAS_CURRENT) {
       dac_count=halReadDAC(DOM_HAL_DAC_ATWD0_TRIGGER_BIAS);
       dac_to_adc=dac_count*2.5/4096*500;            /* SPE_ADC_count = [SPE_DAC_count * 2.5V/4096]/2mV = 260 */
       pass_or_fail[i]=(adc_count<=(dac_to_adc+5) && adc_count>=(dac_to_adc-5)) ? 1:0;           
     }
     else if (i == DOM_HAL_ADC_ATWDA_RAMP_TOP_VOLTAGE) {
       dac_count=halReadDAC(   DOM_HAL_DAC_ATWD0_RAMP_TOP);
       dac_to_adc=dac_count*2.5/4096*400;            /* SPE_ADC_count = [SPE_DAC_count * 2.5V/4096 *2* 100K/(150K+100K)]/2mV = 512 */
       pass_or_fail[i]=(adc_count<=(dac_to_adc+10) && adc_count>=(dac_to_adc-10)) ? 1:0;           
     }
     else if (i == DOM_HAL_ADC_ATWDA_RAMP_BIAS_CURRENT) {
       dac_count=halReadDAC(DOM_HAL_DAC_ATWD0_RAMP_RATE);
       dac_to_adc=dac_count*2.5/4096*500;            /* SPE_ADC_count = [SPE_DAC_count * 2.5V/4096]/2mV = 183 */
       pass_or_fail[i]=(adc_count<=(dac_to_adc+6) && adc_count>=(dac_to_adc-6)) ? 1:0;           
     }
     else if (i == DOM_HAL_ADC_ANALOG_REF) {
       dac_count=halReadDAC(DOM_HAL_DAC_ATWD_ANALOG_REF);
       dac_to_adc=dac_count*2.5/4096*400;            /* SPE_ADC_count = [SPE_DAC_count * 2.5V/4096 *2* 100K/(150K+100K)]/2mV = 500 */
       pass_or_fail[i]=(adc_count<=(dac_to_adc+10) && adc_count>=(dac_to_adc-10)) ? 1:0;           
     }
     else if (i == DOM_HAL_ADC_ATWDB_TRIGGER_BIAS_CURRENT) {
       dac_count=halReadDAC(DOM_HAL_DAC_ATWD1_TRIGGER_BIAS);
       dac_to_adc=dac_count*2.5/4096*500;            /* SPE_ADC_count = [SPE_DAC_count * 2.5V/4096]/2mV = 260 */
       pass_or_fail[i]=(adc_count<=(dac_to_adc+5) && adc_count>=(dac_to_adc-5)) ? 1:0;           
     }
     else if (i == DOM_HAL_ADC_ATWDB_RAMP_TOP_VOLTAGE) {
       dac_count=halReadDAC(DOM_HAL_DAC_ATWD1_RAMP_TOP);
       dac_to_adc=dac_count*2.5/4096*400;            /* SPE_ADC_count = [SPE_DAC_count * 2.5V/4096 *2* 100K/(150K+100K)]/2mV = 512 */
       pass_or_fail[i]=(adc_count<=(dac_to_adc+10) && adc_count>=(dac_to_adc-10)) ? 1:0;           
     }
     else if (i == DOM_HAL_ADC_ATWDB_RAMP_BIAS_CURRENT) {
       dac_count=halReadDAC(DOM_HAL_DAC_ATWD1_RAMP_RATE);
       dac_to_adc=dac_count*2.5/4096*500;            /* SPE_ADC_count = [SPE_DAC_count * 2.5V/4096]/2mV = 183 */
       pass_or_fail[i]=(adc_count<=(dac_to_adc+6) && adc_count>=(dac_to_adc-6)) ? 1:0;           
     }
     else if (i == DOM_HAL_ADC_PEDESTAL) {
       dac_count=halReadDAC(DOM_HAL_DAC_PMT_FE_PEDESTAL);
       dac_to_adc=dac_count*2.5/4096*400;            /* SPE_ADC_count = [SPE_DAC_count * 2.5V/4096 *2* 100K/(150K+100K)]/2mV = 470 */
       pass_or_fail[i]=(adc_count<=(dac_to_adc+10) && adc_count>=(dac_to_adc-10)) ? 1:0;           
     }
     else if (i == DOM_HAL_ADC_FE_TEST_PULSE_AMPL) {
       dac_count=halReadDAC(DOM_HAL_DAC_INTERNAL_PULSER);
       dac_to_adc=dac_count*2.5/1023*400;            /* SPE_ADC_count = [SPE_DAC_count * 2.5V/1023 *2* 100K/(100K+150K)]/2mV */
       pass_or_fail[i]=(adc_count<=(dac_to_adc+5) && adc_count>=(dac_to_adc-5)) ? 1:0;           
     }
    }

   {  
      unsigned sum=0;
      for (i=0; i<total_channel; i++) sum+=pass_or_fail[i];
      return sum == total_channel;
   }
}
