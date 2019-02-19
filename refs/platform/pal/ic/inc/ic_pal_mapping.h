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

#ifndef IC_PAL_MAPPING_H
#define IC_PAL_MAPPING_H


/* Include peripheral drivers */
#if ((defined(CPU_S32K144HFT0VLLT) || defined(CPU_S32K144LFT0MLLT) || \
      defined(CPU_S32K148) || defined(CPU_S32K142) || defined(CPU_S32K146)))
    #include "ftm_ic_driver.h"
    #include "ftm_mc_driver.h"

    /* The maximum of channel in each instance */
    #define IC_PAL_NUM_OF_CHANNEL_MAX  FEATURE_FTM_CHANNEL_COUNT
    /* The maximum of counter */
    #define MAX_COUNTER_VALUE          FTM_FEATURE_CNT_MAX_VALUE_U32
    /* The number of instances are available */
    #define NUMBER_OF_IC_PAL_INSTANCES NO_OF_FTM_INSTS_FOR_IC
    /* The maximum of instance in FTM */
    #define IC_PAL_INSTANCE_MAX        FTM_INSTANCE_COUNT
#endif /* End of definition for S32k14x */

#if (defined(CPU_MPC5748G) || defined(CPU_MPC5746C))
    #include "emios_mc_driver.h"
    #include "emios_ic_driver.h"

    /* The maximum of channel in each instance */
    #define IC_PAL_NUM_OF_CHANNEL_MAX  eMIOS_UC_COUNT
    /* The maximum of counter */
    #define MAX_COUNTER_VALUE          (0xFFFFU)
    /* The number of instances are available */
    #define NUMBER_OF_IC_PAL_INSTANCES NO_OF_EMIOS_INSTS_FOR_IC
    /* The maximum of instance in eMIOS */
    #define IC_PAL_INSTANCE_MAX        eMIOS_INSTANCE_COUNT
#endif /* End of definition for MPC574x */

#endif /* IC_PAL_MAPPING_H */
