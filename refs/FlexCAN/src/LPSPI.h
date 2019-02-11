/* LPSPI.h              (c) 2016 Freescale Semiconductor, Inc.
 * Descriptions: LPSPI example code.
 * May 31 2016: Initial version
 * Sep 01 2016: Updated for MC33904 init
 */


#ifndef LPSPI_H_
#define LPSPI_H_

void LPSPI1_init_master (void);
void LPSPI1_init_MC33903 (void);
void LPSPI1_transmit_16bits (uint16_t);
uint16_t LPSPI1_receive_16bits (void);

#endif /* LPSPI_H_ */
