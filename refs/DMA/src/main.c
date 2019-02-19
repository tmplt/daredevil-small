/*
 * main.c              Copyright NXP 2016
 * Description:  Simple program to exercise DMA
 * 2016 Jul 07 S Mihalik - initial version
 * 2016 Oct 25 S Mihalik - Revised software DMA minor loop logic
 * 2016 Oct 31 SM: Clocks adjusted for 160 MHz SPLL
 */


#include "S32K144.h"  /* Include peripheral declarations S32K144 */
#include "dma.h"
#include "clocks_and_modes.h"

void WDOG_disable (void){
  WDOG->CNT=0xD928C520; 	/* Unlock watchdog */
  WDOG->TOVAL=0x0000FFFF;	/* Maximum timeout value */
  WDOG->CS = 0x00002100;    /* Disable watchdog */
}

int main(void) {

  WDOG_disable();
  SOSC_init_8MHz();        /* Initialize system oscillator for 8 MHz xtal */
  SPLL_init_160MHz();      /* Initialize SPLL to 160 MHz with 8 MHz SOSC */
  NormalRUNmode_80MHz();   /* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */

  DMA_init();              /* Init DMA controller */
  DMA_TCD_init();          /* Init DMA Transfer Control Descriptor(s) */

  DMA->SSRT = 0;           /* Set chan 0 START bit to initiate first minor loop */
  while (((DMA->TCD[0].CSR >> DMA_TCD_CSR_START_SHIFT) & 1) |    /* Wait for START = 0 */
         ((DMA->TCD[0].CSR >> DMA_TCD_CSR_ACTIVE_SHIFT)  & 1))  {} /* and ACTIVE = 0 */
                                     /* Now minor loop has completed */

  while (!((DMA->TCD[0].CSR >> DMA_TCD_CSR_DONE_SHIFT) & 1) ) {    /* Loop till DONE = 1 */
    /* Place breakpoint at next instruction & observe expressions TCD0_Source, TCD0_Dest */
    DMA->SSRT = 0;                   /* Set chan 0 START bit to initiate next minor loop */
    while (((DMA->TCD[0].CSR >> DMA_TCD_CSR_START_SHIFT) & 1) |    /* Wait for START = 0 */
           ((DMA->TCD[0].CSR >> DMA_TCD_CSR_ACTIVE_SHIFT)  & 1))  {} /* and ACTIVE = 0 */
                                     /* Now minor loop has completed */
  }

  DMA->TCD[0].CSR &= ~(DMA_TCD_CSR_DONE_MASK);   /* Clear DONE bit */

  while (1) {}                                   /* Wait forever */
}
