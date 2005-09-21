/* fadc_fe_external.c, skeleton file created by George
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "stf/stf.h"
#include "stf-apps/fadc_fe_external.h"

#include "hal/DOM_MB_hal.h"
#include "hal/DOM_MB_fpga.h"
#include "stf-apps/atwdUtils.h"

BOOLEAN fadc_fe_externalInit(STF_DESCRIPTOR *d) {
   return TRUE;
}

BOOLEAN fadc_fe_externalEntry(STF_DESCRIPTOR *d,
                    unsigned fadc_reference_dac,
                    unsigned atwd_pedestal_dac,
                    unsigned triggerable_spe_dac,
                    unsigned loop_count,
                    unsigned *fadc_fe_pulser_amplitude,
                    unsigned *fadc_fe_pulser_width,
                    unsigned *fadc_fe_pulser_position,
                    unsigned *fadc_fe_pulser_waveform) {

  /* pretest 1) both input dac settings are programmed... */
  halWriteDAC(DOM_HAL_DAC_FAST_ADC_REF,fadc_reference_dac);
  halUSleep(500000);
  halWriteDAC(DOM_HAL_DAC_PMT_FE_PEDESTAL,atwd_pedestal_dac);
  halUSleep(500000);

   /* pretest 2) hv is off */
   halPowerDownBase();



   return FALSE; 
}
