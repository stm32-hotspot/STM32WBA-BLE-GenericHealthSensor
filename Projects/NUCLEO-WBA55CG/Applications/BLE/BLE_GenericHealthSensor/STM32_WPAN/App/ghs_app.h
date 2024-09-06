/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    service1_app.h
  * @author  MCD Application Team
  * @brief   Header for service1_app.c
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef GHS_APP_H
#define GHS_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ghs_racp.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  GHS_CONN_HANDLE_EVT,
  GHS_DISCON_HANDLE_EVT,

  /* USER CODE BEGIN Service1_OpcodeNotificationEvt_t */

  /* USER CODE END Service1_OpcodeNotificationEvt_t */

  GHS_LAST_EVT,
} GHS_APP_OpcodeNotificationEvt_t;

typedef struct
{
  GHS_APP_OpcodeNotificationEvt_t          EvtOpcode;
  uint16_t                                 ConnectionHandle;

  /* USER CODE BEGIN GHS_APP_ConnHandleNotEvt_t */

  /* USER CODE END GHS_APP_ConnHandleNotEvt_t */
} GHS_APP_ConnHandleNotEvt_t;
/* USER CODE BEGIN ET */
#define MAX_RECORDS_COUNT                                                      5
/**
 * Enabled or Disable GHS debug or PTS testing
 */
#define BLE_CFG_GHS_DEBUG                                                      0
#define BLE_CFG_GHS_PTS                                                        0
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Exported macros -----------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void GHS_APP_Init(void);
void GHS_APP_EvtRx(GHS_APP_ConnHandleNotEvt_t *p_Notification);
/* USER CODE BEGIN EFP */
uint8_t GHS_APP_GetRACPCharacteristicIndicationEnabled(void);
uint8_t GHS_APP_GetGHSCPCharacteristicIndicationEnabled(void);
uint32_t GHS_APP_GetLHOTimerStarted(void);
void GHS_APP_UpdateFeature(void);
void GHS_APP_UpdateLiveHealthObservation(void);
tBleStatus GHS_APP_UpdateStoredHealthObservation(GHS_RACP_Record_t *pStoredHealthObservation);
void GHS_APP_UpdateObservationScheduleChanged(void);
void GHS_APP_PTS_TestDBInit(void);
void GHS_APP_UpdateDbRecordCount(uint8_t count);
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /*GHS_APP_H */
