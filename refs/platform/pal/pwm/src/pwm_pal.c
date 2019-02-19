/*
 * Copyright 2017 NXP
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

#include "pwm_pal.h"

/* Define state structures for FTM */
#if (defined(PWM_OVER_FTM))
    /*! @brief FTM state structures */
    ftm_state_t FtmState[NO_OF_FTM_INSTS_FOR_PWM];
    /*! @brief FTM state-instance matching */
    static pwm_instance_t FtmStateInstanceMapping[NO_OF_FTM_INSTS_FOR_PWM];
    /*! @brief FTM  available resources table */
    static bool FtmStateIsAllocated[NO_OF_FTM_INSTS_FOR_PWM];
#endif

/*FUNCTION**********************************************************************
 *
 * Function Name : PwmAllocateState
 * Description   : Allocates one of the available state structure.
 *
 *END**************************************************************************/
static uint8_t PwmAllocateState(bool* isAllocated, pwm_instance_t* instanceMapping, pwm_instance_t instance, uint8_t numberOfinstances)
{
    uint8_t i;
    /* Allocate one of the FTM state structure for this instance */
    for (i = 0;i < numberOfinstances;i++)
    {
        if (isAllocated[i] == false)
        {
            instanceMapping[i] = instance;
            isAllocated[i] = true;
            break;
        }
    }
    return i;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : PwmFreeState
 * Description   : Deallocates one of the available state structure.
 *
 *END**************************************************************************/
static void PwmFreeState(bool* isAllocated, pwm_instance_t* instanceMapping, pwm_instance_t instance, uint8_t numberOfinstances)
{
    uint8_t i;
    /* Allocate one of the FTM state structure for this instance */
    for (i = 0;i < numberOfinstances;i++)
    {
        if (instanceMapping[i] == instance)
        {
            isAllocated[i] = false;
            break;
        }
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : PWM_Init
 * Description   : Initialize PWM channels based on config parameter.
 *
 * Implements    : PWM_Init_Activity
 *END**************************************************************************/
status_t PWM_Init(pwm_instance_t instance, pwm_global_config_t* config)
{
    /* Declare internal variables */
    uint8_t channel;
    uint16_t firstEdge;
    uint8_t index;
    /* Declare the configuration structure for FTM */
    ftm_pwm_param_t ftmPwmConfig;
    ftm_independent_ch_param_t pwmIndependentChannelConfig[FEATURE_FTM_CHANNEL_COUNT];
    ftm_combined_ch_param_t pwmCombinedChannelConfig[FEATURE_FTM_CHANNEL_COUNT >> 1U];
    ftm_user_config_t ftmGlobalConfig;
    /* Fault control feature is not supported, but this structure is filled with values which disable fault control */
    ftm_pwm_fault_param_t faultConfig =
    {
        false, /* Output pin state on fault */
        false, /* PWM fault interrupt state */
        0U, /* Fault filter value */
        FTM_FAULT_CONTROL_DISABLED,  /* Fault mode */
        {
            {
                false, /* Fault channel state (Enabled/Disabled) */
                false, /* Fault channel filter state (Enabled/Disabled) */
                FTM_POLARITY_LOW, /* Fault channel state (Enabled/Disabled) */
            },
            {
                false, /* Fault channel state (Enabled/Disabled) */
                false, /* Fault channel filter state (Enabled/Disabled) */
                FTM_POLARITY_LOW, /* Fault channel state (Enabled/Disabled) */
            },
            {
                false, /* Fault channel state (Enabled/Disabled) */
                false, /* Fault channel filter state (Enabled/Disabled) */
                FTM_POLARITY_LOW, /* Fault channel state (Enabled/Disabled) */
            },
            {
                false, /* Fault channel state (Enabled/Disabled) */
                false, /* Fault channel filter state (Enabled/Disabled) */
                FTM_POLARITY_LOW, /* Fault channel state (Enabled/Disabled) */
            }
       }
    };

    ftmPwmConfig.pwmCombinedChannelConfig = pwmCombinedChannelConfig;
    ftmPwmConfig.pwmIndependentChannelConfig = pwmIndependentChannelConfig;
    ftmPwmConfig.faultConfig = &faultConfig;

    /* Because FTM has only one timebase first channel is used to configure FTM clocking*/
    ftmGlobalConfig.ftmClockSource = ((pwm_ftm_timebase_t*)(config->pwmChannels[0].timebase))->sourceClock;
    ftmGlobalConfig.ftmPrescaler = ((pwm_ftm_timebase_t*)(config->pwmChannels[0].timebase))->prescaler;
    ftmPwmConfig.deadTimePrescaler = ((pwm_ftm_timebase_t*)(config->pwmChannels[0].timebase))->deadtimePrescaler;

    /* Add dummy value for frequency */
    ftmPwmConfig.uFrequencyHZ = 0x4000;

    /* Configure FTM mode according to first channel setup.
     * All PWM channels must be PWM_EDGE_ALIGNED/PWM_Shifted or PWM_CENTER_ALIGNED */
    if ((config->pwmChannels[0].channelType) == PWM_CENTER_ALIGNED)
    {
        ftmPwmConfig.mode = FTM_MODE_CEN_ALIGNED_PWM;
    }
    else
    {
        ftmPwmConfig.mode = FTM_MODE_EDGE_ALIGNED_PWM;
    }
    ftmGlobalConfig.ftmMode =  ftmPwmConfig.mode;

    /* The synchronization for duty, period and phase shift will be update when
     * signal period is done. Only overwrite function shall take effect immediate.
     */
    ftmGlobalConfig.syncMethod.softwareSync = true;
    ftmGlobalConfig.syncMethod.hardwareSync1 = false;
    ftmGlobalConfig.syncMethod.hardwareSync2 = false;
    ftmGlobalConfig.syncMethod.autoClearTrigger = false;
    ftmGlobalConfig.syncMethod.maskRegSync = FTM_SYSTEM_CLOCK;
    ftmGlobalConfig.syncMethod.initCounterSync = FTM_PWM_SYNC;
    ftmGlobalConfig.syncMethod.inverterSync = FTM_PWM_SYNC;
    ftmGlobalConfig.syncMethod.outRegSync = FTM_PWM_SYNC;
    ftmGlobalConfig.syncMethod.maxLoadingPoint = true;
    ftmGlobalConfig.syncMethod.minLoadingPoint = false;
    ftmGlobalConfig.syncMethod.syncPoint = FTM_WAIT_LOADING_POINTS;
    ftmGlobalConfig.isTofIsrEnabled = false;
    ftmGlobalConfig.BDMMode = FTM_BDM_MODE_00;

    /* Configure FTM channels */
    ftmPwmConfig.nNumCombinedPwmChannels = 0;
    ftmPwmConfig.nNumIndependentPwmChannels = 0;
    for (channel = 0; channel < config->numberOfPwmChannels; channel++)
    {
        /* This value will be update later because initialization function doesn't support update in ticks. */
        pwmIndependentChannelConfig[ftmPwmConfig.nNumIndependentPwmChannels].uDutyCyclePercent = 0U;
        /* Configure channel number */
        pwmIndependentChannelConfig[ftmPwmConfig.nNumIndependentPwmChannels].hwChannelId = config->pwmChannels[channel].channel;

        /*Configure complementary mode */
        pwmIndependentChannelConfig[ftmPwmConfig.nNumIndependentPwmChannels].enableSecondChannelOutput = config->pwmChannels[channel].enableComplementaryChannel;
        if ((config->pwmChannels[channel].complementaryChannelPolarity) == PWM_DUPLICATED)
        {
            pwmIndependentChannelConfig[ftmPwmConfig.nNumIndependentPwmChannels].secondChannelPolarity = FTM_MAIN_DUPLICATED;
        }
        else
        {
            pwmIndependentChannelConfig[ftmPwmConfig.nNumIndependentPwmChannels].secondChannelPolarity = FTM_MAIN_INVERTED;
        }

        /* Configure channel polarity */
        if (config->pwmChannels[channel].polarity == PWM_ACTIVE_HIGH)
        {
            pwmIndependentChannelConfig[ftmPwmConfig.nNumIndependentPwmChannels].polarity = FTM_POLARITY_HIGH;
        }
        else
        {
            pwmIndependentChannelConfig[ftmPwmConfig.nNumIndependentPwmChannels].polarity = FTM_POLARITY_LOW;
        }
        /* Configure dead-time insertion. For them the dead time configuration is available for all channels.  */
        ftmPwmConfig.deadTimePrescaler = ((pwm_ftm_timebase_t*)(config->pwmChannels[0].timebase))->deadtimePrescaler;
        ftmPwmConfig.deadTimeValue = config->pwmChannels[0].deadtime;
        /* Configure default value for fail safe value. */
        pwmIndependentChannelConfig[ftmPwmConfig.nNumIndependentPwmChannels].levelSelect = FTM_HIGH_TRUE_PULSE;
        pwmIndependentChannelConfig[ftmPwmConfig.nNumIndependentPwmChannels].enableExternalTrigger = false;
        pwmIndependentChannelConfig[ftmPwmConfig.nNumIndependentPwmChannels].deadTime = 0;
        ftmPwmConfig.nNumIndependentPwmChannels++;
    }
    /* Initialize FTM as PWM generator */
    index = PwmAllocateState(FtmStateIsAllocated, FtmStateInstanceMapping, instance, NO_OF_FTM_INSTS_FOR_PWM);
    FTM_DRV_Init(instance, &ftmGlobalConfig, (ftm_state_t*)(&FtmState[index]));
    FTM_DRV_InitPwm(instance, &ftmPwmConfig);

    /* Configure duty and period for all FTM PWM channels */
    for (channel = 0; channel < config->numberOfPwmChannels; channel++)
    {
        DEV_ASSERT(config->pwmChannels[channel].duty <= config->pwmChannels[channel].period);
        /* This cast is assumed because FTM counter is 16 bits wide */
        firstEdge = (uint16_t)config->pwmChannels[channel].duty;
        FTM_DRV_FastUpdatePwmChannels(instance, 1U, &(config->pwmChannels[channel].channel), &firstEdge, false);
    }

    /* Update period and generate software trigger. */
    FTM_DRV_UpdatePwmPeriod(instance, FTM_PWM_UPDATE_IN_TICKS, config->pwmChannels[0].period , true);
    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : PWM_UpdateDuty
 * Description   : Update duty cycle. The measurement unit for duty is clock ticks.
 * Implements    : PWM_UpdateDuty_Activity
 *
 *END**************************************************************************/
status_t PWM_UpdateDuty(pwm_instance_t instance, uint8_t channel, uint32_t duty)
{
    FTM_DRV_UpdatePwmChannel(instance, channel, FTM_PWM_UPDATE_IN_TICKS, (uint16_t)duty, 0, true);
    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : PWM_UpdatePeriod
 * Description   : Update period for specific a specific channel. This function changes period for
 * all channels which shares the timebase with targeted channel.
 * Implements    : PWM_UpdatePeriod_Activity
 *
 *END**************************************************************************/
status_t PWM_UpdatePeriod(pwm_instance_t instance, uint8_t channel, uint32_t period)
{
    FTM_DRV_UpdatePwmPeriod(instance, FTM_PWM_UPDATE_IN_TICKS, (uint16_t)period, true);
    (void)channel;
    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : PWM_OverwriteOutputChannels
 * Description   : This function change the output value for some channels. channelsMask select
 * which channels will be overwrite, each bit filed representing one channel: 1 - channel is controlled
 * by channelsValues, 0 - channel is controlled by pwm. channelsValues select output values to be write on corresponding
 * channel.
 * Implements    : PWM_OverwriteOutputChannels_Activity
 *
 *END**************************************************************************/
status_t PWM_OverwriteOutputChannels(pwm_instance_t instance, uint32_t channelsMask, uint32_t channelsValues)
{
    FTM_DRV_SetAllChnSoftwareOutputControl(instance, (uint16_t)channelsMask, (uint16_t)channelsValues);
    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : PWM_Deinit
 * Description   : Uninitialised PWM instance.
 * Implements    : PWM_Deinit_Activity
 *
 *END**************************************************************************/
status_t PWM_Deinit(pwm_instance_t instance)
{
    FTM_DRV_DeinitPwm(instance);
    FTM_DRV_Deinit(instance);
    PwmFreeState(FtmStateIsAllocated, FtmStateInstanceMapping, instance, NO_OF_FTM_INSTS_FOR_PWM);
    return STATUS_SUCCESS;
}
