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

#include "i2c_pal.h"
#include "device_registers.h"

/* Include PD files */
#if (defined (I2C_OVER_LPI2C))
    #include "lpi2c_driver.h"
#endif

#if (defined (I2C_OVER_FLEXIO))
    #include "flexio.h"
    #include "flexio_i2c_driver.h"
#endif

#if (defined (I2C_OVER_I2C))
    #include "i2c_driver.h"
#endif

/* Define state structures for LPI2C */
#if (defined(I2C_OVER_LPI2C))
    /*! @brief I2C state structures */
    lpi2c_master_state_t Lpi2cMasterState[NO_OF_LPI2C_INSTS_FOR_I2C];
    lpi2c_slave_state_t Lpi2cSlaveState[NO_OF_LPI2C_INSTS_FOR_I2C];
    /*! @brief LPI2C state-instance matching */
    static i2c_instance_t Lpi2cStateInstanceMapping[NO_OF_LPI2C_INSTS_FOR_I2C];
    /*! @brief LPI2C  available resources table */
    static bool Lpi2cStateIsAllocated[NO_OF_LPI2C_INSTS_FOR_I2C];
#endif

/* Define state structure for FLEXIO */
#if (defined(I2C_OVER_FLEXIO))
    /*! @brief FLEXIO state structures */
    flexio_i2c_master_state_t FlexioI2CState[NO_OF_FLEXIO_INSTS_FOR_I2C];
    flexio_device_state_t FlexioState;
    /*! @brief FLEXIO state-instance matching */
    static i2c_instance_t FlexioI2CStateInstanceMapping[NO_OF_FLEXIO_INSTS_FOR_I2C];
    /*! @brief FLEXIO  available resources table */
    static bool FlexioI2CStateIsAllocated[NO_OF_FLEXIO_INSTS_FOR_I2C];
#endif

/* Define state structure for I2C */
#if (defined(I2C_OVER_I2C))
    /*! @brief I2C state structures */
    i2c_master_state_t I2CMasterState[NO_OF_I2C_INSTS_FOR_I2C];
    i2c_slave_state_t I2CSlaveState[NO_OF_I2C_INSTS_FOR_I2C];
    /*! @brief I2C state-instance matching */
    static i2c_instance_t I2CStateInstanceMapping[NO_OF_I2C_INSTS_FOR_I2C];
    /*! @brief I2C available resources table */
    static bool I2CStateIsAllocated[NO_OF_I2C_INSTS_FOR_I2C];
#endif

/*FUNCTION**********************************************************************
*
* Function Name : I2CAllocateState
* Description   : Allocates one of the available state structure.
*
*END**************************************************************************/
static uint8_t I2CAllocateState(bool* isAllocated, i2c_instance_t* instanceMapping, i2c_instance_t instance, uint8_t numberOfinstances)
{
    uint8_t i;
    /* Allocate one of the I2C state structure for this instance */
    for (i = 0;i < numberOfinstances;i++)
    {
        if (isAllocated[i] == false)
        {
            instanceMapping[i] = instance;
            isAllocated[i] = true;
            break;
        }
     }

    return i;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2CFreeState
 * Description   : Deallocates one of the available state structure.
 *
 *END**************************************************************************/
static void I2CFreeState(bool* isAllocated, i2c_instance_t* instanceMapping, i2c_instance_t instance, uint8_t numberOfinstances)
{
    uint8_t i;
    /* Deallocate one of the available state structure*/
    for (i = 0;i < numberOfinstances;i++)
    {
        if (instanceMapping[i] == instance)
        {
            isAllocated[i] = false;
            break;
        }
    }
}

#if (defined(I2C_OVER_FLEXIO))
/*FUNCTION**********************************************************************
 *
 * Function Name : FindFlexioState
 * Description   : Search the state structure of the flexio instance
 *
 *END**************************************************************************/
static uint8_t FindFlexioState(i2c_instance_t instance)
{
    uint8_t i;
    for (i = 0;i<NO_OF_FLEXIO_INSTS_FOR_I2C;i++)
    {
        if (FlexioI2CStateInstanceMapping[i] == instance)
        {
            break;
        }
    }
    return i;
}
#endif

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C_MasterInit
 * Description   : Configures the I2C in master mode
 * Implements    : I2C_MasterInit_Activity
 *END**************************************************************************/
status_t I2C_MasterInit(i2c_instance_t instance, i2c_master_t *config)
{
    status_t status = STATUS_ERROR;
    uint8_t index = 0;

    /* Define I2C PAL over I2C */
    #if (defined (I2C_OVER_LPI2C))
    if(instance <= LPI2C_HIGH_INDEX)
    {
         lpi2c_master_user_config_t lpi2cConfig;
         lpi2cConfig.slaveAddress = config->slaveAddress;
         lpi2cConfig.is10bitAddr = config->is10bitAddr;
         switch(config->transferType)
         {
             case I2C_PAL_USING_DMA:      lpi2cConfig.transferType = LPI2C_USING_DMA; break;
             case I2C_PAL_USING_INTERRUPTS : lpi2cConfig.transferType = LPI2C_USING_INTERRUPTS; break;
         }
         lpi2cConfig.masterCallback = config->callback;
         lpi2cConfig.callbackParam = config->callbackParam;

         switch(config->operatingMode)
         {
             case I2C_PAL_STANDARD_MODE: lpi2cConfig.operatingMode = LPI2C_STANDARD_MODE; break;
             case I2C_PAL_FAST_MODE: lpi2cConfig.operatingMode = LPI2C_FAST_MODE; break;
             case I2C_PAL_FASTPLUS_MODE:
                 #if (LPI2C_HAS_FAST_PLUS_MODE)
                     lpi2cConfig.operatingMode = LPI2C_FASTPLUS_MODE;
                 #else
                     lpi2cConfig.operatingMode = LPI2C_STANDARD_MODE;
                 #endif
             break;
             case I2C_PAL_HIGHSPEED_MODE:
                 #if (LPI2C_HAS_HIGH_SPEED_MODE)
                     lpi2cConfig.operatingMode = LPI2C_HIGHSPEED_MODE;
                 #else
                     lpi2cConfig.operatingMode = LPI2C_STANDARD_MODE;
                 #endif
             break;
             case I2C_PAL_ULTRAFAST_MODE:
                 #if (LPI2C_HAS_ULTRA_FAST_MODE)
                     lpi2cConfig.operatingMode = LPI2C_ULTRAFAST_MODE;
                 #else
                     lpi2cConfig.operatingMode = LPI2C_STANDARD_MODE;
                 #endif
             break;
         }

         lpi2cConfig.baudRate = config->baudRate;
         /* DMA channel */
         lpi2cConfig.dmaChannel = config->dmaChannel1;
         /* Allocate one of the LPI2C state structure for this instance */
         index = I2CAllocateState(Lpi2cStateIsAllocated, Lpi2cStateInstanceMapping, instance, NO_OF_LPI2C_INSTS_FOR_I2C);
         status = LPI2C_DRV_MasterInit((uint32_t)instance, &lpi2cConfig, &Lpi2cMasterState[index]);
    }
    #endif

    #if(defined (I2C_OVER_I2C))
    if(instance <= I2C_HIGH_INDEX)
    {
        i2c_master_user_config_t i2cConfig;
        i2cConfig.slaveAddress = config->slaveAddress;
        switch(config->transferType)
        {
            case I2C_PAL_USING_DMA:      i2cConfig.transferType = I2C_USING_DMA; break;
            case I2C_PAL_USING_INTERRUPTS : i2cConfig.transferType = I2C_USING_INTERRUPTS; break;
        }
        i2cConfig.masterCallback = config->callback;
        i2cConfig.callbackParam = (uint8_t *)instance;
        i2cConfig.dmaChannel = config->dmaChannel1;
        i2cConfig.baudRate = config->baudRate;
        /*Allocate one of the I2C state structure for this instance */
        index = I2CAllocateState(I2CStateIsAllocated, I2CStateInstanceMapping, instance, NO_OF_I2C_INSTS_FOR_I2C);
        status = I2C_DRV_MasterInit((uint8_t)instance, &i2cConfig, &I2CMasterState[index]);
    }
    #endif

    #if(defined (I2C_OVER_FLEXIO))
    if((instance >= FLEXIO_I2C_LOW_INDEX) && (instance <= FLEXIO_I2C_HIGH_INDEX))
    {
        flexio_i2c_master_user_config_t flexioI2CConfig;
        flexioI2CConfig.slaveAddress = config->slaveAddress;
        switch(config->transferType)
        {
            case I2C_PAL_USING_DMA:      flexioI2CConfig.driverType = FLEXIO_DRIVER_TYPE_DMA; break;
            case I2C_PAL_USING_INTERRUPTS : flexioI2CConfig.driverType = FLEXIO_DRIVER_TYPE_INTERRUPTS; break;
        }
        flexioI2CConfig.sdaPin = ((extension_flexio_for_i2c_t*)(config->extension))->sdaPin;
        flexioI2CConfig.sclPin = ((extension_flexio_for_i2c_t*)(config->extension))->sclPin;
        flexioI2CConfig.callback = config->callback;
        flexioI2CConfig.callbackParam = config->callbackParam;
        flexioI2CConfig.baudRate = config->baudRate;
        flexioI2CConfig.rxDMAChannel = config -> dmaChannel2;
        flexioI2CConfig.txDMAChannel = config -> dmaChannel1;
        status = FLEXIO_DRV_InitDevice(0U, &FlexioState);

        if(status == STATUS_SUCCESS)
        {
            /* Allocate one of the Flexio state structure for this instance */
            index = I2CAllocateState(FlexioI2CStateIsAllocated, FlexioI2CStateInstanceMapping, instance, NO_OF_FLEXIO_INSTS_FOR_I2C);
            status = FLEXIO_I2C_DRV_MasterInit(0U, &flexioI2CConfig, (flexio_i2c_master_state_t*)&FlexioI2CState[index]);
        }
    }
    #endif

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C_SlaveInit
 * Description   : Configures the I2C in slave mode
 * Implements    : I2C_SlaveInit_Activity
 *END**************************************************************************/
status_t I2C_SlaveInit(i2c_instance_t instance, i2c_slave_t *config)
{
    status_t status = STATUS_ERROR;
    uint8_t index = 0;

    /* Define I2C PAL over LPI2C */
    #if (defined (I2C_OVER_LPI2C))
        if(instance <= LPI2C_HIGH_INDEX)
        {
            lpi2c_slave_user_config_t lpi2cConfig;
            lpi2cConfig.slaveAddress = config->slaveAddress;
            lpi2cConfig.transferType = (lpi2c_transfer_type_t)config->transferType;
            lpi2cConfig.dmaChannel = config->dmaChannel;
            lpi2cConfig.is10bitAddr = config->is10bitAddr;
            lpi2cConfig.slaveListening = config->slaveListening;
            lpi2cConfig.slaveCallback = config->callback;
            lpi2cConfig.callbackParam = config->callbackParam;
            /*Allocate one of the LPI2C state structure for this instance */
            index = I2CAllocateState(Lpi2cStateIsAllocated, Lpi2cStateInstanceMapping, instance, NO_OF_LPI2C_INSTS_FOR_I2C);
            status = LPI2C_DRV_SlaveInit((uint32_t)instance, &lpi2cConfig, &Lpi2cSlaveState[index]);
        }
    #endif

    #if (defined (I2C_OVER_I2C))
        if(instance <= I2C_HIGH_INDEX)
        {
            i2c_slave_user_config_t i2cConfig;
            i2cConfig.slaveAddress = config->slaveAddress;
            i2cConfig.slaveListening = config->slaveListening;
            i2cConfig.slaveCallback = config->callback;
            i2cConfig.callbackParam = config->callbackParam;
            /*Allocate one of the LPI2C state structure for this instance */
            index = I2CAllocateState(I2CStateIsAllocated, I2CStateInstanceMapping, instance, NO_OF_I2C_INSTS_FOR_I2C);
            status = I2C_DRV_SlaveInit((uint8_t)instance, &i2cConfig, (i2c_slave_state_t*)(&I2CSlaveState[index]));
        }
    #endif

    #if (defined (I2C_OVER_FLEXIO))
        if((instance >= FLEXIO_I2C_LOW_INDEX) && (instance <= FLEXIO_I2C_HIGH_INDEX))
        {
            /* Cast to void to avoid compiler warnings */
            (void) config;
            (void) index;
            status = STATUS_UNSUPPORTED;
        }
    #endif

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C_MasterSendData
 * Description   : Initializes a non-blocking master send data transfer
 * Implements    : I2C_MasterSendData_Activity
 *
 *END**************************************************************************/
status_t I2C_MasterSendData(i2c_instance_t instance, const uint8_t *txBuff, uint32_t txSize, bool sendStop)
{
    status_t status = STATUS_ERROR;

    /* Define I2C PAL over LPI2C */
    #if defined (I2C_OVER_LPI2C)
    if(instance <= LPI2C_HIGH_INDEX)
    {
        status = LPI2C_DRV_MasterSendData((uint32_t)instance, txBuff, txSize, sendStop);
    }
    #endif

    /* Define I2C PAL over I2C */
    #if defined (I2C_OVER_I2C)
    if(instance <= I2C_HIGH_INDEX)
    {
        status = I2C_DRV_MasterSendData((uint8_t)instance, txBuff, txSize, sendStop);
    }
    #endif

    /* Define I2C PAL over FLEXIO */
    #if defined (I2C_OVER_FLEXIO)
    if((instance >= FLEXIO_I2C_LOW_INDEX) && (instance <= FLEXIO_I2C_HIGH_INDEX))
    {
        flexio_i2c_master_state_t *master;
        uint8_t instFlexio;

        instFlexio = FindFlexioState(instance);
        master = &FlexioI2CState[instFlexio];
        status = FLEXIO_I2C_DRV_MasterSendData(master, txBuff, txSize, sendStop);
    }
    #endif

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C_MasterSendDataBlocking
 * Description   : Initializes a blocking master send data transfer with time-out
 * Implements    : I2C_MasterSendDataBlocking_Activity
 *
 *END**************************************************************************/
status_t I2C_MasterSendDataBlocking(i2c_instance_t instance, const uint8_t *txBuff, uint32_t txSize, bool sendStop, uint32_t timeout)
{
    status_t status = STATUS_ERROR;

    /* Define I2C PAL over LPI2C */
    #if defined (I2C_OVER_LPI2C)
    if(instance <= LPI2C_HIGH_INDEX)
    {
        status = LPI2C_DRV_MasterSendDataBlocking((uint32_t)instance, txBuff, txSize, sendStop, timeout);
    }
    #endif

    /* Define I2C PAL over I2C */
    #if defined (I2C_OVER_I2C)
    if(instance <= I2C_HIGH_INDEX)
    {
        status = I2C_DRV_MasterSendDataBlocking((uint8_t)instance, txBuff, txSize, sendStop, timeout);
    }
    #endif

    /* Define I2C PAL over FLEXIO */
    #if defined (I2C_OVER_FLEXIO)
    if((instance >= FLEXIO_I2C_LOW_INDEX) && (instance <= FLEXIO_I2C_HIGH_INDEX))
    {
        flexio_i2c_master_state_t *master;
        uint8_t instFlexio;

        instFlexio = FindFlexioState(instance);
        master = &FlexioI2CState[instFlexio];
        status = FLEXIO_I2C_DRV_MasterSendDataBlocking(master, txBuff, txSize, sendStop, timeout);
    }
    #endif

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C_MasterReceiveData
 * Description   : Initializes a non-blocking master receive transfer
 * Implements    : I2C_MasterReceiveData_Activity
 *
 *END**************************************************************************/
status_t I2C_MasterReceiveData(i2c_instance_t instance, uint8_t *rxBuff, uint32_t rxSize, bool sendStop)
{
    status_t status = STATUS_ERROR;

    #if defined(I2C_OVER_LPI2C)
    if(instance <= LPI2C_HIGH_INDEX)
    {
        status = LPI2C_DRV_MasterReceiveData((uint32_t)instance, rxBuff, rxSize, sendStop);
    }
    #endif

    #if defined(I2C_OVER_I2C)
    if(instance <= I2C_HIGH_INDEX)
    {
        status = I2C_DRV_MasterReceiveData((uint8_t)instance, rxBuff, rxSize, sendStop);
    }
    #endif

    #if defined(I2C_OVER_FLEXIO)
    if((instance >= FLEXIO_I2C_LOW_INDEX) && (instance <= FLEXIO_I2C_HIGH_INDEX))
    {
        flexio_i2c_master_state_t *master;
        uint8_t instFlexio;

        instFlexio = FindFlexioState(instance);
        master = &FlexioI2CState[instFlexio];
        status = FLEXIO_I2C_DRV_MasterReceiveData(master, rxBuff, rxSize, sendStop);
    }
    #endif

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C_MasterReceiveDataBlocking
 * Description   : Initializes a blocking master receive transfer
 * Implements    : I2C_MasterReceiveDataBlocking_Activity
 *
 *END**************************************************************************/
status_t I2C_MasterReceiveDataBlocking(i2c_instance_t instance, uint8_t *rxBuff, uint32_t rxSize, bool sendStop, uint32_t timeout)
{
    status_t status = STATUS_ERROR;

    #if defined(I2C_OVER_LPI2C)
    if(instance <= LPI2C_HIGH_INDEX)
    {
        status = LPI2C_DRV_MasterReceiveDataBlocking((uint32_t)instance, rxBuff, rxSize, sendStop, timeout);
    }
    #endif

    #if defined(I2C_OVER_I2C)
    if(instance <= I2C_HIGH_INDEX)
    {
        status = I2C_DRV_MasterReceiveDataBlocking((uint8_t)instance, rxBuff, rxSize, sendStop, timeout);
    }
    #endif

    #if defined(I2C_OVER_FLEXIO)
    if((instance >= FLEXIO_I2C_LOW_INDEX) && (instance <= FLEXIO_I2C_HIGH_INDEX))
    {
        flexio_i2c_master_state_t *master;
        uint32_t instFlexio;

        instFlexio = FindFlexioState(instance);
        master = &FlexioI2CState[instFlexio];
        status = FLEXIO_I2C_DRV_MasterReceiveDataBlocking(master, rxBuff, rxSize, sendStop, timeout);
    }
    #endif

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C_MasterDeinit
 * Description   : De-initializes the i2c master module
 * Implements    : I2C_MasterDeinit_Activity
 *
 *END**************************************************************************/
status_t I2C_MasterDeinit(i2c_instance_t instance)
{
    status_t status = STATUS_ERROR;

    #if defined(I2C_OVER_LPI2C)
    if(instance <= LPI2C_HIGH_INDEX)
    {
        status = LPI2C_DRV_MasterDeinit((uint32_t)instance);
        if (status == STATUS_SUCCESS)
        {
            I2CFreeState(Lpi2cStateIsAllocated, Lpi2cStateInstanceMapping, instance, NO_OF_LPI2C_INSTS_FOR_I2C);
        }
    }
    #endif

    #if defined(I2C_OVER_I2C)
    if(instance <= I2C_HIGH_INDEX)
    {
        status = I2C_DRV_MasterDeinit((uint8_t)instance);
        if (status == STATUS_SUCCESS)
        {
            I2CFreeState(I2CStateIsAllocated, I2CStateInstanceMapping, instance, NO_OF_I2C_INSTS_FOR_I2C);
        }
    }
    #endif

    #if defined(I2C_OVER_FLEXIO)
    if((instance >= FLEXIO_I2C_LOW_INDEX) && (instance <= FLEXIO_I2C_HIGH_INDEX))
    {
        flexio_i2c_master_state_t *master;
        uint32_t instFlexio;

        instFlexio = FindFlexioState(instance);
        master = &FlexioI2CState[instFlexio];
        status = FLEXIO_I2C_DRV_MasterDeinit(master);
        if (status == STATUS_SUCCESS)
        {
            I2CFreeState(FlexioI2CStateIsAllocated, FlexioI2CStateInstanceMapping, instance, NO_OF_FLEXIO_INSTS_FOR_I2C);
        }
    }
    #endif

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C_MasterSetSlaveAddress
 * Description   : set the slave address for any subsequent I2C communication
 * Implements : I2C_MasterSetSlaveAddress_Activity
 *
 *END**************************************************************************/
status_t I2C_MasterSetSlaveAddress(i2c_instance_t instance, const uint16_t address, const bool is10bitAddr)
{
    status_t status = STATUS_ERROR;

    #if defined (I2C_OVER_LPI2C)
    if(instance <= LPI2C_HIGH_INDEX)
    {
        (void) LPI2C_DRV_MasterSetSlaveAddr((uint32_t) instance, address, is10bitAddr);
        status = STATUS_SUCCESS;
    }
    #endif

    #if defined (I2C_OVER_I2C)
    if(instance <= I2C_HIGH_INDEX)
    {
        (void) I2C_DRV_MasterSetSlaveAddress((uint32_t) instance, address);
        status = STATUS_SUCCESS;
    }
    #endif

    #if defined (I2C_OVER_FLEXIO)
    if ((instance >= FLEXIO_I2C_LOW_INDEX) && (instance <= FLEXIO_I2C_HIGH_INDEX))
    {
        flexio_i2c_master_state_t *master;
        uint32_t instFlexio;

        instFlexio = FindFlexioState(instance);
        master = &FlexioI2CState[instFlexio];
        status = FLEXIO_I2C_DRV_MasterSetSlaveAddr(master, address);
    }
    #endif

    /* Cast to void to avoid compiler warnings for the cases where is10bitAddr is not used */
    (void) is10bitAddr;

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C_DRV_MasterSetBaudRate
 * Description   : set the baud rate for any subsequent I2C communication
 * Implements : I2C_MasterSetBaudRate_Activity
 *
 *END**************************************************************************/
status_t I2C_MasterSetBaudRate(i2c_instance_t instance, const i2c_master_t *config, uint32_t baudRate)
{
    status_t status = STATUS_ERROR;

    #if defined(I2C_OVER_LPI2C)
    if(instance <= LPI2C_HIGH_INDEX)
    {
        lpi2c_baud_rate_params_t baudrateLpi2c;

        baudrateLpi2c.baudRate = baudRate;
        (void)LPI2C_DRV_MasterSetBaudRate((uint32_t)instance, (lpi2c_mode_t) config->operatingMode, baudrateLpi2c);
        status = STATUS_SUCCESS;
    }
    #endif

    #if defined(I2C_OVER_I2C)
    if(instance <= I2C_HIGH_INDEX)
    {
        status = I2C_DRV_MasterSetBaudRate((uint32_t) instance, baudRate);
    }
    #endif

    #if defined(I2C_OVER_FLEXIO)
    if((instance >= FLEXIO_I2C_LOW_INDEX) && (instance <= FLEXIO_I2C_HIGH_INDEX))
    {
        flexio_i2c_master_state_t *masterFlexio;
        uint32_t instFlexio;

        /* Cast to void to avoid compiler warnings */
        (void) config;

        instFlexio = FindFlexioState(instance);
        masterFlexio = &FlexioI2CState[instFlexio];
        status = FLEXIO_I2C_DRV_MasterSetBaudRate(masterFlexio, baudRate);
    }
    #endif

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C_DRV_MasterGetBaudRate
 * Description   : get the baud rate for any subsequent I2C communication
 * Implements : I2C_MasterGetBaudRate_Activity
 *
 *END**************************************************************************/
status_t I2C_MasterGetBaudRate(i2c_instance_t instance, uint32_t *baudRate)
{
    status_t status = STATUS_ERROR;

    #if defined (I2C_OVER_LPI2C)
    if (instance <= LPI2C_HIGH_INDEX)
    {
        lpi2c_baud_rate_params_t *baudrate = NULL;

        (void) LPI2C_DRV_MasterGetBaudRate((uint32_t) instance, baudrate);
        *baudRate = baudrate->baudRate;
        status = STATUS_SUCCESS;
    }
    #endif

    #if defined(I2C_OVER_I2C)
    if (instance <= I2C_HIGH_INDEX)
    {
        *baudRate = I2C_DRV_MasterGetBaudRate((uint32_t) instance);
        status = STATUS_SUCCESS;
    }
    #endif

    #if defined(I2C_OVER_FLEXIO)
    if ((instance >= FLEXIO_I2C_LOW_INDEX) && (instance <= FLEXIO_I2C_HIGH_INDEX))
    {
        flexio_i2c_master_state_t *master;
        uint32_t instFlexio;

        instFlexio = FindFlexioState(instance);
        master = &FlexioI2CState[instFlexio];
        status = FLEXIO_I2C_DRV_MasterGetBaudRate(master, baudRate);
    }
    #endif

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C_GetDefaultMasterConfig
 * Description   : Gets the default configuration structure for master
 * Implements    : I2C_GetDefaultMasterConfig_Activity
 *
 *END**************************************************************************/
status_t I2C_GetDefaultMasterConfig(i2c_master_t *config)
{
    config->slaveAddress = 32U;
    config->is10bitAddr = false;
    config->baudRate = 100000U;
    config->transferType = I2C_PAL_USING_INTERRUPTS;
    config->operatingMode = I2C_PAL_STANDARD_MODE;
    config->callback = NULL;
    config->callbackParam = NULL;
    config->extension = NULL;

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C_GetDefaultSlaveConfig
 * Description   : Gets the default configuration structure for slave
 * Implements    : I2C_GetDefaultSlaveConfig_Activity
 *
 *END**************************************************************************/
status_t I2C_GetDefaultSlaveConfig(i2c_slave_t *config)
{
    config->slaveAddress = 32U;
    config->is10bitAddr = false;
    config->slaveListening = true;
    config->transferType = I2C_PAL_USING_INTERRUPTS;
    config->callback = NULL;
    config->callbackParam = NULL;

    return STATUS_SUCCESS;
}
/*FUNCTION**********************************************************************
 *
 * Function Name : I2C_SlaveSendData
 * Description   : Initializes a non-blocking master transfer
 * Implements    : I2C_SlaveSendData_Activity
 *
 *END**************************************************************************/
status_t I2C_SlaveSendData(i2c_instance_t instance, const uint8_t *txBuff, uint32_t txSize)
{
    status_t status = STATUS_ERROR;

    #if defined(I2C_OVER_LPI2C)
    if(instance <= LPI2C_HIGH_INDEX)
    {
        status = LPI2C_DRV_SlaveSendData((uint32_t)instance, txBuff, txSize);
    }
    #endif

    #if defined(I2C_OVER_I2C)
    if(instance <= I2C_HIGH_INDEX)
    {
        status = I2C_DRV_SlaveSendData((uint8_t)instance, txBuff, txSize);
    }
    #endif

    #if defined(I2C_OVER_FLEXIO)
    if((instance >= FLEXIO_I2C_LOW_INDEX) && (instance <= FLEXIO_I2C_HIGH_INDEX))
    {
        /* Cast to void to avoid compiler warnings for the unused parameters */
        (void) txBuff;
        (void) txSize;
        status = STATUS_UNSUPPORTED;
    }
    #endif

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C_SlaveSendDataBlocking
 * Description   : perform a blocking receive transaction on the I2C bus
 * Implements : I2C_SlaveSendDataBlocking_Activity
 *
 *END**************************************************************************/
status_t I2C_SlaveSendDataBlocking(i2c_instance_t instance, const uint8_t *txBuff, uint32_t txSize, uint32_t timeout)
{
    status_t status = STATUS_ERROR;

    #if defined(I2C_OVER_LPI2C)
    if(instance <= LPI2C_HIGH_INDEX)
    {
        status = LPI2C_DRV_SlaveSendDataBlocking((uint32_t)instance, txBuff, txSize, timeout);
    }
    #endif

    #if defined(I2C_OVER_I2C)
    if(instance <= I2C_HIGH_INDEX)
    {
        status = I2C_DRV_SlaveSendDataBlocking((uint8_t)instance, txBuff, txSize, timeout);
    }
    #endif

    #if defined(I2C_OVER_FLEXIO)
    if((instance >= FLEXIO_I2C_LOW_INDEX) && (instance <= FLEXIO_I2C_HIGH_INDEX))
    {
        /* Cast to void to avoid compiler warnings for the unused parameter */
        (void) txBuff;
        (void) txSize;
        (void) timeout;
        status = STATUS_UNSUPPORTED;
    }
    #endif

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C_SlaveReceiveData
 * Description   : perform a non-blocking receive transaction on the I2C bus
 * Implements : I2C_SlaveReceiveData_Activity
 *
 *END**************************************************************************/
status_t I2C_SlaveReceiveData(i2c_instance_t instance, uint8_t *rxBuff, uint32_t rxSize)
{
    status_t status = STATUS_ERROR;

    #if defined(I2C_OVER_LPI2C)
    if(instance <= LPI2C_HIGH_INDEX)
    {
        status = LPI2C_DRV_SlaveReceiveData((uint32_t)instance, rxBuff, rxSize);
    }
    #endif

    #if defined(I2C_OVER_I2C)
    if(instance <= I2C_HIGH_INDEX)
    {
        status = I2C_DRV_SlaveReceiveData((uint8_t)instance, rxBuff, rxSize);
    }
    #endif

    #if defined(I2C_OVER_FLEXIO)
    if((instance >= FLEXIO_I2C_LOW_INDEX) && (instance <= FLEXIO_I2C_HIGH_INDEX))
    {
        /* Cast to void to avoid compiler warnings */
        (void) rxBuff;
        (void) rxSize;
        status = STATUS_UNSUPPORTED;
    }
    #endif

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C_SlaveReceiveDataBlocking
 * Description   : perform a blocking receive transaction on the I2C bus
 * Implements : I2C_SlaveReceiveDataBlocking_Activity
 *
 *END**************************************************************************/
status_t I2C_SlaveReceiveDataBlocking(i2c_instance_t instance, uint8_t *rxBuff,
        uint32_t rxSize, uint32_t timeout)
{
    status_t status = STATUS_ERROR;

    #if defined(I2C_OVER_LPI2C)
    if(instance <= LPI2C_HIGH_INDEX)
    {
        status = LPI2C_DRV_SlaveReceiveDataBlocking((uint32_t)instance, rxBuff, rxSize, timeout);
    }
    #endif

    #if defined(I2C_OVER_I2C)
    if(instance <= I2C_HIGH_INDEX)
    {
        status = I2C_DRV_SlaveReceiveDataBlocking((uint8_t)instance, rxBuff, rxSize, timeout);
    }
    #endif

    #if defined(I2C_OVER_FLEXIO)
    if((instance >= FLEXIO_I2C_LOW_INDEX) && (instance <= FLEXIO_I2C_HIGH_INDEX))
    {
        /* Cast to void to avoid compiler warnings */
        (void) rxBuff;
        (void) rxSize;
        (void) timeout;
        status = STATUS_UNSUPPORTED;
    }
    #endif

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C_SlaveSetRxBuffer
 * Description   : Provide a buffer for receiving data.
 *
 * Implements : I2C_SlaveSetRxBuffer_Activity
 *END***************************************************************************/
status_t I2C_SlaveSetRxBuffer(i2c_instance_t instance, uint8_t *rxBuff, uint32_t rxSize)
{
    status_t status = STATUS_ERROR;
    #if defined(I2C_OVER_LPI2C)
    if(instance <= LPI2C_HIGH_INDEX)
    {
        status = LPI2C_DRV_SlaveSetRxBuffer(instance, rxBuff, rxSize);
    }
    #endif

    #if defined(I2C_OVER_I2C)
    if(instance <= I2C_HIGH_INDEX)
    {
        status = I2C_DRV_SlaveSetRxBuffer((uint8_t) instance, rxBuff, rxSize);
    }
    #endif

    #if defined(I2C_OVER_FLEXIO)
    if((instance >= FLEXIO_I2C_LOW_INDEX) && (instance <= FLEXIO_I2C_HIGH_INDEX))
    {
        /* Cast to void to avoid compiler warnings */
        (void) rxBuff;
        (void) rxSize;
        status = STATUS_UNSUPPORTED;
    }
    #endif

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C_SlaveSetTxBuffer
 * Description   : Provide a buffer for transmitting data.
 *
 * Implements : I2C_SlaveSetTxBuffer_Activity
 *END***************************************************************************/
status_t I2C_SlaveSetTxBuffer(i2c_instance_t instance, uint8_t *txBuff, uint32_t txSize)
{
    status_t status = STATUS_ERROR;

    #if defined(I2C_OVER_LPI2C)
    if(instance <= LPI2C_HIGH_INDEX)
    {
        status = LPI2C_DRV_SlaveSetTxBuffer(instance, txBuff, txSize);
    }
    #endif

    #if defined(I2C_OVER_I2C)
    if(instance <= I2C_HIGH_INDEX)
    {
        status = I2C_DRV_SlaveSetTxBuffer((uint8_t) instance, txBuff, txSize);
    }
    #endif

    #if defined(I2C_OVER_FLEXIO)
    if((instance >= FLEXIO_I2C_LOW_INDEX) && (instance <= FLEXIO_I2C_HIGH_INDEX))
    {
        /* Cast to void to avoid compiler warnings */
        (void) txBuff;
        (void) txSize;
        status = STATUS_UNSUPPORTED;
    }
    #endif

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C_SlaveDeinit
 * Description   : De-initializes the i2c slave module
 * Implements    : I2C_SlaveDeinit_Activity
 *
 *END**************************************************************************/
status_t I2C_SlaveDeinit(i2c_instance_t instance)
{
    status_t status = STATUS_ERROR;

    #if defined(I2C_OVER_LPI2C)
    if(instance <= LPI2C_HIGH_INDEX)
    {
        status = LPI2C_DRV_SlaveDeinit((uint32_t)instance);
    }
    #endif

    #if defined(I2C_OVER_I2C)
    if(instance <= I2C_HIGH_INDEX)
    {
        status = I2C_DRV_SlaveDeinit((uint8_t)instance);
    }
    #endif

    #if defined(I2C_OVER_FLEXIO)
    if((instance >= FLEXIO_I2C_LOW_INDEX) && (instance <= FLEXIO_I2C_HIGH_INDEX))
    {
        status = STATUS_UNSUPPORTED;
    }
    #endif

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C_MasterGetTransferStatus
 * Description   : Get the status of the current non-blocking I2C master transaction
 * Implements    : I2C_MasterGetTransferStatus_Activity
 *END**************************************************************************/
status_t I2C_MasterGetTransferStatus(i2c_instance_t instance, uint32_t *bytesRemaining)
{
    status_t status = STATUS_ERROR;

    /* Define I2C PAL over I2C */
    #if defined(I2C_OVER_LPI2C)
    if(instance <= LPI2C_HIGH_INDEX)
    {
        status = LPI2C_DRV_MasterGetTransferStatus(instance, bytesRemaining);
    }
    #endif

    /* Define I2C PAL over FLEXIO */
    #if defined(I2C_OVER_FLEXIO)
    if((instance>=FLEXIO_I2C_LOW_INDEX) && (instance <= FLEXIO_I2C_HIGH_INDEX))
    {
        flexio_i2c_master_state_t master;
        uint32_t instFlexio;

        instFlexio = FindFlexioState(instance);
        master = FlexioI2CState[instFlexio];

        status = FLEXIO_I2C_DRV_MasterGetStatus(&master, bytesRemaining);
    }
    #endif

    /* Define I2C PAL over LPI2C */
    #if defined(I2C_OVER_I2C)
    if(instance <= I2C_HIGH_INDEX)
    {
        status = I2C_DRV_MasterGetTransferStatus(instance);

        /* Cast to void to avoid compiler warnings */
        (void)bytesRemaining;
    }
    #endif

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C_SlaveGetTransferStatus
 * Description   : Get the status of the current non-blocking I2C slave transaction
 * Implements    : I2C_SlaveGetTransferStatus_Activity
 *
 *END**************************************************************************/
status_t I2C_SlaveGetTransferStatus(i2c_instance_t instance, uint32_t *bytesRemaining)
{
    status_t status = STATUS_ERROR;

    /* Define I2C PAL over I2C */
    #if defined(I2C_OVER_LPI2C)
    if(instance <= LPI2C_HIGH_INDEX)
    {
        status = LPI2C_DRV_SlaveGetTransferStatus(instance, bytesRemaining);
    }
    #endif

    /* Define I2C PAL over FLEXIO */
    #if defined(I2C_OVER_FLEXIO)
    if((instance>=FLEXIO_I2C_LOW_INDEX) && (instance <= FLEXIO_I2C_HIGH_INDEX))
    {
        status = STATUS_UNSUPPORTED;
    }
    #endif

    /* Define I2C PAL over LPI2C */
    #if defined(I2C_OVER_I2C)
    if(instance <= I2C_HIGH_INDEX)
    {
        status = I2C_DRV_SlaveGetTransferStatus(instance);
    }
    #endif

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C_MasterAbortTransfer
 * Description   : abort a non-blocking I2C Master transmission or reception
 * Implements    : I2C_MasterAbortTransfer_Activity
 *
 *END**************************************************************************/
status_t I2C_MasterAbortTransfer(i2c_instance_t instance)
{
    status_t status = STATUS_ERROR;

    /* Define I2C PAL over I2C */
    #if defined(I2C_OVER_LPI2C)
    if (instance <= LPI2C_HIGH_INDEX)
    {
        status = LPI2C_DRV_MasterAbortTransferData(instance);
    }
    #endif

    /* Define I2C PAL over FLEXIO */
    #if defined(I2C_OVER_FLEXIO)
    if((instance>=FLEXIO_I2C_LOW_INDEX) && (instance <= FLEXIO_I2C_HIGH_INDEX))
    {
        flexio_i2c_master_state_t *master;
        uint32_t instFlexio;

        instFlexio = FindFlexioState(instance);
        master = &FlexioI2CState[instFlexio];

        status = FLEXIO_I2C_DRV_MasterTransferAbort(master);
    }
    #endif

    /* Define I2C PAL over LPI2C */
    #if defined(I2C_OVER_I2C)
    if(instance <= I2C_HIGH_INDEX)
    {
        status = I2C_DRV_MasterAbortTransferData(instance);

    }
    #endif

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C_SlaveAbortTransfer
 * Description   : abort a non-blocking I2C slave transmission or reception
 * Implements    : I2C_SlaveAbortTransfer_Activity
 *
 *END**************************************************************************/
status_t I2C_SlaveAbortTransfer(i2c_instance_t instance)
{
    status_t status = STATUS_ERROR;

    /* Define I2C PAL over I2C */
    #if defined(I2C_OVER_LPI2C)
    if (instance <= LPI2C_HIGH_INDEX)
    {
        status = LPI2C_DRV_SlaveAbortTransferData(instance);
    }
    #endif

    /* Define I2C PAL over FLEXIO */
    #if defined(I2C_OVER_FLEXIO)
    if((instance>=FLEXIO_I2C_LOW_INDEX) && (instance <= FLEXIO_I2C_HIGH_INDEX))
    {
        status = STATUS_UNSUPPORTED;
    }
    #endif

    /* Define I2C PAL over LPI2C */
    #if defined(I2C_OVER_I2C)
    if(instance <= I2C_HIGH_INDEX)
    {
        status = I2C_DRV_SlaveAbortTransferData(instance);
    }
    #endif

    return status;
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
