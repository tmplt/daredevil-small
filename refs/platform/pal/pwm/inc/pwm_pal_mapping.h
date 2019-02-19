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

#ifndef PAL_mapping_H
#define PAL_mapping_H
#include "device_registers.h"
/*
 * Define the available instances for PWM functionality
 */
#if (defined(CPU_S32K144HFT0VLLT) || defined(CPU_S32K144LFT0MLLT) || defined(CPU_S32K142))
/*
 * @brief Define instances for S32K142 and S32K144
 * Implements : pwm_instance_t_Class
 */

typedef enum
{
    PWM_OVER_FTM0_INSTANCE = 0U,
    PWM_OVER_FTM1_INSTANCE = 1U,
    PWM_OVER_FTM2_INSTANCE = 2U,
    PWM_OVER_FTM3_INSTANCE = 3U,
} pwm_instance_t;

/* On S32K142 and S32K144 are available 4 instances which supports PWM */
#define NUMBER_OF_PWM_PAL_INSTANCES 4U
/* Defines the index limits for all SPIs */
#define FTM_HIGH_INDEX 3U
#endif

#if (defined(CPU_S32K146))
/*
 * @brief Define instances for S32K146
 * Implements : pwm_instance_t_Class
 */

typedef enum
{
    PWM_OVER_FTM0_INSTANCE = 0U,
    PWM_OVER_FTM1_INSTANCE = 1U,
    PWM_OVER_FTM2_INSTANCE = 2U,
    PWM_OVER_FTM3_INSTANCE = 3U,
    PWM_OVER_FTM4_INSTANCE = 4U,
    PWM_OVER_FTM5_INSTANCE = 5U,
} pwm_instance_t;

/* On S32K146 are available 6 instances which supports PWM */
#define NUMBER_OF_PWM_PAL_INSTANCES 6U
/* Defines the index limits for all SPIs */
#define PWM_HIGH_INDEX 5U
#endif

#if (defined(CPU_S32K148))
/*
 * @brief Define instances for S32K148
 * Implements : pwm_instance_t_Class
 */

typedef enum
{
    PWM_OVER_FTM0_INSTANCE = 0U,
    PWM_OVER_FTM1_INSTANCE = 1U,
    PWM_OVER_FTM2_INSTANCE = 2U,
    PWM_OVER_FTM3_INSTANCE = 3U,
    PWM_OVER_FTM4_INSTANCE = 4U,
    PWM_OVER_FTM5_INSTANCE = 5U,
    PWM_OVER_FTM6_INSTANCE = 6U,
    PWM_OVER_FTM7_INSTANCE = 7U,
} pwm_instance_t;

/* On S32K146 are available 8 instances which supports PWM */
#define NUMBER_OF_PWM_PAL_INSTANCES 8U
/* Defines the index limits for all PWM */
#define PWM_HIGH_INDEX 7U
#endif

#endif /* PAL_mapping_H */
