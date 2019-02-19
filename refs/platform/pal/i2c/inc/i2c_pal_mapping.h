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
 * Define the available instances for I2C functionality
 */
#if (defined(CPU_S32K144HFT0VLLT) || defined(CPU_S32K144LFT0MLLT) || defined(CPU_S32K142) || defined(CPU_S32K144) || defined(CPU_S32K146))
/*
 * @brief Define instances for S32K142, S32K144, S32K146, S32K148 (LPI2C and FlexIO)
 * Implements : i2c_instance_t_Class
 */

 typedef enum
 {
    I2C_OVER_LPI2C0_INSTANCE = 0U,
    I2C_OVER_FLEXIO0_INSTANCE = 1U, /* This is a virtual I2C instance over FlexIO */
    I2C_OVER_FLEXIO1_INSTANCE = 2U, /* This is a virtual I2C instance over FlexIO */

 }i2c_instance_t;

 /* On S32K142, S32K144 and S32K146 are 3 instances which support I2C */
 #define NUMBER_OF_I2C_PAL_INSTANCES 3U
 /* Defines the index limits for all I2Cs */
 #define LPI2C_HIGH_INDEX 0U
 #define FLEXIO_I2C_LOW_INDEX 1U
 #define FLEXIO_I2C_HIGH_INDEX 2U
 #endif

 #if (defined(CPU_S32K148))
/*
 * @brief Define instances for S32K148 (LPI2C and FlexIO)
 * Implements : i2c_instance_t_Class
 */

 typedef enum
 {
    I2C_OVER_LPI2C0_INSTANCE = 0U,
    I2C_OVER_LPI2C1_INSTANCE = 1U,
    I2C_OVER_FLEXIO0_INSTANCE = 2U, /* This is a virtual I2C instance over FlexIO */
    I2C_OVER_FLEXIO1_INSTANCE = 3U, /* This is a virtual I2C instance over FlexIO */
 }i2c_instance_t;

 /* On S32K148 are 4 instances which support I2C */
 #define NUMBER_OF_I2C_PAL_INSTANCES 4U
 /* Defines the index limits for all I2Cs */
 #define LPI2C_LOW_INDEX 0U
 #define LPI2C_HIGH_INDEX 1U
 #define FLEXIO_I2C_LOW_INDEX 2U
 #define FLEXIO_I2C_HIGH_INDEX 3U
 #endif

#if ((defined(CPU_MPC5748G)) || (defined(CPU_MPC5746C)))
/*
 * @brief Define instances for MPC5746C and MPC5748G
 * Implements : i2c_instance_t_Class
 */
typedef enum
{
    I2C_OVER_I2C0_INSTANCE = 0U,
    I2C_OVER_I2C1_INSTANCE = 1U,
    I2C_OVER_I2C2_INSTANCE = 2U,
    I2C_OVER_I2C3_INSTANCE = 3U,

}i2c_instance_t;

/* On MPC5746C and MPC5748G are 4 instances which support I2C */
#define NUMBER_OF_I2C_PAL_INSTANCES 4U
/* Defines the index limits for all I2Cs */
#define I2C_LOW_INDEX 0U
#define I2C_HIGH_INDEX 3U
#endif

#endif /* PAL_mapping_H */

/*******************************************************************************
 * EOF
 ******************************************************************************/


