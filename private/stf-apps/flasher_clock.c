/*
 *
 * STF Test -- Flasherboard clock stability test
 *
 * Enables the flasherboard and checks the clock stability
 * time, as determined by the HAL and the flasherboard CPLD
 * measurement.
 * 
 * Check is more stringent than the one in the flasherboard
 * HAL and is meant to screen out bad LVDS drivers.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "stf/stf.h"
#include "hal/DOM_MB_hal.h"

/* Pass/fail defines */
/* Maximum allowed clock validation time, in us */
/* Pass criterion is about 50us of instability */
#define MAX_CLOCK_TIME_US  50

BOOLEAN flasher_clockInit(STF_DESCRIPTOR *desc) { return TRUE; }

BOOLEAN flasher_clockEntry(STF_DESCRIPTOR *desc,
                           char ** flasher_id,
                           unsigned int * config_time_us,
                           unsigned int * valid_time_us,
                           unsigned int * reset_time_us
                           ) {

    char dummy_id[9] = "deadbeef";
    *flasher_id = dummy_id;

    /* Make sure PMT is off */
    halPowerDownBase();

    /* Initialize the flaherboard and power up */
    /* Record configuration and clock validation times */
    int err = hal_FB_enable(config_time_us, valid_time_us, reset_time_us);
    if (err != 0) {
#ifdef VERBOSE
        printf("Flasher board enable failure (%d)!  Aborting test!\r\n", err);
#endif
        return FALSE;
    }

    /* Read the flasher board firmware version */
#ifdef VERBOSE
    printf("Flasher board FW version = %d\r\n", hal_FB_get_fw_version());
#endif
    
    /* Read the flasherboard ID */
    hal_FB_get_serial(flasher_id);
    
#ifdef VERBOSE
    printf("Flasher board ID = %s\n", *flasher_id);
#endif
    
    /* Turn the flasherboard off */
    hal_FB_disable();

    /* Check pass/fail conditions */
    BOOLEAN passed = (*valid_time_us <= MAX_CLOCK_TIME_US);

    return passed;

}
