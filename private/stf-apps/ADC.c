#include <stdio.h>
#include <stdlib.h>

#include <math.h>

#include "stf/stf.h"
#include "hal/DOM_MB_hal.h"

BOOLEAN ADCInit(STF_DESCRIPTOR *desc) { return TRUE; }
BOOLEAN ADCEntry(STF_DESCRIPTOR *desc,
		 unsigned adc_chip,
		 unsigned adc_channel,
		 unsigned loop_count,
		 unsigned *adc_mean_counts,
		 unsigned *adc_rms_counts,
		 unsigned *adc_max_counts,
		 unsigned *adc_min_counts,
		 unsigned *adc_mean_mvolt_or_mamp,
		 unsigned *adc_rms_mvolt_or_mamp,
		 unsigned *adc_max_mvolt_or_mamp,
		 unsigned *adc_min_mvolt_or_mamp) {
   /* fill in details from azriels document...
    */
   const int ch = adc_chip*8 + adc_channel;
   unsigned i;
   unsigned short *buf = (unsigned short *) calloc(loop_count, sizeof(short));
   unsigned sum = 0;
   unsigned sd2 = 0;

   for (i=0; i<loop_count; i++) {
      buf[i] = halReadADC(ch);
      sum+=buf[i];
   }

   *adc_mean_counts = sum/loop_count;
   *adc_min_counts = *adc_max_counts = buf[0];
   for (i=1; i<loop_count; i++) {
      if (buf[i]>*adc_max_counts) *adc_max_counts = buf[i];
      else if (buf[i]<*adc_min_counts) *adc_min_counts = buf[i];
      sd2 += (buf[i]-*adc_mean_counts)*(buf[i]-*adc_mean_counts);
   }

   /* avoid div by zero... */
   if (loop_count==1) loop_count=2;
   
   *adc_rms_counts = (unsigned) sqrt(sd2/(loop_count-1));
   
   if (ch==0 || ch==7) {
      *adc_mean_mvolt_or_mamp = *adc_mean_counts/2;
      *adc_rms_mvolt_or_mamp = *adc_rms_counts/2;
      *adc_max_mvolt_or_mamp = *adc_max_counts/2;
      *adc_min_mvolt_or_mamp = *adc_min_counts/2;
   }
   else if (ch==1) {
      /* no translation... */
   }
   else if (ch==2) {
      float factor = 2500.0/4095.0 * ((10+24.9)/10);
      
      *adc_mean_mvolt_or_mamp = (unsigned) (*adc_mean_counts*factor);
      *adc_rms_mvolt_or_mamp = (unsigned) (*adc_rms_counts*factor);
      *adc_max_mvolt_or_mamp = (unsigned) (*adc_max_counts*factor);
      *adc_min_mvolt_or_mamp = (unsigned) (*adc_min_counts*factor);
      
   }
   else if (ch>=3 && ch<=6) {
      *adc_mean_mvolt_or_mamp = *adc_mean_counts/20;
      *adc_rms_mvolt_or_mamp = *adc_rms_counts/20;
      *adc_max_mvolt_or_mamp = *adc_max_counts/20;
      *adc_min_mvolt_or_mamp = *adc_min_counts/20;
   }

   if (*adc_max_counts>4095) return FALSE;

   if (ch==0) {
      if ( *adc_mean_mvolt_or_mamp < 200-20 || 
	   *adc_mean_mvolt_or_mamp > 200+20 ) return FALSE;

      if (*adc_rms_mvolt_or_mamp > 20) return FALSE;
      
      if (*adc_max_mvolt_or_mamp > 200+40) return FALSE;
      
      if (*adc_min_mvolt_or_mamp < 200-40) return FALSE;
   }
   else if (ch==2) {
      if ( *adc_mean_mvolt_or_mamp < 2000-100 || 
	   *adc_mean_mvolt_or_mamp > 2000+100 ) return FALSE;

      if (*adc_rms_mvolt_or_mamp > 20) return FALSE;
      
      if (*adc_max_mvolt_or_mamp > 2000+120) return FALSE;
      
      if (*adc_min_mvolt_or_mamp < 2000-120) return FALSE;
   }
   
   return TRUE;
}
