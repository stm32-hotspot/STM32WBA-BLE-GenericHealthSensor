/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    service4_app.h
  * @author  MCD Application Team
  * @brief   Header for service4_app.c
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
#ifndef UDS_APP_H
#define UDS_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  UDS_CONN_HANDLE_EVT,
  UDS_DISCON_HANDLE_EVT,

  /* USER CODE BEGIN Service4_OpcodeNotificationEvt_t */

  /* USER CODE END Service4_OpcodeNotificationEvt_t */

  UDS_LAST_EVT,
} UDS_APP_OpcodeNotificationEvt_t;

typedef struct
{
  UDS_APP_OpcodeNotificationEvt_t          EvtOpcode;
  uint16_t                                 ConnectionHandle;

  /* USER CODE BEGIN UDS_APP_ConnHandleNotEvt_t */

  /* USER CODE END UDS_APP_ConnHandleNotEvt_t */
} UDS_APP_ConnHandleNotEvt_t;
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
void UDS_APP_Init(void);
void UDS_APP_EvtRx(UDS_APP_ConnHandleNotEvt_t *p_Notification);
/* USER CODE BEGIN EFP */
void UDS_App_SetAccessPermitted(void);
uint8_t UDS_App_AccessPermitted(void);
uint8_t UDS_App_UcpCCCDEnabled(void);
uint8_t UDS_App_GetUserID(void);
uint8_t UDS_App_ProcedureInProgress(void);
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /*UDS_APP_H */
