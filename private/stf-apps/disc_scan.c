/* disc_scan.c, skeleton file created by gendir
 */
#include <string.h>

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
   int zero_dac, window_dac;
   int dac = (disc_spe_or_mpe) ? DOM_HAL_DAC_MULTIPLE_SPE_THRESH :
      DOM_HAL_DAC_SINGLE_SPE_THRESH;
   int i, maxi;
   unsigned edge_pos=0;
   DOM_HAL_FPGA_PULSER_RATES rate;
   static const unsigned nominalRate = (1000*78)/100;

   /* set pulser amplitude... */
   halWriteDAC(DOM_HAL_DAC_INTERNAL_PULSER,
               (int) (30.0*pulser_amplitude_uvolt/5000.0));

   /* pretest 4) turn on fe pulser */
   lookupPulserRate(78e3, &rate, NULL);
   hal_FPGA_TEST_set_pulser_rate(rate);
   hal_FPGA_TEST_set_scalar_period(DOM_HAL_FPGA_SCALAR_10MS);
   hal_FPGA_TEST_enable_pulser();

   /* 5. calculate discriminator level value */
   /* 6. program spe/mpe dac */
   if (disc_spe_or_mpe) {
      zero_dac = mpeUVoltToDAC(0, atwd_pedestal_dac);
      window_dac = mpeUVoltToDAC(disc_scan_window_uvolts, 0);
   }
   else {
      zero_dac = speUVoltToDAC(0, atwd_pedestal_dac);
      window_dac = speUVoltToDAC(disc_scan_window_uvolts, 0);
   }

   /* clear waveform */
   memset(disc_sum_waveform, 0, sizeof(unsigned)*1024);

   /* create waveform */
   for (i=0; i<loop_count; i++) {
      int j, n = 0;
      for (j=zero_dac-window_dac; j<=zero_dac+window_dac; j++, n++) {
         /* set the new dac value... */
         halWriteDAC(dac, j);
         
         /* wait 20ms for counts to show up */
         halUSleep(20*1000);

         /* readout the rate... */
         disc_sum_waveform[n] = hal_FPGA_TEST_get_spe_rate();
      }
   }

   /* turn off the pulser... */
   hal_FPGA_TEST_disable_pulser();

   /* average... */
   for (i=0; i<1024; i++) disc_sum_waveform[i] /= loop_count;

   /* maximum is noise band */
   {  
      unsigned max = disc_sum_waveform[0];
      int maxi = 0;
      for (i=1; i<1024; i++) {
         if (disc_sum_waveform[i]>max) {
            max = disc_sum_waveform[i];
            maxi = i;
         }
      }
      *disc_scan_noise_band = zero_dac - window_dac + maxi;
   }

   /* find first dac value at >50% of nominal (save position) */
   for (i=1023; i>0; i--) 
      if (disc_sum_waveform[i]>(unsigned) (0.5*nominalRate)) 
         break;
    edge_pos = i-(*disc_scan_noise_band-zero_dac+window_dac);
   *disc_scan_edge_pos = speDACToUVolt(i, atwd_pedestal_dac)-
                         speDACToUVolt((i-edge_pos), atwd_pedestal_dac);
   
   /* find the flat range, the maximum contiguous set of samples +-5% from
    * nominal rate...
    */
   *disc_scan_begin_flat_range = *disc_scan_end_flat_range = 0;
   {  int bfr = 0, efr = 0;
      int j, active = 0;

      for (j=(*disc_scan_noise_band-zero_dac+window_dac); j<1024; j++) {
         const unsigned top = (unsigned) (1.05 * nominalRate);
         const unsigned bot = (unsigned) (0.95 * nominalRate);
         
         if (disc_sum_waveform[j]<=top && disc_sum_waveform[j]>=bot) {
            /* is this the first one?
             */
            if (!active) {
               active = 1;
               bfr = j;
            }
            else efr = j;
         }
         else {
            if (active) {
               if (efr-bfr+1 >= *disc_scan_end_flat_range - 
                                 *disc_scan_begin_flat_range + 1) {
                  *disc_scan_end_flat_range = zero_dac - window_dac + efr;
                  *disc_scan_begin_flat_range = zero_dac - window_dac + bfr;
               }
             else  efr=bfr=active=0;
            }
         }
      }
   }

   /* now calculate the average rate... */
   {  unsigned sum = 0;
      for (i=*disc_scan_begin_flat_range; i<=*disc_scan_end_flat_range; i++) {
         sum += disc_sum_waveform[i - (zero_dac - window_dac)];
      }
      sum /= (*disc_scan_end_flat_range - *disc_scan_begin_flat_range + 1);
      *disc_scan_flat_range_rate = sum;
   }

   /* get the 50% -> 95% delta uVolt */
   {  int start=(edge_pos+*disc_scan_noise_band-zero_dac+window_dac);
      for(i=start; i>0; i--)
         if (disc_sum_waveform[i]>(unsigned) (0.95*nominalRate))
	   break;

      if (disc_spe_or_mpe) {    
         *disc_scan_noise_uvolt = 
            mpeDACToUVolt(start, atwd_pedestal_dac) -
            mpeDACToUVolt(i, atwd_pedestal_dac);
      }
      else {
         *disc_scan_noise_uvolt = 
            speDACToUVolt(start, atwd_pedestal_dac) -
            speDACToUVolt(i, atwd_pedestal_dac);
      }
   }

   return 
      (*disc_scan_end_flat_range - *disc_scan_begin_flat_range + 1) > 2 &&
      *disc_scan_noise_uvolt < 150;
}
