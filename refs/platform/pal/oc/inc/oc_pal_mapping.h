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

#ifndef OC_PAL_MAPPING_H
#define OC_PAL_MAPPING_H

#include "oc_pal_cfg.h"

/* Include peripheral drivers */
#if defined(OC_PAL_OVER_FTM)
    #include "ftm_oc_driver.h"
    #include "ftm_mc_driver.h"

    /* The maximum of channel in each instance */
    #define OC_PAL_NUM_OF_CHANNEL_MAX FEATURE_FTM_CHANNEL_COUNT
    /* The number of instances are available */
    #define NUMBER_OF_OC_PAL_INSTANCES NO_OF_FTM_INSTS_FOR_OC
    /* The maximum of instances in FTM */
    #define OC_PAL_INSTANCES_MAX      FTM_INSTANCE_COUNT
#endif /* defined(OC_PAL_OVER_FTM) */

#if defined(OC_PAL_OVER_EMIOS)
    #include "emios_mc_driver.h"
    #include "emios_oc_driver.h"

    /* The maximum of channel in each instance */
    #define OC_PAL_NUM_OF_CHANNEL_MAX  eMIOS_UC_COUNT
    /* The number of instances are available */
    #define NUMBER_OF_OC_PAL_INSTANCES NO_OF_EMIOS_INSTS_FOR_OC
    /* The maximum of instances in eMIOS */
    #define OC_PAL_INSTANCES_MAX      eMIOS_INSTANCE_COUNT

#endif /* defined(OC_PAL_OVER_EMIOS) */

#endif /* OC_PAL_MAPPING_H */
