/* disc_scan.c, skeleton file created by gendir
 */
#include "stf/stf.h"
#include "stf-apps/disc_scan.h"

#include "stf-apps/atwdUtils.h"

BOOLEAN disc_scanInit(STF_DESCRIPTOR *d) {
   return FALSE;
}

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
   int window_dac /* = () */;
   return FALSE;
}





