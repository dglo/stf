/* disc_scan.c, skeleton file created by gendir
 */
#include "stf/stf.h"
#include "stf-apps/disc_scan.h"

#include "stf-apps/atwdUtils.h"

#include "hal/DOM_MB_hal.h"
#include "hal/DOM_MB_fpga.h"

BOOLEAN disc_scanInit(STF_DESCRIPTOR *d) { return TRUE; }

BOOLEAN disc_scanEntry(STF_DESCRIPTOR *d,
		       unsigned disc_scan_window_uvolts,
		       unsigned disc_spe_or_mpe,
		       unsigned pulser_amplitude_uvolt,
		       unsigned pulser_rate_hz,
		       unsigned atwd_pedestal_dac,
		       unsigned loop_count,
		       unsigned *disc_scan_edge_pos,
		       unsigned *disc_scan_begin_flat_range,
		       unsigned *disc_scan_end_flat_range,
		       unsigned *disc_scan_flat_range_rate,
		       unsigned *disc_scan_noise_band,
		       unsigned *disc_scan_noise_uvolt,
		       unsigned *disc_sum_waveform) {
   int zero_dac = speUVoltToDAC(0, atwd_pedestal_dac);
   int window_dac = speUVoltToDAC(disc_scan_window_uvolts, 0);
   int i;
   DOM_HAL_FPGA_PULSER_RATES rate;

   /* pretest 4) turn on fe pulser */
   lookupPulserRate(78e3, &rate, NULL);
   hal_FPGA_TEST_set_pulser_rate(rate);
   hal_FPGA_TEST_enable_pulser();

   prescanATWD(HAL_FPGA_TEST_TRIGGER_ATWD0);

   return FALSE;
}





