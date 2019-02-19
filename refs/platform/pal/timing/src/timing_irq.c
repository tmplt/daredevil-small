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
 * @file timing_irq.c
 *
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 8.4, external symbol defined without a prior
 * declaration.
 * These are symbols weak symbols defined in platform startup files (.s).
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.7, Function not defined with external linkage.
 * The functions are not defined static because they are referenced in .s startup files.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.9, Could define variable at block scope
 * The variable is used in driver c file, so it must remain global.
 */

#include "timing_irq.h"

/*******************************************************************************
 * Code
 ******************************************************************************/

/* Define interrupt handler for Timing instance */

/* Define TIMING PAL over LPIT */
#if (defined (TIMING_OVER_LPIT))

#if (NUMBER_OF_TIMING_OVER_LPIT_INSTANCES > 0U)
void LPIT0_Ch0_IRQHandler(void)
{
    TIMING_Lpit_IrqHandler(0U, 0U);
}

void LPIT0_Ch1_IRQHandler(void)
{
    TIMING_Lpit_IrqHandler(0U, 1U);
}

void LPIT0_Ch2_IRQHandler(void)
{
    TIMING_Lpit_IrqHandler(0U, 2U);
}

void LPIT0_Ch3_IRQHandler(void)
{
    TIMING_Lpit_IrqHandler(0U, 3U);
}
#endif /* NUMBER_OF_TIMING_OVER_LPIT_INSTANCES > 0U */

#endif /* TIMING_OVER_LPIT */

/* Define TIMING PAL over LPTMR */
#if (defined (TIMING_OVER_LPTMR))

#if (NUMBER_OF_TIMING_OVER_LPTMR_INSTANCES > 0U)
void LPTMR0_IRQHandler(void)
{
    TIMING_Lptmr_IrqHandler(0U, 0U);
}
#endif /* NUMBER_OF_TIMING_OVER_LPTMR_INSTANCES > 0U */

#endif /* TIMING_OVER_LPTMR */

/* Define TIMING PAL over PIT */
#if (defined(TIMING_OVER_PIT) && (defined(CPU_MPC5748G) || defined(CPU_MPC5746C)))

#if (NUMBER_OF_TIMING_OVER_PIT_INSTANCES > 0U)
void PIT0_Ch0_IRQHandler(void)
{
    TIMING_Pit_IrqHandler(0U, 0U);
}

void PIT0_Ch1_IRQHandler(void)
{
    TIMING_Pit_IrqHandler(0U, 1U);
}

void PIT0_Ch2_IRQHandler(void)
{
    TIMING_Pit_IrqHandler(0U, 2U);
}

void PIT0_Ch3_IRQHandler(void)
{
    TIMING_Pit_IrqHandler(0U, 3U);
}

void PIT0_Ch4_IRQHandler(void)
{
    TIMING_Pit_IrqHandler(0U, 4U);
}

void PIT0_Ch5_IRQHandler(void)
{
    TIMING_Pit_IrqHandler(0U, 5U);
}

void PIT0_Ch6_IRQHandler(void)
{
    TIMING_Pit_IrqHandler(0U, 6U);
}

void PIT0_Ch7_IRQHandler(void)
{
    TIMING_Pit_IrqHandler(0U, 7U);
}

void PIT0_Ch8_IRQHandler(void)
{
    TIMING_Pit_IrqHandler(0U, 8U);
}

void PIT0_Ch9_IRQHandler(void)
{
    TIMING_Pit_IrqHandler(0U, 9U);
}

void PIT0_Ch10_IRQHandler(void)
{
    TIMING_Pit_IrqHandler(0U, 10U);
}

void PIT0_Ch11_IRQHandler(void)
{
    TIMING_Pit_IrqHandler(0U, 11U);
}

void PIT0_Ch12_IRQHandler(void)
{
    TIMING_Pit_IrqHandler(0U, 12U);
}

void PIT0_Ch13_IRQHandler(void)
{
    TIMING_Pit_IrqHandler(0U, 13U);
}

void PIT0_Ch14_IRQHandler(void)
{
    TIMING_Pit_IrqHandler(0U, 14U);
}

void PIT0_Ch15_IRQHandler(void)
{
    TIMING_Pit_IrqHandler(0U, 15U);
}
#endif /* NUMBER_OF_TIMING_OVER_PIT_INSTANCES > 0U */

#endif /* TIMING_OVER_PIT */

/* Define TIMING PAL over FTM */
#if (defined(TIMING_OVER_FTM))

#if (NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 0U)
void FTM0_Ch0_Ch1_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[0];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(0U);
    bool chan0IntFlag = FTM_DRV_GetChnEventStatus(base, 0U);
    bool chan1IntFlag = FTM_DRV_GetChnEventStatus(base, 1U);
    bool chan0EnabledInt = ((enabledInterrupts & (1UL << 0U)) != 0U) ? true : false;
    bool chan1EnabledInt = ((enabledInterrupts & (1UL << 1U)) != 0U) ? true : false;

    if (chan0EnabledInt && chan0IntFlag)
    {
        TIMING_Ftm_IrqHandler(0U, 0U);
    }

    if (chan1EnabledInt && chan1IntFlag)
    {
        TIMING_Ftm_IrqHandler(0U, 1U);
    }
}

void FTM0_Ch2_Ch3_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[0];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(0U);
    bool chan2IntFlag = FTM_DRV_GetChnEventStatus(base, 2U);
    bool chan3IntFlag = FTM_DRV_GetChnEventStatus(base, 3U);
    bool chan2EnabledInt = ((enabledInterrupts & (1UL << 2U)) != 0U) ? true : false;
    bool chan3EnabledInt = ((enabledInterrupts & (1UL << 3U)) != 0U) ? true : false;

    if (chan2EnabledInt && chan2IntFlag)
    {
        TIMING_Ftm_IrqHandler(0U, 2U);
    }

    if (chan3EnabledInt && chan3IntFlag)
    {
        TIMING_Ftm_IrqHandler(0U, 3U);
    }
}

void FTM0_Ch4_Ch5_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[0];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(0U);
    bool chan4IntFlag = FTM_DRV_GetChnEventStatus(base, 4U);
    bool chan5IntFlag = FTM_DRV_GetChnEventStatus(base, 5U);
    bool chan4EnabledInt = ((enabledInterrupts & (1UL << 4U)) != 0U) ? true : false;
    bool chan5EnabledInt = ((enabledInterrupts & (1UL << 5U)) != 0U) ? true : false;

    if (chan4EnabledInt && chan4IntFlag)
    {
        TIMING_Ftm_IrqHandler(0U, 4U);
    }

    if (chan5EnabledInt && chan5IntFlag)
    {
        TIMING_Ftm_IrqHandler(0U, 5U);
    }
}

void FTM0_Ch6_Ch7_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[0];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(0U);
    bool chan6IntFlag = FTM_DRV_GetChnEventStatus(base, 6U);
    bool chan7IntFlag = FTM_DRV_GetChnEventStatus(base, 7U);
    bool chan6EnabledInt = ((enabledInterrupts & (1UL << 6U)) != 0U) ? true : false;
    bool chan7EnabledInt = ((enabledInterrupts & (1UL << 7U)) != 0U) ? true : false;

    if (chan6EnabledInt && chan6IntFlag)
    {
        TIMING_Ftm_IrqHandler(0U, 6U);
    }

    if (chan7EnabledInt && chan7IntFlag)
    {
        TIMING_Ftm_IrqHandler(0U, 7U);
    }
}
#endif /* NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 0U */

#if (NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 1U)
void FTM1_Ch0_Ch1_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[1];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(1U);
    bool chan0IntFlag = FTM_DRV_GetChnEventStatus(base, 0U);
    bool chan1IntFlag = FTM_DRV_GetChnEventStatus(base, 1U);
    bool chan0EnabledInt = ((enabledInterrupts & (1UL << 0U)) != 0U) ? true : false;
    bool chan1EnabledInt = ((enabledInterrupts & (1UL << 1U)) != 0U) ? true : false;

    if (chan0EnabledInt && chan0IntFlag)
    {
        TIMING_Ftm_IrqHandler(1U, 0U);
    }

    if (chan1EnabledInt && chan1IntFlag)
    {
        TIMING_Ftm_IrqHandler(1U, 1U);
    }
}

void FTM1_Ch2_Ch3_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[1];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(1U);
    bool chan2IntFlag = FTM_DRV_GetChnEventStatus(base, 2U);
    bool chan3IntFlag = FTM_DRV_GetChnEventStatus(base, 3U);
    bool chan2EnabledInt = ((enabledInterrupts & (1UL << 2U)) != 0U) ? true : false;
    bool chan3EnabledInt = ((enabledInterrupts & (1UL << 3U)) != 0U) ? true : false;

    if (chan2EnabledInt && chan2IntFlag)
    {
        TIMING_Ftm_IrqHandler(1U, 2U);
    }

    if (chan3EnabledInt && chan3IntFlag)
    {
        TIMING_Ftm_IrqHandler(1U, 3U);
    }
}

void FTM1_Ch4_Ch5_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[1];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(1U);
    bool chan4IntFlag = FTM_DRV_GetChnEventStatus(base, 4U);
    bool chan5IntFlag = FTM_DRV_GetChnEventStatus(base, 5U);
    bool chan4EnabledInt = ((enabledInterrupts & (1UL << 4U)) != 0U) ? true : false;
    bool chan5EnabledInt = ((enabledInterrupts & (1UL << 5U)) != 0U) ? true : false;

    if (chan4EnabledInt && chan4IntFlag)
    {
        TIMING_Ftm_IrqHandler(1U, 4U);
    }

    if (chan5EnabledInt && chan5IntFlag)
    {
        TIMING_Ftm_IrqHandler(1U, 5U);
    }
}

void FTM1_Ch6_Ch7_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[1];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(1U);
    bool chan6IntFlag = FTM_DRV_GetChnEventStatus(base, 6U);
    bool chan7IntFlag = FTM_DRV_GetChnEventStatus(base, 7U);
    bool chan6EnabledInt = ((enabledInterrupts & (1UL << 6U)) != 0U) ? true : false;
    bool chan7EnabledInt = ((enabledInterrupts & (1UL << 7U)) != 0U) ? true : false;

    if (chan6EnabledInt && chan6IntFlag)
    {
        TIMING_Ftm_IrqHandler(1U, 6U);
    }

    if (chan7EnabledInt && chan7IntFlag)
    {
        TIMING_Ftm_IrqHandler(1U, 7U);
    }
}
#endif /* NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 1U */

#if (NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 2U)
void FTM2_Ch0_Ch1_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[2];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(2U);
    bool chan0IntFlag = FTM_DRV_GetChnEventStatus(base, 0U);
    bool chan1IntFlag = FTM_DRV_GetChnEventStatus(base, 1U);
    bool chan0EnabledInt = ((enabledInterrupts & (1UL << 0U)) != 0U) ? true : false;
    bool chan1EnabledInt = ((enabledInterrupts & (1UL << 1U)) != 0U) ? true : false;

    if (chan0EnabledInt && chan0IntFlag)
    {
        TIMING_Ftm_IrqHandler(2U, 0U);
    }

    if (chan1EnabledInt && chan1IntFlag)
    {
        TIMING_Ftm_IrqHandler(2U, 1U);
    }
}

void FTM2_Ch2_Ch3_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[2];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(2U);
    bool chan2IntFlag = FTM_DRV_GetChnEventStatus(base, 2U);
    bool chan3IntFlag = FTM_DRV_GetChnEventStatus(base, 3U);
    bool chan2EnabledInt = ((enabledInterrupts & (1UL << 2U)) != 0U) ? true : false;
    bool chan3EnabledInt = ((enabledInterrupts & (1UL << 3U)) != 0U) ? true : false;

    if (chan2EnabledInt && chan2IntFlag)
    {
        TIMING_Ftm_IrqHandler(2U, 2U);
    }

    if (chan3EnabledInt && chan3IntFlag)
    {
        TIMING_Ftm_IrqHandler(2U, 3U);
    }
}

void FTM2_Ch4_Ch5_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[2];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(2U);
    bool chan4IntFlag = FTM_DRV_GetChnEventStatus(base, 4U);
    bool chan5IntFlag = FTM_DRV_GetChnEventStatus(base, 5U);
    bool chan4EnabledInt = ((enabledInterrupts & (1UL << 4U)) != 0U) ? true : false;
    bool chan5EnabledInt = ((enabledInterrupts & (1UL << 5U)) != 0U) ? true : false;

    if (chan4EnabledInt && chan4IntFlag)
    {
        TIMING_Ftm_IrqHandler(2U, 4U);
    }

    if (chan5EnabledInt && chan5IntFlag)
    {
        TIMING_Ftm_IrqHandler(2U, 5U);
    }
}

void FTM2_Ch6_Ch7_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[2];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(2U);
    bool chan6IntFlag = FTM_DRV_GetChnEventStatus(base, 6U);
    bool chan7IntFlag = FTM_DRV_GetChnEventStatus(base, 7U);
    bool chan6EnabledInt = ((enabledInterrupts & (1UL << 6U)) != 0U) ? true : false;
    bool chan7EnabledInt = ((enabledInterrupts & (1UL << 7U)) != 0U) ? true : false;

    if (chan6EnabledInt && chan6IntFlag)
    {
        TIMING_Ftm_IrqHandler(2U, 6U);
    }

    if (chan7EnabledInt && chan7IntFlag)
    {
        TIMING_Ftm_IrqHandler(2U, 7U);
    }
}
#endif /* NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 2U */

#if (NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 3U)
void FTM3_Ch0_Ch1_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[3];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(3U);
    bool chan0IntFlag = FTM_DRV_GetChnEventStatus(base, 0U);
    bool chan1IntFlag = FTM_DRV_GetChnEventStatus(base, 1U);
    bool chan0EnabledInt = ((enabledInterrupts & (1UL << 0U)) != 0U) ? true : false;
    bool chan1EnabledInt = ((enabledInterrupts & (1UL << 1U)) != 0U) ? true : false;

    if (chan0EnabledInt && chan0IntFlag)
    {
        TIMING_Ftm_IrqHandler(3U, 0U);
    }

    if (chan1EnabledInt && chan1IntFlag)
    {
        TIMING_Ftm_IrqHandler(3U, 1U);
    }
}

void FTM3_Ch2_Ch3_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[3];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(3U);
    bool chan2IntFlag = FTM_DRV_GetChnEventStatus(base, 2U);
    bool chan3IntFlag = FTM_DRV_GetChnEventStatus(base, 3U);
    bool chan2EnabledInt = ((enabledInterrupts & (1UL << 2U)) != 0U) ? true : false;
    bool chan3EnabledInt = ((enabledInterrupts & (1UL << 3U)) != 0U) ? true : false;

    if (chan2EnabledInt && chan2IntFlag)
    {
        TIMING_Ftm_IrqHandler(3U, 2U);
    }

    if (chan3EnabledInt && chan3IntFlag)
    {
        TIMING_Ftm_IrqHandler(3U, 3U);
    }
}

void FTM3_Ch4_Ch5_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[3];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(3U);
    bool chan4IntFlag = FTM_DRV_GetChnEventStatus(base, 4U);
    bool chan5IntFlag = FTM_DRV_GetChnEventStatus(base, 5U);
    bool chan4EnabledInt = ((enabledInterrupts & (1UL << 4U)) != 0U) ? true : false;
    bool chan5EnabledInt = ((enabledInterrupts & (1UL << 5U)) != 0U) ? true : false;

    if (chan4EnabledInt && chan4IntFlag)
    {
        TIMING_Ftm_IrqHandler(3U, 4U);
    }

    if (chan5EnabledInt && chan5IntFlag)
    {
        TIMING_Ftm_IrqHandler(3U, 5U);
    }
}

void FTM3_Ch6_Ch7_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[3];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(3U);
    bool chan6IntFlag = FTM_DRV_GetChnEventStatus(base, 6U);
    bool chan7IntFlag = FTM_DRV_GetChnEventStatus(base, 7U);
    bool chan6EnabledInt = ((enabledInterrupts & (1UL << 6U)) != 0U) ? true : false;
    bool chan7EnabledInt = ((enabledInterrupts & (1UL << 7U)) != 0U) ? true : false;

    if (chan6EnabledInt && chan6IntFlag)
    {
        TIMING_Ftm_IrqHandler(3U, 6U);
    }

    if (chan7EnabledInt && chan7IntFlag)
    {
        TIMING_Ftm_IrqHandler(3U, 7U);
    }
}
#endif /* NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 3U */

#if (NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 4U)
void FTM4_Ch0_Ch1_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[4];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(4U);
    bool chan0IntFlag = FTM_DRV_GetChnEventStatus(base, 0U);
    bool chan1IntFlag = FTM_DRV_GetChnEventStatus(base, 1U);
    bool chan0EnabledInt = ((enabledInterrupts & (1UL << 0U)) != 0U) ? true : false;
    bool chan1EnabledInt = ((enabledInterrupts & (1UL << 1U)) != 0U) ? true : false;

    if (chan0EnabledInt && chan0IntFlag)
    {
        TIMING_Ftm_IrqHandler(4U, 0U);
    }

    if (chan1EnabledInt && chan1IntFlag)
    {
        TIMING_Ftm_IrqHandler(4U, 1U);
    }
}

void FTM4_Ch2_Ch3_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[4];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(4U);
    bool chan2IntFlag = FTM_DRV_GetChnEventStatus(base, 2U);
    bool chan3IntFlag = FTM_DRV_GetChnEventStatus(base, 3U);
    bool chan2EnabledInt = ((enabledInterrupts & (1UL << 2U)) != 0U) ? true : false;
    bool chan3EnabledInt = ((enabledInterrupts & (1UL << 3U)) != 0U) ? true : false;

    if (chan2EnabledInt && chan2IntFlag)
    {
        TIMING_Ftm_IrqHandler(4U, 2U);
    }

    if (chan3EnabledInt && chan3IntFlag)
    {
        TIMING_Ftm_IrqHandler(4U, 3U);
    }
}

void FTM4_Ch4_Ch5_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[4];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(4U);
    bool chan4IntFlag = FTM_DRV_GetChnEventStatus(base, 4U);
    bool chan5IntFlag = FTM_DRV_GetChnEventStatus(base, 5U);
    bool chan4EnabledInt = ((enabledInterrupts & (1UL << 4U)) != 0U) ? true : false;
    bool chan5EnabledInt = ((enabledInterrupts & (1UL << 5U)) != 0U) ? true : false;

    if (chan4EnabledInt && chan4IntFlag)
    {
        TIMING_Ftm_IrqHandler(4U, 4U);
    }

    if (chan5EnabledInt && chan5IntFlag)
    {
        TIMING_Ftm_IrqHandler(4U, 5U);
    }
}

void FTM4_Ch6_Ch7_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[4];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(4U);
    bool chan6IntFlag = FTM_DRV_GetChnEventStatus(base, 6U);
    bool chan7IntFlag = FTM_DRV_GetChnEventStatus(base, 7U);
    bool chan6EnabledInt = ((enabledInterrupts & (1UL << 6U)) != 0U) ? true : false;
    bool chan7EnabledInt = ((enabledInterrupts & (1UL << 7U)) != 0U) ? true : false;

    if (chan6EnabledInt && chan6IntFlag)
    {
        TIMING_Ftm_IrqHandler(4U, 6U);
    }

    if (chan7EnabledInt && chan7IntFlag)
    {
        TIMING_Ftm_IrqHandler(4U, 7U);
    }
}
#endif /* NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 4U */

#if (NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 5U)
void FTM5_Ch0_Ch1_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[5];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(5U);
    bool chan0IntFlag = FTM_DRV_GetChnEventStatus(base, 0U);
    bool chan1IntFlag = FTM_DRV_GetChnEventStatus(base, 1U);
    bool chan0EnabledInt = ((enabledInterrupts & (1UL << 0U)) != 0U) ? true : false;
    bool chan1EnabledInt = ((enabledInterrupts & (1UL << 1U)) != 0U) ? true : false;

    if (chan0EnabledInt && chan0IntFlag)
    {
        TIMING_Ftm_IrqHandler(5U, 0U);
    }

    if (chan1EnabledInt && chan1IntFlag)
    {
        TIMING_Ftm_IrqHandler(5U, 1U);
    }
}

void FTM5_Ch2_Ch3_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[5];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(5U);
    bool chan2IntFlag = FTM_DRV_GetChnEventStatus(base, 2U);
    bool chan3IntFlag = FTM_DRV_GetChnEventStatus(base, 3U);
    bool chan2EnabledInt = ((enabledInterrupts & (1UL << 2U)) != 0U) ? true : false;
    bool chan3EnabledInt = ((enabledInterrupts & (1UL << 3U)) != 0U) ? true : false;

    if (chan2EnabledInt && chan2IntFlag)
    {
        TIMING_Ftm_IrqHandler(5U, 2U);
    }

    if (chan3EnabledInt && chan3IntFlag)
    {
        TIMING_Ftm_IrqHandler(5U, 3U);
    }
}

void FTM5_Ch4_Ch5_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[5];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(5U);
    bool chan4IntFlag = FTM_DRV_GetChnEventStatus(base, 4U);
    bool chan5IntFlag = FTM_DRV_GetChnEventStatus(base, 5U);
    bool chan4EnabledInt = ((enabledInterrupts & (1UL << 4U)) != 0U) ? true : false;
    bool chan5EnabledInt = ((enabledInterrupts & (1UL << 5U)) != 0U) ? true : false;

    if (chan4EnabledInt && chan4IntFlag)
    {
        TIMING_Ftm_IrqHandler(5U, 4U);
    }

    if (chan5EnabledInt && chan5IntFlag)
    {
        TIMING_Ftm_IrqHandler(5U, 5U);
    }
}

void FTM5_Ch6_Ch7_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[5];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(5U);
    bool chan6IntFlag = FTM_DRV_GetChnEventStatus(base, 6U);
    bool chan7IntFlag = FTM_DRV_GetChnEventStatus(base, 7U);
    bool chan6EnabledInt = ((enabledInterrupts & (1UL << 6U)) != 0U) ? true : false;
    bool chan7EnabledInt = ((enabledInterrupts & (1UL << 7U)) != 0U) ? true : false;

    if (chan6EnabledInt && chan6IntFlag)
    {
        TIMING_Ftm_IrqHandler(5U, 6U);
    }

    if (chan7EnabledInt && chan7IntFlag)
    {
        TIMING_Ftm_IrqHandler(5U, 7U);
    }
}
#endif /* NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 5U */

#if (NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 6U)
void FTM6_Ch0_Ch1_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[6];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(6U);
    bool chan0IntFlag = FTM_DRV_GetChnEventStatus(base, 0U);
    bool chan1IntFlag = FTM_DRV_GetChnEventStatus(base, 1U);
    bool chan0EnabledInt = ((enabledInterrupts & (1UL << 0U)) != 0U) ? true : false;
    bool chan1EnabledInt = ((enabledInterrupts & (1UL << 1U)) != 0U) ? true : false;

    if (chan0EnabledInt && chan0IntFlag)
    {
        TIMING_Ftm_IrqHandler(6U, 0U);
    }

    if (chan1EnabledInt && chan1IntFlag)
    {
        TIMING_Ftm_IrqHandler(6U, 1U);
    }
}

void FTM6_Ch2_Ch3_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[6];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(6U);
    bool chan2IntFlag = FTM_DRV_GetChnEventStatus(base, 2U);
    bool chan3IntFlag = FTM_DRV_GetChnEventStatus(base, 3U);
    bool chan2EnabledInt = ((enabledInterrupts & (1UL << 2U)) != 0U) ? true : false;
    bool chan3EnabledInt = ((enabledInterrupts & (1UL << 3U)) != 0U) ? true : false;

    if (chan2EnabledInt && chan2IntFlag)
    {
        TIMING_Ftm_IrqHandler(6U, 2U);
    }

    if (chan3EnabledInt && chan3IntFlag)
    {
        TIMING_Ftm_IrqHandler(6U, 3U);
    }
}

void FTM6_Ch4_Ch5_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[6];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(6U);
    bool chan4IntFlag = FTM_DRV_GetChnEventStatus(base, 4U);
    bool chan5IntFlag = FTM_DRV_GetChnEventStatus(base, 5U);
    bool chan4EnabledInt = ((enabledInterrupts & (1UL << 4U)) != 0U) ? true : false;
    bool chan5EnabledInt = ((enabledInterrupts & (1UL << 5U)) != 0U) ? true : false;

    if (chan4EnabledInt && chan4IntFlag)
    {
        TIMING_Ftm_IrqHandler(6U, 4U);
    }

    if (chan5EnabledInt && chan5IntFlag)
    {
        TIMING_Ftm_IrqHandler(6U, 5U);
    }
}

void FTM6_Ch6_Ch7_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[6];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(6U);
    bool chan6IntFlag = FTM_DRV_GetChnEventStatus(base, 6U);
    bool chan7IntFlag = FTM_DRV_GetChnEventStatus(base, 7U);
    bool chan6EnabledInt = ((enabledInterrupts & (1UL << 6U)) != 0U) ? true : false;
    bool chan7EnabledInt = ((enabledInterrupts & (1UL << 7U)) != 0U) ? true : false;

    if (chan6EnabledInt && chan6IntFlag)
    {
        TIMING_Ftm_IrqHandler(6U, 6U);
    }

    if (chan7EnabledInt && chan7IntFlag)
    {
        TIMING_Ftm_IrqHandler(6U, 7U);
    }
}
#endif /* NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 6U */

#if (NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 7U)
void FTM7_Ch0_Ch1_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[7];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(7U);
    bool chan0IntFlag = FTM_DRV_GetChnEventStatus(base, 0U);
    bool chan1IntFlag = FTM_DRV_GetChnEventStatus(base, 1U);
    bool chan0EnabledInt = ((enabledInterrupts & (1UL << 0U)) != 0U) ? true : false;
    bool chan1EnabledInt = ((enabledInterrupts & (1UL << 1U)) != 0U) ? true : false;

    if (chan0EnabledInt && chan0IntFlag)
    {
        TIMING_Ftm_IrqHandler(7U, 0U);
    }

    if (chan1EnabledInt && chan1IntFlag)
    {
        TIMING_Ftm_IrqHandler(7U, 1U);
    }
}

void FTM7_Ch2_Ch3_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[7];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(7U);
    bool chan2IntFlag = FTM_DRV_GetChnEventStatus(base, 2U);
    bool chan3IntFlag = FTM_DRV_GetChnEventStatus(base, 3U);
    bool chan2EnabledInt = ((enabledInterrupts & (1UL << 2U)) != 0U) ? true : false;
    bool chan3EnabledInt = ((enabledInterrupts & (1UL << 3U)) != 0U) ? true : false;

    if (chan2EnabledInt && chan2IntFlag)
    {
        TIMING_Ftm_IrqHandler(7U, 2U);
    }

    if (chan3EnabledInt && chan3IntFlag)
    {
        TIMING_Ftm_IrqHandler(7U, 3U);
    }
}

void FTM7_Ch4_Ch5_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[7];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(7U);
    bool chan4IntFlag = FTM_DRV_GetChnEventStatus(base, 4U);
    bool chan5IntFlag = FTM_DRV_GetChnEventStatus(base, 5U);
    bool chan4EnabledInt = ((enabledInterrupts & (1UL << 4U)) != 0U) ? true : false;
    bool chan5EnabledInt = ((enabledInterrupts & (1UL << 5U)) != 0U) ? true : false;

    if (chan4EnabledInt && chan4IntFlag)
    {
        TIMING_Ftm_IrqHandler(7U, 4U);
    }

    if (chan5EnabledInt && chan5IntFlag)
    {
        TIMING_Ftm_IrqHandler(7U, 5U);
    }
}

void FTM7_Ch6_Ch7_IrqHandler(void)
{
    const FTM_Type * const base = ftmBase[7];
    uint32_t enabledInterrupts = FTM_DRV_GetEnabledInterrupts(7U);
    bool chan6IntFlag = FTM_DRV_GetChnEventStatus(base, 4U);
    bool chan7IntFlag = FTM_DRV_GetChnEventStatus(base, 5U);
    bool chan6EnabledInt = ((enabledInterrupts & (1UL << 6U)) != 0U) ? true : false;
    bool chan7EnabledInt = ((enabledInterrupts & (1UL << 7U)) != 0U) ? true : false;

    if (chan6EnabledInt && chan6IntFlag)
    {
        TIMING_Ftm_IrqHandler(7U, 6U);
    }

    if (chan7EnabledInt && chan7IntFlag)
    {
        TIMING_Ftm_IrqHandler(7U, 7U);
    }
}
#endif /* NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 7U */

/* Array storing references to TIMING over FTM irq handlers */
const isr_t s_timingOverFtmIsr[NUMBER_OF_TIMING_OVER_FTM_INSTANCES][FTM_CONTROLS_COUNT] =
{
#if (NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 0U)
    {FTM0_Ch0_Ch1_IrqHandler,
     FTM0_Ch0_Ch1_IrqHandler,
     FTM0_Ch2_Ch3_IrqHandler,
     FTM0_Ch2_Ch3_IrqHandler,
     FTM0_Ch4_Ch5_IrqHandler,
     FTM0_Ch4_Ch5_IrqHandler,
     FTM0_Ch6_Ch7_IrqHandler,
     FTM0_Ch6_Ch7_IrqHandler},
#endif
#if (NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 1U)
    {FTM1_Ch0_Ch1_IrqHandler,
     FTM1_Ch0_Ch1_IrqHandler,
     FTM1_Ch2_Ch3_IrqHandler,
     FTM1_Ch2_Ch3_IrqHandler,
     FTM1_Ch4_Ch5_IrqHandler,
     FTM1_Ch4_Ch5_IrqHandler,
     FTM1_Ch6_Ch7_IrqHandler,
     FTM1_Ch6_Ch7_IrqHandler},
#endif
#if (NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 2U)
    {FTM2_Ch0_Ch1_IrqHandler,
     FTM2_Ch0_Ch1_IrqHandler,
     FTM2_Ch2_Ch3_IrqHandler,
     FTM2_Ch2_Ch3_IrqHandler,
     FTM2_Ch4_Ch5_IrqHandler,
     FTM2_Ch4_Ch5_IrqHandler,
     FTM2_Ch6_Ch7_IrqHandler,
     FTM2_Ch6_Ch7_IrqHandler},
#endif
#if (NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 3U)
    {FTM3_Ch0_Ch1_IrqHandler,
     FTM3_Ch0_Ch1_IrqHandler,
     FTM3_Ch2_Ch3_IrqHandler,
     FTM3_Ch2_Ch3_IrqHandler,
     FTM3_Ch4_Ch5_IrqHandler,
     FTM3_Ch4_Ch5_IrqHandler,
     FTM3_Ch6_Ch7_IrqHandler,
     FTM3_Ch6_Ch7_IrqHandler},
#endif
#if (NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 4U)
    {FTM4_Ch0_Ch1_IrqHandler,
     FTM4_Ch0_Ch1_IrqHandler,
     FTM4_Ch2_Ch3_IrqHandler,
     FTM4_Ch2_Ch3_IrqHandler,
     FTM4_Ch4_Ch5_IrqHandler,
     FTM4_Ch4_Ch5_IrqHandler,
     FTM4_Ch6_Ch7_IrqHandler,
     FTM4_Ch6_Ch7_IrqHandler},
#endif
#if (NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 5U)
    {FTM5_Ch0_Ch1_IrqHandler,
     FTM5_Ch0_Ch1_IrqHandler,
     FTM5_Ch2_Ch3_IrqHandler,
     FTM5_Ch2_Ch3_IrqHandler,
     FTM5_Ch4_Ch5_IrqHandler,
     FTM5_Ch4_Ch5_IrqHandler,
     FTM5_Ch6_Ch7_IrqHandler,
     FTM5_Ch6_Ch7_IrqHandler},
#endif
#if (NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 6U)
    {FTM6_Ch0_Ch1_IrqHandler,
     FTM6_Ch0_Ch1_IrqHandler,
     FTM6_Ch2_Ch3_IrqHandler,
     FTM6_Ch2_Ch3_IrqHandler,
     FTM6_Ch4_Ch5_IrqHandler,
     FTM6_Ch4_Ch5_IrqHandler,
     FTM6_Ch6_Ch7_IrqHandler,
     FTM6_Ch6_Ch7_IrqHandler},
#endif
#if (NUMBER_OF_TIMING_OVER_FTM_INSTANCES > 7U)
    {FTM7_Ch0_Ch1_IrqHandler,
     FTM7_Ch0_Ch1_IrqHandler,
     FTM7_Ch2_Ch3_IrqHandler,
     FTM7_Ch2_Ch3_IrqHandler,
     FTM7_Ch4_Ch5_IrqHandler,
     FTM7_Ch4_Ch5_IrqHandler,
     FTM7_Ch6_Ch7_IrqHandler,
     FTM7_Ch6_Ch7_IrqHandler}
#endif
};

#endif /* TIMING_OVER_FTM */

/* Define TIMING PAL over STM */
#if (defined (TIMING_OVER_STM))

#if (NUMBER_OF_TIMING_OVER_STM_INSTANCES > 0U)
void STM0_Ch0_IRQHandler(void)
{
    TIMING_Stm_IrqHandler(0U, 0U);
}

void STM0_Ch1_IRQHandler(void)
{
    TIMING_Stm_IrqHandler(0U, 1U);
}

void STM0_Ch2_IRQHandler(void)
{
    TIMING_Stm_IrqHandler(0U, 2U);
}

void STM0_Ch3_IRQHandler(void)
{
    TIMING_Stm_IrqHandler(0U, 3U);
}
#endif /* NUMBER_OF_TIMING_OVER_STM_INSTANCES > 0U */

#if (NUMBER_OF_TIMING_OVER_STM_INSTANCES > 1U)
void STM1_Ch0_IRQHandler(void)
{
    TIMING_Stm_IrqHandler(1U, 0U);
}

void STM1_Ch1_IRQHandler(void)
{
    TIMING_Stm_IrqHandler(1U, 1U);
}

void STM1_Ch2_IRQHandler(void)
{
    TIMING_Stm_IrqHandler(1U, 2U);
}

void STM1_Ch3_IRQHandler(void)
{
    TIMING_Stm_IrqHandler(1U, 3U);
}
#endif /* NUMBER_OF_TIMING_OVER_STM_INSTANCES > 1U */

#if (NUMBER_OF_TIMING_OVER_STM_INSTANCES > 2U)
void STM2_Ch0_IRQHandler(void)
{
    TIMING_Stm_IrqHandler(2U, 0U);
}

void STM2_Ch1_IRQHandler(void)
{
    TIMING_Stm_IrqHandler(2U, 1U);
}

void STM2_Ch2_IRQHandler(void)
{
    TIMING_Stm_IrqHandler(2U, 2U);
}

void STM2_Ch3_IRQHandler(void)
{
    TIMING_Stm_IrqHandler(2U, 3U);
}
#endif /* NUMBER_OF_TIMING_OVER_STM_INSTANCES > 2U */

#endif /* TIMING_OVER_STM */

/*******************************************************************************
 * EOF
 ******************************************************************************/
