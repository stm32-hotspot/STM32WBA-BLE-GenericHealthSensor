/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    service6_app.h
  * @author  MCD Application Team
  * @brief   Header for service6_app.c
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
#ifndef RCS_APP_H
#define RCS_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  RCS_CONN_HANDLE_EVT,
  RCS_DISCON_HANDLE_EVT,

  /* USER CODE BEGIN Service6_OpcodeNotificationEvt_t */

  /* USER CODE END Service6_OpcodeNotificationEvt_t */

  RCS_LAST_EVT,
} RCS_APP_OpcodeNotificationEvt_t;

typedef struct
{
  RCS_APP_OpcodeNotificationEvt_t          EvtOpcode;
  uint16_t                                 ConnectionHandle;

  /* USER CODE BEGIN RCS_APP_ConnHandleNotEvt_t */

  /* USER CODE END RCS_APP_ConnHandleNotEvt_t */
} RCS_APP_ConnHandleNotEvt_t;
/* USER CODE BEGIN ET */

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
void RCS_APP_Init(void);
void RCS_APP_EvtRx(RCS_APP_ConnHandleNotEvt_t *p_Notification);
/* USER CODE BEGIN EFP */
uint32_t RCS_APP_ComputeCRC(uint8_t * pData, uint8_t dataLength);
void RCS_APP_UpdateFeature(void);
uint8_t RCS_APP_IsE2ECRCSupported(void);
uint16_t RCS_APP_GetConnectionHandle(void);
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /*RCS_APP_H */
