#include "hal/DOM_MB_hal.h"
#include "hal/DOM_MB_fpga.h"

void prescanATWD(unsigned trigger_mask);

void lookupPulserRate(int repetition_rate, DOM_HAL_FPGA_PULSER_RATES *rate, 
		      unsigned *hz);

void reverseATWDWaveform(short *p);
void reverseATWDIntWaveform(unsigned *p);

int speDACNominal(float disc_mv, int pedestal_dac);

void getSummedWaveform(int loop_count, unsigned trigger_mask, int channel,
		       unsigned *waveform);

/* convert between spe uvolts and dac units...
 */
int speUVoltToDAC(float uv, int pedestal_dac);
float speDACToUVolt(int dac, int pedestal_dac);
int mpeUVoltToDAC(float uv, int pedestal_dac);
float mpeDACToUVolt(int dac, int pedestal_dac);
int upper_extreme_rate(int spe_dac_nominal);
int lower_extreme_rate(int spe_dac_nominal);

int scanSPE(int atwd_pedestal_dac, unsigned *, int pulser_or_not);

/* calculate the atwd sampling speed, also return the
 * clock waveform in wf...
 */
unsigned calcATWDSamplingSpeed(unsigned *wf, 
                               unsigned trigger_mask,
                               unsigned loop_count);

unsigned calcATWDSamplingSpeedDAC(int dac_ch, 
                                  unsigned trigger_mask,
                                  int atwd_sampling_speed);
