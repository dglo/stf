/* atwd_pedestal_sweep_forced.c, skeleton file created by gendir
 */
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include "stf/stf.h"
#include "stf-apps/atwd_baseline.h"

#include "hal/DOM_MB_hal.h"
#include "hal/DOM_MB_fpga.h"

#include "stf-apps/atwdUtils.h"

BOOLEAN atwd_pedestal_sweep_forcedInit(STF_DESCRIPTOR *d) {
   return TRUE;
}

BOOLEAN atwd_pedestal_sweep_forcedEntry(STF_DESCRIPTOR *d,
                    unsigned atwd_sampling_speed_dac,
                    unsigned atwd_ramp_top_dac,
                    unsigned atwd_ramp_bias_dac,
                    unsigned atwd_analog_ref_dac,
                    unsigned atwd_pedestal_dac,
                    unsigned atwd_ch0_clamp,
                    unsigned atwd_chip_a_or_b,
                    unsigned atwd_channel,
                    unsigned *half_range_counts_per_volt,
                    unsigned *full_range_counts_per_volt,
                    unsigned *linearity_pedestal_percent,
                    unsigned *atwd_pedestal_sweep_forced) {
   const int ch = (atwd_chip_a_or_b) ? 0 : 4;
   int i;
   unsigned pedestal_nominal, nominal_mvolts,
      pedestal_half_range, half_range_mvolts,
      pedestal_full_range, full_range_mvolts;
   const int cnt = 4096;
   short *channels[4] = { NULL, NULL, NULL, NULL };
   short *buffer = (short *) calloc(128, sizeof(short));
   int trigger_mask = (atwd_chip_a_or_b) ? 
      HAL_FPGA_TEST_TRIGGER_ATWD0 : HAL_FPGA_TEST_TRIGGER_ATWD1;

   /* pretest 1) all five atwd dac settings are programmed... */
   halWriteDAC(ch, atwd_sampling_speed_dac);
   halWriteDAC(ch+1, atwd_ramp_top_dac);
   halWriteDAC(ch+2, atwd_ramp_bias_dac);
   halWriteDAC(DOM_HAL_DAC_ATWD_ANALOG_REF, atwd_analog_ref_dac);
   halWriteDAC(DOM_HAL_DAC_PMT_FE_PEDESTAL, atwd_pedestal_dac);
   /* FIXME: clamp? */

   /* pretest 2) hv is off */
   halDisablePMT_HV();

   /* clear atwd */
   prescanATWD(trigger_mask);

   for (i=0; i<4096; i++) {
      int j;
      
      unsigned sum = 0;

      /* 1) set pedestal dac */
      
      /* 2) take a waveform with forced cpu... */
      hal_FPGA_TEST_trigger_forced(trigger_mask);
      channels[atwd_channel] = buffer;
      hal_FPGA_TEST_readout(channels[0], channels[1], channels[2], 
			    channels[3], 
			    channels[0], channels[1], channels[2], 
			    channels[3],
			    cnt, NULL, 0, trigger_mask);

      /* get summed waveform... */
      for (j=0; j<128; j++) sum+=buffer[j];

      /* add to array... */
      atwd_pedestal_sweep_forced[i] = sum;
   }

   free(buffer);

   /* 5) obtain pedestal values for nominal pedestal DAC values...
    */
   pedestal_nominal = atwd_pedestal_sweep_forced[atwd_pedestal_dac];
   nominal_mvolts = (int)(atwd_pedestal_dac*5000.0/4096.0);

   if (atwd_ch0_clamp==0) {
      const int idx = (int)(atwd_pedestal_dac - 750.0*4096.0/5000.0);
      const int fidx = (int)(atwd_pedestal_dac - 1500.0*4096.0/5000.0);
      
      pedestal_half_range = 
	 atwd_pedestal_sweep_forced[idx];
      half_range_mvolts = (int) (idx * 5000.0/4096.0);

      pedestal_full_range = 
	 atwd_pedestal_sweep_forced[idx];
      full_range_mvolts = (int) (fidx * 5000.0/4096.0);
   }
   else {
      /* 6) if atwd_ch0_clamp is not zero measure only in the 
       * non-clamped region
       */
      const float idx = atwd_ch0_clamp*(2500.0/1024.0);
      const int   midx = (int)(idx*4096.0/5000.0);
      const int   midx2 = (int)((idx*4096.0/5000.0)/2.0);

      pedestal_half_range = 
	 atwd_pedestal_sweep_forced[atwd_pedestal_dac + midx];
      half_range_mvolts = (int) ( midx2 * 5000.0/4096.0);

      pedestal_full_range = atwd_pedestal_sweep_forced[midx];
      full_range_mvolts = (int) idx;
   }

   /* 7) compute counts/volt using half of the range...
    */
   *half_range_counts_per_volt = 
      (int)((pedestal_half_range - pedestal_nominal)*1000.0/
	    (nominal_mvolts - half_range_mvolts));

   *full_range_counts_per_volt =
      (int)((pedestal_full_range - pedestal_nominal)*1000.0/
	    (nominal_mvolts - full_range_mvolts));

   /* 8) calculate the linearity
    */
   *linearity_pedestal_percent = 
      (int)(100.0*
	    *half_range_counts_per_volt /
	    *full_range_counts_per_volt);

   return 
      *full_range_counts_per_volt > 400 &&
      *full_range_counts_per_volt < 700 &&
      *linearity_pedestal_percent > 95 &&
      *linearity_pedestal_percent > 105; 
}






