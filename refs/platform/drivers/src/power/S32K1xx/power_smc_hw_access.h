/*
 * Copyright (c) 2013-2016, Freescale Semiconductor, Inc.
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

#ifndef POWER_SMC_HW_ACCESS_H
#define POWER_SMC_HW_ACCESS_H

#include "status.h"
#include "device_registers.h"
#include "power_manager_S32K1xx.h"

/*!
 * @file power_smc_hw_access.h
 *
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 2.3, Global typedef not referenced.
 * User configuration structure is defined in Hal and is referenced from Driver.
 */

/*!
 * power_smc_hw_access
 * @{
 */

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

/*! @name System mode controller APIs*/
/*@{*/

/*!
 * @brief Get the version of the SMC module
 *
 * @param[in]  baseAddr  Base address of the SMC module
 * @param[out] versionInfo  Device Version Number
 */
void SMC_GetVersion(const SMC_Type * const baseAddr,
                    smc_version_info_t * const versionInfo);

/*!
 * @brief Configures the power mode.
 *
 * This function configures the power mode control for both run, stop, and
 * stop sub mode if needed. Also it configures the power options for a specific
 * power mode. An application should follow the proper procedure to configure and
 * switch power modes between  different run and stop modes. For proper procedures
 * and supported power modes, see an appropriate chip reference
 * manual. See the smc_power_mode_config_t for required
 * parameters to configure the power mode and the supported options. Other options
 * may need to be individually configured through the HAL driver. See the HAL driver
 * header file for details.
 *
 * @param baseAddr  Base address for current SMC instance.
 * @param powerModeConfig Power mode configuration structure smc_power_mode_config_t
 * @return errorCode SMC error code
 */
status_t SMC_SetPowerMode(SMC_Type * const baseAddr,
                          const smc_power_mode_config_t * const powerModeConfig);

/*!
 * @brief Configures all power mode protection settings.
 *
 * This function configures the power mode protection settings for
 * supported power modes in the specified chip family. The available power modes
 * are defined in the smc_power_mode_protection_config_t. An application should provide
 * the protect settings for all supported power modes on the chip. This
 * should be done at an early system level initialization stage. See the reference manual
 * for details. This register can only write once after the power reset. If the user has
 * only a single option to set,
 * either use this function or use the individual set function.
 *
 *
 * @param[in] baseAddr  Base address for current SMC instance.
 * @param[in] protectConfig Configurations for the supported power mode protect settings
 *                      - See smc_power_mode_protection_config_t for details.
 */
void SMC_SetProtectionMode(SMC_Type * const baseAddr,
                           const smc_power_mode_protection_config_t * const protectConfig);

/*!
 * @brief Gets the the current power mode protection setting.
 *
 * This function gets the current power mode protection settings for
 * a specified power mode.
 *
 * @param baseAddr[in]  Base address for current SMC instance.
 * @param protect[in]   Power mode to set for protection
 * @return state  Status of the protection setting
 *                - true: Allowed
 *                - false: Not allowed
 */
bool SMC_GetProtectionMode(const SMC_Type * const baseAddr,
                           const power_modes_protect_t protect);

/*!
 * @brief Configures the the RUN mode control setting.
 *
 * This function sets the run mode settings, for example, normal run mode,
 * very lower power run mode, etc. See the smc_run_mode_t for supported run
 * mode on the chip family and the reference manual for details about the
 * run mode.
 *
 * @param[in] baseAddr  Base address for current SMC instance.
 * @param[in] runMode   Run mode setting defined in smc_run_mode_t
 */
static inline void SMC_SetRunModeControl(SMC_Type * const baseAddr,
                                         const smc_run_mode_t runMode)
{
    uint32_t regValue = baseAddr->PMCTRL;
    regValue &= ~(SMC_PMCTRL_RUNM_MASK);
    regValue |= SMC_PMCTRL_RUNM(runMode);
    baseAddr->PMCTRL = regValue;
}

/*!
 * @brief Gets  the current RUN mode configuration setting.
 *
 * This function gets the run mode settings. See the smc_run_mode_t
 * for a supported run mode on the chip family and the reference manual for
 * details about the run mode.
 *
 * @param[in] baseAddr  Base address for current SMC instance.
 * @return setting Run mode configuration setting
 */
static inline smc_run_mode_t SMC_GetRunModeControl(const SMC_Type * const baseAddr)
{
    smc_run_mode_t retValue;
    uint32_t regValue = baseAddr->PMCTRL;
    regValue = (regValue & SMC_PMCTRL_RUNM_MASK) >> SMC_PMCTRL_RUNM_SHIFT;

    switch (regValue)
    {
        case 0UL:
            retValue = SMC_RUN;
            break;
        case 1UL:
            retValue = SMC_RESERVED_RUN;
            break;
        case 2UL:
            retValue = SMC_VLPR;
            break;
        case 3UL:
        default:
            retValue = SMC_HSRUN;
            break;
    }

    return retValue;
}

/*!
 * @brief Configures  the STOP mode control setting.
 *
 * This function sets the stop mode settings, for example, normal stop mode,
 * very lower power stop mode, etc. See the smc_stop_mode_t for supported stop
 * mode on the chip family and the reference manual for details about the
 * stop mode.
 *
 * @param[in] baseAddr  Base address for current SMC instance.
 * @param[in] stopMode  Stop mode defined in smc_stop_mode_t
 */
static inline void SMC_SetStopModeControl(SMC_Type * const baseAddr,
                                          const smc_stop_mode_t stopMode)
{
    uint32_t regValue = baseAddr->PMCTRL;
    regValue &= ~(SMC_PMCTRL_STOPM_MASK);
    regValue |= SMC_PMCTRL_STOPM(stopMode);
    baseAddr->PMCTRL = regValue;
}

/*!
 * @brief Checks whether the last very low power stop sequence has been aborted.
 *
 * This function checks whether the last very low power stop sequence has been aborted.
 *
 * @param[in] baseAddr  Base address for current SMC instance.
 * @return aborted      Aborted or not
 */
static inline smc_stop_mode_t SMC_GetVlpsaModeControl(const SMC_Type * const baseAddr)
{
    smc_stop_mode_t retValue;
    uint32_t regValue = baseAddr->PMCTRL;
    regValue = (regValue & SMC_PMCTRL_VLPSA_MASK) >> SMC_PMCTRL_VLPSA_SHIFT;

    switch (regValue)
    {
        case 0UL:
            retValue = SMC_STOP;
            break;
        case 2UL:
            retValue = SMC_VLPS;
            break;
        case 1UL:
        /* pass-through */
        default:
            retValue = SMC_RESERVED_STOP1;
            break;
    }

    return retValue;
}

/*!
 * @brief Gets the current STOP mode control settings.
 *
 * This function gets the stop mode settings, for example, normal stop mode,
 * very lower power stop mode, etc. See the smc_stop_mode_t for supported stop
 * mode on the chip family and the reference manual for details about the
 * stop mode.
 *
 * @param[in] baseAddr  Base address for current SMC instance.
 * @return setting Current stop mode configuration setting
 */
static inline smc_stop_mode_t SMC_GetStopModeControl(const SMC_Type * const baseAddr)
{
    smc_stop_mode_t retValue;
    uint32_t regValue = baseAddr->PMCTRL;
    regValue = (regValue & SMC_PMCTRL_STOPM_MASK) >> SMC_PMCTRL_STOPM_SHIFT;

    switch (regValue)
    {
        case 0UL:
            retValue = SMC_STOP;
            break;
        case 2UL:
            retValue = SMC_VLPS;
            break;
        case 1UL:
        /* pass-through */
        default:
            retValue = SMC_RESERVED_STOP1;
            break;
    }

    return retValue;
}

#if FEATURE_SMC_HAS_STOPO
/*!
 * @brief Configures the STOPO (Stop Option).
 *
 * It controls the type of the stop operation when STOPM=STOP. When entering Stop mode
 * from RUN mode, the PMC, SCG and flash remain fully powered, allowing the device to
 * wakeup almost instantaneously at the expense of higher power consumption. In STOP2,
 * only system clocks are gated allowing peripherals running on bus clock to remain fully
 * functional. In STOP1, both system and bus clocks are gated.
 *
 * @param[in] baseAddr  Base address for current SMC instance.
 * @param[in] option STOPO option setting defined in smc_stop_option_t
 */
static inline void SMC_SetStopOption(SMC_Type * const baseAddr,
                                     const smc_stop_option_t option)
{
    uint32_t regValue = baseAddr->STOPCTRL;
    regValue &= ~(SMC_STOPCTRL_STOPO_MASK);
    regValue |= SMC_STOPCTRL_STOPO(option);
    baseAddr->STOPCTRL = regValue;
}

/*!
 * @brief Gets the configuration of the STOPO option.
 *
 * This function gets the current STOPO option setting. See the configuration
 * function for more details.
 *
 * @param[in] baseAddr  Base address for current SMC instance.
 * @return option Current STOPO option setting
 */
static inline smc_stop_option_t SMC_GetStopOption(const SMC_Type * const baseAddr)
{
    smc_stop_option_t retValue;
    uint32_t regValue = baseAddr->STOPCTRL;
    regValue = (regValue & SMC_STOPCTRL_STOPO_MASK) >> SMC_STOPCTRL_STOPO_SHIFT;

    switch (regValue)
    {
        case 1UL:
            retValue = SMC_STOP1;
            break;
        case 2UL:
            retValue = SMC_STOP2;
            break;
        case 0UL:
        /* Pass-through */
        default:
            retValue = SMC_STOP_RESERVED;
            break;
    }

    return retValue;
}

#endif /* if FEATURE_SMC_HAS_STOPO */

#if FEATURE_SMC_HAS_PSTOPO

#error "Unimplemented"

#endif

/*!
 * @brief Gets the current power mode stat.
 *
 * This function returns the current power mode stat. Once application
 * switches the power mode, it should always check the stat to check whether it
 * runs into the specified mode or not. An application should check
 * this mode before switching to a different mode. The system requires that
 * only certain modes can switch to other specific modes. See the
 * reference manual for details and the power_mode_stat for information about
 * the power stat.
 *
 * @param[in] baseAddr  Base address for current SMC instance.
 * @return stat  Current power mode stat
 */
static inline power_mode_stat_t SMC_GetPowerModeStatus(const SMC_Type * const baseAddr)
{
    power_mode_stat_t retValue;
    uint32_t regValue = baseAddr->PMSTAT;
    regValue = (regValue & SMC_PMSTAT_PMSTAT_MASK) >> SMC_PMSTAT_PMSTAT_SHIFT;

    switch (regValue)
    {
        case 1UL:
            retValue = STAT_RUN;
            break;
        case 2UL:
            retValue = STAT_STOP;
            break;
        case 4UL:
            retValue = STAT_VLPR;
            break;
        case 8UL:
            retValue = STAT_VLPW;
            break;
        case 16UL:
            retValue = STAT_VLPS;
            break;
        case 128UL:
            retValue = STAT_HSRUN;
            break;
        case 255UL:
        default:
            retValue = STAT_INVALID;
            break;
    }

    return retValue;
}

/*@}*/

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

/*! @}*/

#endif /* POWER_SMC_HW_ACCESS_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
