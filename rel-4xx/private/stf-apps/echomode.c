/* echomode.c, echo packets back to sender...
 */
#include <stdio.h>
#include <string.h>

#include "hal/DOM_MB_fpga.h"
#include "hal/DOM_MB_pld.h"

int main(void) {
   char key[]=
      "AAAAB3NzaC1yc2EAAAABIwAAAIEA1sgW+ZwJB3VEVMV2utdo2G8M4ixD9pj"
      "QXB3tfCX6FGyxV1cG3MQdsBa2Fk+UwgdS/080fAKHBLHMpU2TSdGvW3NrdN"
      "cLd1uYIwKnKI/cHjCJwBlYITGF1IcnOI3s1W2PiCN2wmFjJ2raFo3ECqpFQ"
      "NIw+nYn4HEoLHYMIYfM+aU=";
   
   while (1) {
      int type, len;
      char msg[4096];
      hal_FPGA_TEST_receive(&type, &len, msg);

      if (len==sizeof(key) && memcmp(key, msg, sizeof(key))==0) {
         halBoardReboot();
      }
      else hal_FPGA_TEST_send(type, len, msg);
   }
   return 0;
}
