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
 * @file mpu_pal.h
 */

#ifndef MPU_PAL_H
#define MPU_PAL_H

#include "status.h"
#include "mpu_pal_mapping.h"

/*!
 * @defgroup mpu_pal_code MPU PAL
 * @ingroup mpu_pal
 * @brief Memory Protection Unit Peripheral Abstraction Layer.
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*!
 * @brief MPU access error
 * Implements : mpu_error_access_type_t_Class
 */
typedef enum
{
    MPU_ERROR_TYPE_READ  = 0U, /*!< Error type: read */
    MPU_ERROR_TYPE_WRITE = 1U  /*!< Error type: write */
} mpu_error_access_type_t;

/*!
 * @brief MPU access error attributes
 * Implements : mpu_error_attributes_t_Class
 */
typedef enum
{
    MPU_ERROR_USER_MODE_INSTRUCTION_ACCESS       = 0U, /*!< Instruction access error in user mode */
    MPU_ERROR_USER_MODE_DATA_ACCESS              = 1U, /*!< Data access error in user mode */
    MPU_ERROR_SUPERVISOR_MODE_INSTRUCTION_ACCESS = 2U, /*!< Instruction access error in supervisor mode */
    MPU_ERROR_SUPERVISOR_MODE_DATA_ACCESS        = 3U  /*!< Data access error in supervisor mode */
} mpu_error_attributes_t;

/*!
 * @brief MPU detail error access info
 * Implements : mpu_error_info_t_Class
 */
typedef struct
{
    uint8_t                 master;     /*!< Access error master */
    bool                    overrun;    /*!< Access error master overrun */
    mpu_error_attributes_t  attributes; /*!< Access error attributes */
    mpu_error_access_type_t accessType; /*!< Access error type */
    uint32_t                accessCtr;  /*!< Access error control */
    uint32_t                addr;       /*!< Access error address */
    uint8_t                 processId;  /*!< Access error process identification */
} mpu_error_info_t;

/*!
 * @brief MPU detail access rights
 * For specific master:
 * |      Code                     |  Supervisor  |     User    |  Description                                                                        |
 * |-------------------------------|--------------|-------------|-------------------------------------------------------------------------------------|
 * |  MPU_SUPERVISOR_RWX_USER_NONE |    r w x     |    - - -    | Allow Read, write, execute in supervisor mode; no access in user mode               |
 * |  MPU_SUPERVISOR_RWX_USER_X    |    r w x     |    - - x    | Allow Read, write, execute in supervisor mode; execute in user mode                 |
 * |  MPU_SUPERVISOR_RWX_USER_W    |    r w x     |    - w -    | Allow Read, write, execute in supervisor mode; write in user mode                   |
 * |  MPU_SUPERVISOR_RWX_USER_WX   |    r w x     |    - w x    | Allow Read, write, execute in supervisor mode; write and execute in user mode       |
 * |  MPU_SUPERVISOR_RWX_USER_R    |    r w x     |    r - -    | Allow Read, write, execute in supervisor mode; read in user mode                    |
 * |  MPU_SUPERVISOR_RWX_USER_RX   |    r w x     |    r - x    | Allow Read, write, execute in supervisor mode; read and execute in user mode        |
 * |  MPU_SUPERVISOR_RWX_USER_RW   |    r w x     |    r w -    | Allow Read, write, execute in supervisor mode; read and write in user mode          |
 * |  MPU_SUPERVISOR_RWX_USER_RWX  |    r w x     |    r w x    | Allow Read, write, execute in supervisor mode; read, write and execute in user mode |
 * |  MPU_SUPERVISOR_RX_USER_NONE  |    r - x     |    - - -    | Allow Read, execute in supervisor mode; no access in user mode                      |
 * |  MPU_SUPERVISOR_RX_USER_X     |    r - x     |    - - x    | Allow Read, execute in supervisor mode; execute in user mode                        |
 * |  MPU_SUPERVISOR_RX_USER_W     |    r - x     |    - w -    | Allow Read, execute in supervisor mode; write in user mode                          |
 * |  MPU_SUPERVISOR_RX_USER_WX    |    r - x     |    - w x    | Allow Read, execute in supervisor mode; write and execute in user mode              |
 * |  MPU_SUPERVISOR_RX_USER_R     |    r - x     |    r - -    | Allow Read, execute in supervisor mode; read in user mode                           |
 * |  MPU_SUPERVISOR_RX_USER_RX    |    r - x     |    r - x    | Allow Read, execute in supervisor mode; read and execute in user mode               |
 * |  MPU_SUPERVISOR_RX_USER_RW    |    r - x     |    r w -    | Allow Read, execute in supervisor mode; read and write in user mode                 |
 * |  MPU_SUPERVISOR_RX_USER_RWX   |    r - x     |    r w x    | Allow Read, execute in supervisor mode; read, write and execute in user mode        |
 * |  MPU_SUPERVISOR_RW_USER_NONE  |    r w -     |    - - -    | Allow Read, write in supervisor mode; no access in user mode                        |
 * |  MPU_SUPERVISOR_RW_USER_X     |    r w -     |    - - x    | Allow Read, write in supervisor mode; execute in user mode                          |
 * |  MPU_SUPERVISOR_RW_USER_W     |    r w -     |    - w -    | Allow Read, write in supervisor mode; write in user mode                            |
 * |  MPU_SUPERVISOR_RW_USER_WX    |    r w -     |    - w x    | Allow Read, write in supervisor mode; write and execute in user mode                |
 * |  MPU_SUPERVISOR_RW_USER_R     |    r w -     |    r - -    | Allow Read, write in supervisor mode; read in user mode                             |
 * |  MPU_SUPERVISOR_RW_USER_RX    |    r w -     |    r - x    | Allow Read, write in supervisor mode; read and execute in user mode                 |
 * |  MPU_SUPERVISOR_RW_USER_RW    |    r w -     |    r w -    | Allow Read, write in supervisor mode; read and write in user mode                   |
 * |  MPU_SUPERVISOR_RW_USER_RWX   |    r w -     |    r w x    | Allow Read, write in supervisor mode; read, write and execute in user mode          |
 * |  MPU_SUPERVISOR_USER_NONE     |    - - -     |    - - -    | No access allowed in user and supervisor modes                                      |
 * |  MPU_SUPERVISOR_USER_X        |    - - x     |    - - x    | Execute operation is allowed in user and supervisor modes                           |
 * |  MPU_SUPERVISOR_USER_W        |    - w -     |    - w -    | Write operation is allowed in user and supervisor modes                             |
 * |  MPU_SUPERVISOR_USER_WX       |    - w x     |    - w x    | Write and execute operations are allowed in user and supervisor modes               |
 * |  MPU_SUPERVISOR_USER_R        |    r - -     |    r - -    | Read operation is allowed in user and supervisor modes                              |
 * |  MPU_SUPERVISOR_USER_RX       |    r - x     |    r - x    | Read and execute operations are allowed in user and supervisor modes                |
 * |  MPU_SUPERVISOR_USER_RW       |    r w -     |    r w -    | Read and write operations are allowed in user and supervisor modes                  |
 * |  MPU_SUPERVISOR_USER_RWX      |    r w x     |    r w x    | Read write and execute operations are allowed in user and supervisor modes          |
 *
 * For normal master:
 * |      Code                     | Read/Write permission | Description                     |
 * |-------------------------------|-----------------------|---------------------------------|
 * |  MPU_NONE                     |          - -          | No Read/Write access permission |
 * |  MPU_W                        |          - w          | Write access permission         |
 * |  MPU_R                        |          r -          | Read access permission          |
 * |  MPU_RW                       |          r w          | Read/Write access permission    |
 * Implements : mpu_privilege_rights_t_Class
 */
#if defined(MPU_OVER_MPU)
typedef mpu_access_rights_t mpu_privilege_rights_t; /* Should be made enum inside MPU driver */
#elif defined(MPU_OVER_SMPU)
typedef enum
{
    MPU_NONE = 0U,
    MPU_W    = 1U,
    MPU_R    = 2U,
    MPU_RW   = 3U
} mpu_privilege_rights_t;
#endif /* defined(MPU_OVER_MPU) & defined(MPU_OVER_SMPU) */



/*!
 * @brief MPU master access rights.
 * Implements : mpu_master_privilege_right_t_Class
 */
typedef struct
{
    uint8_t                masterNum;       /*!< Master number */
    mpu_privilege_rights_t accessRight;     /*!< Privilege right */
} mpu_master_privilege_right_t;

/*!
 * @brief MPU region configuration structure.
 * Implements : mpu_region_config_t_Class
 */
typedef struct
{
    uint32_t                           startAddr;         /*!< Memory region start address */
    uint32_t                           endAddr;           /*!< Memory region end address */
    const mpu_master_privilege_right_t *masterAccRight;   /*!< Access permission for masters */
    uint8_t                            processIdEnable;   /*!< Process identifier enable
                                                          For MPU: the bit index corresponding with masters
                                                          For SMPU: disable if equal zero, otherwise enable */
    uint8_t                            processIdentifier; /*!< Process identifier*/
    uint8_t                            processIdMask;     /*!< Process identifier mask. The setting bit will
                                                          ignore the same bit in process identifier */
    void                               *extension;        /*!< This field will be used to add extra settings
                                                          to the basic region configuration */
} mpu_region_config_t;

#if defined(MPU_OVER_SMPU)
/*!
 * @brief Lock configuration.
 * Implements : mpu_lock_t_Class
 */
typedef enum
{
    MPU_UNLOCK     = 0U, /*!< Unlocked */
#if FEATURE_SMPU_HAS_OWNER_LOCK
    MPU_OWNER_LOCK = 1U, /*!< Locked by the master wrote this register and LCK bit
                         Attempted writes by other masters are ignored */
#endif/* FEATURE_SMPU_HAS_OWNER_LOCK */
    MPU_ALL_LOCK   = 3U  /*!< Attempted writes to any location in the region descriptor are ignored */
} mpu_lock_t;

/*!
 * @brief Region lock configuration structure.
 * Implements : mpu_region_lock_t_Class
 */
typedef struct
{
    uint8_t    regionNum;   /*!< Region number */
#if FEATURE_SMPU_HAS_OWNER_LOCK
    uint8_t    masterOwner; /*!< Master number */
#endif /* FEATURE_SMPU_HAS_OWNER_LOCK */
    mpu_lock_t lockConfig;  /*!< Lock configuration */
} mpu_region_lock_t;

/*!
 * @brief Defines the region extension structure for the MPU over SMPU
 * Implements : mpu_extension_smpu_region_t_Class
 */
typedef struct
{
    bool cacheInhibitEnable; /*!< Cache Inhibit */
    mpu_lock_t lockConfig;   /*!< Lock configuration */
} mpu_extension_smpu_region_t;
#endif /* defined(MPU_OVER_SMPU) */

/*******************************************************************************
 * API
 ******************************************************************************/
/*!
 * @name MPU PAL API
 * @{
 */

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Initializes memory protection unit by allocating regions
 * and granting access rights for masters.
 *
 * @param[in] instance The MPU instance number.
 * @param[in] regionCnt The number of regions configured.
 * @param[in] configPtr The pointer to regions configuration structure, see #mpu_region_config_t.
 * @return operation status
 *        - STATUS_SUCCESS : Operation was successful.
 *        - STATUS_ERROR   : Operation failed due to invalid master number
 *                           or the region was locked by another master
 *                           or all masters are locked.
 */
status_t MPU_Init(mpu_instance_t instance,
                  uint8_t regionCnt,
                  const mpu_region_config_t * configPtr);

/*!
 * @brief De-initializes memory protection unit by reseting all regions
 * and masters to default and disable module.
 *
 * @param[in] instance The MPU instance number.
 * @return operation status
 *        - STATUS_SUCCESS : Operation was successful.
 *        - STATUS_ERROR   : Operation failed due to the region was locked by another master
 *                           or all masters are locked.
 */
status_t MPU_Deinit(mpu_instance_t instance);

/*!
 * @brief Gets default region configuration. Grants all access rights for masters;
 * disable PID and cache; unlock region descriptor.
 *
 * @param[out] masterAccRight The pointer to master configuration structure, see #mpu_master_privilege_right_t.
 *                            The length of array should be defined by number of masters supported by hardware.
 * @param[out] regionConfig The pointer to default region configuration structure, see #mpu_region_config_t.
 */
void MPU_GetDefautRegionConfig(mpu_master_privilege_right_t * masterAccRight,
                               mpu_region_config_t * regionConfig);

/*!
 * @brief Updates region configuration.
 *
 * @param[in] instance The MPU instance number.
 * @param[in] regionNum The region number.
 * @param[in] configPtr The pointer to region configuration structure, see #mpu_region_config_t.
 * @return operation status
 *        - STATUS_SUCCESS : Operation was successful.
 *        - STATUS_ERROR   : Operation failed due to invalid master number
 *                           or the region was locked by another master
 *                           or all masters are locked.
 */
status_t MPU_UpdateRegion(mpu_instance_t instance,
                          uint8_t regionNum,
                          const mpu_region_config_t * configPtr);

/*!
 * @brief Enables or disables an exist region configuration.
 *
 * @param[in] instance The MPU instance number.
 * @param[in] regionNum The region number.
 * @param[in] enable Valid state
 *            - true  : Enable region.
 *            - false : Disable region.
 * @return operation status
 *        - STATUS_SUCCESS : Operation was successful.
 *        - STATUS_ERROR   : Operation failed due to the region was locked by another master
 *                           or all masters are locked.
 */
status_t MPU_EnableRegion(mpu_instance_t instance,
                          uint8_t regionNum,
                          bool enable);

/*!
 * @brief Checks and gets the access error detail information
 * then clear error flag if the error caused by a master.
 *
 * @param[in] instance The MPU instance number.
 * @param[in] channel The error capture channel
 *                    For MPU: corresponding with the slave port number
 *                    For SMPU: corresponding with the the master number
 * @param[out] errPtr The pointer to access error info structure, see #smpu_error_info_t.
 * @return operation status
 *        - true  : An error has occurred.
 *        - false : No error has occurred.
 */
bool MPU_GetError(mpu_instance_t instance,
                  uint8_t channel,
                  mpu_error_info_t * errPtr);

#if defined(MPU_OVER_SMPU)
/*!
 * @brief Updates lock configuration on a region.
 *
 * @param[in] instance The MPU instance number.
 * @param[in] regionNum The region number.
 * @param[in] lockConfig The lock configuration, see #mpu_lock_t.
 * @return operation status
 *        - STATUS_SUCCESS : Operation was successful.
 *        - STATUS_ERROR   : Operation failed due to the region was locked by another master
 *                           or all masters are locked.
 */
status_t MPU_UpdateRegionLock(mpu_instance_t instance,
                              uint8_t regionNum,
                              mpu_lock_t lockConfig);

/*!
 * @brief Reports the region lock status.
 *
 * @param[in] instance The MPU instance number.
 * @param[in] regionNum The region number.
 * @param[out] regionLock The pointer to region lock status, see mpu_region_lock_t.
 */
void MPU_GetRegionLockInfo(mpu_instance_t instance,
                           uint8_t regionNum,
                           mpu_region_lock_t * regionLock);
#endif /* defined(MPU_OVER_SMPU) */


/*! @} */

#if defined(__cplusplus)
}
#endif

/*! @} */

#endif /* MPU_PAL_H */
/*******************************************************************************
 * EOF
 ******************************************************************************/
