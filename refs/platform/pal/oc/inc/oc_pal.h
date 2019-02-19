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

#ifndef OC_PAL_H
#define OC_PAL_H

#include "oc_pal_mapping.h"
#include "status.h"


/*! @file oc_pal.h */

/*!
 * @defgroup oc_pal OC PAL
 * @ingroup oc_pal
 * @addtogroup oc_pal
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#if (defined(OC_PAL_OVER_EMIOS))
/*!
 * @brief Select either one of the counter buses or the internal counter to be used by
 * the Unified Channel.
 * Implements : oc_bus_select_t_Class
 */
typedef enum
{
    OC_BUS_SEL_A        = 0x00U,    /*!< Global counter bus A */
    OC_BUS_SEL_B        = 0x01U,    /*!< Local counter bus B */
    OC_BUS_SEL_C        = 0x02U,    /*!< Local counter bus C */
    OC_BUS_SEL_D        = 0x03U,    /*!< Local counter bus D */
    OC_BUS_SEL_E        = 0x04U,    /*!< Local counter bus E */
    OC_BUS_SEL_F        = 0x05U,    /*!< Global counter bus F */
    OC_BUS_SEL_INTERNAL = 0x06U     /*!< Internal counter bus */
} oc_bus_select_t;

#endif


/*!
 * @brief The type of comparison for output compare mode
 * Implements : oc_option_mode_t_Class
 */
typedef enum
{
    OC_DISABLE_OUTPUT  = 0x00U,    /*!< No action on output pin */
    OC_TOGGLE_ON_MATCH = 0x01U,    /*!< Toggle on match */
    OC_CLEAR_ON_MATCH  = 0x02U,    /*!< Clear on match */
    OC_SET_ON_MATCH    = 0x03U     /*!< Set on match */
} oc_option_mode_t;

/*!
 * @brief The type of update on the channel match
 * Implements : oc_option_mode_t_Class
 */
typedef enum
{
    OC_RELATIVE_VALUE = 0x00U,    /*!< Next compared value is relative to current value */
    OC_ABSOLUTE_VALUE = 0x01U     /*!< Next compared value is absolute */
} oc_option_update_t;

/*!
 * @brief The configuration structure of output compare parameters for each channel
 *
 * Implements : oc_output_ch_param_t_Class
 */
typedef struct
{
    uint8_t hwChannelId;        /*!< Physical hardware channel ID */
    oc_option_mode_t chMode;    /*!< Channel output mode*/
    uint16_t comparedValue;     /*!< The compared value */
    void * channelExtension;    /*!< The IP specific configuration structure for channel */
} oc_output_ch_param_t;

/*!
 * @brief Defines the configuration structures are used in the output compare mode
 *
 * Implements : oc_config_t_Class
 */
typedef struct
{
    uint8_t nNumChannels;                           /*!< Number of output compare channel used */
    const oc_output_ch_param_t * outputChConfig;    /*!< Output compare channels configuration */
    void * extension;                               /*!< IP specific configuration structure */
} oc_config_t;

#if (defined(OC_PAL_OVER_FTM))
/*!
 * @brief Defines the extension structure for the output compare mode over FTM
 *
 * Part of FTM configuration structure
 * Implements : extension_ftm_for_oc_t_Class
 */
typedef struct
{
    uint16_t maxCountValue;               /*!< Maximum count value in ticks */
    ftm_clock_source_t ftmClockSource;    /*!< Select clock source for FTM */
    ftm_clock_ps_t ftmPrescaler;          /*!< Register pre-scaler options available in the
                                           *   ftm_clock_ps_t enumeration */
} extension_ftm_for_oc_t;
#endif

#if (defined(OC_PAL_OVER_EMIOS))
/*!
 * @brief Defines the extension structure for the channel configuration over EMIOS
 *
 * Part of EMIOS channel configuration structure
 * Implements : channel_extension_emios_for_oc_t_Class
 */
typedef struct
{
    oc_bus_select_t timebase;      /*!< Counter bus selected */
    uint32_t period;               /*!< If up mode period = A1, period = 2(A1) with MC up/down mode,
                                    *   period = 2(A1) -2 with MCB up/down mode */
    emios_clock_internal_ps_t prescaler;    /*!< Internal prescaler, pre-scale channel clock by internalPrescaler + 1 */
} channel_extension_emios_for_oc_t;

/*!
 * @brief Defines the extension structure for the output compare mode over EMIOS
 *
 * Part of eMIOS configuration structure
 * Implements : extension_emios_for_oc_t_Class
 */
typedef struct
{
    uint16_t clkDivVal;            /*!< Select the clock divider value for the global prescaler in range (1-256) */
    bool enableGlobalPrescaler;    /*!< Enable global prescaler or disable */
    bool enableGlobalTimeBase;     /*!< Enable global timebase or disable */
} extension_emios_for_oc_t;
#endif

/*!
 * @brief The internal context structure
 *
 * This structure is used by the driver for its internal logic. It must
 * be provided by the application through the OC_Init() function, then
 * it cannot be freed until the driver is de-initialized using OC_Deinit().
 * The application should make no assumptions about the content of this structure.
 */
typedef struct
{
/*! @cond DRIVER_INTERNAL_USE_ONLY */
    uint8_t nNumChannels;                                      /*!< Number of output compare channel used */
    uint8_t channelConfigArray[OC_PAL_NUM_OF_CHANNEL_MAX];     /*!< Store the hardware channel IDs are used output compare mode */
    oc_option_mode_t ocChannelMode[OC_PAL_NUM_OF_CHANNEL_MAX]; /*!< Output compare mode of operation */
/*! @endcond */
} oc_pal_state_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Initializes the output compare mode
 *
 * This function will initialize the OC PAL instance, including the
 * other platform specific HW units used together in the output compare mode. This
 * function configures a group of channels in instance to set, clear toggle the
 * output signal.
 *
 * @param[in] instance The output compare instance number.
 * @param[in] configPtr The pointer to configuration structure.
 * @return Operation status
 *        - STATUS_SUCCESS : Completed successfully.
 *        - STATUS_ERROR : Error occurred.
 */
status_t OC_Init(uint32_t instance,
                 const oc_config_t * const configPtr);

/*!
 * @brief De-initialize the output compare instance
 *
 * This function will disable the output compare mode. The driver
 * can't be used again until reinitialized. The context structure is no longer
 * needed by the driver and can be freed after calling this function.
 *
 * @param[in] instance The output compare instance number.
 * @return Operation status
 *         - STATUS_SUCCESS: Operation was successful
 */
status_t OC_Deinit(uint32_t instance);

/*!
 * @brief Start the counter
 *
 * This function start channel counting.
 *
 * @param[in] instance The output compare instance number.
 * @param[in] channel The channel number.
 */
void OC_StartChannel(uint32_t instance,
                     uint8_t channel);

/*!
 * @brief Stop the counter
 *
 * This function stop channel counting.
 *
 * @param[in] instance The output compare instance number.
 * @param[in] channel The channel number.
 */
void OC_StopChannel(uint32_t instance,
                    uint8_t channel);

/*!
 * @brief Control the channel output by software
 *
 * This function is used to forces the output pin to a specified
 * value. It can be used to control the output pin value when the OC channel
 * is disabled.
 *
 * @param[in] instance The output compare instance number.
 * @param[in] channel The channel number.
 * @param[in] outputValue The output value:
 *            - false : The software output control forces 0 to the channel output.
 *            - true : The software output control forces 1 to the channel output.
 * @return Operation status
 *        - STATUS_SUCCESS : Completed successfully.
 *        - STATUS_ERROR : Error occurred.
 */
status_t OC_SetOutputState(uint32_t instance,
                           uint8_t channel,
                           bool outputValue);

/*!
 * @brief Set the operation mode of channel output
 *
* This function will set the action executed on a compare
 * match value to set output pin, clear output pin, toggle output pin.
 *
 * @param[in] instance The output compare instance number.
 * @param[in] channel The channel number.
 * @param[in] channelMode The channel mode in output compare:
 *            - OC_DISABLE_OUTPUT : No action on output pin
 *            - OC_TOGGLE_ON_MATCH : Toggle on match
 *            - OC_CLEAR_ON_MATCH : Clear on match
 *            - OC_SET_ON_MATCH : Set on match
 * @return Operation status
 *        - STATUS_SUCCESS : Completed successfully.
 *        - STATUS_ERROR : Error occurred.
 */
status_t OC_SetOutputAction(uint32_t instance,
                            uint8_t channel,
                            oc_option_mode_t channelMode);

/*!
 * @brief Update the match value on the channel
 *
 * This function will update the value of an output compare channel to
 * the counter matches to this value.
 *
 * @param[in] instance The output compare instance number.
 * @param[in] channel The channel number.
 * @param[in] nextCompareMatchValue The timer value in ticks until the next compare match event should be appeared.
 * @param[in] typeOfupdate The type of update:
 *        - OC_RELATIVE_VALUE : nextCompareMatchValue will be added to current counter value into the channel value register
 *        - OC_ABSOLUTE_VALUE : nextCompareMatchValue will be written into the channel value register
 * @return Operation status
 *        - STATUS_SUCCESS : Completed successfully.
 *        - STATUS_ERROR : Error occurred.
 */
status_t OC_SetCompareValue(uint32_t instance,
                            uint8_t channel,
                            uint32_t nextCompareMatchValue,
                            oc_option_update_t typeOfupdate);

#if defined(__cplusplus)
}
#endif

/*! @}*/
#endif /* OC_PAL_H */


/*******************************************************************************
 * EOF
 ******************************************************************************/
