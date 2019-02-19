/*
 * dma.c              Copyright NXP 2016
 * Description:  Minimal DMA initialization and simple transfer example TCD
 * 2016 Jul 07 S Mihalik - initial version
 * 2016 Oct 28 SM - removed unnecessary PCCn initialization
 *
*/
#include "S32K144.h" /* include peripheral declarations S32K144 */
#include "dma.h"

uint8_t TCD0_Source[] = {"Hello World"};          /* TCD 0 source (11 byte string) */
uint8_t volatile TCD0_Dest = 0;                   /* TCD 0 destination (1 byte) */

void DMA_init(void) {
 /* This is an initialization place holder for:   */
 /* 1. Enabling DMA MUX clock PCC_PCCn[PCC_DMAMUX_INDEX] (not needed when START bit used) */
 /* 2. Enabling desired channels by setting ERQ bit (not needed when START bit used) */
}

void DMA_TCD_init(void) {
	                         /* TCD0: Transfers string to a single memory location */
  DMA->TCD[0].SADDR        = DMA_TCD_SADDR_SADDR((uint32_t volatile) &TCD0_Source); /* Src. */
  DMA->TCD[0].SOFF         = DMA_TCD_SOFF_SOFF(1);   /* Src addr add 1 byte after transfer*/
  DMA->TCD[0].ATTR         = DMA_TCD_ATTR_SMOD(0)  | /* Src modulo feature not used */
                             DMA_TCD_ATTR_SSIZE(0) | /* Src read 2**0 =1 byte per transfer*/
                             DMA_TCD_ATTR_DMOD(0)  | /* Dest modulo feature not used */
                             DMA_TCD_ATTR_DSIZE(0);  /* Dest write 2**0 =1 byte per trans.*/
  DMA->TCD[0].NBYTES.MLNO  = DMA_TCD_NBYTES_MLNO_NBYTES(1); /* Transfer 1 byte /minor loop*/
  DMA->TCD[0].SLAST        = DMA_TCD_SLAST_SLAST(-11); /* Src addr change after major loop*/
  DMA->TCD[0].DADDR        = DMA_TCD_DADDR_DADDR((uint32_t volatile) &TCD0_Dest);/* Dest. */
  DMA->TCD[0].DOFF         = DMA_TCD_DOFF_DOFF(0);     /* No dest adr offset after transfer*/
  DMA->TCD[0].CITER.ELINKNO= DMA_TCD_CITER_ELINKNO_CITER(11) | /* 11 minor loop iterations*/
                             DMA_TCD_CITER_ELINKNO_ELINK(0);   /* No minor loop chan link */
  DMA->TCD[0].DLASTSGA     = DMA_TCD_DLASTSGA_DLASTSGA(0); /* No dest chg after major loop*/
  DMA->TCD[0].CSR          = DMA_TCD_CSR_START(0)       |  /* Clear START status flag */
                             DMA_TCD_CSR_INTMAJOR(0)    |  /* No IRQ after major loop */
                             DMA_TCD_CSR_INTHALF(0)     |  /* No IRQ after 1/2 major loop */
                             DMA_TCD_CSR_DREQ(1)        |  /* Disable chan after major loop*/
                             DMA_TCD_CSR_ESG(0)         |  /* Disable Scatter Gather */
                             DMA_TCD_CSR_MAJORELINK(0)  |  /* No major loop chan link */
                             DMA_TCD_CSR_ACTIVE(0)      |  /* Clear ACTIVE status flag */
                             DMA_TCD_CSR_DONE(0)        |  /* Clear DONE status flag */
                             DMA_TCD_CSR_MAJORLINKCH(0) |  /* Chan # if major loop ch link */
                             DMA_TCD_CSR_BWC(0);           /* No eDMA stalls after R/W */
  DMA->TCD[0].BITER.ELINKNO= DMA_TCD_BITER_ELINKNO_BITER(11) |  /* Initial iteration count*/
                             DMA_TCD_BITER_ELINKNO_ELINK(0);    /* No minor loop chan link */
}


