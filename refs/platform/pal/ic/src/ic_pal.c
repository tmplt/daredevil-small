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
/*!
 * @file ic_pal.c
 *
 * @page misra_violations MISRA-C:2012 violations
 */

#include "ic_pal.h"


/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Define state structures for IC PAL */
static ic_pal_state_t g_icPalStatePtr[NUMBER_OF_IC_PAL_INSTANCES];

/* IC state-instance matching */
static uint32_t icStateInstanceMapping[NUMBER_OF_IC_PAL_INSTANCES];
/* IC  available resources table */
static bool icStateIsAllocated[NUMBER_OF_IC_PAL_INSTANCES];

#if (defined(IC_PAL_OVER_FTM))
/* The FTM state structures */
static ftm_state_t g_ftmState[NUMBER_OF_IC_PAL_INSTANCES];
#endif
/*******************************************************************************
 * Code
 ******************************************************************************/
/*FUNCTION**********************************************************************
 *
 * Function Name : icAllocateState
 * Description   : Allocates one of the available state structure.
 *
 *END**************************************************************************/
static uint8_t icAllocateState(bool * isAllocated,
                               uint32_t * instanceMapping,
                               uint32_t instance)
{
    uint8_t i;

    /* Allocate one of the IC state structure for this instance */
    for (i = 0U;i < NUMBER_OF_IC_PAL_INSTANCES;i++)
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
 * Function Name : icFreeState
 * Description   : Deallocates one of the available state structure.
 *
 *END**************************************************************************/
static void icFreeState(bool * isAllocated,
                        uint32_t * instanceMapping,
                        uint32_t instance)
{
    uint8_t i;

    /* Allocate one of the IC state structure for this instance */
    for (i = 0U;i < NUMBER_OF_IC_PAL_INSTANCES;i++)
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
 * Function Name : FindIcState
 * Description   : Search the state structure of the instance for each IP
 *
 *END**************************************************************************/
static uint8_t FindIcState(uint32_t instance)
{
    uint8_t i;

    for (i = 0U;i < NUMBER_OF_IC_PAL_INSTANCES;i++)
    {
        if (icStateInstanceMapping[i] == instance)
        {
            break;
        }
    }

    return i;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : IC_Init
 * Description   : This function will initialize the IC PAL instance, including the
 * other platform specific HW units used together in the input capture mode. This
 * function configures a group of channels in instance to detect or measure the
 * input signal.
 *
 * Implements    : IC_Init_Activity
 *END**************************************************************************/
status_t IC_Init(uint32_t instance,
                 const ic_config_t * configPtr)
{
    DEV_ASSERT(configPtr != NULL);
    DEV_ASSERT(instance < IC_PAL_INSTANCE_MAX);
    ic_pal_state_t * icState;
    status_t status = STATUS_SUCCESS;
    uint8_t index = 0U;
    uint8_t indexInstance = 0U;

    /* Allocate one of the IC state structure for this instance */
    indexInstance = icAllocateState(icStateIsAllocated, icStateInstanceMapping, instance);
    icState = &g_icPalStatePtr[indexInstance];
    DEV_ASSERT(icState->nNumChannels == 0x0U);

    /* Initialize the internal context to default value */
    icState->nNumChannels = configPtr->nNumChannels;
    for (index = 0U; index < IC_PAL_NUM_OF_CHANNEL_MAX; index++)
    {
        icState->channelConfigArray[index] = 0U;
        icState->icChannelType[index] = IC_DISABLE_OPERATION;
    }

#if (defined(IC_PAL_OVER_FTM))
    uint8_t channel;
    ftm_pwm_sync_t sync;
    ftm_user_config_t info;
    ftm_state_t * ftmState = &g_ftmState[indexInstance];
    ftm_input_param_t param;
    ftm_input_ch_param_t channelConfig[FEATURE_FTM_CHANNEL_COUNT];

    /* Configure the synchronous to default */
    sync.softwareSync = true;
    sync.hardwareSync0 = false;
    sync.hardwareSync1 = false;
    sync.hardwareSync2 = false;
    sync.maxLoadingPoint = true;
    sync.minLoadingPoint = false;
    sync.inverterSync = FTM_SYSTEM_CLOCK;
    sync.outRegSync = FTM_SYSTEM_CLOCK;
    sync.maskRegSync = FTM_SYSTEM_CLOCK;
    sync.initCounterSync = FTM_SYSTEM_CLOCK;
    sync.autoClearTrigger = false;
    sync.syncPoint = FTM_UPDATE_NOW;

    /* Get some information from user configuration */
    info.syncMethod = sync;
    info.ftmMode = FTM_MODE_INPUT_CAPTURE;
    info.ftmPrescaler = ((extension_ftm_for_ic_t *)(configPtr->extension))->ftmPrescaler;
    info.ftmClockSource = ((extension_ftm_for_ic_t *)(configPtr->extension))->ftmClockSource;
    info.BDMMode = FTM_BDM_MODE_11;
    info.isTofIsrEnabled = false;
    info.enableInitializationTrigger = false;

    /* Initializes the FTM driver */
    status = FTM_DRV_Init(instance,
                          &info,
                          ftmState);
    DEV_ASSERT(STATUS_SUCCESS == status);

    param.nNumChannels = configPtr->nNumChannels;
    param.nMaxCountValue = MAX_COUNTER_VALUE;

    /* Configure a list of channels which are used */
    for (index = 0U; index < configPtr->nNumChannels; index++)
    {
        /* Get the hardware channel ID */
        channelConfig[index].hwChannelId = configPtr->inputChConfig[index].hwChannelId;
        channel = channelConfig[index].hwChannelId;
        /* Check the input capture operation mode */
        if ((configPtr->inputChConfig[index].inputCaptureMode == IC_TIMESTAMP_RISING_EDGE) || \
            (configPtr->inputChConfig[index].inputCaptureMode == IC_TIMESTAMP_FALLING_EDGE) || \
            (configPtr->inputChConfig[index].inputCaptureMode == IC_TIMESTAMP_BOTH_EDGES))
        {
            channelConfig[index].inputMode = FTM_EDGE_DETECT;
            channelConfig[index].measurementType = FTM_NO_MEASUREMENT;
            /* Check the type of signal detection */
            if (configPtr->inputChConfig[index].inputCaptureMode == IC_TIMESTAMP_RISING_EDGE)
            {
                channelConfig[index].edgeAlignement = FTM_RISING_EDGE;
            }
            else if (configPtr->inputChConfig[index].inputCaptureMode == IC_TIMESTAMP_FALLING_EDGE)
            {
                channelConfig[index].edgeAlignement = FTM_FALLING_EDGE;
            }
            else
            {
                channelConfig[index].edgeAlignement = FTM_BOTH_EDGES;
            }
        }
        else
        {
            /* Check the channel ID need to even number in the measurement mode */
            DEV_ASSERT((channel % 2U) == 0U);
            channelConfig[index].inputMode = FTM_SIGNAL_MEASUREMENT;
            channelConfig[index].edgeAlignement = FTM_NO_PIN_CONTROL;
            /* Check the type of measurement */
            if (configPtr->inputChConfig[index].inputCaptureMode == IC_MEASURE_RISING_EDGE_PERIOD)
            {
                channelConfig[index].measurementType = FTM_RISING_EDGE_PERIOD_MEASUREMENT;
            }
            else if (configPtr->inputChConfig[index].inputCaptureMode == IC_MEASURE_FALLING_EDGE_PERIOD)
            {
                channelConfig[index].measurementType = FTM_FALLING_EDGE_PERIOD_MEASUREMENT;
            }
            else if (configPtr->inputChConfig[index].inputCaptureMode == IC_MEASURE_PULSE_HIGH)
            {
                channelConfig[index].measurementType = FTM_PERIOD_ON_MEASUREMENT;
            }
            else
            {
                channelConfig[index].measurementType = FTM_PERIOD_OFF_MEASUREMENT;
            }
        }

        /* Set channels configuration from user */
        channelConfig[index].filterEn = configPtr->inputChConfig[index].filterEn;
        channelConfig[index].filterValue = configPtr->inputChConfig[index].filterValue;
        channelConfig[index].continuousModeEn = ((channel_extension_ftm_for_ic_t *)(configPtr->inputChConfig[index].channelExtension))->continuousModeEn;
        channelConfig[index].channelsCallbacksParams = NULL;
        channelConfig[index].channelsCallbacks = NULL;

        /* Store some needed information into state structure */
        icState->channelConfigArray[index] = configPtr->inputChConfig[index].hwChannelId;
        icState->icChannelType[channel] = configPtr->inputChConfig[index].inputCaptureMode;
    }

    param.inputChConfig = channelConfig;
    /* Configure channels in input capture mode */
    status = FTM_DRV_InitInputCapture(instance,
                                      &param);

#endif

#if (defined(IC_PAL_OVER_EMIOS))
    emios_common_param_t commonParam;
    uint8_t channel;
    emios_bus_select_t timeBase;
    emios_mc_mode_param_t mcParam;
    emios_input_capture_param_t icParam;

    /* Get common parameters from user */
    commonParam.allowDebugMode = false;
    commonParam.lowPowerMode = false;
    commonParam.clkDivVal = ((extension_emios_for_ic_t *)(configPtr->extension))->clkDivVal;
    commonParam.enableGlobalPrescaler = ((extension_emios_for_ic_t *)(configPtr->extension))->enableGlobalPrescaler;
    commonParam.enableGlobalTimeBase = ((extension_emios_for_ic_t *)(configPtr->extension))->enableGlobalTimeBase;

    /* Initialize the global for a eMIOS group */
    EMIOS_DRV_InitGlobal((uint8_t)instance,
                          &commonParam);

    /* Get the information from user configuration */
    mcParam.mode = EMIOS_MODE_MCB_UP_COUNTER_INT_CLK;
    mcParam.period = MAX_COUNTER_VALUE;
    mcParam.filterInput = EMIOS_INPUT_FILTER_BYPASS;
    mcParam.filterEn = false;
    mcParam.triggerMode = EMIOS_TRIGGER_EDGE_ANY;

    /* Configure a list of channels which are used */
    for (index = 0U; index < configPtr->nNumChannels; index++)
    {
        mcParam.internalPrescaler = (emios_clock_internal_ps_t)((channel_extension_emios_for_ic_t *)(configPtr->inputChConfig[index].channelExtension))->prescaler;
        mcParam.internalPrescalerEn = true;

        switch (((channel_extension_emios_for_ic_t *)(configPtr->inputChConfig[index].channelExtension))->timebase)
        {
            case IC_BUS_SEL_A:
                /* Set channel to use as a time base */
                channel = 23U;
                timeBase = EMIOS_BUS_SEL_A;
                break;
            case IC_BUS_SEL_B:
                /* Set channel to use as a time base */
                channel = 0U;
                timeBase = EMIOS_BUS_SEL_BCDE;
                break;
            case IC_BUS_SEL_C:
                /* Set channel to use as a time base */
                channel = 8U;
                timeBase = EMIOS_BUS_SEL_BCDE;
                break;
            case IC_BUS_SEL_D:
                /* Set channel to use as a time base */
                channel = 16U;
                timeBase = EMIOS_BUS_SEL_BCDE;
                break;
            case IC_BUS_SEL_E:
                /* Set channel to use as a time base */
                channel = 24U;
                timeBase = EMIOS_BUS_SEL_BCDE;
                break;
            case IC_BUS_SEL_F:
                /* Set channel to use as a time base */
                channel = 22U;
                timeBase = EMIOS_BUS_SEL_F;
                break;
            case IC_BUS_SEL_INTERNAL:
                /* Set channel to use as a time base */
                channel = configPtr->inputChConfig[index].hwChannelId;
                timeBase = EMIOS_BUS_SEL_INTERNAL;
                break;
            default:
                /* Do nothing */
                break;
        }

        /* Initialize the counter mode */
        status = EMIOS_DRV_MC_InitCounterMode((uint8_t)instance,
                                              channel,
                                              &mcParam);

        channel = configPtr->inputChConfig[index].hwChannelId;
        icParam.mode = EMIOS_MODE_IC;
        icParam.timebase = timeBase;
        icParam.filterInput = (emios_input_filter_t)(configPtr->inputChConfig[index].filterValue);
        icParam.filterEn = configPtr->inputChConfig[index].filterEn;
        icParam.inputCaptureMode = (emios_input_capture_mode_t)(configPtr->inputChConfig[index].inputCaptureMode);

        /* Initialize the input capture mode for each channel */
        status = EMIOS_DRV_IC_InitInputCaptureMode((uint8_t)instance,
                                                   channel,
                                                   &icParam);

        /* Store some needed information into state structure */
        icState->channelConfigArray[index] = channel;
        icState->icChannelType[channel] = configPtr->inputChConfig[index].inputCaptureMode;
        icState->timeBaseSelection[channel] = icParam.timebase;
        icState->filterEn[channel] = icParam.filterEn;
        icState->filterInput[channel] = icParam.filterInput;
    }

    /* Enable the global eMIOS to start counter */
    EMIOS_DRV_EnableGlobalEmios((uint8_t)instance);
#endif

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : IC_Deinit
 * Description   : This function will disable the input capture mode. The driver
 * can't be used again until reinitialized. The context structure is no longer
 * needed by the driver and can be freed after calling this function.
 *
 * Implements    : IC_Deinit_Activity
 *END**************************************************************************/
status_t IC_Deinit(uint32_t instance)
{
    DEV_ASSERT(instance < IC_PAL_INSTANCE_MAX);
    status_t status = STATUS_SUCCESS;
    uint8_t index = 0U;
    ic_pal_state_t * icState;

    /* Allocate one of the IC state structure for this instance */
    index = FindIcState(instance);
    icState = &g_icPalStatePtr[index];

#if (defined(IC_PAL_OVER_FTM))
    /* Disable the input capture over FTM */
    status = FTM_DRV_Deinit(instance);
#endif

#if (defined(IC_PAL_OVER_EMIOS))
    uint8_t channel;

    for (index = 0U; index < icState->nNumChannels; index++)
    {
        channel = icState->channelConfigArray[index];
        /* Disable channels in the input capture over EMIOS */
        EMIOS_DRV_DeInitChannel((uint8_t)instance,
                                channel);
    }

    /* Disable the global of EMIOS */
    EMIOS_DRV_DisableGlobalEmios((uint8_t)instance);
#endif

    /* De-Initialize the internal context to default value */
    icState->nNumChannels = 0U;
    for (index = 0U; index < IC_PAL_NUM_OF_CHANNEL_MAX; index++)
    {
        icState->channelConfigArray[index] = 0U;
        icState->icChannelType[index] = IC_DISABLE_OPERATION;
        icState->enableContinuousMode[index] = false;
    }

    if (status == STATUS_SUCCESS)
    {
        /* De-Allocate the instance which is not used */
        icFreeState(icStateIsAllocated, icStateInstanceMapping, instance);
    }

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : IC_StartChannel
 * Description   : This function start channel counting.
 *
 * Implements    : IC_StartChannel_Activity
 *END**************************************************************************/
void IC_StartChannel(uint32_t instance,
                     uint8_t channel)
{
    DEV_ASSERT(instance < IC_PAL_INSTANCE_MAX);

#if (defined(IC_PAL_OVER_FTM))
    uint8_t index = 0U;
    ic_pal_state_t * icState;

    /* Allocate one of the IC state structure for this instance */
    index = FindIcState(instance);
    icState = &g_icPalStatePtr[index];

    /* Re-start the channel mode */
    IC_SetChannelMode(instance,
                      channel,
                      icState->icChannelType[channel]);
#endif

#if (defined(IC_PAL_OVER_EMIOS))
    /* Enable the channel clock */
    EMIOS_DRV_ChannelEnableClk((uint8_t)instance,
                               channel);
#endif
}

/*FUNCTION**********************************************************************
 *
 * Function Name : IC_StopChannel
 * Description   : This function will stop channel counting.
 *
 * Implements    : IC_StopChannel_Activity
 *END**************************************************************************/
void IC_StopChannel(uint32_t instance,
                    uint8_t channel)
{
    DEV_ASSERT(instance < IC_PAL_INSTANCE_MAX);

#if (defined(IC_PAL_OVER_FTM))
    /* Disable pin not used for FTM */
    FTM_DRV_SetOutputlevel(instance,
                           channel,
                           0x0U);
#endif

#if (defined(IC_PAL_OVER_EMIOS))
    /* Disable individual channel by stopping its respective clock*/
    EMIOS_DRV_ChannelDisableClk((uint8_t)instance,
                                channel);
#endif
}

/*FUNCTION**********************************************************************
 *
 * Function Name : IC_SetChannelMode
 * Description   : This function is used to change the channel mode at run time or
 * when stopping channel. The channel mode is selected in the ic_option_mode_t
 * enumeration type.
 *
 * Implements    : IC_SetChannelMode_Activity
 *END**************************************************************************/
status_t IC_SetChannelMode(uint32_t instance,
                           uint8_t channel,
                           ic_option_mode_t channelMode)
{
    DEV_ASSERT(instance < IC_PAL_INSTANCE_MAX);
    DEV_ASSERT(channel < IC_PAL_NUM_OF_CHANNEL_MAX);
    uint8_t index = 0U;
    ic_pal_state_t * icState;
    status_t status = STATUS_SUCCESS;

    /* Allocate one of the IC state structure for this instance */
    index = FindIcState(instance);
    icState = &g_icPalStatePtr[index];

#if (defined(IC_PAL_OVER_FTM))
    bool contModeEnable;

    if (true == icState->enableContinuousMode[channel])
    {
        contModeEnable = true;
    }
    else
    {
        contModeEnable = false;
    }

    /* Set operation mode for channel input */
    status = FTM_IC_DRV_SetChannelMode(instance,
                                       channel,
                                       (ftm_ic_op_mode_t)channelMode,
                                        contModeEnable);
#endif

#if (defined(IC_PAL_OVER_EMIOS))
    emios_input_capture_param_t icParam;
    emios_gpio_mode_param_t gpioParam;

    if (channelMode == IC_DISABLE_OPERATION)
    {
        /* Set default configure for an input pin */
        gpioParam.mode = EMIOS_MODE_GPIO_INPUT;
        gpioParam.filterEn = false;
        gpioParam.filterInput = EMIOS_INPUT_FILTER_BYPASS;
        gpioParam.triggerMode = EMIOS_TRIGGER_EDGE_ANY;

        /* Disable operation on the channel input */
        EMIOS_DRV_InitGpioMode((uint8_t)instance,
                               channel,
                               &gpioParam);
    }
    else
    {
        icParam.mode = EMIOS_MODE_IC;
        icParam.timebase = icState->timeBaseSelection[channel];
        icParam.filterInput = icState->filterInput[channel];
        icParam.filterEn = icState->filterEn[channel];
        icParam.inputCaptureMode = (emios_input_capture_mode_t)((uint8_t)(channelMode) - 1U);

        /* Initialize the input capture mode for each channel */
        status = EMIOS_DRV_IC_InitInputCaptureMode((uint8_t)instance,
                                                   channel,
                                                   &icParam);
    }
#endif

    /* Update the channel mode */
    icState->icChannelType[channel] = channelMode;

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : IC_GetMeasurement
 * Description   : This function will get the value of measured signal in ticks.
 *
 * Implements    : IC_GetMeasurement_Activity
 *END**************************************************************************/
uint16_t IC_GetMeasurement(uint32_t instance,
                           uint8_t channel)
{
    DEV_ASSERT(instance < IC_PAL_INSTANCE_MAX);
    DEV_ASSERT(channel < IC_PAL_NUM_OF_CHANNEL_MAX);
    uint16_t value = 0U;

#if (defined(IC_PAL_OVER_FTM))
    /* Get the measured value over the FTM */
    value = FTM_DRV_GetInputCaptureMeasurement(instance,
                                               channel);
#endif

#if (defined(IC_PAL_OVER_EMIOS))
    status_t status = STATUS_SUCCESS;
    uint32_t retValue = 0U;

    /* Get the measured value over the EMIOS */
    status = EMIOS_DRV_IC_GetLastMeasurement((uint8_t)instance,
                                             channel,
                                             &retValue);
    DEV_ASSERT(STATUS_SUCCESS == status);
    value = (uint16_t)retValue;
#endif

    return value;
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
