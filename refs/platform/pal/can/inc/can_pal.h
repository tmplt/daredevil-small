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

#ifndef CAN_PAL_H
#define CAN_PAL_H

#include "can_pal_cfg.h"
#include "can_pal_mapping.h"
#include "status.h"
#include "callbacks.h"

/* Include PD files */
#if (defined(CAN_OVER_FLEXCAN))
    #include "flexcan_driver.h"
#endif

/*!
 * @defgroup can_pal CAN PAL
 * @ingroup can_pal
 * @addtogroup can_pal
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief CAN controller operation modes
 * Implements : can_operation_modes_t_Class
 */
typedef enum {
    CAN_NORMAL_MODE = 0U,        /*!< Normal mode or user mode */
    CAN_LOOPBACK_MODE = 2U,      /*!< Loop-back mode */
    CAN_DISABLE_MODE = 4U        /*!< Module disable mode */
} can_operation_modes_t;

/*! @brief CAN buffer payload sizes
 * Implements : can_fd_payload_size_t_Class
 */
typedef enum {
    CAN_PAYLOAD_SIZE_8 = 0,  /*!< CAN message buffer payload size in bytes */
    CAN_PAYLOAD_SIZE_16 ,    /*!< CAN message buffer payload size in bytes */
    CAN_PAYLOAD_SIZE_32 ,    /*!< CAN message buffer payload size in bytes */
    CAN_PAYLOAD_SIZE_64      /*!< CAN message buffer payload size in bytes */
} can_fd_payload_size_t;

/*! @brief CAN bit timing variables
 * Implements : can_time_segment_t_Class
 */
typedef struct {
    uint32_t propSeg;         /*!< Propagation segment */
    uint32_t phaseSeg1;       /*!< Phase segment 1 */
    uint32_t phaseSeg2;       /*!< Phase segment 2 */
    uint32_t preDivider;      /*!< Clock prescaler division factor */
    uint32_t rJumpwidth;      /*!< Resync jump width */
} can_time_segment_t;

/*! @brief CAN bitrate phase (nominal/data)
 * Implements : can_bitrate_phase_t_Class
 */
typedef enum {
    CAN_NOMINAL_BITRATE,        /*!< Nominal (FD arbitration) bitrate */
    CAN_FD_DATA_BITRATE         /*!< FD data bitrate */
} can_bitrate_phase_t;

/*! @brief CAN Message Buffer ID type
 * Implements : can_msgbuff_id_type_t_Class
 */
typedef enum {
    CAN_MSG_ID_STD,         /*!< Standard ID */
    CAN_MSG_ID_EXT          /*!< Extended ID */
} can_msg_id_type_t;

/*! @brief CAN buffer configuration
 * Implements : can_buff_config_t_Class
 */
typedef struct {
    bool enableFD;               /*!< Enable flexible data rate */
    bool enableBRS;              /*!< Enable bit rate switch inside a CAN FD frame */
    uint8_t fdPadding;           /*!< Value used for padding when the data length code (DLC)
                                     specifies a bigger payload size than the actual data length */
    can_msg_id_type_t idType;    /*!< Specifies whether the frame format is standard or extended */
    bool isRemote;               /*!< Specifies if the frame is standard or remote */
} can_buff_config_t;

/*! @brief CAN message format
 * Implements : can_message_t_Class
 */
typedef struct {
	uint32_t cs;       /*!< Code and Status*/
    uint32_t id;       /*!< ID of the message */
    uint8_t data[64];  /*!< Data bytes of the CAN message*/
    uint8_t length;    /*!< Length of payload in bytes */
} can_message_t;

/*! @brief CAN controller configuration
 * Implements : can_user_config_t_Class
 */
typedef struct
{
    uint32_t maxBuffNum;                  /*!< Set maximum number of buffers */
    can_operation_modes_t mode;           /*!< Set operation mode */
    bool enableFD;                        /*!< Enable flexible data rate */
    can_fd_payload_size_t payloadSize;    /*!< Set size of buffer payload */
    can_time_segment_t nominalBitrate;    /*!< Bit timing segments for nominal bitrate */
    can_time_segment_t dataBitrate;       /*!< Bit timing segments for data bitrate */
    void *extension;                      /*!< This field will be used to add extra settings to the
                                               basic configuration like FlexCAN Rx FIFO settings */
} can_user_config_t;

#if (defined(CAN_OVER_FLEXCAN))
/*! @brief FlexCAN Rx FIFO configuration
 * Implements : extension_flexcan_rx_fifo_t_Class
 */
typedef struct
{
    flexcan_rx_fifo_id_filter_num_t numIdFilters;   /*!< The number of Rx FIFO ID filters needed */
    flexcan_rx_fifo_id_element_format_t idFormat;   /*!< RX FIFO ID format */
    flexcan_id_table_t *idFilterTable;              /*!< Rx FIFO ID table */
} extension_flexcan_rx_fifo_t;
#endif

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
 * @brief Initializes the CAN module
 *
 * This function initializes and enables the requested CAN module.
 *
 * @param[in] instance Instance number
 * @param[in] config The configuration structure
 * @return    STATUS_SUCCESS if successful;
 *            STATUS_ERROR if unsuccessful or invalid instance number;
 */
status_t CAN_Init(can_instance_t instance, const can_user_config_t *config);

/*!
 * @brief De-initializes the CAN module
 *
 * This function de-initializes the CAN module.
 *
 * @param[in] instance Instance number
 * @return    STATUS_SUCCESS if successful;
 *            STATUS_ERROR if unsuccessful or invalid instance number;
 */
status_t CAN_Deinit(can_instance_t instance);

/*!
 * @brief Configures the CAN bitrate.
 *
 * This function configures the CAN bit timing variables.
 *
 * @param[in] instance  Instance number.
 * @param[in] phase selects between nominal/data phase bitrate.
 * @param[in] bitTiming bit timing variables.
 * @return STATUS_SUCCESS if successful;
 *         STATUS_ERROR if invalid instance number is used;
 */
status_t CAN_SetBitrate(can_instance_t instance,
                        can_bitrate_phase_t phase,
                        const can_time_segment_t *bitTiming);

/*!
 * @brief Returns the CAN bitrate.
 *
 * This function returns the CAN configured bitrate.
 *
 * @param[in] instance  Instance number.
 * @param[in] phase selects between nominal/data phase bitrate.
 * @param[out] bitTiming configured bit timing variables.
 * @return STATUS_SUCCESS if successful;
 *         STATUS_ERROR if invalid instance number is used;
 */
status_t CAN_GetBitrate(can_instance_t instance,
                        can_bitrate_phase_t phase,
                        can_time_segment_t *bitTiming);

/*!
 * @brief Configures a buffer for transmission.
 *
 * This function configures a buffer for transmission.
 *
 * @param[in] instance  Instance number.
 * @param[in] buffIdx buffer index.
 * @param[in] config buffer configuration.
 * @return STATUS_SUCCESS if successful;
 *         STATUS_CAN_BUFF_OUT_OF_RANGE if the buffer index is out of range;
 *         STATUS_ERROR if invalid instance number is used;
 */
status_t CAN_ConfigTxBuff(can_instance_t instance,
                          uint32_t buffIdx,
                          const can_buff_config_t *config);

/*!
 * @brief Configures a buffer for reception.
 *
 * This function configures a buffer for reception.
 *
 * @param[in] instance  Instance number.
 * @param[in] buffIdx buffer index.
 * @param[in] config buffer configuration.
 * @param[in] acceptedId ID used for accepting frames.
 * @return STATUS_SUCCESS if successful;
 *         STATUS_CAN_BUFF_OUT_OF_RANGE if the buffer index is out of range;
 *         STATUS_ERROR if invalid instance number is used;
 */
status_t CAN_ConfigRxBuff(can_instance_t instance,
                          uint32_t buffIdx,
                          const can_buff_config_t *config,
                          uint32_t acceptedId);

/*!
 * @brief Sends a CAN frame using the specified buffer.
 *
 * This function sends a CAN frame using a configured message buffer. The function
 * returns immediately. If a callback is installed, it will be invoked after
 * the frame was sent.
 *
 * @param[in] instance  Instance number.
 * @param[in] buffIdx buffer index.
 * @param[in] message message to be sent.
 * @return STATUS_SUCCESS if successful;
 *         STATUS_BUSY if the current buffer is involved in another transfer;
 *         STATUS_CAN_BUFF_OUT_OF_RANGE if the buffer index is out of range;
 *         STATUS_ERROR if invalid instance number is used;
 */
status_t CAN_Send(can_instance_t instance,
                  uint32_t buffIdx,
                  const can_message_t *message);

/*!
 * @brief Sends a CAN frame using the specified buffer, in a blocking manner.
 *
 * This function sends a CAN frame using a configured buffer. The function
 * blocks until either the frame was sent, or the specified timeoutMs expired.
 *
 * @param[in] instance  Instance number.
 * @param[in] buffIdx buffer index.
 * @param[in] message message to be sent.
 * @param[in] timeoutMs A timeout for the transfer in milliseconds.
 * @return STATUS_SUCCESS if successful;
 *         STATUS_BUSY if the current buffer is involved in another transfer;
 *         STATUS_TIMEOUT if the timeout is reached;
 *         STATUS_CAN_BUFF_OUT_OF_RANGE if the buffer index is out of range;
 *         STATUS_ERROR if invalid instance number is used;
 */
status_t CAN_SendBlocking(can_instance_t instance,
                          uint32_t buffIdx,
                          const can_message_t *message,
                          uint32_t timeoutMs);

/*!
 * @brief Receives a CAN frame using the specified message buffer.
 *
 * This function receives a CAN frame using a configured buffer. The function
 * returns immediately. If a callback is installed, it will be invoked after
 * the frame was received and read into the specified buffer.
 *
 * @param[in] instance  Instance number.
 * @param[in] buffIdx buffer index.
 * @param[out] message received message.
 * @return STATUS_SUCCESS if successful;
 *         STATUS_BUSY if the current buffer is involved in another transfer;
 *         STATUS_CAN_BUFF_OUT_OF_RANGE if the buffer index is out of range;
 *         STATUS_ERROR if invalid instance number is used;
 */
status_t CAN_Receive(can_instance_t instance,
                     uint32_t buffIdx,
                     can_message_t *message);

/*!
 * @brief Receives a CAN frame using the specified buffer, in a blocking manner.
 *
 * This function receives a CAN frame using a configured buffer. The function
 * blocks until either a frame was received, or the specified timeout expired.
 *
 * @param[in] instance  Instance number.
 * @param[in] buffIdx buffer index.
 * @param[out] message received message.
 * @param[in] timeoutMs A timeout for the transfer in milliseconds.
 * @return STATUS_SUCCESS if successful;
 *         STATUS_BUSY if the current buffer is involved in another transfer;
 *         STATUS_TIMEOUT if the timeout is reached;
 *         STATUS_CAN_BUFF_OUT_OF_RANGE if the buffer index is out of range;
 *         STATUS_ERROR if invalid instance number is used;
 */
status_t CAN_ReceiveBlocking(can_instance_t instance,
                             uint32_t buffIdx,
                             can_message_t *message,
                             uint32_t timeoutMs);

/*!
 * @brief Configures an ID filter for a specific reception buffer.
 *
 * This function configures an ID filter for each reception buffer.
 *
 * @param[in] instance  Instance number.
 * @param[in] idType selects between standard and extended ID.
 * @param[in] buffIdx buffer index.
 * @param[in] mask mask value for ID filtering.
 * @return STATUS_SUCCESS if successful;
 *         STATUS_CAN_BUFF_OUT_OF_RANGE if the buffer index is out of range;
 *         STATUS_ERROR if invalid instance number is used;
 */
status_t CAN_SetRxFilter(can_instance_t instance,
                         can_msg_id_type_t idType,
                         uint32_t buffIdx,
                         uint32_t mask);

/*!
 * @brief Returns the state of the previous CAN transfer.
 *
 * When performing an async transfer, call this function to ascertain the state of the
 * current transfer: in progress or complete.
 *
 * @param[in] instance The CAN instance number.
 * @param[in] buffIdx buffer index.
 * @return STATUS_SUCCESS if successful;
 *         STATUS_BUSY if a resource is busy;
 *         STATUS_ERROR if invalid instance number is used;
 */
status_t CAN_GetTransferStatus(can_instance_t instance, uint32_t buffIdx);

/*!
 * @brief Installs a callback function for the IRQ handler.
 *
 * @param[in] instance instance number.
 * @param[in] callback The callback function.
 * @param[in] callbackParam User parameter passed to the callback function through the state parameter.
 * @return STATUS_SUCCESS if successful;
 *         STATUS_ERROR if invalid instance number is used;
 */
status_t CAN_InstallEventCallback(can_instance_t instance,
                                  can_callback_t callback,
                                  void *callbackParam);

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__cplusplus)
}
#endif

/*! @}*/

#endif /* CAN_PAL_H */

/*******************************************************************************
 * EOF
 ******************************************************************************/
