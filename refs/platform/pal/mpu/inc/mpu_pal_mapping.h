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
 * @file mpu_pal_mapping.h
 *
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 2.5, Global macro not referenced.
 * The macros defined are used to define features for each driver, so this might be reported
 * when the analysis is made only on one driver.
 *
 */

#ifndef MPU_PAL_MAPPING_H
#define MPU_PAL_MAPPING_H

#include "device_registers.h"
#include "mpu_pal_cfg.h"

/* Include PD files */
#if defined(MPU_OVER_MPU)
    #include "mpu_driver.h"
#elif defined(MPU_OVER_SMPU)
    #include "smpu_driver.h"
#endif

#if (defined(CPU_S32K142) || defined(CPU_S32K144HFT0VLLT) || defined(CPU_S32K144LFT0MLLT) || defined(CPU_S32K146) || defined(CPU_S32K148) || defined(CPU_S32MTV))
/*!
 * @brief Define instances for S32K14x and S32MTV (MPU).
 * Implements : mpu_instance_t_Class
 */
typedef enum
{
    MPU_OVER_MPU0_INSTANCE = 0U,
} mpu_instance_t;

/* On S32K14x and S32MTV, there is 1 available instance which support MPU PAL */
#define NUMBER_OF_MPU_PAL_INSTANCES (1U)

#elif (defined(CPU_MPC5746C) || defined(CPU_MPC5744P))
/*!
 * @brief Define instances for MPC5746C and MPC5744P (SMPU).
 * Implements : mpu_instance_t_Class
 */
typedef enum
{
    MPU_OVER_SMPU0_INSTANCE  = 0U,
} mpu_instance_t;

/* On MPC5746C and MPC5744P, there is 1 available instance which support MPU PAL*/
#define NUMBER_OF_MPU_PAL_INSTANCES (1U)

#elif (defined(CPU_MPC5748G))
/*!
 * @brief Define instances for MPC5748G (SMPU).
 * Implements : mpu_instance_t_Class
 */
typedef enum
{
    MPU_OVER_SMPU0_INSTANCE  = 0U,
    MPU_OVER_SMPU1_INSTANCE  = 1U
} mpu_instance_t;

/* On MPC5748G, there are 2 available instances which support MPU PAL */
#define NUMBER_OF_MPU_PAL_INSTANCES (2U)

#endif

#endif /* MPU_PAL_MAPPING_H */
