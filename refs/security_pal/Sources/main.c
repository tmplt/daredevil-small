/*
 * Copyright (c) 2015 - 2016 , Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * THIS SOFTWARE IS PROVIDED BY NXP "AS IS" AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL NXP OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */
/*!
** @file main.c
** @version 01.00
** @brief
**     The example runs on the following DEVKITs:
**     -> S32K142EVB-Q100
**     -> S32K144EVB-Q100
**     -> S32K148EVB-Q144
**     The SOC must have FTFC module available.
**     If the program runs without error, then LED GREEN turns on.
**     If the program runs into error, then LED RED turns on.
*/
/*!
**  @addtogroup main_module main module documentation
**  @{
*/

/* Including needed modules to compile this module/procedure */
#include "Cpu.h"
#include "pin_mux.h"

volatile int exit_code = 0;

/* User includes (#include below this line is not maintained by Processor Expert) */
#include "osif.h"

/* Defines */
#if (defined(CPU_S32K142))
#define LED_TYPE_0
#elif (defined(CPU_S32K144) || defined(CPU_S32K144HFT0VLLT) || defined(CPU_S32K144HFT0VLLT))
#define LED_TYPE_1
#elif (defined(CPU_S32K146))
#define LED_TYPE_2
#elif (defined(CPU_S32K148))
#define LED_TYPE_3
#endif

#if (defined(LED_TYPE_0) || defined(LED_TYPE_1))
#define LED_PORT        PTD
#define LED_RED_ERROR   15U
#define LED_GREEN_OK    16U
#elif (defined(LED_TYPE_2))
#elif (defined(LED_TYPE_3))
#define LED_PORT        PTE
#define LED_RED_ERROR   21U
#define LED_GREEN_OK    22U
#endif
#define TIMEOUT ((uint32_t)5)
#define RND_BUFF_LEN ((uint32_t)16)
#define MSG_LEN ((uint32_t)16)

/* Global variables */
static security_user_config_t g_tSecurityUserConfig;

/* ProgramStatus Function */
void ProgramStatus(bool bStatus)
{
    if(bStatus == true)
    {
        /* Example finished ok */
#if (defined(LED_TYPE_0) || defined(LED_TYPE_3))
        PINS_DRV_SetPins(LED_PORT, 1 << LED_GREEN_OK);
#elif (defined(LED_TYPE_1))
        PINS_DRV_ClearPins(LED_PORT, 1 << LED_GREEN_OK);
#elif (defined(LED_TYPE_2))
#endif
    }
    else
    {
        /* Example finished with error */
#if (defined(LED_TYPE_0) || defined(LED_TYPE_3))
        PINS_DRV_SetPins(LED_PORT, 1 << LED_RED_ERROR);
#elif (defined(LED_TYPE_1))
        PINS_DRV_ClearPins(LED_PORT, 1 << LED_RED_ERROR);
#elif (defined(LED_TYPE_2))
#endif
    }
    while(1);
}

/* Callback */
void SecurityCallback(uint32_t ulCmd, void *pvCallParam)
{
    (void)pvCallParam;
      security_cmd_t tSecurityCmd = (security_cmd_t)ulCmd;
    switch (tSecurityCmd)
    {
        case SECURITY_CMD_ENC_CBC:
            /* Do something... */
            break;
        case SECURITY_CMD_DEC_CBC:
            /* Do something... */
            break;
        case SECURITY_CMD_LOAD_PLAIN_KEY:
            /* Do something... */
            break;
        case SECURITY_CMD_INIT_RNG:
              /* Do something... */
              break;
        case SECURITY_CMD_RND:
              /* Do something... */
              break;
        default:
            ProgramStatus(false);
              break;
    }
}

/* Compares strings */
bool StringCompareOk(const uint8_t * pucString0, uint8_t * pucString1, uint32_t ulLength)
{
    uint32_t ulCnt = 0U;
    for(ulCnt = 0; ulCnt < ulLength; ulCnt++)
    {
        if(pucString0[ulCnt] != pucString1[ulCnt])
        {
            return false;
        }
    }
    return true;
}

/* Initialize Flash in order for Security to run over CSEc module */
status_t InitFlashForSecurityOperation(void)
{
    status_t tStatus = STATUS_SUCCESS;
    flash_ssd_config_t flashSSDConfig;

    tStatus = FLASH_DRV_Init(&Flash1_InitConfig0, &flashSSDConfig);
    if(tStatus != STATUS_SUCCESS)
    {
        return STATUS_ERROR;
    }

    if (flashSSDConfig.EEESize == 0)
    {
        uint32_t address;
        uint32_t size;
#if (FEATURE_FLS_HAS_PROGRAM_PHRASE_CMD == 1u)
        uint8_t unsecure_key[FTFx_PHRASE_SIZE] = {0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFEu, 0xFFu, 0xFFu, 0xFFu};
#else   /* FEATURE_FLASH_HAS_PROGRAM_LONGWORD_CMD */
        uint8_t unsecure_key[FTFx_LONGWORD_SIZE] = {0xFEu, 0xFFu, 0xFFu, 0xFFu};
#endif  /* FEATURE_FLS_HAS_PROGRAM_PHRASE_CMD */
        /* First, erase all Flash blocks to ensure the IFR region is blank
         * before partitioning FlexNVM and FlexRAM */
        tStatus = FLASH_DRV_EraseAllBlock(&flashSSDConfig);
        if(tStatus != STATUS_SUCCESS)
        {
            return STATUS_ERROR;
        }
        /* Reprogram secure byte in Flash configuration field */
#if (FEATURE_FLS_HAS_PROGRAM_PHRASE_CMD == 1u)
        address = 0x408u;
        size = FTFx_PHRASE_SIZE;
#else   /* FEATURE_FLASH_HAS_PROGRAM_LONGWORD_CMD == 1u */
        address = 0x40Cu;
        size = FTFx_LONGWORD_SIZE;
#endif /* FEATURE_FLS_HAS_PROGRAM_PHRASE_CMD */
        tStatus = FLASH_DRV_Program(&flashSSDConfig, address, size, unsecure_key);
        if(tStatus != STATUS_SUCCESS)
        {
            return STATUS_ERROR;
        }
        tStatus = FLASH_DRV_DEFlashPartition(&flashSSDConfig, 0x2, 0x4, 0x3, false, true);
        if(tStatus != STATUS_SUCCESS)
        {
            return STATUS_ERROR;
        }
    }
    return tStatus;
}

/*!
  \brief The main function for the project.
  \details The startup initialization sequence is the following:
 * - startup asm routine
 * - main()
*/
int main(void)
{
  /* Write your local variable definition here */

  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  #ifdef PEX_RTOS_INIT
    PEX_RTOS_INIT();                   /* Initialization of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of Processor Expert internal initialization.                    ***/

  /* Write your code here */
  /* For example: for(;;) { } */
    static status_t tStatus = STATUS_SUCCESS;
    static bool bStatus = true;

    const uint8_t ucPlainKey[MSG_LEN] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    const uint8_t ucPlainText[MSG_LEN] = "Key:0123456789ab";
    static uint8_t ucEncText[MSG_LEN];
    static uint8_t ucDecText[MSG_LEN];
    static uint8_t ucInitVct[MSG_LEN] = "1234567887654321";
    static uint8_t ucRndBuf[RND_BUFF_LEN];

    /* Initialize clocks */
    CLOCK_SYS_Init(    g_clockManConfigsArr,   CLOCK_MANAGER_CONFIG_CNT,
                    g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
    CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);

    /* Initialize pins */
    tStatus = PINS_DRV_Init(NUM_OF_CONFIGURED_PINS, g_pin_mux_InitConfigArr);
    if(tStatus != STATUS_SUCCESS)
    {
        ProgramStatus(false);
    }
    /* Set Output value of the LEDs */
    /* Output direction for the LEDs */
#if (defined(LED_TYPE_0) || defined(LED_TYPE_3))
        PINS_DRV_ClearPins(LED_PORT, (1 << LED_RED_ERROR) | (1 << LED_GREEN_OK));
        PINS_DRV_SetPinsDirection(LED_PORT, (1 << LED_RED_ERROR) | (1 << LED_GREEN_OK));
#elif (defined(LED_TYPE_1))
        PINS_DRV_SetPins(LED_PORT, (1 << LED_RED_ERROR) | (1 << LED_GREEN_OK));
        PINS_DRV_SetPinsDirection(LED_PORT, (1 << LED_RED_ERROR) | (1 << LED_GREEN_OK));
#elif (defined(LED_TYPE_2))
#endif

    /* Initialize Flash for Security operation over CSEc */
    tStatus = InitFlashForSecurityOperation();
    if(tStatus != STATUS_SUCCESS)
    {
        ProgramStatus(false);
    }

    /* Initialize Security */
    g_tSecurityUserConfig.callback = SecurityCallback;
    tStatus = SECURITY_Init(SECURITY_INSTANCE0, &g_tSecurityUserConfig);
    if(tStatus != STATUS_SUCCESS)
    {
        ProgramStatus(false);
    }

    tStatus = SECURITY_InitRng(SECURITY_INSTANCE0, TIMEOUT);
    if(tStatus != STATUS_SUCCESS)
    {
        ProgramStatus(false);
    }

    tStatus = SECURITY_GenerateRnd(SECURITY_INSTANCE0, ucRndBuf, TIMEOUT);
    if(tStatus != STATUS_SUCCESS)
    {
        ProgramStatus(false);
    }

    tStatus = SECURITY_LoadPlainKey(SECURITY_INSTANCE0, ucPlainKey, TIMEOUT);
    if(tStatus != STATUS_SUCCESS)
    {
        ProgramStatus(false);
    }

    tStatus = SECURITY_EncryptCbcBlocking(SECURITY_INSTANCE0, SECURITY_RAM_KEY, ucPlainText, MSG_LEN, ucInitVct, ucEncText, TIMEOUT);
    if(tStatus != STATUS_SUCCESS)
    {
        ProgramStatus(false);
    }

    tStatus = SECURITY_DecryptCbcBlocking(SECURITY_INSTANCE0, SECURITY_RAM_KEY, ucEncText, MSG_LEN, ucInitVct, ucDecText, TIMEOUT);
    if(tStatus != STATUS_SUCCESS)
    {
        ProgramStatus(false);
    }

    bStatus = StringCompareOk((uint8_t*)ucPlainText, ucDecText, MSG_LEN);
    if(bStatus != true)
    {
        ProgramStatus(false);
    }

    ProgramStatus(true);

  /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;) {
    if(exit_code != 0) {
      break;
    }
  }
  return exit_code;
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END main */
/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.1 [05.21]
**     for the Freescale S32K series of microcontrollers.
**
** ###################################################################
*/
