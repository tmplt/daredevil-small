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
 * @file i2s_pal_mapping.h
 *
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 2.5, global macro not referenced
 * This macro is used by user.
 */

#ifndef I2S_PAL_MAPPING_H
#define I2S_PAL_MAPPING_H
#include "device_registers.h"

#if (defined(CPU_S32K144HFT0VLLT) || defined(CPU_S32K144LFT0MLLT) || defined(CPU_S32K146) || defined(CPU_S32K142))

/*
 * @brief Define instances for S32K142,S32K144,S32K148 (FlexIO)
 */

typedef enum
{
    I2S_OVER_FLEXIO0_INSTANCE = 0U,
    I2S_OVER_FLEXIO1_INSTANCE = 1U,
} i2s_instance_t;

/* On S32K144, S32K146 and S32K148 are 5 available instances which support I2S */
#define NUMBER_OF_I2S_PAL_INSTANCES 2U

/* Defines the index limits for all I2Ss */
#define FLEXIO_I2S_LOW_INDEX  0U
#define FLEXIO_I2S_HIGH_INDEX 1U

#elif (defined(CPU_S32K148))

/*
 * @brief Define instances for S32K148 (SAI and FlexIO)
 */

typedef enum
{
    I2S_OVER_SAI0_INSTANCE = 0U,
    I2S_OVER_SAI1_INSTANCE = 1U,
    I2S_OVER_FLEXIO0_INSTANCE = 2U,
    I2S_OVER_FLEXIO1_INSTANCE = 3U,
} i2s_instance_t;

/* On S32K148 are 4 available instances which support I2S */
#define NUMBER_OF_I2S_PAL_INSTANCES 4U

/* Defines the index limits for all I2Ss */
#define SAI_HIGH_INDEX      1U
#define FLEXIO_I2S_LOW_INDEX  2U
#define FLEXIO_I2S_HIGH_INDEX 3U

#elif (defined(CPU_MPC5748G) || defined(CPU_MPC5746C))

/*
 * @brief Define instances for MPC5748G, MPC5746C (SAI)
 */

typedef enum
{
    I2S_OVER_SAI0_INSTANCE = 0U,
    I2S_OVER_SAI1_INSTANCE = 1U,
    I2S_OVER_SAI2_INSTANCE = 2U,
} i2s_instance_t;

/* There are 3 available instances which support I2S */
#define NUMBER_OF_I2S_PAL_INSTANCES 3U

/* Defines the index limits for all I2Ss */
#define SAI_LOW_INDEX   0U
#define SAI_HIGH_INDEX  2U
#endif /* CPU_MPC5748G || CPU_MPC5746C */
#endif /* I2S_PAL_MAPPING_H */
