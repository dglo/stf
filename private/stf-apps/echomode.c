/* echomode.c, echo packets back to sender...
 */
#include "hal/DOM_MB_fpga.h"

int main(void) {
   while (1) {
      int type, len;
      char msg[4096];
      hal_FPGA_TEST_receive(&type, &len, msg);
      hal_FPGA_TEST_send(type, len, msg);
   }
   return 0;
}
