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
 * @file can_pal_mapping.h
 *
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 2.5, global macro not referenced
 * This macro is used by user.
 */

#ifndef CAN_PAL_MAPPING_H
#define CAN_PAL_MAPPING_H

#include "device_registers.h"

#if (defined(CPU_S32K144HFT0VLLT) || defined(CPU_S32K144LFT0MLLT) || defined(CPU_S32K146) || \
     defined(CPU_S32K148))

/*
 * @brief Define instances for S32K144, S32K146 and S32K148 (FlexCAN)
 */
typedef enum
{
    CAN_OVER_FLEXCAN00_INSTANCE = 0U,
    CAN_OVER_FLEXCAN01_INSTANCE = 1U,
    CAN_OVER_FLEXCAN02_INSTANCE = 2U,
} can_instance_t;

/* On S32K144, S32K146 and S32K148 are 3 available instances which support CAN */
#define NUMBER_OF_CAN_PAL_INSTANCES 3U

/* Defines the index limits for all CANs */
#define FLEXCAN_HIGH_INDEX      2U

#elif (defined(CPU_S32K142))

/*
 * @brief Define instances for S32K142 (FlexCAN)
 */
typedef enum
{
    CAN_OVER_FLEXCAN00_INSTANCE = 0U,
    CAN_OVER_FLEXCAN01_INSTANCE = 1U,
} can_instance_t;

/* On S32K142 are 2 available instances which support CAN */
#define NUMBER_OF_CAN_PAL_INSTANCES 2U

/* Defines the index limits for all CANs */
#define FLEXCAN_HIGH_INDEX      1U

#elif (defined(CPU_S32V234))

/*
 * @brief Define instances for S32V234 (FlexCAN)
 */
typedef enum
{
    CAN_OVER_FLEXCAN00_INSTANCE = 0U,
    CAN_OVER_FLEXCAN01_INSTANCE = 1U,
} can_instance_t;

/* On S32V234 are 2 available instances which support CAN */
#define NUMBER_OF_CAN_PAL_INSTANCES 2U

/* Defines the index limits for all CANs */
#define FLEXCAN_LOW_INDEX   0U
#define FLEXCAN_HIGH_INDEX  1U

#elif (defined(CPU_MPC5748G))

/*
 * @brief Define instances for MPC5748G (FlexCAN)
 */
typedef enum
{
    CAN_OVER_FLEXCAN00_INSTANCE = 0U,
    CAN_OVER_FLEXCAN01_INSTANCE = 1U,
    CAN_OVER_FLEXCAN02_INSTANCE = 2U,
    CAN_OVER_FLEXCAN03_INSTANCE = 3U,
    CAN_OVER_FLEXCAN04_INSTANCE = 4U,
    CAN_OVER_FLEXCAN05_INSTANCE = 5U,
    CAN_OVER_FLEXCAN06_INSTANCE = 6U,
    CAN_OVER_FLEXCAN07_INSTANCE = 7U,
} can_instance_t;

/* On MPC5748G are 8 available instances which support CAN */
#define NUMBER_OF_CAN_PAL_INSTANCES 8U

/* Defines the index limits for all CANs */
#define FLEXCAN_LOW_INDEX   0U
#define FLEXCAN_HIGH_INDEX  7U

#elif (defined(CPU_MPC5746C))

/*
 * @brief Define instances for MPC5746C (LinFlexD)
 */
typedef enum
{
    CAN_OVER_FLEXCAN00_INSTANCE = 0U,
    CAN_OVER_FLEXCAN01_INSTANCE = 1U,
    CAN_OVER_FLEXCAN02_INSTANCE = 2U,
    CAN_OVER_FLEXCAN03_INSTANCE = 3U,
    CAN_OVER_FLEXCAN04_INSTANCE = 4U,
    CAN_OVER_FLEXCAN05_INSTANCE = 5U,
    CAN_OVER_FLEXCAN06_INSTANCE = 6U,
    CAN_OVER_FLEXCAN07_INSTANCE = 7U,
} can_instance_t;

/* On MPC5746C are 8 available instances which support CAN */
#define NUMBER_OF_CAN_PAL_INSTANCES 8U

/* Defines the index limits for all CANs */
#define FLEXCAN_LOW_INDEX   0U
#define FLEXCAN_HIGH_INDEX  7U

#endif

#endif /* CAN_PAL_MAPPING_H */
