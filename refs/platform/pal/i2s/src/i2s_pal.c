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

#include "i2s_pal.h"
#include "device_registers.h"

/* Include PD files */
#if (defined(I2S_OVER_SAI))
    #include "sai_driver.h"
#endif
#if (defined(I2S_OVER_FLEXIO))
    #include "flexio.h"
    #include "flexio_i2s_driver.h"
#endif

/* Define state structures for SAI */
#if (defined(I2S_OVER_SAI))
    #define LAST_IS_TX 1U
    #define LAST_IS_RX 2U
    #define LAST_IS_NONE 0U
    /* sai state structures */
    static sai_state_t saiTxState[NO_OF_SAI_INSTS_FOR_I2S];
    static sai_state_t saiRxState[NO_OF_SAI_INSTS_FOR_I2S];
    /* sai state-instance matching */
    static i2s_instance_t saiStateInstanceMapping[NO_OF_SAI_INSTS_FOR_I2S];
    /* sai available resources table */
    static bool saiStateIsAllocated[NO_OF_SAI_INSTS_FOR_I2S];
    /* record last transfer is tx or rx to call SelectMaster for sai */
    static uint8_t lastXfer = LAST_IS_NONE;
#endif

/* Define state structures for i2s over FLEXIO */
#if (defined(I2S_OVER_FLEXIO))
    #define NO_OF_FLEXIO_INSTS_FOR_I2S (NO_OF_FLEXIO_MASTER_INSTS_FOR_I2S+NO_OF_FLEXIO_SLAVE_INSTS_FOR_I2S)
    /* FLEXIO state structures */
#if (NO_OF_FLEXIO_MASTER_INSTS_FOR_I2S > 0)
    static flexio_i2s_master_state_t flexioMasterState[NO_OF_FLEXIO_MASTER_INSTS_FOR_I2S];
    static bool flexioMasterStateIsAllocated[NO_OF_FLEXIO_MASTER_INSTS_FOR_I2S];
#endif
#if (NO_OF_FLEXIO_SLAVE_INSTS_FOR_I2S > 0)
    static flexio_i2s_slave_state_t flexioSlaveState[NO_OF_FLEXIO_SLAVE_INSTS_FOR_I2S];
    static bool flexioSlaveStateIsAllocated[NO_OF_FLEXIO_SLAVE_INSTS_FOR_I2S];
#endif
    /* flexio device state */
    flexio_device_state_t flexioDeviceState;
    /* state-instance matching table */
    static i2s_instance_t flexioStateInstanceMapping[NO_OF_FLEXIO_INSTS_FOR_I2S];
    /* state is master or slave */
    static bool flexioIsMaster[NO_OF_FLEXIO_INSTS_FOR_I2S];
    /* pointer to master/slave state */
    static void* flexioState[NO_OF_FLEXIO_INSTS_FOR_I2S];
    /* available resources table */
    static bool flexioStateIsAllocated[NO_OF_FLEXIO_INSTS_FOR_I2S];
    static uint8_t flexioWordSize[NO_OF_FLEXIO_INSTS_FOR_I2S];
#endif

#ifdef I2S_OVER_FLEXIO
/*FUNCTION**********************************************************************
 *
 * Function Name : flexioAllocateState
 * Description   : Allocates one of the available state structures.
 *
 *END**************************************************************************/
static uint8_t flexioAllocateState(i2s_instance_t instance,
                                   bool isMaster)
{
    uint8_t i;
    uint8_t j;
    /* Allocate one of the i2s state structures for this instance */
    for (i = 0; i < NO_OF_FLEXIO_INSTS_FOR_I2S; i++)
    {
        if (!flexioStateIsAllocated[i])
        {
            flexioStateInstanceMapping[i] = instance;
            flexioStateIsAllocated[i] = true;
            flexioIsMaster[i] = isMaster;
            if (isMaster)
            {
#if (NO_OF_FLEXIO_MASTER_INSTS_FOR_I2S > 0)
                for (j = 0; j < NO_OF_FLEXIO_MASTER_INSTS_FOR_I2S; j++)
                {
                    if (!flexioMasterStateIsAllocated[j])
                    {
                        flexioMasterStateIsAllocated[j] = true;
                        flexioState[i] = &flexioMasterState[j];
                        break;
                    }
                }
#endif
            }
            else
            {
#if (NO_OF_FLEXIO_SLAVE_INSTS_FOR_I2S > 0)
                for (j = 0; j < NO_OF_FLEXIO_SLAVE_INSTS_FOR_I2S; j++)
                {
                    if (!flexioSlaveStateIsAllocated[j])
                    {
                        flexioSlaveStateIsAllocated[j] = true;
                        flexioState[i] = &flexioSlaveState[j];
                        break;
                    }
                }
#endif
            }
            break;
        }
    }
    return i;
}
#endif /* I2S_OVER_FLEXIO */

#ifdef I2S_OVER_SAI
/*FUNCTION**********************************************************************
 *
 * Function Name : saiAllocateState
 * Description   : Allocates one of the available state structures.
 *
 *END**************************************************************************/
static uint8_t saiAllocateState(i2s_instance_t instance)
{
    uint8_t i;
    for (i = 0; i < NO_OF_SAI_INSTS_FOR_I2S; i++)
    {
        if (!saiStateIsAllocated[i])
        {
            saiStateIsAllocated[i] = true;
            saiStateInstanceMapping[i] = instance;
            break;
        }
    }
    return i;
}
#endif /* I2S_OVER_SAI */

/*FUNCTION**********************************************************************
 *
 * Function Name : freeState
 * Description   : free allocated state
 *
 *END**************************************************************************/
static void freeState(i2s_instance_t instance)
{
    uint8_t i;
#ifdef I2S_OVER_SAI
    if (instance < SAI_HIGH_INDEX)
    {
        for (i = 0; i < NO_OF_SAI_INSTS_FOR_I2S; i++)
        {
            if ((saiStateInstanceMapping[i] == instance) && saiStateIsAllocated[i])
            {
                saiStateIsAllocated[i] = false;
                break;
            }
        }
    }
#endif /* I2S_OVER_SAI */
#ifdef I2S_OVER_FLEXIO
    uint8_t j;
    if (
#ifdef I2S_OVER_SAI
        instance >= FLEXIO_I2S_LOW_INDEX &&
#endif /* I2S_OVER_SAI */
        instance <= FLEXIO_I2S_HIGH_INDEX)
    {
        for (i = 0; i < NO_OF_FLEXIO_INSTS_FOR_I2S; i++)
        {
            if ((flexioStateInstanceMapping[i] == instance) && flexioStateIsAllocated[i])
            {
                flexioStateIsAllocated[i] = false;
                if (flexioIsMaster[i])
                {
#if (NO_OF_FLEXIO_MASTER_INSTS_FOR_I2S > 0)
                    for (j = 0; j < NO_OF_FLEXIO_MASTER_INSTS_FOR_I2S; j++)
                    {
                        if ((&flexioMasterState[j] == flexioState[i]) && flexioMasterStateIsAllocated[j])
                        {
                            flexioMasterStateIsAllocated[j] = false;
                        }
                    }
#endif
                }
                else
                {
#if (NO_OF_FLEXIO_SLAVE_INSTS_FOR_I2S > 0)
                    for (j = 0; j < NO_OF_FLEXIO_SLAVE_INSTS_FOR_I2S; j++)
                    {
                        if ((&flexioSlaveState[j] == flexioState[i]) && flexioSlaveStateIsAllocated[j])
                        {
                            flexioSlaveStateIsAllocated[j] = false;
                        }
                    }
#endif
                }
                break;
            }
        }
    }
#endif /* I2S_OVER_FLEXIO */
}

#ifdef I2S_OVER_FLEXIO
/*FUNCTION**********************************************************************
 *
 * Function Name : findFlexioState
 * Description   : find state index from instance
 *
 *END**************************************************************************/
static uint8_t findFlexioState(i2s_instance_t instance)
{
    uint8_t i;
    for (i = 0; i < NO_OF_FLEXIO_INSTS_FOR_I2S; i++)
    {
        if ((flexioStateInstanceMapping[i] == instance) && flexioStateIsAllocated[i])
        {
            break;
        }
    }
    /* cannot find state */
    DEV_ASSERT(i < NO_OF_FLEXIO_INSTS_FOR_I2S);
    return i;
}
#endif /* I2S_OVER_FLEXIO */

/*FUNCTION**********************************************************************
 *
 * Function Name : I2S_Init
 * Description   : initialize driver
 *
 * Implements    : I2S_Init_Activity
 *END**************************************************************************/
status_t I2S_Init(i2s_instance_t instance, const i2s_user_config_t * config)
{
    uint8_t stateIndex;
    status_t ret = STATUS_UNSUPPORTED;
#ifdef I2S_OVER_SAI
    sai_user_config_t saiUserConfig;
    if (instance < SAI_HIGH_INDEX)
    {
        stateIndex = saiAllocateState(instance);
        DEV_ASSERT(stateIndex < NO_OF_SAI_INSTS_FOR_I2S);
        saiUserConfig.BitClkNegPolar = true;
        saiUserConfig.ChannelCount = 1U;
        if (config->wordWidth <= 8U)
        {
            saiUserConfig.ElementSize = 1U;
        }
        else if (config->wordWidth <= 16U)
        {
            saiUserConfig.ElementSize = 2U;
        }
        else
        {
            saiUserConfig.ElementSize = 4U;
        }
        saiUserConfig.FirstBitIndex = (uint8_t)(config->wordWidth - 1U);
        saiUserConfig.FrameSize = 2U;
#ifdef FEATURE_SAI_HAS_CHMOD
        saiUserConfig.MaskMode = SAI_MASK_TRISTATE;
#endif /* FEATURE_SAI_HAS_CHMOD */

#ifdef FEATURE_SAI_MSEL_FCD
        saiUserConfig.MasterClkSrc = SAI_FCD_CLK;
#endif /* FEATURE_SAI_MSEL_FCD */
#ifdef FEATURE_SAI_MSEL_BUS_CLK
        saiUserConfig.MasterClkSrc = SAI_BUS_CLK;
#endif /* FEATURE_SAI_MSEL_BUS_CLK */
        saiUserConfig.MsbFirst = true;
        saiUserConfig.MuxMode = SAI_MUX_DISABLED;
        saiUserConfig.SyncEarly = true;
        saiUserConfig.SyncNegPolar = true;
        saiUserConfig.SyncWidth = config->wordWidth;
        saiUserConfig.Word0Width = config->wordWidth;
        saiUserConfig.WordNWidth = config->wordWidth;
        saiUserConfig.FrameStartReport = false;
        saiUserConfig.SyncErrorReport = false;
        saiUserConfig.RunErrorReport = false;
        saiUserConfig.TransferType = (sai_transfer_type_t) config->transferType;
        saiUserConfig.callback = (sai_transfer_callback_t) config->callback;
        saiUserConfig.callbackParam = config->callbackParam;
        saiUserConfig.BitClkFreq = config->baudRate;
        saiUserConfig.SyncMode = SAI_ASYNC;
        /* saiUserConfig.BitClkDiv */
        saiUserConfig.DmaChannel[0] = config->txDMAChannel;
        saiUserConfig.ChannelEnable = 1U;
        if (config->mode == I2S_MASTER)
        {
#ifdef FEATURE_SAI_MSEL_FCD
            SAI_DRV_FCDInit(instance, SAI_FCD_PLL, config->baudRate*2UL, false);
#endif /* FEATURE_SAI_MSEL_FCD */
            saiUserConfig.BitClkInternal = true;
            saiUserConfig.SyncInternal = true;
        }
        else
        {
            saiUserConfig.BitClkInternal = false;
            saiUserConfig.SyncInternal = false;
        }
        SAI_DRV_TxInit(instance, &saiUserConfig, &saiTxState[stateIndex]);
        saiUserConfig.DmaChannel[0] = config->rxDMAChannel;
        if (instance == I2S_OVER_SAI0_INSTANCE)
        {
            saiUserConfig.ChannelEnable = 2U;
        }
        SAI_DRV_RxInit(instance, &saiUserConfig, &saiRxState[stateIndex]);
        ret = STATUS_SUCCESS;
    }
#endif /* I2S_OVER_SAI */
#ifdef I2S_OVER_FLEXIO
    flexio_i2s_master_user_config_t flexioMasterConfig;
    flexio_i2s_slave_user_config_t flexioSlaveConfig;
    if (
#ifdef I2S_OVER_SAI
        instance >= FLEXIO_I2S_LOW_INDEX &&
#endif /* I2S_OVER_SAI */
        instance <= FLEXIO_I2S_HIGH_INDEX)
    {
        stateIndex = flexioAllocateState(instance, (config->mode == I2S_MASTER ? true : false));
        DEV_ASSERT(stateIndex < NO_OF_FLEXIO_INSTS_FOR_I2S);
        FLEXIO_DRV_InitDevice(0, &flexioDeviceState);
        if (config->wordWidth <= 8U)
        {
            flexioWordSize[stateIndex] = 1U;
        }
        else if (config->wordWidth <= 16U)
        {
            flexioWordSize[stateIndex] = 2U;
        }
        else
        {
            flexioWordSize[stateIndex] = 4U;
        }
        if (config->mode == I2S_MASTER)
        {
            flexioMasterConfig.baudRate = config->baudRate;
            flexioMasterConfig.bitsWidth = config->wordWidth;
            flexioMasterConfig.callback = config->callback;
            flexioMasterConfig.callbackParam = config->callbackParam;
            if (config->transferType == I2S_USING_DMA)
            {
                flexioMasterConfig.driverType = FLEXIO_DRIVER_TYPE_DMA;
            }
            if (config->transferType == I2S_USING_INTERRUPT)
            {
                flexioMasterConfig.driverType = FLEXIO_DRIVER_TYPE_INTERRUPTS;
            }
            flexioMasterConfig.rxDMAChannel = config->rxDMAChannel;
            flexioMasterConfig.txDMAChannel = config->txDMAChannel;
            flexioMasterConfig.rxPin = ((extension_flexio_for_i2s_t*)config->extension)->rxPin;
            flexioMasterConfig.sckPin = ((extension_flexio_for_i2s_t*)config->extension)->sckPin;
            flexioMasterConfig.txPin = ((extension_flexio_for_i2s_t*)config->extension)->txPin;
            flexioMasterConfig.wsPin = ((extension_flexio_for_i2s_t*)config->extension)->wsPin;
            ret = FLEXIO_I2S_DRV_MasterInit(0, &flexioMasterConfig, flexioState[stateIndex]);
        }
        else
        {
            flexioSlaveConfig.bitsWidth = config->wordWidth;
            flexioSlaveConfig.callback = config->callback;
            flexioSlaveConfig.callbackParam = config->callbackParam;
            if (config->transferType == I2S_USING_DMA)
            {
                flexioSlaveConfig.driverType = FLEXIO_DRIVER_TYPE_DMA;
            }
            if (config->transferType == I2S_USING_INTERRUPT)
            {
                flexioSlaveConfig.driverType = FLEXIO_DRIVER_TYPE_INTERRUPTS;
            }
            flexioSlaveConfig.rxDMAChannel = config->rxDMAChannel;
            flexioSlaveConfig.txDMAChannel = config->txDMAChannel;
            flexioSlaveConfig.rxPin = ((extension_flexio_for_i2s_t*)config->extension)->rxPin;
            flexioSlaveConfig.sckPin = ((extension_flexio_for_i2s_t*)config->extension)->sckPin;
            flexioSlaveConfig.txPin = ((extension_flexio_for_i2s_t*)config->extension)->txPin;
            flexioSlaveConfig.wsPin = ((extension_flexio_for_i2s_t*)config->extension)->wsPin;
            ret = FLEXIO_I2S_DRV_SlaveInit(0, &flexioSlaveConfig, flexioState[stateIndex]);
        }
    }
#endif /* I2S_OVER_FLEXIO */
    return ret;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2S_Deinit
 * Description   : deinit driver
 *
 * Implements    : I2S_Deinit_Activity
 *END**************************************************************************/
status_t I2S_Deinit(i2s_instance_t instance)
{
    status_t ret = STATUS_UNSUPPORTED;
#ifdef I2S_OVER_FLEXIO
    uint8_t stateIndex;
    if (
#ifdef I2S_OVER_SAI
        instance >= FLEXIO_I2S_LOW_INDEX &&
#endif /* I2S_OVER_SAI */
        instance <= FLEXIO_I2S_HIGH_INDEX)
    {
        stateIndex = findFlexioState(instance);
        if (flexioIsMaster[stateIndex])
        {
            ret = FLEXIO_I2S_DRV_MasterDeinit(flexioState[stateIndex]);
        }
        else
        {
            ret = FLEXIO_I2S_DRV_SlaveDeinit(flexioState[stateIndex]);
        }
        if (ret == STATUS_SUCCESS)
        {
            freeState(instance);
        }
    }
#endif /* I2S_OVER_FLEXIO */
#ifdef I2S_OVER_SAI
    if (instance < SAI_HIGH_INDEX)
    {
        SAI_DRV_TxDeinit(instance);
        SAI_DRV_RxDeinit(instance);
        freeState(instance);
        ret = STATUS_SUCCESS;
    }
#endif /* I2S_OVER_SAI */
    return ret;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2S_GetBaudRate
 * Description   : return true baud rate
 *
 * Implements    : I2S_GetBaudRate_Activity
 *END**************************************************************************/
status_t I2S_GetBaudRate(i2s_instance_t instance,
                         uint32_t * configuredBaudRate)
{
    status_t ret = STATUS_UNSUPPORTED;
#ifdef I2S_OVER_FLEXIO
    uint8_t stateIndex;
    if (
#ifdef I2S_OVER_SAI
        instance >= FLEXIO_I2S_LOW_INDEX &&
#endif /* I2S_OVER_SAI */
        instance <= FLEXIO_I2S_HIGH_INDEX)
    {
        stateIndex = findFlexioState(instance);
        if (flexioIsMaster[stateIndex])
        {
            ret = FLEXIO_I2S_DRV_MasterGetBaudRate(flexioState[stateIndex], configuredBaudRate);
        }
    }
#endif /* I2S_OVER_FLEXIO */
#ifdef I2S_OVER_SAI
    if (instance < SAI_HIGH_INDEX)
    {
        SAI_DRV_TxGetBitClockFreq(instance);
        ret = STATUS_SUCCESS;
    }
#endif /* I2S_OVER_SAI */
    return ret;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2S_SendDataBlocking
 * Description   : Send blocking
 *
 * Implements    : I2S_SendDataBlocking_Activity
 *END**************************************************************************/
status_t I2S_SendDataBlocking(
        i2s_instance_t instance,
        const uint8_t * txBuff,
        uint32_t txSize,
        uint32_t timeout)
{
    status_t ret = STATUS_UNSUPPORTED;
#ifdef I2S_OVER_FLEXIO
    uint8_t stateIndex;
    if (
#ifdef I2S_OVER_SAI
        instance >= FLEXIO_I2S_LOW_INDEX &&
#endif /* I2S_OVER_SAI */
        instance <= FLEXIO_I2S_HIGH_INDEX)
    {
        stateIndex = findFlexioState(instance);
        if (flexioIsMaster[stateIndex])
        {
            ret =  FLEXIO_I2S_DRV_MasterSendDataBlocking(flexioState[stateIndex], txBuff, flexioWordSize[stateIndex] * txSize, timeout);
        }
        else
        {
            ret =  FLEXIO_I2S_DRV_SlaveSendDataBlocking(flexioState[stateIndex], txBuff, flexioWordSize[stateIndex] * txSize, timeout);
        }
    }
#endif /* I2S_OVER_FLEXIO */
#ifdef I2S_OVER_SAI
    const uint8_t* addr;
    if (instance < SAI_HIGH_INDEX)
    {
        if ((lastXfer == LAST_IS_NONE) || (lastXfer == LAST_IS_RX))
        {
            lastXfer = LAST_IS_TX;
            SAI_DRV_SetMaster(instance, true);
        }
        addr = txBuff;
        ret = SAI_DRV_SendBlocking(instance, &addr, txSize, timeout);
    }
#endif /* I2S_OVER_SAI */
    return ret;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2S_SetTxBuffer
 * Description   : Keep sending data
 *
 * Implements    : I2S_SetTxBuffer_Activity
 *END**************************************************************************/
status_t I2S_SetRxBuffer(i2s_instance_t instance,
                      uint8_t * rxBuff,
                      uint32_t rxSize)
{
    status_t ret = STATUS_UNSUPPORTED;
#ifdef I2S_OVER_FLEXIO
    uint8_t stateIndex;
    if (
#ifdef I2S_OVER_SAI
        instance >= FLEXIO_I2S_LOW_INDEX &&
#endif /* I2S_OVER_SAI */
        instance <= FLEXIO_I2S_HIGH_INDEX)
    {
        stateIndex = findFlexioState(instance);
        if (flexioIsMaster[stateIndex])
        {
            ret =  FLEXIO_I2S_DRV_MasterSetRxBuffer(flexioState[stateIndex], rxBuff, flexioWordSize[stateIndex] * rxSize);
        }
        else
        {
            ret =  FLEXIO_I2S_DRV_SlaveSetRxBuffer(flexioState[stateIndex], rxBuff, flexioWordSize[stateIndex] * rxSize);
        }
    }
#endif /* I2S_OVER_FLEXIO */
#ifdef I2S_OVER_SAI
    uint8_t* addr;
    if (instance < SAI_HIGH_INDEX)
    {
        addr = rxBuff;
        SAI_DRV_Receive(instance, &addr, rxSize);
        ret = STATUS_SUCCESS;
    }
#endif /* I2S_OVER_SAI */
    return ret;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2S_SetTxBuffer
 * Description   : Keep sending data
 *
 * Implements    : I2S_SetTxBuffer_Activity
 *END**************************************************************************/
status_t I2S_SetTxBuffer(i2s_instance_t instance,
                      const uint8_t * txBuff,
                      uint32_t txSize)
{
    status_t ret = STATUS_UNSUPPORTED;
#ifdef I2S_OVER_FLEXIO
    uint8_t stateIndex;
    if (
#ifdef I2S_OVER_SAI
        instance >= FLEXIO_I2S_LOW_INDEX &&
#endif /* I2S_OVER_SAI */
        instance <= FLEXIO_I2S_HIGH_INDEX)
    {
        stateIndex = findFlexioState(instance);
        if (flexioIsMaster[stateIndex])
        {
            ret =  FLEXIO_I2S_DRV_MasterSetTxBuffer(flexioState[stateIndex], txBuff, flexioWordSize[stateIndex] * txSize);
        }
        else
        {
            ret =  FLEXIO_I2S_DRV_SlaveSetTxBuffer(flexioState[stateIndex], txBuff, flexioWordSize[stateIndex] * txSize);
        }
    }
#endif /* I2S_OVER_FLEXIO */
#ifdef I2S_OVER_SAI
    const uint8_t* addr;
    if (instance < SAI_HIGH_INDEX)
    {
        addr = txBuff;
        SAI_DRV_Send(instance, &addr, txSize);
        ret = STATUS_SUCCESS;
    }
#endif /* I2S_OVER_SAI */
    return ret;
}
/*FUNCTION**********************************************************************
 *
 * Function Name : I2S_SendData
 * Description   : Send non-blocking
 *
 * Implements    : I2S_SendData_Activity
 *END**************************************************************************/
status_t I2S_SendData(i2s_instance_t instance,
                      const uint8_t * txBuff,
                      uint32_t txSize)
{
    status_t ret = STATUS_UNSUPPORTED;
#ifdef I2S_OVER_FLEXIO
    uint8_t stateIndex;
    if (
#ifdef I2S_OVER_SAI
        instance >= FLEXIO_I2S_LOW_INDEX &&
#endif /* I2S_OVER_SAI */
        instance <= FLEXIO_I2S_HIGH_INDEX)
    {
        stateIndex = findFlexioState(instance);
        if (flexioIsMaster[stateIndex])
        {
            ret =  FLEXIO_I2S_DRV_MasterSendData(flexioState[stateIndex], txBuff, flexioWordSize[stateIndex] * txSize);
        }
        else
        {
            ret =  FLEXIO_I2S_DRV_SlaveSendData(flexioState[stateIndex], txBuff, flexioWordSize[stateIndex] * txSize);
        }
    }
#endif /* I2S_OVER_FLEXIO */
#ifdef I2S_OVER_SAI
    const uint8_t* addr;
    if (instance < SAI_HIGH_INDEX)
    {
        if ((lastXfer == LAST_IS_NONE) || (lastXfer == LAST_IS_RX))
        {
            lastXfer = LAST_IS_TX;
            SAI_DRV_SetMaster(instance, true);
        }
        addr = txBuff;
        SAI_DRV_Send(instance, &addr, txSize);
        ret = STATUS_SUCCESS;
    }
#endif /* I2S_OVER_SAI */
    return ret;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2S_I2S_AbortTransfer
 * Description   : Abort ongoing transfer
 *
 * Implements    : I2S_AbortTransfer_Activity
 *END**************************************************************************/
status_t I2S_Abort(i2s_instance_t instance)
{
    status_t ret = STATUS_UNSUPPORTED;
#ifdef I2S_OVER_FLEXIO
    uint8_t stateIndex;
    if (
#ifdef I2S_OVER_SAI
        instance >= FLEXIO_I2S_LOW_INDEX &&
#endif /* I2S_OVER_SAI */
        instance <= FLEXIO_I2S_HIGH_INDEX)
    {
        stateIndex = findFlexioState(instance);
        if (flexioIsMaster[stateIndex])
        {
            ret = FLEXIO_I2S_DRV_MasterTransferAbort(flexioState[stateIndex]);
        }
        else
        {
            ret = FLEXIO_I2S_DRV_SlaveTransferAbort(flexioState[stateIndex]);
        }
    }
#endif /* I2S_OVER_FLEXIO */
#ifdef I2S_OVER_SAI
    if (instance < SAI_HIGH_INDEX)
    {
        if (lastXfer == LAST_IS_TX)
        {
            SAI_DRV_AbortSending(instance);
        }
        else if (lastXfer == LAST_IS_RX)
        {
            SAI_DRV_AbortReceiving(instance);
        }
        ret = STATUS_SUCCESS;
    }
#endif /* I2S_OVER_SAI */
    return ret;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2S_GetStatus
 * Description   : get status of driver
 *
 * Implements    : I2S_GetStatus_Activity
 *END**************************************************************************/
status_t I2S_GetStatus(i2s_instance_t instance,
                       uint32_t * countRemaining)
{
    status_t ret = STATUS_UNSUPPORTED;
#ifdef I2S_OVER_FLEXIO
    uint8_t stateIndex;
    if (
#ifdef I2S_OVER_SAI
        instance >= FLEXIO_I2S_LOW_INDEX &&
#endif /* I2S_OVER_SAI */
        instance <= FLEXIO_I2S_HIGH_INDEX)
    {
        stateIndex = findFlexioState(instance);
        if (flexioIsMaster[stateIndex])
        {
            ret =  FLEXIO_I2S_DRV_MasterGetStatus(flexioState[stateIndex], countRemaining);
            if (countRemaining != NULL)
            {
                *countRemaining /= flexioWordSize[stateIndex];
            }
        }
        else
        {
            ret =  FLEXIO_I2S_DRV_SlaveGetStatus(flexioState[stateIndex], countRemaining);
            if (countRemaining != NULL)
            {
                *countRemaining /= flexioWordSize[stateIndex];
            }
        }
    }
#endif /* I2S_OVER_FLEXIO */
#ifdef I2S_OVER_SAI
    if (instance < SAI_HIGH_INDEX)
    {
        if (lastXfer == LAST_IS_TX)
        {
            ret = SAI_DRV_GetSendingStatus(instance, countRemaining);
        }
        else if (lastXfer == LAST_IS_RX)
        {
            ret = SAI_DRV_GetReceivingStatus(instance, countRemaining);
        }
    }
#endif /* I2S_OVER_SAI */
    return ret;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2S_ReceiveDataBlocking
 * Description   : receive blocking
 *
 * Implements    : I2S_ReceiveDataBlocking_Activity
 *END**************************************************************************/
status_t I2S_ReceiveDataBlocking(
        i2s_instance_t instance,
        uint8_t * rxBuff,
        uint32_t rxSize,
        uint32_t timeout)
{
    status_t ret = STATUS_UNSUPPORTED;
#ifdef I2S_OVER_FLEXIO
    uint8_t stateIndex;
    if (
#ifdef I2S_OVER_SAI
        instance >= FLEXIO_I2S_LOW_INDEX &&
#endif /* I2S_OVER_SAI */
        instance <= FLEXIO_I2S_HIGH_INDEX)
    {
        stateIndex = findFlexioState(instance);
        if (flexioIsMaster[stateIndex])
        {
            ret =  FLEXIO_I2S_DRV_MasterReceiveDataBlocking(flexioState[stateIndex], rxBuff, flexioWordSize[stateIndex] * rxSize, timeout);
        }
        else
        {
            ret =  FLEXIO_I2S_DRV_SlaveReceiveDataBlocking(flexioState[stateIndex], rxBuff, flexioWordSize[stateIndex] * rxSize, timeout);
        }
    }
#endif /* I2S_OVER_FLEXIO */
#ifdef I2S_OVER_SAI
    uint8_t* addr;
    if (instance < SAI_HIGH_INDEX)
    {
        if ((lastXfer == LAST_IS_NONE) || (lastXfer == LAST_IS_TX))
        {
            lastXfer = LAST_IS_RX;
            SAI_DRV_SetMaster(instance, false);
        }
        addr = rxBuff;
        ret = SAI_DRV_ReceiveBlocking(instance, &addr, rxSize, timeout);
    }
#endif /* I2S_OVER_SAI */
    return ret;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2S_ReceiveData
 * Description   : receive non blocking
 *
 * Implements    : I2S_ReceiveData_Activity
 *END**************************************************************************/
status_t I2S_ReceiveData(i2s_instance_t instance,
                         uint8_t * rxBuff,
                         uint32_t rxSize)
{
    status_t ret = STATUS_UNSUPPORTED;
#ifdef I2S_OVER_FLEXIO
    uint8_t stateIndex;
    if (
#ifdef I2S_OVER_SAI
        instance >= FLEXIO_I2S_LOW_INDEX &&
#endif /* I2S_OVER_SAI */
        instance <= FLEXIO_I2S_HIGH_INDEX)
    {
        stateIndex = findFlexioState(instance);
        if (flexioIsMaster[stateIndex])
        {
            ret =  FLEXIO_I2S_DRV_MasterReceiveData(flexioState[stateIndex], rxBuff, flexioWordSize[stateIndex] * rxSize);
        }
        else
        {
            ret =  FLEXIO_I2S_DRV_SlaveReceiveData(flexioState[stateIndex], rxBuff, flexioWordSize[stateIndex] * rxSize);
        }
    }
#endif /* I2S_OVER_FLEXIO */
#ifdef I2S_OVER_SAI
    uint8_t* addr;
    if (instance < SAI_HIGH_INDEX)
    {
        if ((lastXfer == LAST_IS_NONE) || (lastXfer == LAST_IS_TX))
        {
            lastXfer = LAST_IS_RX;
            SAI_DRV_SetMaster(instance, false);
        }
        addr = rxBuff;
        SAI_DRV_Receive(instance, &addr, rxSize);
        ret = STATUS_SUCCESS;
    }
#endif /* I2S_OVER_SAI */
    return ret;
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
