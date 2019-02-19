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

#ifndef PWM_PAL_H
#define PWM_PAL_H

#include "pwm_pal_mapping.h"
#include "pwm_pal_cfg.h"
#include "status.h"
#include "callbacks.h"

/* Include PD files */
#if (defined (PWM_OVER_FTM))
    #include "ftm_pwm_driver.h"
    #include "ftm_common.h"
#endif

/*! @file pwm_pal.h */

/*!
 * @ingroup pwm_pal
 * @addtogroup pwm_pal
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief Defines the channel types
 * Implements : pwm_channel_type_t_Class
 */
typedef enum
{
    PWM_EDGE_ALIGNED   = 0, /*!< Counter used by this type of channel is in up counting mode and the edge is aligned to PWM period */
    PWM_CENTER_ALIGNED = 1, /*!< Counter used by this type of channel is in up-down counting mode and the duty is inserted in center of PWM period */
} pwm_channel_type_t;

/*!
 * @brief Defines the polarity of pwm channels
 * Implements : pwm_polarity_t_Class
 */
typedef enum
{
    PWM_ACTIVE_HIGH = 0, /*!< Polarity is active high */
    PWM_ACTIVE_LOW  = 1  /*!< Polarity is active low */
} pwm_polarity_t;

/*!
 * @brief Defines the polarity of complementary pwm channels relative to main channel
 * Implements : pwm_complementarty_mode_t_Class
 */
typedef enum
{
    PWM_DUPLICATED = 0,   /*!< Complementary channel is the same as main channel */
    PWM_INVERTED   = 1    /*!< Complementary channel is inverted relative to main channel */
} pwm_complementarty_mode_t;

/*!
 * @brief This structure is specific for platforms where FTM is available.
 * Implements : pwm_ftm_timebase_t_class
 */
typedef struct
{
    ftm_clock_source_t sourceClock; /*!< Clock source for FTM timebase */
    ftm_clock_ps_t     prescaler; /*!< Prescaler for FTM timebase */
    ftm_deadtime_ps_t  deadtimePrescaler; /*!< Prescaler for FTM dead-time insertion */
} pwm_ftm_timebase_t;

/*!
 * @brief This structure includes the configuration for each channel
 * Implements : pwm_channel_t_Class
 */
typedef struct
{
    uint8_t                     channel;                        /*!< Channel number */
    pwm_channel_type_t          channelType;                    /*!< Channel waveform type */
    uint32_t                    period;                         /*!< Period of the PWM signal in ticks */
    uint32_t                    duty;                           /*!< Duty cycle in ticks */
    pwm_polarity_t              polarity;                       /*!< Channel polarity */
    bool                        insertDeadtime;                 /*!< Enable/disable dead-time insertion. This  feature is available only if complementary mode is enabled */
    uint8_t                     deadtime;                       /*!< Dead-time value in ticks*/
    bool                        enableComplementaryChannel;     /*!< Enable a complementary channel. This option can take control over other channel than the channel configured in this structure. */
    pwm_complementarty_mode_t   complementaryChannelPolarity;   /*!< Configure the polarity of the complementary channel relative to the main channel */
    void*                       timebase;                       /*!< This field is platform specific and it's used to configure the clocking tree for different time-bases.
                                                                     If FTM is use this field must be filled by a pointer to pwm_ftm_timebase_t */
} pwm_channel_t;

/*!
 * @brief This structure is the configuration for initialization of PWM channels.
 * Implements : pwm_global_config_t_Class
 */
typedef struct
{
    pwm_channel_t*  pwmChannels;          /*!< Pointer to channels configurations */
    uint8_t         numberOfPwmChannels;  /*!< Number of channels which are configured */
} pwm_global_config_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Initialize PWM channels based on config parameter.
 *
 * @param[in] instance The name of the instance
 * @param[in] config The configuration structure used to initialize PWM modules
 * @return    Error or success status returned by API
 */
 status_t PWM_Init(pwm_instance_t instance, pwm_global_config_t* config);

 /*!
 * @brief Update duty cycle. The measurement unit for duty is clock ticks.
 *
 * @param[in] instance The name of the instance
 * @param[in] channel The channel which is update
 * @param[in] duty The duty cycle measured in ticks
 * @return    Error or success status returned by API
 */
 status_t PWM_UpdateDuty(pwm_instance_t instance, uint8_t channel, uint32_t duty);

/*!
 * @brief  Update period for specific a specific channel. This function changes period for
 * all channels which shares the timebase with targeted channel.
 *
 * @param[in] instance The name of the instance
 * @param[in] channel The channel which is update
 * @param[in] period The period measured in ticks
 * @return    Error or success status returned by API
 */
 status_t PWM_UpdatePeriod(pwm_instance_t instance, uint8_t channel, uint32_t period);

 /*!
 * @brief This function change the output value for some channels. channelsMask select
 * which channels will be overwrite, each bit filed representing one channel: 1 - channel is controlled
 * by channelsValues, 0 - channel is controlled by pwm. channelsValues select output values to be write on corresponding
 * channel.
 *
 * @param[in] instance The name of the instance
 * @param[in] channelsMask The name mask used to select which channel is overwrite
 * @param[in] channelsValues The name overwrite values for all channels
 * @return    Error or success status returned by API
 */
 status_t PWM_OverwriteOutputChannel(pwm_instance_t instance, uint32_t channelsMask, uint32_t channelsValues);

 /*!
 * @brief Uninitialised PWM instance.
 *
 * @param[in] instance The name of the instance
 * @return    Error or success status returned by API
 */
 status_t PWM_Deinit(pwm_instance_t instance);

#if defined(__cplusplus)
}
#endif

/*! @}*/
#endif /* PWM_PAL_H */


/*******************************************************************************
 * EOF
 ******************************************************************************/







