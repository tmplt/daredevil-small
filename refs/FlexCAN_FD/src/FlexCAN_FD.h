/* FlexCAN_FD.h              (c) 2015 Freescale Semiconductor, Inc.
 * Descriptions: FTM example code.
 * 2016n Sep 16: SM - Initial version
 */


#ifndef FLEXCAN_FD_H_
#define FLEXCAN_FD_H_

#define NODE_A        /* If using 2 boards as 2 nodes, NODE A transmits first to NODE_B */

void FLEXCAN0_init (void);
void FLEXCAN0_transmit_msg (void);
void FLEXCAN0_receive_msg (void);

#endif /* FLEXCAN_FD_H_ */
