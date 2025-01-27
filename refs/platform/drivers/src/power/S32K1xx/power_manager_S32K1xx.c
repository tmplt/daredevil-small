/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
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
 * @file power_manager_S32K1xx.c
 *
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 1.3, There shall be no occurrence of
 * undefined or critical unspecified behaviour.
 * The addresses of the stack variables are only used at local scope.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.7, External could be made static.
 * Function is defined for usage by application code.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.6, A cast shall not be performed
 * between pointer to void and an arithmetic type.
 * The base address parameter from HAL functions is provided as integer so
 * it needs to be cast to pointer.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 11.4, A conversion should not be performed
 * between a pointer to object and an integer type.
 * The base address parameter from HAL functions is provided as integer so
 * a conversion between a pointer and an integer has to be performed
 */

#include <stddef.h>
#include "power_manager.h"
#include "power_smc_hw_access.h"
#include "power_rcm_hw_access.h"
#include "clock_manager.h"

/*! Timeout used for waiting to set new mode */
#define POWER_SET_MODE_TIMEOUT 1000U

/*! @brief Power manager internal structure. */
extern power_manager_state_t gPowerManagerState;

/*******************************************************************************
 * INTERNAL FUNCTIONS
 ******************************************************************************/
static status_t POWER_SYS_WaitForModeStatus(smc_run_mode_t mode);

static status_t POWER_SYS_SwitchToSleepingPowerMode(const power_manager_user_config_t * const configPtr);

static status_t POWER_SYS_SwitchToRunningPowerMode(const power_manager_user_config_t * const configPtr);

static status_t POWER_DRV_SwitchVlprClk(const sys_clk_config_t * const sysClock);

static status_t POWER_DRV_UpdateInitClk(const sys_clk_config_t * const sysClk, bool allowUpdate);

/*******************************************************************************
 * Code
 ******************************************************************************/

/*******************************************************************************
 *
 * It is expected that prior to the POWER_SYS_Init() call the write-once protection
 * register was configured appropriately allowing entry to all required low power
 * modes.
 * The following is an example of how to set up two power modes and one
 * callback, and initialize the Power manager with structures containing their settings.
 * The example shows two possible ways the configuration structures can be stored
 * (ROM or RAM), although it is expected that they will be placed in the read-only
 * memory to save the RAM space. (Note: In the example it is assumed that the programmed chip
 * doesn't support any optional power options described in the power_manager_user_config_t)
 * :
 * @code
 *
 *  power_manager_user_config_t vlprConfig = {   vlprConfig power mode configuration
 *     .powerMode = POWER_MANAGER_VLPR,
 *     .sleepOnExitValue = false,
 *  };
 *
 *  power_manager_user_config_t stopConfig = {   stopConfig power mode configuration
 *     .powerMode = POWER_MANAGER_STOP,
 *     .sleepOnExitValue = false,
 *  };
 *
 *  power_manager_user_config_t const * powerConfigsArr[] = {    Power mode configurations array
 *     &vlprConfig,
 *     &stopConfig
 *  };
 *
 *  power_manager_callback_user_config_t callbackCfg0 = {  Callback configuration structure callbackCfg0
 *     .callbackFunction                     = &callback0,
 *     .callbackType                         = POWER_MANAGER_CALLBACK_BEFORE_AFTER,
 *     .callbackData                         = (void *)0,
 *  };
 *
 *  power_manager_callback_user_config_t const * callbacksConfigsArr[] = {  Callback configuration structures array
 *     &callbackCfg0
 *  };
 *
 *  status_t callback0(power_manager_notify_struct_t * notify,   Definition of power manager callback
 *                                      power_manager_callback_data_t * dataPtr)
 *  {
 *   status_t ret = STATUS_SUCCESS;
 *   ...
 *   return ret;
 *  }
 *
 *  int main(void) Main function
 *  {
 *   status_t ret = STATUS_SUCCESS;
 *
 *   Calling of init method
 *   POWER_SYS_Init(&powerConfigsArr, 2U, &powerStaticCallbacksConfigsArr, 1U);
 *
 *   Switch to VLPR mode
 *   ret = POWER_SYS_SetMode(MODE_VLPR,POWER_MANAGER_POLICY_AGREEMENT);
 *
 *   if (ret != STATUS_SUCCESS)
 *   {
 *     return -1;
 *   }
 *   return 0;
 *  }
 *
 * @endcode
 *
 *END**************************************************************************/
status_t POWER_SYS_DoInit(void)
{
    uint8_t k = 0U;

    smc_power_mode_protection_config_t powerModeProtConfig;
#if FEATURE_SMC_HAS_HIGH_SPEED_RUN_MODE
    powerModeProtConfig.hsrunProt = false;
#endif /* #if FEATURE_SMC_HAS_HIGH_SPEED_RUN_MODE */
    powerModeProtConfig.vlpProt = false;

    for (k = 0;k < gPowerManagerState.configsNumber;k++)
    {
        const power_manager_user_config_t * const config = (*gPowerManagerState.configs)[k];
#if FEATURE_SMC_HAS_HIGH_SPEED_RUN_MODE
        if (config->powerMode == POWER_MANAGER_HSRUN)
        {
            powerModeProtConfig.hsrunProt  =   true;  /* High speed mode is allowed. */
        }
#endif /* #if FEATURE_SMC_HAS_HIGH_SPEED_RUN_MODE */
        if ((config->powerMode == POWER_MANAGER_VLPR) || (config->powerMode == POWER_MANAGER_VLPS))
        {
            powerModeProtConfig.vlpProt    =   true; /* Very low power mode is allowed. */
        }
    }
    /* Very low power modes and high speed mode are not protected. */
    SMC_SetProtectionMode(SMC, &powerModeProtConfig);

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : POWER_SYS_DoDeinit
 * Description   : This function performs the actual implementation-specific de-initialization.
 *
 *
 *END**************************************************************************/
status_t POWER_SYS_DoDeinit(void)
{
    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : POWER_SYS_DoSetMode
 * Description   : This function performs the actual implementation-specific logic to switch
 * to one of the defined power modes.
 *
 *
 *END**************************************************************************/
status_t POWER_SYS_DoSetMode(const power_manager_user_config_t * const configPtr)
{
    status_t returnCode; /* Function return */

    /* Check whether the power mode is a sleeping or a running power mode */
    if (configPtr->powerMode <= POWER_MANAGER_VLPR)
    {
        /* Switch to a running power mode */
        returnCode = POWER_SYS_SwitchToRunningPowerMode(configPtr);
    }
    else
    {
        /* Switch to a sleeping power mode */
        returnCode = POWER_SYS_SwitchToSleepingPowerMode(configPtr);
    }

    return returnCode;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : POWER_SYS_GetCurrentMode
 * Description   : Returns currently running power mode.
 *
 * Implements POWER_SYS_GetCurrentMode_Activity
 *
 *END**************************************************************************/
power_manager_modes_t POWER_SYS_GetCurrentMode(void)
{
    power_manager_modes_t retVal;

    switch (SMC_GetPowerModeStatus(SMC))
    {
#if FEATURE_SMC_HAS_HIGH_SPEED_RUN_MODE
        /* High speed run mode */
        case STAT_HSRUN:
            retVal = POWER_MANAGER_HSRUN;
            break;
#endif
        /* Run mode */
        case STAT_RUN:
            retVal = POWER_MANAGER_RUN;
            break;
        /* Very low power run mode */
        case STAT_VLPR:
            retVal = POWER_MANAGER_VLPR;
            break;
        /* This should never happen - core has to be in some run mode to execute code */
        default:
            retVal = POWER_MANAGER_MAX;
            break;
    }

    return retVal;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : POWER_SYS_WaitForModeStatus
 * Description   : Internal function used by POWER_SYS_SwitchToSleepingPowerMode and
 * POWER_SYS_SwitchToRunningPowerMode functions
 *
 *END**************************************************************************/
static status_t POWER_SYS_WaitForModeStatus(smc_run_mode_t mode)
{
    status_t retCode;
    power_mode_stat_t modeStat;
    uint32_t i = 0U;

    switch (mode)
    {
        case SMC_RUN:
            modeStat = STAT_RUN;
            retCode = STATUS_SUCCESS;
            break;
        case SMC_VLPR:
            modeStat = STAT_VLPR;
            retCode = STATUS_SUCCESS;
            break;
#if FEATURE_SMC_HAS_HIGH_SPEED_RUN_MODE
        case SMC_HSRUN:
            modeStat = STAT_HSRUN;
            retCode = STATUS_SUCCESS;
            break;
#endif /* #if FEATURE_SMC_HAS_HIGH_SPEED_RUN_MODE */
        default:
            /* invalid parameter */
            modeStat = STAT_INVALID;
            retCode = STATUS_UNSUPPORTED;
            break;
    }

    if (STATUS_SUCCESS == retCode)
    {
        for ( ; i < POWER_SET_MODE_TIMEOUT; i++)
        {
            if (SMC_GetPowerModeStatus(SMC) == modeStat)
            {
                break;
            }
        }
    }

    if (i >= POWER_SET_MODE_TIMEOUT)
    {
        retCode = STATUS_MCU_TRANSITION_FAILED;
    }

    return retCode;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : POWER_SYS_SwitchToRunningPowerMode
 * Description   :Internal function used by POWER_SYS_SetMode function to switch to a running power mode
 * configPtr   pointer to the requested user-defined power mode configuration.
 * System clock source must be SIRC or SOSC in Run mode before transition very low power run  mode.
 * Update initialization  or default clock source in run mode when came back from very low power run mode.
 *
 *END**************************************************************************/
static status_t POWER_SYS_SwitchToRunningPowerMode(const power_manager_user_config_t * const configPtr)
{
        /* Save system clock configure */
    static sys_clk_config_t sysClkConfig;
    /* Confirm change clock when switch very low power run mode */
    static bool changeClkVlpr = false;
    smc_power_mode_config_t modeConfig; /* SMC hardware layer configuration structure */
    power_mode_stat_t currentMode = SMC_GetPowerModeStatus(SMC);
    status_t returnCode = STATUS_SUCCESS;

    /* Configure the running mode */
    switch (configPtr->powerMode)
    {
#if FEATURE_SMC_HAS_HIGH_SPEED_RUN_MODE
        /* High speed run mode */
        case POWER_MANAGER_HSRUN:
            /* High speed run mode can be entered only from Run mode */
            if (currentMode != STAT_HSRUN)
            {
                if (currentMode != STAT_RUN)
                {
                    SMC_SetRunModeControl(SMC, SMC_RUN);
                    returnCode = POWER_SYS_WaitForModeStatus(SMC_RUN);
                }
                if (returnCode == STATUS_SUCCESS)
                {
                    /* Update init clock configuration */
                    returnCode = POWER_DRV_UpdateInitClk(&sysClkConfig, changeClkVlpr);
                    if (returnCode == STATUS_SUCCESS)
                    {
                        changeClkVlpr = false;
                        modeConfig.powerModeName = POWER_MANAGER_HSRUN;
                        /* Switch the mode */
                        returnCode = SMC_SetPowerMode(SMC, &modeConfig);
                    }
                }
            }
            else
            {
                returnCode = STATUS_SUCCESS;
            }
            break;
#endif /* if FEATURE_SMC_HAS_HIGH_SPEED_RUN_MODE */
        /* Run mode */
        case POWER_MANAGER_RUN:
            if (currentMode != STAT_RUN)
            {
                modeConfig.powerModeName = POWER_MANAGER_RUN;
                /* Switch the mode */
                returnCode = SMC_SetPowerMode(SMC, &modeConfig);
            }
            if (returnCode == STATUS_SUCCESS)
            {
                /* Update init clock configuration */
                returnCode = POWER_DRV_UpdateInitClk(&sysClkConfig, changeClkVlpr);
                /* Clear value of changeClkVlpr after switch clock configure successfully. */
                if (returnCode == STATUS_SUCCESS)
                {
                    changeClkVlpr = false;
                }
            }

            break;
        /* Very low power run mode */
        case POWER_MANAGER_VLPR:
            if (currentMode != STAT_VLPR)
            {
                /* Very low power run mode can be entered only from Run mode */
                if (SMC_GetPowerModeStatus(SMC) != STAT_RUN)
                {
                    SMC_SetRunModeControl(SMC, SMC_RUN);
                    returnCode = POWER_SYS_WaitForModeStatus(SMC_RUN);
                }
                if (STATUS_SUCCESS == returnCode)
                {
                    CLOCK_DRV_GetSystemClockSource(&sysClkConfig);
                    returnCode = POWER_DRV_SwitchVlprClk(&sysClkConfig);
                    if (STATUS_SUCCESS == returnCode)
                    {
                        changeClkVlpr = true;
                        modeConfig.powerModeName = POWER_MANAGER_VLPR;
                        /* Switch the mode */
                        returnCode = SMC_SetPowerMode(SMC, &modeConfig);
                    }
                }
            }
            else
            {
                returnCode = STATUS_SUCCESS;
            }

            break;
        /* Wait mode */
        default:
            /* invalid power mode */
            returnCode = STATUS_UNSUPPORTED;
            modeConfig.powerModeName = POWER_MANAGER_MAX;
            break;
    }

    return returnCode;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : POWER_SYS_SwitchToSleepingPowerMode
 * Description   :Internal function used by POWER_SYS_SetMode function to switch to a sleeping power mode
 * configPtr   pointer to the requested user-defined power mode configuration
 *
 *END**************************************************************************/
static status_t POWER_SYS_SwitchToSleepingPowerMode(const power_manager_user_config_t * const configPtr)
{
    smc_power_mode_config_t modeConfig; /* SMC hardware layer configuration structure */
    status_t returnCode = STATUS_SUCCESS; /* return value */
    power_mode_stat_t pwrModeStat = SMC_GetPowerModeStatus(SMC);         /* power mode stat */

    /* Configure the hardware layer */
    switch (configPtr->powerMode)
    {
#if FEATURE_SMC_HAS_WAIT_VLPW
        /* Wait mode */
        case POWER_MANAGER_WAIT:
            /* Wait mode can be entered only from Run mode */
            if (pwrModeStat != STAT_RUN)
            {
                SMC_SetRunModeControl(SMC, SMC_RUN);
                returnCode = POWER_SYS_WaitForModeStatus(SMC_RUN);
            }

            modeConfig.powerModeName = POWER_MANAGER_WAIT;
            break;
        /* Very low power wait mode */
        case POWER_MANAGER_VLPW:
            /* Very low power wait mode can be entered only from Very low power run mode */
            if (pwrModeStat != STAT_VLPR)
            {
                SMC_SetRunModeControl(SMC, SMC_VLPR);
                returnCode = POWER_SYS_WaitForModeStatus(SMC_VLPR);
            }

            modeConfig.powerModeName = POWER_MANAGER_VLPW;
            break;
#endif /* if FEATURE_SMC_HAS_WAIT_VLPW */
#if FEATURE_SMC_HAS_PSTOPO
        /* Partial stop modes */
        case POWER_MANAGER_PSTOP1:
        /* fall-through */
        case POWER_MANAGER_PSTOP2:
            /* fall-through */
#endif
#if FEATURE_SMC_HAS_STOPO
        /* Stop modes */
        case POWER_MANAGER_STOP1:
        /* fall-through */
        case POWER_MANAGER_STOP2:
            /* Stop1 and Stop2 mode can be entered only from Run mode */
            if (pwrModeStat != STAT_RUN)
            {
                SMC_SetRunModeControl(SMC, SMC_RUN);
                returnCode = POWER_SYS_WaitForModeStatus(SMC_RUN);
            }

            modeConfig.powerModeName = configPtr->powerMode;
#endif /* #if FEATURE_SMC_HAS_STOPO */
#if FEATURE_SMC_HAS_PSTOPO
            modeConfig.pstopOption = true;
            /* Set the partial stop option value */
            if (POWER_MANAGER_PSTOP1 == configPtr->powerMode)
            {
                modeConfig.pstopOptionValue = SMC_PSTOP_STOP1;
            }
            else if (POWER_MANAGER_PSTOP2 == configPtr->powerMode)
            {
                modeConfig.pstopOptionValue = SMC_PSTOP_STOP2;
            }
            else
            {
                modeConfig.pstopOptionValue = SMC_PSTOP_STOP;
            }

#endif /* if FEATURE_SMC_HAS_PSTOPO */
#if FEATURE_SMC_HAS_STOPO
            /* Set the stop option value */
            if (POWER_MANAGER_STOP1 == configPtr->powerMode)
            {
                modeConfig.stopOptionValue = SMC_STOP1;
            }
            else
            {
                modeConfig.stopOptionValue = SMC_STOP2;
            }
#endif /* if FEATURE_SMC_HAS_STOPO */
            break;
        /* Very low power stop mode */
        case POWER_MANAGER_VLPS:
            /* Very low power stop mode can be entered only from Run mode or Very low power run mode*/
            if ((pwrModeStat != STAT_RUN) && (pwrModeStat != STAT_VLPR))
            {
                modeConfig.powerModeName = POWER_MANAGER_RUN;
                returnCode = SMC_SetPowerMode(SMC, &modeConfig);
            }

            modeConfig.powerModeName = POWER_MANAGER_VLPS;
            break;
        default:
            /* invalid power mode */
            returnCode = STATUS_UNSUPPORTED;
            modeConfig.powerModeName = POWER_MANAGER_MAX;
            break;
    }

    if (STATUS_SUCCESS == returnCode)
    {
        /* Configure ARM core what to do after interrupt invoked in (deep) sleep state */
            if (configPtr->sleepOnExitValue)
            {
                /* Go back to (deep) sleep state on ISR exit */
                S32_SCB->SCR |= S32_SCB_SCR_SLEEPONEXIT_MASK;
            }
            else
            {
                /* Do not re-enter (deep) sleep state on ISR exit */
                S32_SCB->SCR &= ~(S32_SCB_SCR_SLEEPONEXIT_MASK);
            }

        /* Switch the mode */
        if (SMC_SetPowerMode(SMC, &modeConfig) != STATUS_SUCCESS)
        {
            returnCode = STATUS_MCU_TRANSITION_FAILED;
        }
    }

    return returnCode;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : POWER_DRV_SwitchVlprClk
 * Description   : This function will change system clock in run mode before MCU enter very low power run mode.
 *
 *
 *END**************************************************************************/
static status_t POWER_DRV_SwitchVlprClk(const sys_clk_config_t * const sysClock)
{
    status_t retCode = STATUS_SUCCESS;
    sys_clk_config_t sysClkVlprConfig;
    clock_names_t currentSystemClockSource = sysClock->src;

    if ((currentSystemClockSource != SIRC_CLK) && (currentSystemClockSource != SOSC_CLK))
    {
        /* Set SIRC the system clock source */
        sysClkVlprConfig.src = SIRC_CLK;
        sysClkVlprConfig.dividers[0U] = 1U;                      /* Core clock divider, do not divide */
        sysClkVlprConfig.dividers[1U] = 1U;                      /* Bus clock divider, do not divide */
        sysClkVlprConfig.dividers[2U] = 1U;                      /* Slow clock divider, do not divide */
        retCode = CLOCK_DRV_SetSystemClock(NULL,&sysClkVlprConfig);
        if (STATUS_SUCCESS != retCode)
        {
            /* Set SOSC the system clock source */
            sysClkVlprConfig.src = SOSC_CLK;
            sysClkVlprConfig.dividers[0U] = 1U;                 /* Core clock divider, do not divide */
            sysClkVlprConfig.dividers[1U] = 2U;                 /* Bus clock divider, divide by two */
            sysClkVlprConfig.dividers[2U] = 2U;                 /* Slow clock divider, divide by two */

            retCode = CLOCK_DRV_SetSystemClock(NULL,&sysClkVlprConfig);
            if (STATUS_SUCCESS != retCode)
            {
                /* Can't switch clock before enter VLPR */
                retCode = STATUS_ERROR;
            }
        }
    }

    return retCode;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : POWER_DRV_UpdateInitClk
 * Description   : This function will  update initialization or default clock source of run mode when MCU come back run mode.
 *
 *
 *END**************************************************************************/
static status_t POWER_DRV_UpdateInitClk(const sys_clk_config_t * const sysClk, bool allowUpdate)
{
    status_t retCode = STATUS_SUCCESS;
    sys_clk_config_t sysClkDefault;

    /* Update if it 's allowed */
    if (allowUpdate)
    {
        retCode = CLOCK_DRV_SetSystemClock(NULL,sysClk);
        if (retCode != STATUS_SUCCESS)
        {
            sysClkDefault.src = FIRC_CLK;
            sysClkDefault.dividers[0U] = 1U;                 /* Core clock divider, do not divide */
            sysClkDefault.dividers[1U] = 2U;                 /* Bus clock divider, divide by two */
            sysClkDefault.dividers[2U] = 2U;                 /* Slow clock divider, divide by two */

            retCode = CLOCK_DRV_SetSystemClock(NULL,&sysClkDefault);
            if (STATUS_SUCCESS != retCode)
            {
                /* Can't transition clock in Run mode */
                retCode = STATUS_ERROR;
            }
        }
    }

    return retCode;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : POWER_SYS_GetResetSrcStatusCmd
 * Description   : This function will get the current reset source status for specified source
 *
 * Implements POWER_SYS_GetResetSrcStatusCmd_Activity
 *
 *END**************************************************************************/
bool POWER_SYS_GetResetSrcStatusCmd(const RCM_Type * const baseAddr , const rcm_source_names_t srcName)
{
    return RCM_GetSrcStatusCmd(baseAddr , srcName);
}


/*******************************************************************************
 * EOF
 ******************************************************************************/
