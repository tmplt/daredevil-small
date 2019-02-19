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
 * @file timing_pal_mapping.h
 *
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 2.5, global macro not referenced
 * This macro is used by user.
 */

#ifndef TIMING_PAL_MAPPING_H
#define TIMING_PAL_MAPPING_H

#include "timing_pal_cfg.h"

/* Include PD files */
#if (defined(TIMING_OVER_LPIT))
    #include "lpit_driver.h"
#endif

#if (defined(TIMING_OVER_LPTMR))
    #include "lptmr_driver.h"
#endif

#if (defined(TIMING_OVER_FTM))
    #include "ftm_oc_driver.h"
    #include "ftm_mc_driver.h"
#endif

#if (defined(TIMING_OVER_PIT))
    #include "pit_driver.h"
#endif

#if (defined(TIMING_OVER_STM))
    #include "stm_driver.h"
#endif

#if (defined(CPU_S32K144HFT0VLLT) || defined(CPU_S32K144LFT0MLLT) || defined(CPU_S32K142))

/*
 * @brief Define instances for S32K144 and S32K142 (LPTMR, LPIT, FTM)
 */
typedef enum
{
    TIMING_OVER_LPIT0_INSTANCE  = 0U,
    TIMING_OVER_LPTMR0_INSTANCE = 1U,
    TIMING_OVER_FTM0_INSTANCE   = 2U,
    TIMING_OVER_FTM1_INSTANCE   = 3U,
    TIMING_OVER_FTM2_INSTANCE   = 4U,
    TIMING_OVER_FTM3_INSTANCE   = 5U
} timer_instance_t;

/* On S32K14x are 6 available instances which support TIMING */
#define NUMBER_OF_TIMING_PAL_INSTANCES 6U
#define NUMBER_OF_TIMING_OVER_LPIT_INSTANCES 1U
#define NUMBER_OF_TIMING_OVER_LPTMR_INSTANCES 1U
#define NUMBER_OF_TIMING_OVER_FTM_INSTANCES 4U

/* Defines the index limits for all TIMINGs */
#define LPIT_TIMING_HIGH_INDEX  0U
#define LPTMR_TIMING_LOW_INDEX  1U
#define LPTMR_TIMING_HIGH_INDEX 1U
#define FTM_TIMING_LOW_INDEX    2U
#define FTM_TIMING_HIGH_INDEX   5U

#elif (defined(CPU_S32K146))

/*
 * @brief Define instances for S32K146 (LPTMR, LPIT, FTM)
 */
typedef enum
{
    TIMING_OVER_LPIT0_INSTANCE  = 0U,
    TIMING_OVER_LPTMR0_INSTANCE = 1U,
    TIMING_OVER_FTM0_INSTANCE   = 2U,
    TIMING_OVER_FTM1_INSTANCE   = 3U,
    TIMING_OVER_FTM2_INSTANCE   = 4U,
    TIMING_OVER_FTM3_INSTANCE   = 5U,
    TIMING_OVER_FTM4_INSTANCE   = 6U,
    TIMING_OVER_FTM5_INSTANCE   = 7U
} timer_instance_t;

/* On S32K14x are 8 available instances which support TIMING */
#define NUMBER_OF_TIMING_PAL_INSTANCES 8U
#define NUMBER_OF_TIMING_OVER_LPIT_INSTANCES 1U
#define NUMBER_OF_TIMING_OVER_LPTMR_INSTANCES 1U
#define NUMBER_OF_TIMING_OVER_FTM_INSTANCES 6U

/* Defines the index limits for all TIMINGs */
#define LPIT_TIMING_HIGH_INDEX  0U
#define LPTMR_TIMING_LOW_INDEX  1U
#define LPTMR_TIMING_HIGH_INDEX 1U
#define FTM_TIMING_LOW_INDEX    2U
#define FTM_TIMING_HIGH_INDEX   7U

#elif (defined(CPU_S32K148))

/*
 * @brief Define instances for S32K148 (LPTMR, LPIT, FTM)
 */
typedef enum
{
    TIMING_OVER_LPIT0_INSTANCE  = 0U,
    TIMING_OVER_LPTMR0_INSTANCE = 1U,
    TIMING_OVER_FTM0_INSTANCE   = 2U,
    TIMING_OVER_FTM1_INSTANCE   = 3U,
    TIMING_OVER_FTM2_INSTANCE   = 4U,
    TIMING_OVER_FTM3_INSTANCE   = 5U,
    TIMING_OVER_FTM4_INSTANCE   = 6U,
    TIMING_OVER_FTM5_INSTANCE   = 7U,
    TIMING_OVER_FTM6_INSTANCE   = 8U,
    TIMING_OVER_FTM7_INSTANCE   = 9U
} timer_instance_t;

/* On S32K14x are 10 available instances which support TIMING */
#define NUMBER_OF_TIMING_PAL_INSTANCES 10U
#define NUMBER_OF_TIMING_OVER_LPIT_INSTANCES 1U
#define NUMBER_OF_TIMING_OVER_LPTMR_INSTANCES 1U
#define NUMBER_OF_TIMING_OVER_FTM_INSTANCES 8U

/* Defines the index limits for all TIMINGs */
#define LPIT_TIMING_HIGH_INDEX  0U
#define LPTMR_TIMING_LOW_INDEX  1U
#define LPTMR_TIMING_HIGH_INDEX 1U
#define FTM_TIMING_LOW_INDEX    2U
#define FTM_TIMING_HIGH_INDEX   9U

#elif (defined(CPU_S32V234))

/*
 * @brief Define instances for S32V234 (PIT, STM)
 */
typedef enum
{
    TIMING_OVER_PIT0_INSTANCE = 0U,
    TIMING_OVER_PIT1_INSTANCE = 1U,
    TIMING_OVER_FTM0_INSTANCE = 2U,
    TIMING_OVER_FTM1_INSTANCE = 3U,
    TIMING_OVER_STM0_INSTANCE = 4U,
    TIMING_OVER_STM1_INSTANCE = 5U
} timer_instance_t;

/* On S32V234 are 6 available instances which support TIMING */
#define NUMBER_OF_TIMING_PAL_INSTANCES 6U
#define NUMBER_OF_TIMING_OVER_PIT_INSTANCES 2U
#define NUMBER_OF_TIMING_OVER_STM_INSTANCES 2U
#define NUMBER_OF_TIMING_OVER_FTM_INSTANCES 2U

/* Defines the index limits for all TIMINGs */
#define PIT_TIMING_HIGH_INDEX  1U
#define FTM_TIMING_LOW_INDEX   2U
#define FTM_TIMING_HIGH_INDEX  3U
#define STM_TIMING_LOW_INDEX   4U
#define STM_TIMING_HIGH_INDEX  5U

#elif (defined(CPU_MPC5748G))

/*
 * @brief Define instances for MPC5748G (PIT, STM)
 */
typedef enum
{
    TIMING_OVER_PIT0_INSTANCE = 0U,
    TIMING_OVER_STM0_INSTANCE = 1U,
    TIMING_OVER_STM1_INSTANCE = 2U,
    TIMING_OVER_STM2_INSTANCE = 3U
} timer_instance_t;

/* On MPC5748G are 4 available instances which support TIMING */
#define NUMBER_OF_TIMING_PAL_INSTANCES 4U
#define NUMBER_OF_TIMING_OVER_PIT_INSTANCES 1U
#define NUMBER_OF_TIMING_OVER_STM_INSTANCES 3U

/* Defines the index limits for all TIMINGs */
#define PIT_TIMING_HIGH_INDEX 0U
#define STM_TIMING_LOW_INDEX  1U
#define STM_TIMING_HIGH_INDEX 3U

#elif (defined(CPU_MPC5746C))

/*
 * @brief Define instances for MPC5746C (PIT, STM)
 */
typedef enum
{
    TIMING_OVER_PIT0_INSTANCE = 0U,
    TIMING_OVER_STM0_INSTANCE = 1U,
    TIMING_OVER_STM1_INSTANCE = 2U
} timer_instance_t;

/* On MPC5746C are 3 available instances which support TIMING */
#define NUMBER_OF_TIMING_PAL_INSTANCES 3U
#define NUMBER_OF_TIMING_OVER_PIT_INSTANCES 1U
#define NUMBER_OF_TIMING_OVER_STM_INSTANCES 2U

/* Defines the index limits for all TIMINGs */
#define PIT_TIMING_HIGH_INDEX 0U
#define STM_TIMING_LOW_INDEX  1U
#define STM_TIMING_HIGH_INDEX 2U

#elif (defined(CPU_MPC5744P))

/*
 * @brief Define instances for MPC5744P (PIT, STM)
 */
typedef enum
{
    TIMING_OVER_PIT0_INSTANCE = 0U,
    TIMING_OVER_STM0_INSTANCE = 1U
} timer_instance_t;

/* On MPC5746C are 2 available instances which support TIMING */
#define NUMBER_OF_TIMING_PAL_INSTANCES 2U
#define NUMBER_OF_TIMING_OVER_PIT_INSTANCES 1U
#define NUMBER_OF_TIMING_OVER_STM_INSTANCES 1U

/* Defines the index limits for all TIMINGs */
#define PIT_TIMING_HIGH_INDEX 0U
#define STM_TIMING_LOW_INDEX  1U
#define STM_TIMING_HIGH_INDEX 1U

#endif

#endif /* TIMING_PAL_MAPPING_H */
