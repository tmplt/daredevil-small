/*
 * main implementation: use this 'C' sample to create your own application
 * 2016 Jul 26 S. Mihalik: Initial version
 * 2016 Sep 20 SM - Updated with SBC init, Node A - B communication
 * 2016 Oct 31 SM- Clocks adjusted for 160 MHz SPLL, updated loop logic
 * 2017 Jul 03 SM- Removed code for: MC33903 on obsolete EVB,
 *                 initial transmit for node B, tx_msg_count
 */

#include "S32K144.h" /* include peripheral declarations S32K144 */
#include "FlexCAN_FD.h"
#include "clocks_and_modes.h"

void WDOG_disable (void){
  WDOG->CNT=0xD928C520; 	/* Unlock watchdog */
  WDOG->TOVAL=0x0000FFFF;	/* Maximum timeout value */
  WDOG->CS = 0x00002100;    /* Disable watchdog */
}

void PORT_init (void) {
  PCC->PCCn[PCC_PORTE_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clock for PORTE */
  PORTE->PCR[4] |= PORT_PCR_MUX(5); /* Port E4: MUX = ALT5, CAN0_RX */
  PORTE->PCR[5] |= PORT_PCR_MUX(5); /* Port E5: MUX = ALT5, CAN0_TX */
  PCC->PCCn[PCC_PORTD_INDEX ]|=PCC_PCCn_CGC_MASK;   /* Enable clock for PORTD */
  PORTD->PCR[16] =  0x00000100;  /* Port D16: MUX = GPIO (to green LED) */
  PTD->PDDR |= 1<<16;            /* Port D16: Data direction = output */
}

int main(void) {
  uint32_t rx_msg_count = 0;

  WDOG_disable();
  SOSC_init_8MHz();      /* Initialize system oscillator for 8 MHz xtal */
  SPLL_init_160MHz();    /* Initialize SPLL to 160 MHz with 8 MHz SOSC */
  NormalRUNmode_80MHz(); /* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */

  FLEXCAN0_init();         /* Init FlexCAN0 */
  PORT_init();             /* Configure ports */
#ifdef NODE_A              /* Node A transmits first; Node B transmits after reception */
  FLEXCAN0_transmit_msg(); /* Transmit initial message from Node A to Node B */
#endif

  for (;;) {                        /* Loop: if a msg is received, transmit a msg */
    if ((CAN0->IFLAG1 >> 4) & 1) {  /* If CAN 0 MB 4 flag is set (received msg), read MB4 */
      FLEXCAN0_receive_msg ();      /* Read message */
      rx_msg_count++;               /* Increment receive msg counter */

      if (rx_msg_count == 1000) {   /* If 1000 messages have been received, */
        PTD->PTOR |= 1<<16;         /*   toggle output port D16 (Green LED) */
        rx_msg_count = 0;           /*   and reset message counter */
      }

      FLEXCAN0_transmit_msg ();     /* Transmit message using MB0 */
    }
  }
}
