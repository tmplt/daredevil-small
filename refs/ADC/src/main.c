/*
 * main.c              Copyright NXP 2016
 * Description:  Simple program to exercise ADC.
 * Use potentiometer on board to turn on RGB led
 * 2016 Jul 16 Osvaldo Romero: Initial version
 * 2016 Oct 31 SM: Clocks adjusted for 160 MHz SPLL
 * 2017 Jun 30 SM  Split adcResultInMv into separate global variables
 */

#include "S32K144.h" /* include peripheral declarations S32K144 */
#include "clocks_and_modes.h"
#include "ADC.h"
#define PTD15 15 /* RED LED*/
#define PTD16 16 /* GREEN LED*/
#define PTD0 0   /* BLUE LED */

  uint32_t adcResultInMv_pot = 0;
  uint32_t adcResultInMv_Vrefsh = 0;

void PORT_init (void) {
  PCC->PCCn[PCC_PORTD_INDEX ]|=PCC_PCCn_CGC_MASK;   /* Enable clock for PORTD */
  PORTD->PCR[PTD0]  =  0x00000100;  /* Port D0: MUX = GPIO */
  PORTD->PCR[PTD15] =  0x00000100;  /* Port D15: MUX = GPIO */
  PORTD->PCR[PTD16] =  0x00000100;  /* Port D16: MUX = GPIO */

  PTD->PDDR |= 1<<PTD0;       	  /* Port D0:  Data Direction= output */
  PTD->PDDR |= 1<<PTD15;          /* Port D15: Data Direction= output */
  PTD->PDDR |= 1<<PTD16;          /* Port D16: Data Direction= output */
}

void WDOG_disable (void){
  WDOG->CNT=0xD928C520;     /* Unlock watchdog */
  WDOG->TOVAL=0x0000FFFF;   /* Maximum timeout value */
  WDOG->CS = 0x00002100;    /* Disable watchdog */
}

int main(void)
{
  WDOG_disable();        /* Disable WDOG*/
  SOSC_init_8MHz();      /* Initialize system oscillator for 8 MHz xtal */
  SPLL_init_160MHz();    /* Initialize SPLL to 160 MHz with 8 MHz SOSC */
  NormalRUNmode_80MHz(); /* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */
  PORT_init();		     /* Init  port clocks and gpio outputs */
  ADC_init();            /* Init ADC resolution 12 bit*/

  for(;;) {
    convertAdcChan(12);                   /* Convert Channel AD12 to pot on EVB */
    while(adc_complete()==0){}            /* Wait for conversion complete flag */
    adcResultInMv_pot = read_adc_chx();       /* Get channel's conversion results in mv */

    if (adcResultInMv_pot > 3750) {       /* If result > 3.75V */
      PTD->PSOR |= 1<<PTD0 | 1<<PTD16;    /* turn off blue, green LEDs */
      PTD->PCOR |= 1<<PTD15;              /* turn on red LED */
    }
    else if (adcResultInMv_pot > 2500) {  /* If result > 3.75V */
      PTD->PSOR |= 1<<PTD0 | 1<<PTD15;    /* turn off blue, red LEDs */
      PTD->PCOR |= 1<<PTD16;     	      /* turn on green LED */
    }
    else if (adcResultInMv_pot >1250) {       /* If result > 3.75V */
      PTD->PSOR |= 1<<PTD15 | 1<<PTD16;   /* turn off red, green LEDs */
      PTD->PCOR |= 1<<PTD0;     	      /* turn on blue LED */
    }
    else {
      PTD->PSOR |= 1<<PTD0 | 1<< PTD15 | 1<<PTD16; /* Turn off all LEDs */
    }

    convertAdcChan(29);                   /* Convert chan 29, Vrefsh */
    while(adc_complete()==0){}            /* Wait for conversion complete flag */
    adcResultInMv_Vrefsh = read_adc_chx();       /* Get channel's conversion results in mv */
  }
}
