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


