/* FlexCAN.h              (c) 2015 Freescale Semiconductor, Inc.
 * Descriptions: FTM example code.
 * 16 Sep 2016 SM: Initial version
 */


#ifndef FLEXCAN_H_
#define FLEXCAN_H_

#define NODE_A        /* If using 2 boards as 2 nodes, NODE A & B use different CAN IDs */

void FLEXCAN0_init (void);
void FLEXCAN0_transmit_msg (void);
void FLEXCAN0_receive_msg (void);

#endif /* FLEXCAN_H_ */
