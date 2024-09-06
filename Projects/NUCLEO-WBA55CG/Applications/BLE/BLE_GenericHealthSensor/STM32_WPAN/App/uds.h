/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    service4.h
  * @author  MCD Application Team
  * @brief   Header for service4.c
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
#ifndef UDS_H
#define UDS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported defines ----------------------------------------------------------*/
/* USER CODE BEGIN ED */
#define UDS_USER_CONTROL_POINT_MAX_PARAMETER           8
#define UDS_PROCEDURE_COMPLETE_MAX_PARAMETER           18
#define UDS_APPLICATION_ERROR_CODE                     0x80
#define UDS_USER_INDEX_UNKNOW                          0xFF
/* USER CODE END ED */

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  UDS_FIN,
  UDS_LANA,
  UDS_DCI,
  UDS_USI,
  UDS_UCP,
  /* USER CODE BEGIN Service4_CharOpcode_t */

  /* USER CODE END Service4_CharOpcode_t */
  UDS_CHAROPCODE_LAST
} UDS_CharOpcode_t;

typedef enum
{
  UDS_FIN_READ_EVT,
  UDS_FIN_WRITE_EVT,
  UDS_LANA_READ_EVT,
  UDS_LANA_WRITE_EVT,
  UDS_DCI_READ_EVT,
  UDS_DCI_WRITE_EVT,
  UDS_DCI_NOTIFY_ENABLED_EVT,
  UDS_DCI_NOTIFY_DISABLED_EVT,
  UDS_USI_READ_EVT,
  UDS_UCP_WRITE_EVT,
  UDS_UCP_INDICATE_ENABLED_EVT,
  UDS_UCP_INDICATE_DISABLED_EVT,
  /* USER CODE BEGIN Service4_OpcodeEvt_t */
  UDS_UCP_CONFIRMATION_EVT,
  /* USER CODE END Service4_OpcodeEvt_t */
  UDS_BOOT_REQUEST_EVT
} UDS_OpcodeEvt_t;

typedef struct
{
  uint8_t *p_Payload;
  uint8_t Length;

  /* USER CODE BEGIN Service4_Data_t */

  /* USER CODE END Service4_Data_t */
} UDS_Data_t;

typedef struct
{
  UDS_OpcodeEvt_t       EvtOpcode;
  UDS_Data_t             DataTransfered;
  uint16_t                ConnectionHandle;
  uint16_t                AttributeHandle;
  uint8_t                 ServiceInstance;
  /* USER CODE BEGIN Service4_NotificationEvt_t */

  /* USER CODE END Service4_NotificationEvt_t */
} UDS_NotificationEvt_t;

/* USER CODE BEGIN ET */
typedef enum 
{
  UDS_ERROR_CODE_UserDataAccessNotPermitted = 0x80,
  UDS_ERROR_CODE_CCCDImproperlyCconfigured = 0xFD,
  UDS_ERROR_CODE_ProcesdureInProgress = 0xFE
} UDS_ErrorCode_t;

typedef struct
{
  uint8_t op_code;
  uint8_t parameter_length;
  uint8_t parameter[UDS_USER_CONTROL_POINT_MAX_PARAMETER];
}UDS_App_Notification_UCP_t;

typedef enum 
{
  UDS_UCP_OPCODE_REGISTER_NEW_USER = 1,
  UDS_UCP_OPCODE_CONSENT = 2,
  UDS_UCP_OPCODE_DELETE_USER_DATA = 3,
  UDS_UCP_OPCODE_LIST_ALL_USERS = 4,
  UDS_UCP_OPCODE_DELETE_USER = 5,
  UDS_UCP_OPCODE_RESPONSE_CODE = 0x20,
} UDS_UserControlPoint_OpCode_t;

typedef struct 
{
  uint8_t ResponseCodeOpCode;
  uint8_t RequestOpCode;
  uint8_t ResponseValue;
  uint8_t ResponseParameter[UDS_PROCEDURE_COMPLETE_MAX_PARAMETER];
  uint8_t ResponseParameterLength;
} UDS_ProcedureComplete_t;

typedef enum 
{
  UDS_RESPONSE_VALUE_SUCCESS = 1,
  UDS_RESPONSE_VALUE_OP_CODE_NOT_SUPPORTED = 2,
  UDS_RESPONSE_VALUE_INVALID_PARAMETER = 3,
  UDS_RESPONSE_VALUE_OPERATION_FAILED = 4,
  UDS_RESPONSE_VALUE_USER_NOT_AUTHORIZED = 5
} UDS_ProcedureComplete_ResponseValue_t;
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
void UDS_Init(void);
void UDS_Notification(UDS_NotificationEvt_t *p_Notification);
tBleStatus UDS_UpdateValue(UDS_CharOpcode_t CharOpcode, UDS_Data_t *pData);
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /*UDS_H */
