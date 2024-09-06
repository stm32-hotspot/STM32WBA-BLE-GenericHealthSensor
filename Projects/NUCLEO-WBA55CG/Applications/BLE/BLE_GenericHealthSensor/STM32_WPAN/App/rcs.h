/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    service6.h
  * @author  MCD Application Team
  * @brief   Header for service6.c
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
#ifndef RCS_H
#define RCS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported defines ----------------------------------------------------------*/
/* USER CODE BEGIN ED */

/* USER CODE END ED */

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  RCS_RCF,
  /* USER CODE BEGIN Service6_CharOpcode_t */

  /* USER CODE END Service6_CharOpcode_t */
  RCS_CHAROPCODE_LAST
} RCS_CharOpcode_t;

typedef enum
{
  RCS_RCF_READ_EVT,
  RCS_RCF_INDICATE_ENABLED_EVT,
  RCS_RCF_INDICATE_DISABLED_EVT,
  /* USER CODE BEGIN Service6_OpcodeEvt_t */

  /* USER CODE END Service6_OpcodeEvt_t */
  RCS_BOOT_REQUEST_EVT
} RCS_OpcodeEvt_t;

typedef struct
{
  uint8_t *p_Payload;
  uint8_t Length;

  /* USER CODE BEGIN Service6_Data_t */

  /* USER CODE END Service6_Data_t */
} RCS_Data_t;

typedef struct
{
  RCS_OpcodeEvt_t       EvtOpcode;
  RCS_Data_t             DataTransfered;
  uint16_t                ConnectionHandle;
  uint16_t                AttributeHandle;
  uint8_t                 ServiceInstance;
  /* USER CODE BEGIN Service6_NotificationEvt_t */

  /* USER CODE END Service6_NotificationEvt_t */
} RCS_NotificationEvt_t;

/* USER CODE BEGIN ET */
typedef enum
{
  E2E_CRC_SUPPORTED                               = (1<<0),
  ENABLED_DISCONNECT_SUPPORTED                    = (1<<1),
  READY_FOR_DISCONNECT_SUPPORTED                  = (1<<2),
  PROPOSE_RECONNECTION_TIMEOUT_SUPPORTED          = (1<<3),
  PROPOSE_CONNECTION_INTERVAL_SUPPORTED           = (1<<4),
  PROPOSE_PERIPHERAL_LATENCY_SUPPORTED            = (1<<5),
  PROPOSE_SUPERVISION_TIMEOUT_SUPPORTED           = (1<<6),
  PROPOSE_ADVERTISEMENT_INTERVAL_SUPPORTED        = (1<<7),
  PROPOSE_ADVERTISEMENT_COUNT_SUPPORTED           = (1<<8),
  PROPOSE_ADVERTISEMENT_REPETITION_TIME_SUPPORTED = (1<<9),
  ADVERTISING_CONFIGURATION_1_SUPPORTED           = (1<<10),
  ADVERTISING_CONFIGURATION_2_SUPPORTED           = (1<<11),
  ADVERTISING_CONFIGURATION_3_SUPPORTED           = (1<<12),
  ADVERTISING_CONFIGURATION_4_SUPPORTED           = (1<<13),
  UPGRADE_TO_LESC_ONLY_SUPPORTED                  = (1<<14),
  NEXT_PAIRING_OOB_SUPPORTED                      = (1<<15),
  USE_A_FILTER_ACCEPT_LIST_SUPPORTED              = (1<<16),
  LIMITED_ACCESS_SUPPORTED                        = (1<<17),
  FEATURE_EXTENSION                               = (1<<23)
} RC_Feature_t;

typedef enum
{
  LESC_ONLY                 = (1<<1),
  USE_OOB_PAIRING           = (1<<2),
  READY_FOR_DISCONNECT      = (1<<4),
  LIMITED_ACCESS            = (1<<5),
  ACCESS_PERMITTED          = (1<<6),
  RC_ADV_IND                = (0<<8),
  RC_ADV_SCAN_IND           = (1<<8),
  RC_ADV_NONCONN_IND        = (2<<8),
  RC_ADV_DIRECT_IND         = (3<<8)
} RC_Settings_t;

typedef struct
{
  uint16_t E2E_CRC;
  uint32_t RCFeature;
}RCS_FEATURE_t;

typedef struct
{
  uint16_t E2E_CRC;
  uint8_t Length;
  uint16_t Settings;
}RCS_SETTINGS_t;

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
void RCS_Init(void);
void RCS_Notification(RCS_NotificationEvt_t *p_Notification);
tBleStatus RCS_UpdateValue(RCS_CharOpcode_t CharOpcode, RCS_Data_t *pData);
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /*RCS_H */
