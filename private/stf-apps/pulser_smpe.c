#include <stddef.h>

#include "stf/stf.h"
#include "stf-apps/atwd_baseline.h"

#include "hal/DOM_MB_hal.h"
#include "hal/DOM_MB_fpga.h"

BOOLEAN pulser_spe_mpeInit(STF_DESCRIPTOR *d) {
   return TRUE;
}

BOOLEAN pulser_spe_mpeEntry(STF_DESCRIPTOR *d,
			    unsigned int pulse_height,
			    unsigned int discriminator_level,
			    BOOLEAN mpe_discriminator,
			    unsigned int pedestal_level,
			    unsigned int repetition_rate,
			    unsigned int max_rate_deviation,
			    unsigned int *measured_rate) {
   int pedestal_dac;
   
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
#if 0
   if (mpe_discriminator) {
      halWriteDAC(DOM_HAL_DAC_MULTIPLE_SPE_THRESH,
		  (int)());
   }
   else {
      halWriteDAC(DOM_HAL_DAC_SINGLE_SPE_THRESH,
		  (int) ());
   }
#endif

   /* 7. the pulser is set to fire */

   /* 8. the spe/mpe counter is read */

   /* 9. check counter value */

   /* 10. turn off pulser */

   return TRUE;
}

