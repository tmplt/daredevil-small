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

 /*!
 * @file flexcan_irq.c
 *
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.7, Function not defined with external linkage.
 * The functions are not defined static because they are referenced in .s startup files.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.9, could define variable at block scope
 * The variable is defined in the common source file to make transition to other
 * platforms easier.
 */

#include "flexcan_irq.h"

#if (defined(CPU_S32K142) || defined(CPU_S32K144HFT0VLLT) || defined(CPU_S32K144LFT0MLLT) || \
     defined(CPU_S32K146) || defined(CPU_S32K148))

/*******************************************************************************
 * Code
 ******************************************************************************/
#if (CAN_INSTANCE_COUNT > 0U)
/* Implementation of CAN0 IRQ handler for OR'ed interrupts (Bus Off,
Transmit Warning, Receive Warning). */
void CAN0_ORed_IRQHandler(void)
{
    FLEXCAN_IRQHandler(0U);
}

/* Implementation of CAN0 IRQ handler for interrupts indicating that errors were
detected on the CAN bus. */
void CAN0_Error_IRQHandler(void)
{
    FLEXCAN_IRQHandler(0U);
}

#if FEATURE_CAN_HAS_WAKE_UP_IRQ

/* Implementation of CAN0 IRQ handler for interrupts indicating a wake up
event. */
void CAN0_Wake_Up_IRQHandler(void)
{
    FLEXCAN_WakeUpHandler(0U);
}

#endif /* FEATURE_CAN_HAS_WAKE_UP_IRQ */

/* Implementation of CAN0 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 0-15. */
void CAN0_ORed_0_15_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(0U);
}

/* Implementation of CAN0 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 16-31. */
void CAN0_ORed_16_31_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(0U);
}
#endif /* (CAN_INSTANCE_COUNT > 0U) */

#if (CAN_INSTANCE_COUNT > 1U)
/* Implementation of CAN1 IRQ handler for OR'ed interrupts (Bus Off,
Transmit Warning, Receive Warning). */
void CAN1_ORed_IRQHandler(void)
{
    FLEXCAN_IRQHandler(1U);
}

/* Implementation of CAN1 IRQ handler for interrupts indicating that errors were
detected on the CAN bus. */
void CAN1_Error_IRQHandler(void)
{
    FLEXCAN_IRQHandler(1U);
}

/* Implementation of CAN1 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 0-15. */
void CAN1_ORed_0_15_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(1U);
}

/* Implementation of CAN1 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 16-31. */
void CAN1_ORed_16_31_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(1U);
}
#endif /* (CAN_INSTANCE_COUNT > 1U) */

#if (CAN_INSTANCE_COUNT > 2U)
/* Implementation of CAN2 IRQ handler for OR'ed interrupts (Bus Off,
Transmit Warning, Receive Warning). */
void CAN2_ORed_IRQHandler(void)
{
    FLEXCAN_IRQHandler(2U);
}

/* Implementation of CAN2 IRQ handler for interrupts indicating that errors were
detected on the CAN bus. */
void CAN2_Error_IRQHandler(void)
{
    FLEXCAN_IRQHandler(2U);
}

/* Implementation of CAN2 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 0-15. */
void CAN2_ORed_0_15_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(2U);
}

/* Implementation of CAN2 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 16-31. */
void CAN2_ORed_16_31_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(2U);
}
#endif /* (CAN_INSTANCE_COUNT > 2U) */

#elif defined(CPU_S32V234)

/*******************************************************************************
 * Code
 ******************************************************************************/
#if (CAN_INSTANCE_COUNT > 0U)
/* Implementation of CAN0 handler named in startup code. */
void CAN0_IRQHandler(void)
{
    FLEXCAN_IRQHandler(0U);
}

/* Implementation of CAN0 handler named in startup code. */
void CAN0_Buff_IRQHandler(void)
{
    FLEXCAN_IRQHandler(0U);
}
#endif /* (CAN_INSTANCE_COUNT > 0U) */

#if (CAN_INSTANCE_COUNT > 1U)
/* Implementation of CAN1 handler named in startup code. */
void CAN1_IRQHandler(void)
{
    FLEXCAN_IRQHandler(1U);
}

/* Implementation of CAN1 handler named in startup code. */
void CAN1_Buff_IRQHandler(void)
{
    FLEXCAN_IRQHandler(1U);
}
#endif /* (CAN_INSTANCE_COUNT > 1U) */

#elif (defined(CPU_MPC5748G) || defined(CPU_MPC5746C))

/*******************************************************************************
 * Code
 ******************************************************************************/
#if (CAN_INSTANCE_COUNT > 0U)
/* Implementation of CAN0 IRQ handler for OR'ed interrupts (Bus Off,
Transmit Warning, Receive Warning). */
void CAN0_ORed_IRQHandler(void)
{
    FLEXCAN_IRQHandler(0U);
}

/* Implementation of CAN0 IRQ handler for interrupts indicating that errors were
detected on the CAN bus. */
void CAN0_Error_IRQHandler(void)
{
    FLEXCAN_IRQHandler(0U);
}

#if FEATURE_CAN_HAS_WAKE_UP_IRQ

/* Implementation of CAN0 IRQ handler for interrupts indicating a wake up
event. */
void CAN0_Wake_Up_IRQHandler(void)
{
    FLEXCAN_WakeUpHandler(0U);
}

#endif /* FEATURE_CAN_HAS_WAKE_UP_IRQ */

/* Implementation of CAN0 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 0-3. */
void CAN0_ORed_00_03_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(0U);
}

/* Implementation of CAN0 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 4-7. */
void CAN0_ORed_04_07_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(0U);
}

/* Implementation of CAN0 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 08-11. */
void CAN0_ORed_08_11_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(0U);
}

/* Implementation of CAN0 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 12-15. */
void CAN0_ORed_12_15_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(0U);
}

/* Implementation of CAN0 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 16-31. */
void CAN0_ORed_16_31_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(0U);
}

/* Implementation of CAN0 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 32-63. */
void CAN0_ORed_32_63_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(0U);
}

/* Implementation of CAN0 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 64-95. */
void CAN0_ORed_64_95_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(0U);
}
#endif /* (CAN_INSTANCE_COUNT > 0U) */

#if (CAN_INSTANCE_COUNT > 1U)
/* Implementation of CAN1 IRQ handler for OR'ed interrupts (Bus Off,
Transmit Warning, Receive Warning). */
void CAN1_ORed_IRQHandler(void)
{
    FLEXCAN_IRQHandler(1U);
}

/* Implementation of CAN1 IRQ handler for interrupts indicating that errors were
detected on the CAN bus. */
void CAN1_Error_IRQHandler(void)
{
    FLEXCAN_IRQHandler(1U);
}

/* Implementation of CAN1 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 0-3. */
void CAN1_ORed_00_03_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(1U);
}

/* Implementation of CAN1 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 4-7. */
void CAN1_ORed_04_07_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(1U);
}

/* Implementation of CAN1 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 08-11. */
void CAN1_ORed_08_11_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(1U);
}

/* Implementation of CAN1 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 12-15. */
void CAN1_ORed_12_15_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(1U);
}

/* Implementation of CAN1 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 16-31. */
void CAN1_ORed_16_31_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(1U);
}

/* Implementation of CAN1 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 32-63. */
void CAN1_ORed_32_63_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(1U);
}

/* Implementation of CAN1 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 64-95. */
void CAN1_ORed_64_95_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(1U);
}
#endif /* (CAN_INSTANCE_COUNT > 1U) */

#if (CAN_INSTANCE_COUNT > 2U)
/* Implementation of CAN2 IRQ handler for OR'ed interrupts (Bus Off,
Transmit Warning, Receive Warning). */
void CAN2_ORed_IRQHandler(void)
{
    FLEXCAN_IRQHandler(2U);
}

/* Implementation of CAN2 IRQ handler for interrupts indicating that errors were
detected on the CAN bus. */
void CAN2_Error_IRQHandler(void)
{
    FLEXCAN_IRQHandler(2U);
}

/* Implementation of CAN2 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 0-3. */
void CAN2_ORed_00_03_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(2U);
}

/* Implementation of CAN2 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 4-7. */
void CAN2_ORed_04_07_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(2U);
}

/* Implementation of CAN2 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 08-11. */
void CAN2_ORed_08_11_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(2U);
}

/* Implementation of CAN2 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 12-15. */
void CAN2_ORed_12_15_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(2U);
}

/* Implementation of CAN2 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 16-31. */
void CAN2_ORed_16_31_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(2U);
}

/* Implementation of CAN2 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 32-63. */
void CAN2_ORed_32_63_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(2U);
}

/* Implementation of CAN2 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 64-95. */
void CAN2_ORed_64_95_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(2U);
}
#endif /* (CAN_INSTANCE_COUNT > 2U) */

#if (CAN_INSTANCE_COUNT > 3U)
/* Implementation of CAN3 IRQ handler for OR'ed interrupts (Bus Off,
Transmit Warning, Receive Warning). */
void CAN3_ORed_IRQHandler(void)
{
    FLEXCAN_IRQHandler(3U);
}

/* Implementation of CAN3 IRQ handler for interrupts indicating that errors were
detected on the CAN bus. */
void CAN3_Error_IRQHandler(void)
{
    FLEXCAN_IRQHandler(3U);
}

/* Implementation of CAN3 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 0-3. */
void CAN3_ORed_00_03_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(3U);
}

/* Implementation of CAN3 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 4-7. */
void CAN3_ORed_04_07_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(3U);
}

/* Implementation of CAN3 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 08-11. */
void CAN3_ORed_08_11_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(3U);
}

/* Implementation of CAN3 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 12-15. */
void CAN3_ORed_12_15_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(3U);
}

/* Implementation of CAN3 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 16-31. */
void CAN3_ORed_16_31_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(3U);
}

/* Implementation of CAN3 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 32-63. */
void CAN3_ORed_32_63_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(3U);
}

/* Implementation of CAN3 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 64-95. */
void CAN3_ORed_64_95_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(3U);
}
#endif /* (CAN_INSTANCE_COUNT > 3) */

#if (CAN_INSTANCE_COUNT > 4U)
/* Implementation of CAN4 IRQ handler for OR'ed interrupts (Bus Off,
Transmit Warning, Receive Warning). */
void CAN4_ORed_IRQHandler(void)
{
    FLEXCAN_IRQHandler(4U);
}

/* Implementation of CAN4 IRQ handler for interrupts indicating that errors were
detected on the CAN bus. */
void CAN4_Error_IRQHandler(void)
{
    FLEXCAN_IRQHandler(4U);
}

/* Implementation of CAN4 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 0-3. */
void CAN4_ORed_00_03_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(4U);
}

/* Implementation of CAN4 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 4-7. */
void CAN4_ORed_04_07_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(4U);
}

/* Implementation of CAN4 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 08-11. */
void CAN4_ORed_08_11_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(4U);
}

/* Implementation of CAN4 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 12-15. */
void CAN4_ORed_12_15_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(4U);
}

/* Implementation of CAN4 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 16-31. */
void CAN4_ORed_16_31_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(4U);
}

/* Implementation of CAN4 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 32-63. */
void CAN4_ORed_32_63_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(4U);
}

/* Implementation of CAN4 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 64-95. */
void CAN4_ORed_64_95_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(4U);
}
#endif /* (CAN_INSTANCE_COUNT > 4) */

#if (CAN_INSTANCE_COUNT > 5U)
/* Implementation of CAN5 IRQ handler for OR'ed interrupts (Bus Off,
Transmit Warning, Receive Warning). */
void CAN5_ORed_IRQHandler(void)
{
    FLEXCAN_IRQHandler(5U);
}

/* Implementation of CAN5 IRQ handler for interrupts indicating that errors were
detected on the CAN bus. */
void CAN5_Error_IRQHandler(void)
{
    FLEXCAN_IRQHandler(5U);
}

/* Implementation of CAN5 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 0-3. */
void CAN5_ORed_00_03_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(5U);
}

/* Implementation of CAN5 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 4-7. */
void CAN5_ORed_04_07_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(5U);
}

/* Implementation of CAN5 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 08-11. */
void CAN5_ORed_08_11_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(5U);
}

/* Implementation of CAN5 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 12-15. */
void CAN5_ORed_12_15_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(5U);
}

/* Implementation of CAN5 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 16-31. */
void CAN5_ORed_16_31_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(5U);
}

/* Implementation of CAN5 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 32-63. */
void CAN5_ORed_32_63_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(5U);
}

/* Implementation of CAN5 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 64-95. */
void CAN5_ORed_64_95_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(5U);
}
#endif /* (CAN_INSTANCE_COUNT > 5) */

#if (CAN_INSTANCE_COUNT > 6U)
/* Implementation of CAN6 IRQ handler for OR'ed interrupts (Bus Off,
Transmit Warning, Receive Warning). */
void CAN6_ORed_IRQHandler(void)
{
    FLEXCAN_IRQHandler(6U);
}

/* Implementation of CAN6 IRQ handler for interrupts indicating that errors were
detected on the CAN bus. */
void CAN6_Error_IRQHandler(void)
{
    FLEXCAN_IRQHandler(6U);
}

/* Implementation of CAN6 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 0-3. */
void CAN6_ORed_00_03_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(6U);
}

/* Implementation of CAN6 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 4-7. */
void CAN6_ORed_04_07_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(6U);
}

/* Implementation of CAN6 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 08-11. */
void CAN6_ORed_08_11_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(6U);
}

/* Implementation of CAN6 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 12-15. */
void CAN6_ORed_12_15_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(6U);
}

/* Implementation of CAN6 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 16-31. */
void CAN6_ORed_16_31_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(6U);
}

/* Implementation of CAN6 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 32-63. */
void CAN6_ORed_32_63_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(6U);
}

/* Implementation of CAN6 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 64-95. */
void CAN6_ORed_64_95_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(6U);
}
#endif /* (CAN_INSTANCE_COUNT > 6) */

#if (CAN_INSTANCE_COUNT > 7U)
/* Implementation of CAN7 IRQ handler for OR'ed interrupts (Bus Off,
Transmit Warning, Receive Warning). */
void CAN7_ORed_IRQHandler(void)
{
    FLEXCAN_IRQHandler(7U);
}

/* Implementation of CAN7 IRQ handler for interrupts indicating that errors were
detected on the CAN bus. */
void CAN7_Error_IRQHandler(void)
{
    FLEXCAN_IRQHandler(7U);
}

/* Implementation of CAN7 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 0-3. */
void CAN7_ORed_00_03_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(7U);
}

/* Implementation of CAN7 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 4-7. */
void CAN7_ORed_04_07_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(7U);
}

/* Implementation of CAN7 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 08-11. */
void CAN7_ORed_08_11_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(7U);
}

/* Implementation of CAN7 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 12-15. */
void CAN7_ORed_12_15_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(7U);
}

/* Implementation of CAN7 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 16-31. */
void CAN7_ORed_16_31_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(7U);
}

/* Implementation of CAN7 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 32-63. */
void CAN7_ORed_32_63_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(7U);
}

/* Implementation of CAN7 IRQ handler for interrupts indicating a successful
transmission or reception for Message Buffers 64-95. */
void CAN7_ORed_64_95_MB_IRQHandler(void)
{
    FLEXCAN_IRQHandler(7U);
}
#endif /* (CAN_INSTANCE_COUNT > 7) */

#else
    #error "No valid CPU defined!"
#endif

/*******************************************************************************
 * EOF
 ******************************************************************************/
