/*
 *
 *
 * copywrite info
 *
 */

#include <stf/stf.h>

extern STF_DESCRIPTOR testExample_descriptor;
extern STF_DESCRIPTOR memoryTest_descriptor;

/* Prototypes
 */

/* Module level variables
 */
STF_DESCRIPTOR *moduleDirectory[]={
	&testExample_descriptor,
	&memoryTest_descriptor,
	NULL};
