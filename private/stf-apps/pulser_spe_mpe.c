#include <stddef.h>
#include <stdlib.h>

#include "stf/stf.h"
#include "stf-apps/pulser_spe_mpe.h"

#include "hal/DOM_MB_hal.h"
#include "hal/DOM_MB_fpga.h"

#include "stf-apps/atwdUtils.h"

BOOLEAN pulser_spe_mpeInit(STF_DESCRIPTOR *d) {
   return TRUE;
}

BOOLEAN pulser_spe_mpeEntry(STF_DESCRIPTOR *d,
			    unsigned int pulse_height, /* uV */
			    unsigned int discriminator_level, /* uV */
			    BOOLEAN mpe_discriminator, /* use MPE disc? */
			    unsigned int pedestal_dac, /* counts */
			    unsigned int repetition_rate, /* Hz */
			    unsigned int *measured_rate, /* Hz */
			    unsigned int *actual_rate /* Hz */,
			    unsigned int *discriminator_dac /* counts */) {
   DOM_HAL_FPGA_PULSER_RATES rate;
   
   /* 1. amplitude of pulses is used to calc the value of pulser dac */
   /* 2. pulser dac is programmed. */
   halWriteDAC(DOM_HAL_DAC_INTERNAL_PULSER, 
	       (int) (30.0*pulse_height/5000.0));
   
   /* 3. pedestal dac value is calculated */
   /* 4. pedestal dac is set */
   halWriteDAC(DOM_HAL_DAC_PMT_FE_PEDESTAL, pedestal_dac);

   /* 5. calculate discriminator level value */
   /* 6. program spe/mpe dac */
   if (mpe_discriminator) {
      halWriteDAC(DOM_HAL_DAC_MULTIPLE_SPE_THRESH,
		  *discriminator_dac = mpeUVoltToDAC(discriminator_level,
                                                     pedestal_dac));
   }
   else {
      halWriteDAC(DOM_HAL_DAC_SINGLE_SPE_THRESH,
		  *discriminator_dac = speUVoltToDAC(discriminator_level,
                                                     pedestal_dac));
   }

   /* 7. the pulser is set to fire at repetition_rate */
   lookupPulserRate(repetition_rate, &rate, actual_rate);
   
   /* wait for dacs to settle...
    */
   halUSleep(100*1000);

   hal_FPGA_TEST_set_pulser_rate(rate);
   hal_FPGA_TEST_enable_pulser();
   
   /* 8. the spe/mpe counter is read */
   halUSleep(200*1000);
   
   if (mpe_discriminator) {
      *measured_rate = (hal_FPGA_TEST_get_mpe_rate()*10);
   }
   else {
      *measured_rate = (hal_FPGA_TEST_get_spe_rate()*10);
   }

   /* 10. turn off pulser */
   hal_FPGA_TEST_disable_pulser();

   /* 9. check counter value, within +-10% */
   return abs((int)( 100.0 * (*measured_rate - *actual_rate) / 
		     (float) (*actual_rate)))<=10;
}



