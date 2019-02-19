/*
 * Copyright (c) 2013 - 2014, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
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

#ifndef TIMING_IRQ_H
#define TIMING_IRQ_H

#include "device_registers.h"
#include "timing_pal_mapping.h"
#include "interrupt_manager.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

#if (defined (TIMING_OVER_FTM))
/*! @brief Table of base addresses for FTM instances. */
extern FTM_Type * const ftmBase[FTM_INSTANCE_COUNT];
#endif /* TIMING_OVER_FTM */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
#if (defined (TIMING_OVER_LPIT))
void TIMING_Lpit_IrqHandler(uint32_t instance, uint8_t channel);
#endif

#if (defined (TIMING_OVER_LPTMR))
void TIMING_Lptmr_IrqHandler(uint32_t instance, uint8_t channel);
#endif

#if (defined (TIMING_OVER_FTM))
void TIMING_Ftm_IrqHandler(uint32_t instance, uint8_t channel);
#endif

#if (defined (TIMING_OVER_PIT))
void TIMING_Pit_IrqHandler(uint32_t instance, uint8_t channel);
#endif

#if (defined (TIMING_OVER_STM))
void TIMING_Stm_IrqHandler(uint32_t instance, uint8_t channel);
#endif
/*******************************************************************************
 *  Default interrupt handlers signatures
 ******************************************************************************/

/* Define TIMING PAL over FTM */
#if (defined(TIMING_OVER_FTM))

#if (NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 0U)
void FTM0_Ch0_Ch1_IrqHandler(void);

void FTM0_Ch2_Ch3_IrqHandler(void);

void FTM0_Ch4_Ch5_IrqHandler(void);

void FTM0_Ch6_Ch7_IrqHandler(void);
#endif

#if (NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 1U)
void FTM1_Ch0_Ch1_IrqHandler(void);

void FTM1_Ch2_Ch3_IrqHandler(void);

void FTM1_Ch4_Ch5_IrqHandler(void);

void FTM1_Ch6_Ch7_IrqHandler(void);
#endif

#if (NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 2U)
void FTM2_Ch0_Ch1_IrqHandler(void);

void FTM2_Ch2_Ch3_IrqHandler(void);

void FTM2_Ch4_Ch5_IrqHandler(void);

void FTM2_Ch6_Ch7_IrqHandler(void);
#endif

#if (NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 3U)
void FTM3_Ch0_Ch1_IrqHandler(void);

void FTM3_Ch2_Ch3_IrqHandler(void);

void FTM3_Ch4_Ch5_IrqHandler(void);

void FTM3_Ch6_Ch7_IrqHandler(void);
#endif

#if (NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 4U)
void FTM4_Ch0_Ch1_IrqHandler(void);

void FTM4_Ch2_Ch3_IrqHandler(void);

void FTM4_Ch4_Ch5_IrqHandler(void);

void FTM4_Ch6_Ch7_IrqHandler(void);
#endif

#if (NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 5U)
void FTM5_Ch0_Ch1_IrqHandler(void);

void FTM5_Ch2_Ch3_IrqHandler(void);

void FTM5_Ch4_Ch5_IrqHandler(void);

void FTM5_Ch6_Ch7_IrqHandler(void);
#endif

#if (NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 6U)
void FTM6_Ch0_Ch1_IrqHandler(void);

void FTM6_Ch2_Ch3_IrqHandler(void);

void FTM6_Ch4_Ch5_IrqHandler(void);

void FTM6_Ch6_Ch7_IrqHandler(void);
#endif

#if (NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 7U)
void FTM7_Ch0_Ch1_IrqHandler(void);

void FTM7_Ch2_Ch3_IrqHandler(void);

void FTM7_Ch4_Ch5_IrqHandler(void);

void FTM7_Ch6_Ch7_IrqHandler(void);
#endif
/* Array storing references to TIMING over FTM irq handlers */
extern const isr_t s_timingOverFtmIsr[NUMBER_OF_TIMING_OVER_FTM_INSTANCES][FTM_CONTROLS_COUNT];

#endif /* TIMING_OVER_FTM */

#endif /* TIMING_IRQ_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
