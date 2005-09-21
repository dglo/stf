#include <stddef.h>
#include <stdlib.h>

#include "stf/stf.h"
#include "stf-apps/pulser_smpe.h"

#include "hal/DOM_MB_hal.h"
#include "hal/DOM_MB_fpga.h"

BOOLEAN pulser_spe_mpeInit(STF_DESCRIPTOR *d) {
   return TRUE;
}

BOOLEAN pulser_spe_mpeEntry(STF_DESCRIPTOR *d,
			    unsigned int pulse_height, /* uV */
			    unsigned int discriminator_level, /* uV */
			    BOOLEAN mpe_discriminator, /* use MPE disc? */
			    unsigned int pedestal_level, /* uV */
			    unsigned int repetition_rate, /* Hz */
			    unsigned int max_rate_deviation, /* Hz */
			    unsigned int *measured_rate, /* Hz */
			    unsigned int *actual_rate /* Hz */) {
   int pedestal_dac;
   DOM_HAL_FPGA_PULSER_RATES rate;
   
   /* 1. amplitude of pulses is used to calc the value of pulser dac */
   /* 2. pulser dac is programmed. */
   halWriteDAC(DOM_HAL_DAC_INTERNAL_PULSER, 
	       (int) (30.0*pulse_height/5000.0));
   
   /* 3. pedestal dac value is calculated */
   /* 4. pedestal dac is set */
   halWriteDAC(DOM_HAL_DAC_PMT_FE_PEDESTAL, 
	       pedestal_dac = (int) (pedestal_level*4096.0/5000000.0));

   /* 5. calculate discriminator level value */
   /* 6. program spe/mpe dac */
   if (mpe_discriminator) {
      halWriteDAC(DOM_HAL_DAC_MULTIPLE_SPE_THRESH,
		  (int)( (discriminator_level*9.6* +
			  (pedestal_dac*5000000.0/4096.0)) *
			 1024/5000000.0));
   }
   else {
      halWriteDAC(DOM_HAL_DAC_MULTIPLE_SPE_THRESH,
		  (int)( (discriminator_level*9.6*((2200+1000.0)/1000.0) +
			  (pedestal_dac*5000000.0/4096.0))*
			 1024/5000000.0));
   }

   /* 7. the pulser is set to fire at repetition_rate */
   {  struct {
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
      rate = rates[idx].val;
      *actual_rate = (int) rates[idx].rate;
   }
   hal_FPGA_TEST_set_pulser_rate(rate);
   hal_FPGA_TEST_enable_pulser();
   
   /* 8. the spe/mpe counter is read */
   halUSleep(1000);
   
   if (mpe_discriminator) {
      *measured_rate = (hal_FPGA_TEST_get_mpe_rate()*10);
   }
   else {
      *measured_rate = (hal_FPGA_TEST_get_spe_rate()*10);
   }
   /* 10. turn off pulser */
   hal_FPGA_TEST_disable_pulser();

   /* 9. check counter value */
   return abs(*measured_rate - *actual_rate)<=max_rate_deviation;
}



