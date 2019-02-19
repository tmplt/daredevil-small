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
 * @file oc_pal.c
 *
 * @page misra_violations MISRA-C:2012 violations
 * @section [global]
 * Violates MISRA 2012 Required Rule 1.3, Taking address of near auto variable.
 * The code is not dynamically linked. An absolute stack address is obtained
 * when taking the address of the near auto variable. A source of error in
 * writing dynamic code is that the stack segment may be different from the data
 * segment.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.7, External could be made static.
 * Function is defined for usage by application code.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.13, Pointer parameter 'ocState' could be declared as pointing to const.
 * This is a pointer to the driver context structure which is for internal use only, and the application
 * must make no assumption about the content of this structure. Therefore it is irrelevant for the application
 * whether the structure is changed in the function or not. The fact that in a particular implementation of some
 * functions there is no write in the context structure is an implementation detail and there is no reason to
 * propagate it in the interface. That would compromise the stability of the interface, if this implementation
 * detail is changed in later releases or on other platforms.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.3, Cast performed between a pointer to object type
 * and a pointer to a different object type.
 * This is needed for the extension of the user configuration structure, for which the actual type
 * cannot be known.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 11.5, Conversion from pointer to void to pointer to other type
 * This is needed for the extension of the user configuration structure, for which the actual type
 * cannot be known.
 *
 */

#include "oc_pal.h"
#include "oc_pal_cfg.h"


/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Pointer to runtime state structures */
static oc_pal_state_t g_ocPalStatePtr[NUMBER_OF_OC_PAL_INSTANCES];

/* OC state-instance matching */
static uint32_t ocStateInstanceMapping[NUMBER_OF_OC_PAL_INSTANCES];
/* OC  available resources table */
static bool ocStateIsAllocated[NUMBER_OF_OC_PAL_INSTANCES];

#if (defined(OC_PAL_OVER_FTM))
/* The FTM state structures */
static ftm_state_t g_ocFtmState[NUMBER_OF_OC_PAL_INSTANCES];
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

/*FUNCTION**********************************************************************
 *
 * Function Name : ocAllocateState
 * Description   : Allocates one of the available state structure.
 *
 *END**************************************************************************/
static uint8_t ocAllocateState(bool * isAllocated,
                               uint32_t * instanceMapping,
                               uint32_t instance,
                               uint8_t numberOfInstances)
{
    uint8_t i;

    /* Allocate one of the OC state structure for this instance */
    for (i = 0;i < numberOfInstances;i++)
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
 * Function Name : ocFreeState
 * Description   : Deallocates one of the available state structure.
 *
 *END**************************************************************************/
static void ocFreeState(bool * isAllocated,
                        uint32_t * instanceMapping,
                        uint32_t instance,
                        uint8_t numberOfInstances)
{
    uint8_t i;

    /* Allocate one of the OC state structure for this instance */
    for (i = 0;i < numberOfInstances;i++)
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
 * Function Name : FindOcState
 * Description   : Search the state structure of the instance for each IP
 *
 *END**************************************************************************/
static uint8_t FindOcState(uint32_t instance)
{
    uint8_t i;

    for (i = 0U;i < NUMBER_OF_OC_PAL_INSTANCES;i++)
    {
        if (ocStateInstanceMapping[i] == instance)
        {
            break;
        }
    }

    return i;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : OC_Init
 * Description   : This function will initialize the OC PAL instance, including the
 * other platform specific HW units used together in the output compare mode. This
 * function configures a group of channels in instance to generate timed pulses with
 * programmable position, polarity, duration, and frequency. The channel (n) output
 * can be set, cleared, or toggled.
 *
 * Implements    : OC_Init_Activity
 *END**************************************************************************/
status_t OC_Init(uint32_t instance,
                 const oc_config_t * configPtr)
{
    DEV_ASSERT(configPtr != NULL);
    DEV_ASSERT(instance < OC_PAL_INSTANCES_MAX);
    DEV_ASSERT(configPtr->nNumChannels > 0U);
    status_t status = STATUS_ERROR;
    static oc_pal_state_t * ocState;
    uint8_t index = 0U;
    uint8_t indexInstance = 0U;
    uint8_t channel = 0U;

    /* Allocate one of the OC state structure for this instance */
    indexInstance = ocAllocateState(ocStateIsAllocated, ocStateInstanceMapping, instance, NUMBER_OF_OC_PAL_INSTANCES);
    ocState = &g_ocPalStatePtr[indexInstance];
    DEV_ASSERT(ocState->nNumChannels == 0x0U);

    /* Initialize the internal context to default value */
    ocState->nNumChannels = configPtr->nNumChannels;
    for (index = 0U; index < OC_PAL_NUM_OF_CHANNEL_MAX; index++)
    {
        ocState->channelConfigArray[index] = 0U;
        ocState->ocChannelMode[index] = OC_DISABLE_OUTPUT;
    }

#if (defined(OC_PAL_OVER_FTM))
    ftm_pwm_sync_t sync;
    ftm_user_config_t info;
    ftm_state_t * ftmState = &g_ocFtmState[indexInstance];
    ftm_output_cmp_param_t param;
    ftm_output_cmp_ch_param_t channelConfig[FEATURE_FTM_CHANNEL_COUNT];

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
    info.ftmMode = FTM_MODE_OUTPUT_COMPARE;
    info.ftmPrescaler = ((extension_ftm_for_oc_t *)(configPtr->extension))->ftmPrescaler;
    info.ftmClockSource = ((extension_ftm_for_oc_t *)(configPtr->extension))->ftmClockSource;
    info.BDMMode = FTM_BDM_MODE_11;
    info.isTofIsrEnabled = false;
    info.enableInitializationTrigger = false;

    /* Initializes the FTM driver */
    status = FTM_DRV_Init((uint32_t)instance,
                          &info,
                          ftmState);
    DEV_ASSERT(STATUS_SUCCESS == status);

    param.nNumOutputChannels = configPtr->nNumChannels;
    param.mode = FTM_MODE_OUTPUT_COMPARE;
    param.maxCountValue = ((extension_ftm_for_oc_t *)(configPtr->extension))->maxCountValue;

    /* Configure a list of channels which are used */
    for (index = 0U; index < configPtr->nNumChannels; index++)
    {
        /* Get the hardware channel ID */
        channelConfig[index].hwChannelId = configPtr->outputChConfig[index].hwChannelId;
        channel = channelConfig[index].hwChannelId;
        /* Set channels configuration from user */
        channelConfig[index].chMode = (ftm_output_compare_mode_t)(configPtr->outputChConfig[index].chMode);
        channelConfig[index].comparedValue = configPtr->outputChConfig[index].comparedValue;
        channelConfig[index].enableExternalTrigger = false;

        /* Store some needed information into state structure */
        ocState->channelConfigArray[index] = configPtr->outputChConfig[index].hwChannelId;
        ocState->ocChannelMode[channel] = configPtr->outputChConfig[index].chMode;

        /* Disable pin not used for FTM */
        FTM_DRV_SetOutputlevel(instance,
                               channel,
                               0x0U);
    }

    param.outputChannelConfig = channelConfig;

    /* Configure channels in output compare mode */
    status = FTM_DRV_InitOutputCompare((uint32_t)instance,
                                      &param);

#endif

#if (defined(OC_PAL_OVER_EMIOS))
    emios_common_param_t commonParam;
    emios_mc_mode_param_t mcParam;
    emios_oc_param_t ocParam;
    emios_bus_select_t timeBaseSelection = EMIOS_BUS_SEL_BCDE;

    /* Get common parameters from user */
    commonParam.allowDebugMode = false;
    commonParam.lowPowerMode = false;
    commonParam.clkDivVal = ((extension_emios_for_oc_t *)(configPtr->extension))->clkDivVal;
    commonParam.enableGlobalPrescaler = ((extension_emios_for_oc_t *)(configPtr->extension))->enableGlobalPrescaler;
    commonParam.enableGlobalTimeBase = ((extension_emios_for_oc_t *)(configPtr->extension))->enableGlobalTimeBase;

    /* Initialize the global for a eMIOS group */
    EMIOS_DRV_InitGlobal((uint8_t)instance,
                          &commonParam);

    /* Get the information from user configuration */
    mcParam.mode = EMIOS_MODE_MCB_UP_COUNTER_INT_CLK;
    mcParam.filterInput = EMIOS_INPUT_FILTER_BYPASS;
    mcParam.filterEn = false;
    mcParam.triggerMode = EMIOS_TRIGGER_EDGE_ANY;

    for (index = 0U; index < configPtr->nNumChannels; index++)
    {
        mcParam.period = ((channel_extension_emios_for_oc_t *)(configPtr->outputChConfig[index].channelExtension))->period;
        mcParam.internalPrescaler = ((channel_extension_emios_for_oc_t *)(configPtr->outputChConfig[index].channelExtension))->prescaler;
        mcParam.internalPrescalerEn = true;

        switch (((channel_extension_emios_for_oc_t *)(configPtr->outputChConfig[index].channelExtension))->timebase)
        {
            case OC_BUS_SEL_A:
                /* Set channel to use as a time base */
                channel = 23U;
                timeBaseSelection = EMIOS_BUS_SEL_A;
                break;
            case OC_BUS_SEL_B:
                /* Set channel to use as a time base */
                channel = 0U;
                timeBaseSelection = EMIOS_BUS_SEL_BCDE;
                break;
            case OC_BUS_SEL_C:
                /* Set channel to use as a time base */
                channel = 8U;
                timeBaseSelection = EMIOS_BUS_SEL_BCDE;
                break;
            case OC_BUS_SEL_D:
                /* Set channel to use as a time base */
                channel = 16U;
                timeBaseSelection = EMIOS_BUS_SEL_BCDE;
                break;
            case OC_BUS_SEL_E:
                /* Set channel to use as a time base */
                channel = 24U;
                timeBaseSelection = EMIOS_BUS_SEL_BCDE;
                break;
            case OC_BUS_SEL_F:
                /* Set channel to use as a time base */
                channel = 22U;
                timeBaseSelection = EMIOS_BUS_SEL_F;
                break;
            case OC_BUS_SEL_INTERNAL:
                /* Set channel to use as a time base */
                channel = configPtr->outputChConfig[index].hwChannelId;
                timeBaseSelection = EMIOS_BUS_SEL_INTERNAL;
                break;
            default:
                /* Do nothing */
                break;
        }

        /* Initialize the counter mode */
        status = EMIOS_DRV_MC_InitCounterMode((uint8_t)instance,
                                              channel,
                                              &mcParam);
        DEV_ASSERT(status == STATUS_SUCCESS);

        channel = configPtr->outputChConfig[index].hwChannelId;

        ocParam.mode = EMIOS_MODE_SAOC;
        ocParam.timebase = timeBaseSelection;
        ocParam.matchLeadingEdgeValue = (uint32_t)(configPtr->outputChConfig[index].comparedValue);
        ocParam.matchTrailingEdgeValue = 0U;

        /* Check the operation mode of channel output */
        if (OC_CLEAR_ON_MATCH == configPtr->outputChConfig[index].chMode)
        {
            ocParam.outputActiveMode = EMIOS_OUTPUT_ACTIVE_LOW;
        }
        else if (OC_SET_ON_MATCH == configPtr->outputChConfig[index].chMode)
        {
            ocParam.outputActiveMode = EMIOS_OUTPUT_ACTIVE_HIGH;
        }
        else if (OC_TOGGLE_ON_MATCH == configPtr->outputChConfig[index].chMode)
        {
            ocParam.outputActiveMode = EMIOS_OUTPUT_ACTIVE_TOGGLE;
        }
        else
        {
            DEV_ASSERT(false);
        }

        /* Initialize the output compare mode for each channel */
        status = EMIOS_DRV_OC_InitOutputCompareMode((uint8_t)instance,
                                                    channel,
                                                    &ocParam);

        /* Store some needed information into state structure */
        ocState->channelConfigArray[index] = channel;
        ocState->ocChannelMode[channel] = configPtr->outputChConfig[index].chMode;
    }

    /* Enable the global eMIOS to start counter */
    EMIOS_DRV_EnableGlobalEmios((uint8_t)instance);
#endif

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : OC_Deinit
 * Description   : This function will disable the output compare mode. The driver
 * can't be used again until reinitialized. The context structure is no longer
 * needed by the driver and can be freed after calling this function.
 *
 * Implements    : OC_Deinit_Activity
 *END**************************************************************************/
status_t OC_Deinit(uint32_t instance)
{
    DEV_ASSERT(instance < OC_PAL_INSTANCES_MAX);
    status_t status = STATUS_ERROR;
    uint8_t index;
    oc_pal_state_t * ocState;

    /* Allocate one of the OC state structure for this instance */
    index = FindOcState(instance);
    ocState = &g_ocPalStatePtr[index];

#if (defined(OC_PAL_OVER_FTM))
    /* Disable the output compare over FTM */
    status = FTM_DRV_Deinit(instance);
#endif

#if (defined(OC_PAL_OVER_EMIOS))
    uint8_t channel;

    for (index = 0U; index < ocState->nNumChannels; index++)
    {
        channel = ocState->channelConfigArray[index];
        /* Disable channels in the output compare over EMIOS */
        EMIOS_DRV_DeInitChannel((uint8_t)instance,
                                channel);
    }
    status = STATUS_SUCCESS;

#endif

    /* De-Initialize the internal context to default value */
    ocState->nNumChannels = 0U;
    for (index = 0U; index < OC_PAL_NUM_OF_CHANNEL_MAX; index++)
    {
        ocState->channelConfigArray[index] = 0U;
        ocState->ocChannelMode[index] = OC_DISABLE_OUTPUT;
    }

    if (status == STATUS_SUCCESS)
    {
        /* De-Allocate the instance which is not used */
        ocFreeState(ocStateIsAllocated, ocStateInstanceMapping, instance, NUMBER_OF_OC_PAL_INSTANCES);
    }

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : OC_StartChannel
 * Description   : This function start channel counting.
 *
 * Implements    : OC_StartChannel_Activity
 *END**************************************************************************/
void OC_StartChannel(uint32_t instance,
                     uint8_t channel)
{
    DEV_ASSERT(instance < OC_PAL_INSTANCES_MAX);

#if (defined(OC_PAL_OVER_FTM))
    uint8_t index;
    oc_pal_state_t * ocState;

    /* Allocate one of the OC state structure for this instance */
    index = FindOcState(instance);
    ocState = &g_ocPalStatePtr[index];
    oc_option_mode_t channelMode = ocState->ocChannelMode[channel];

    /* Set the channel output mode */
    FTM_DRV_SetOutputlevel(instance,
                           channel,
                           (uint8_t)channelMode);
#endif

#if (defined(OC_PAL_OVER_EMIOS))
    /* Enable the channel clock */
    EMIOS_DRV_ChannelEnableClk((uint8_t)instance,
                               channel);
#endif
}

/*FUNCTION**********************************************************************
 *
 * Function Name : OC_StopChannel
 * Description   : This function will stop channel counting.
 *
 * Implements    : OC_StopChannel_Activity
 *END**************************************************************************/
void OC_StopChannel(uint32_t instance,
                    uint8_t channel)
{
    DEV_ASSERT(instance < NUMBER_OF_OC_PAL_INSTANCES);

#if (defined(OC_PAL_OVER_FTM))
    /* Disable pin not used for FTM */
    FTM_DRV_SetOutputlevel(instance,
                           channel,
                           0x0U);
#endif

#if (defined(OC_PAL_OVER_EMIOS))
    /* Disable individual channel by stopping its respective clock*/
    EMIOS_DRV_ChannelDisableClk((uint8_t)instance,
                                channel);
#endif
}

/*FUNCTION**********************************************************************
 *
 * Function Name : OC_SetOutputState
 * Description   : This function is used to forces the output pin to a specified
 * value. It can be used to control the output pin value when the OC channel
 * is disabled.
 *
 * Implements    : OC_SetOutputState_Activity
 *END**************************************************************************/
status_t OC_SetOutputState(uint32_t instance,
                           uint8_t channel,
                           bool outputValue)
{
    DEV_ASSERT(instance < OC_PAL_INSTANCES_MAX);
    DEV_ASSERT(channel < OC_PAL_NUM_OF_CHANNEL_MAX);

#if (defined(OC_PAL_OVER_FTM))
    uint8_t channelMask = (uint8_t)(1U << channel);
    /* Enable the software output control */
    FTM_DRV_SetSoftwareOutputChannelControl(instance, channelMask, false);
    /* Set the value of channel output */
    FTM_DRV_SetSoftOutChnValue(instance, outputValue ? channelMask : 0x00U, true);
#endif

#if (defined(OC_PAL_OVER_EMIOS))
    /* Set the state on the output signal */
    EMIOS_DRV_SetOutputLevel((uint8_t)instance, channel, false, outputValue);
    /* Force the channel output which set by software */
    EMIOS_DRV_OC_ForceSingleActOutputCmpMatch((uint8_t)instance, channel);
#endif

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : OC_SetOutputAction
 * Description   : This function is used to sets the action executed on a compare
 * match value to set output pin, clear output pin, toggle output pin.
 *
 * Implements    : OC_SetOutputAction_Activity
 *END**************************************************************************/
status_t OC_SetOutputAction(uint32_t instance,
                            uint8_t channel,
                            oc_option_mode_t channelMode)
{
    DEV_ASSERT(instance < OC_PAL_INSTANCES_MAX);
    DEV_ASSERT(channel < OC_PAL_NUM_OF_CHANNEL_MAX);
    uint8_t index;
    oc_pal_state_t * ocState;

    /* Allocate one of the OC state structure for this instance */
    index = FindOcState(instance);
    ocState = &g_ocPalStatePtr[index];

#if (defined(OC_PAL_OVER_FTM))
    /* Set the channel output mode */
    FTM_DRV_SetOutputlevel(instance,
                           channel,
                           (uint8_t)channelMode);
#endif

#if (defined(OC_PAL_OVER_EMIOS))
    /* Set the channel output mode */
    EMIOS_DRV_SetOutputLevel((uint8_t)instance,
                             channel,
                             ((((uint8_t)channelMode & 0x02U) == 0U) ? true : false),
                             ((((uint8_t)channelMode & 0x01U) == 0U) ? false : true));
#endif

    /* Update the channel mode */
    ocState->ocChannelMode[channel] = channelMode;

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : OC_SetCompareValue
 * Description   : This function will update the compare value to change the
 * output signal in the output compare mode.
 *
 * Implements    : OC_SetCompareValue_Activity
 *END**************************************************************************/
status_t OC_SetCompareValue(uint32_t instance,
                            uint8_t channel,
                            uint32_t nextCompareMatchValue,
                            oc_option_update_t typeOfupdate)
{
    DEV_ASSERT(instance < OC_PAL_INSTANCES_MAX);
    DEV_ASSERT(channel < OC_PAL_NUM_OF_CHANNEL_MAX);
    status_t status = STATUS_ERROR;

#if (defined(OC_PAL_OVER_FTM))
    /* Update the output compare value over FTM */
    status = FTM_DRV_UpdateOutputCompareChannel(instance,
                                                channel,
                                                (uint16_t)nextCompareMatchValue,
                                                (ftm_output_compare_update_t)typeOfupdate,
                                                true);
#endif

#if (defined(OC_PAL_OVER_EMIOS))
    uint32_t compareValue = 0U;
    uint32_t maxCounterValue = 0U;
    uint32_t counterValue = EMIOS_DRV_MC_CounterRead((uint8_t)instance, channel);

    if (typeOfupdate == OC_RELATIVE_VALUE)
    {
        maxCounterValue = EMIOS_DRV_MC_GetCounterPeriod((uint8_t)instance, channel);
        /* Configure channel compare register */
        if (nextCompareMatchValue > (maxCounterValue - counterValue))
        {
            compareValue = (nextCompareMatchValue - (maxCounterValue - counterValue));
        }
        else
        {
            compareValue = (counterValue + nextCompareMatchValue);
        }
    }
    else
    {
        compareValue = nextCompareMatchValue;
    }

    /* Update the output compare value over EMIOS */
    EMIOS_DRV_OC_SetSingleActOutputCmpMatch((uint8_t)instance,
                                            channel,
                                            compareValue);
#endif

    return status;
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
