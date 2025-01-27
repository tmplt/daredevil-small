/* ###################################################################
**     This component module is generated by Processor Expert. Do not modify it.
**     Filename    : security_pal1.h
**     Project     : security_pal_s32k144
**     Processor   : S32K144_100
**     Component   : security_pal
**     Version     : Component SDK_S32K14x_09, Driver 01.00, CPU db: 3.00.000
**     Repository  : SDK_S32K14x_09
**     Compiler    : GNU C Compiler
**     Date/Time   : 2019-02-19, 13:30, # CodeGen: 0
**     Contents    :
**         SECURITY_Init                - status_t SECURITY_Init(security_instance_t instance, security_user_config_t *...
**         SECURITY_Deinit              - status_t SECURITY_Deinit(security_instance_t instance);
**         SECURITY_EncryptEcbBlocking  - status_t SECURITY_EncryptEcbBlocking(security_instance_t instance,...
**         SECURITY_EncryptEcb          - status_t SECURITY_EncryptEcb(security_instance_t instance, security_key_id_t...
**         SECURITY_DecryptEcbBlocking  - status_t SECURITY_DecryptEcbBlocking(security_instance_t instance,...
**         SECURITY_DecryptEcb          - status_t SECURITY_DecryptEcb(security_instance_t instance, security_key_id_t...
**         SECURITY_EncryptCbcBlocking  - status_t SECURITY_EncryptCbcBlocking(security_instance_t instance,...
**         SECURITY_EncryptCbc          - status_t SECURITY_EncryptCbc(security_instance_t instance, security_key_id_t...
**         SECURITY_DecryptCbcBlocking  - status_t SECURITY_DecryptCbcBlocking(security_instance_t instance,...
**         SECURITY_DecryptCbc          - status_t SECURITY_DecryptCbc(security_instance_t instance, security_key_id_t...
**         SECURITY_GenerateMacBlocking - status_t SECURITY_GenerateMacBlocking(security_instance_t instance,...
**         SECURITY_GenerateMac         - status_t SECURITY_GenerateMac(security_instance_t instance, security_key_id_t...
**         SECURITY_VerifyMacBlocking   - status_t SECURITY_VerifyMacBlocking(security_instance_t instance,...
**         SECURITY_VerifyMac           - status_t SECURITY_VerifyMac(security_instance_t instance, security_key_id_t...
**         SECURITY_LoadKey             - status_t SECURITY_LoadKey(security_instance_t instance, security_key_id_t...
**         SECURITY_LoadPlainKey        - status_t SECURITY_LoadPlainKey(security_instance_t instance const uint8_t *...
**         SECURITY_ExportRAMKey        - status_t SECURITY_ExportRAMKey(security_instance_t instance, uint8_t * m1,...
**         SECURITY_InitRNG             - status_t SECURITY_InitRNG(security_instance_t instance, uint32_t timeout);
**         SECURITY_ExtendSeed          - status_t SECURITY_ExtendSeed(security_instance_t instance, const uint8_t *...
**         SECURITY_GenerateRND         - status_t SECURITY_GenerateRND(security_instance_t instance, uint8_t *rnd,...
**         SECURITY_GetID               - status_t SECURITY_GetID(security_instance_t instance, const uint8_t *...
**         SECURITY_GenerateTrnd        - status_t SECURITY_GenerateTrnd(security_instance_t instance, uint8_t *trnd,...
**         SECURITY_SecureBoot          - status_t SECURITY_SecureBoot(security_instance_t instance, uint32_t...
**         SECURITY_BootFailure         - status_t SECURITY_BootFailure(security_instance_t instance, uint32_t timeout);
**         SECURITY_BootOk              - status_t SECURITY_BootOk(security_instance_t instance, uint32_t timeout);
**         SECURITY_DbgChal             - status_t SECURITY_DbgChal(security_instance_t instance, uint8_t *challenge,...
**         SECURITY_DbgAuth             - status_t SECURITY_DbgAuth(security_instance_t instance, const uint8_t...
**         SECURITY_MPCompress          - status_t SECURITY_MPCompress(security_instance_t instance, const uint8_t...
**         SECURITY_GetAsyncCmdStatus   - status_t SECURITY_GetAsyncCmdStatus(security_instance_t instance);
**         SECURITY_CancelCommand       - status_t SECURITY_CancelCommand(security_instance_t instance);
**
**     Copyright 1997 - 2015 Freescale Semiconductor, Inc.
**     Copyright 2016-2017 NXP
**     All Rights Reserved.
**     
**     THIS SOFTWARE IS PROVIDED BY NXP "AS IS" AND ANY EXPRESSED OR
**     IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
**     OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
**     IN NO EVENT SHALL NXP OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
**     INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
**     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
**     SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
**     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
**     STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
**     IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
**     THE POSSIBILITY OF SUCH DAMAGE.
** ###################################################################*/
/*!
** @file security_pal1.h
** @version 01.00
*/
/*!
**  @addtogroup security_pal1_module security_pal1 module documentation
**  @{
*/
#ifndef security_pal1_H
#define security_pal1_H
/* MODULE security_pal1. */

/* Include inherited beans */
#include "Cpu.h"

#endif
/* ifndef security_pal1_H */

/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.1 [05.21]
**     for the Freescale S32K series of microcontrollers.
**
** ###################################################################
*/
