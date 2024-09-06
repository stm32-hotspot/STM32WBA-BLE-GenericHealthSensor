/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    service6_app.c
  * @author  MCD Application Team
  * @brief   service6_app application definition.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_common.h"
#include "app_ble.h"
#include "ll_sys_if.h"
#include "dbg_trace.h"
#include "ble.h"
#include "rcs_app.h"
#include "rcs.h"
#include "stm32_seq.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_ble.h"
#include "crc_ctrl.h"
#include "crc_ctrl_conf.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

typedef enum
{
  Rcf_INDICATION_OFF,
  Rcf_INDICATION_ON,
  /* USER CODE BEGIN Service6_APP_SendInformation_t */

  /* USER CODE END Service6_APP_SendInformation_t */
  RCS_APP_SENDINFORMATION_LAST
} RCS_APP_SendInformation_t;

typedef struct
{
  RCS_APP_SendInformation_t     Rcf_Indication_Status;
  /* USER CODE BEGIN Service6_APP_Context_t */
  RCS_FEATURE_t RCS_APP_Feature;
  /* USER CODE END Service6_APP_Context_t */
  uint16_t              ConnectionHandle;
} RCS_APP_Context_t;

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */
CRCCTRL_Handle_t RC_Handle;
/* USER CODE END PD */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */
extern CRC_HandleTypeDef hcrc;
extern CurrentGapParam_t StoredValues;
extern CurrentGapParam_t CurrentGapPram;
/* USER CODE END EV */

/* Private macros ------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
static RCS_APP_Context_t RCS_APP_Context;

uint8_t a_RCS_UpdateCharData[247];

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void RCS_Rcf_SendIndication(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Functions Definition ------------------------------------------------------*/
void RCS_Notification(RCS_NotificationEvt_t *p_Notification)
{
  /* USER CODE BEGIN Service6_Notification_1 */

  /* USER CODE END Service6_Notification_1 */
  switch(p_Notification->EvtOpcode)
  {
    /* USER CODE BEGIN Service6_Notification_Service6_EvtOpcode */

    /* USER CODE END Service6_Notification_Service6_EvtOpcode */

    case RCS_RCF_READ_EVT:
      /* USER CODE BEGIN Service6Char1_READ_EVT */

      /* USER CODE END Service6Char1_READ_EVT */
      break;

    case RCS_RCF_INDICATE_ENABLED_EVT:
      /* USER CODE BEGIN Service6Char1_INDICATE_ENABLED_EVT */
      LOG_INFO_APP("RCS_RCF_INDICATE_ENABLED_EVT\r\n");
      RCS_APP_Context.Rcf_Indication_Status = Rcf_INDICATION_ON;
      /* USER CODE END Service6Char1_INDICATE_ENABLED_EVT */
      break;

    case RCS_RCF_INDICATE_DISABLED_EVT:
      /* USER CODE BEGIN Service6Char1_INDICATE_DISABLED_EVT */
      LOG_INFO_APP("RCS_RCF_INDICATE_DISABLED_EVT\r\n"); 
      RCS_APP_Context.Rcf_Indication_Status = Rcf_INDICATION_OFF;
      /* USER CODE END Service6Char1_INDICATE_DISABLED_EVT */
      break;

    default:
      /* USER CODE BEGIN Service6_Notification_default */

      /* USER CODE END Service6_Notification_default */
      break;
  }
  /* USER CODE BEGIN Service6_Notification_2 */

  /* USER CODE END Service6_Notification_2 */
  return;
}

void RCS_APP_EvtRx(RCS_APP_ConnHandleNotEvt_t *p_Notification)
{
  /* USER CODE BEGIN Service6_APP_EvtRx_1 */

  /* USER CODE END Service6_APP_EvtRx_1 */

  switch(p_Notification->EvtOpcode)
  {
    /* USER CODE BEGIN Service6_APP_EvtRx_Service6_EvtOpcode */

    /* USER CODE END Service6_APP_EvtRx_Service6_EvtOpcode */
    case RCS_CONN_HANDLE_EVT :
      /* USER CODE BEGIN Service6_APP_CONN_HANDLE_EVT */
      RCS_APP_Context.ConnectionHandle = p_Notification->ConnectionHandle;
      /* USER CODE END Service6_APP_CONN_HANDLE_EVT */
      break;

    case RCS_DISCON_HANDLE_EVT :
      /* USER CODE BEGIN Service6_APP_DISCON_HANDLE_EVT */

      /* USER CODE END Service6_APP_DISCON_HANDLE_EVT */
      break;

    default:
      /* USER CODE BEGIN Service6_APP_EvtRx_default */

      /* USER CODE END Service6_APP_EvtRx_default */
      break;
  }

  /* USER CODE BEGIN Service6_APP_EvtRx_2 */

  /* USER CODE END Service6_APP_EvtRx_2 */

  return;
}

void RCS_APP_Init(void)
{
  UNUSED(RCS_APP_Context);
  RCS_Init();

  /* USER CODE BEGIN Service6_APP_Init */
  CRCCTRL_Cmd_Status_t result;
  RCS_Data_t msg_conf;
  uint8_t length;
  
  /* RC CRC initializations */
  RC_Handle.Uid = 0x00;
  RC_Handle.PreviousComputedValue = 0x00;
  RC_Handle.State = HANDLE_NOT_REG;
  RC_Handle.Configuration.DefaultPolynomialUse = hcrc.Init.DefaultPolynomialUse;
  RC_Handle.Configuration.DefaultInitValueUse = hcrc.Init.DefaultInitValueUse;
  RC_Handle.Configuration.GeneratingPolynomial = hcrc.Init.GeneratingPolynomial;
  RC_Handle.Configuration.CRCLength = hcrc.Init.CRCLength;
  RC_Handle.Configuration.InputDataInversionMode = hcrc.Init.InputDataInversionMode;
  RC_Handle.Configuration.OutputDataInversionMode = hcrc.Init.OutputDataInversionMode;
  RC_Handle.Configuration.InputDataFormat = hcrc.InputDataFormat;

  /* Register CRC Handle */
  result = CRCCTRL_RegisterHandle(&RC_Handle);
  if (result != CRCCTRL_OK)
  {
    Error_Handler();
  }
  
  /* RC Feature initialization */
  RCS_APP_Context.RCS_APP_Feature.RCFeature = E2E_CRC_SUPPORTED;                       
  length = 2;
  a_RCS_UpdateCharData[length] = (RCS_APP_Context.RCS_APP_Feature.RCFeature) & 0xFF;
  length++;
  a_RCS_UpdateCharData[length] = ((RCS_APP_Context.RCS_APP_Feature.RCFeature) >> 8) & 0xFF;
  length++;
  a_RCS_UpdateCharData[length] = ((RCS_APP_Context.RCS_APP_Feature.RCFeature) >> 16) & 0xFF;
  length++;
  if ((RCS_APP_Context.RCS_APP_Feature.RCFeature & E2E_CRC_SUPPORTED) == E2E_CRC_SUPPORTED)
  {
    uint16_t CRCValue;
    
    CRCValue = RCS_APP_ComputeCRC(&a_RCS_UpdateCharData[2], length -2);
    a_RCS_UpdateCharData[0] = (uint8_t)(CRCValue & 0xFF);
    a_RCS_UpdateCharData[1] = (uint8_t)((CRCValue >> 8) & 0xFF);
  }
  else
  {
    uint16_t CRCValue = 0xFFFF;
    
    a_RCS_UpdateCharData[0] = (uint8_t)(CRCValue & 0xFF);
    a_RCS_UpdateCharData[1] = (uint8_t)((CRCValue >> 8) & 0xFF);
  } 
  msg_conf.Length = length;
  msg_conf.p_Payload = a_RCS_UpdateCharData;
  RCS_UpdateValue(RCS_RCF, &msg_conf);
  /* USER CODE END Service6_APP_Init */
  return;
}

/* USER CODE BEGIN FD */
uint32_t RCS_APP_ComputeCRC(uint8_t * pData, uint8_t dataLength)
{
  CRCCTRL_Cmd_Status_t result;
  uint32_t crcValue;
  
  result = CRCCTRL_Calculate (&RC_Handle,
                              (uint32_t *)pData,
                              dataLength,
                              &crcValue);
  
  if (result != CRCCTRL_OK)
  {
    Error_Handler();
  }
  return crcValue;
}

void RCS_APP_UpdateFeature(void)
{
  RCS_Data_t msg_conf;
  uint8_t length;

  RCS_APP_Context.RCS_APP_Feature.RCFeature = E2E_CRC_SUPPORTED;                       
  length = 2;
  a_RCS_UpdateCharData[length] = (RCS_APP_Context.RCS_APP_Feature.RCFeature) & 0xFF;
  length++;
  a_RCS_UpdateCharData[length] = ((RCS_APP_Context.RCS_APP_Feature.RCFeature) >> 8) & 0xFF;
  length++;
  a_RCS_UpdateCharData[length] = ((RCS_APP_Context.RCS_APP_Feature.RCFeature) >> 16) & 0xFF;
  length++;
  if ((RCS_APP_Context.RCS_APP_Feature.RCFeature & E2E_CRC_SUPPORTED) == E2E_CRC_SUPPORTED)
  {
    uint16_t CRCValue;
    
    CRCValue = RCS_APP_ComputeCRC(&a_RCS_UpdateCharData[2], length -2);
    a_RCS_UpdateCharData[0] = (uint8_t)(CRCValue & 0xFF);
    a_RCS_UpdateCharData[1] = (uint8_t)((CRCValue >> 8) & 0xFF);
  }
  else
  {
    uint16_t CRCValue = 0xFFFF;
    
    a_RCS_UpdateCharData[0] = (uint8_t)(CRCValue & 0xFF);
    a_RCS_UpdateCharData[1] = (uint8_t)((CRCValue >> 8) & 0xFF);
  } 
   
  msg_conf.Length = length;
  msg_conf.p_Payload = a_RCS_UpdateCharData;
  RCS_UpdateValue(RCS_RCF, &msg_conf);
}

uint8_t RCS_APP_IsE2ECRCSupported(void)
{
  return (((RCS_APP_Context.RCS_APP_Feature.RCFeature & E2E_CRC_SUPPORTED) == E2E_CRC_SUPPORTED) ? TRUE: FALSE);
} /* end of RCS_APP_IsE2ECRCSupported() */

uint16_t RCS_APP_GetConnectionHandle(void)
{
  return (RCS_APP_Context.ConnectionHandle);
} /* end of RCS_APP_GetConnectionHandle() */

/* USER CODE END FD */

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/
__USED void RCS_Rcf_SendIndication(void) /* Property Indication */
{
  RCS_APP_SendInformation_t indication_on_off = Rcf_INDICATION_OFF;
  RCS_Data_t rcs_indication_data;

  rcs_indication_data.p_Payload = (uint8_t*)a_RCS_UpdateCharData;
  rcs_indication_data.Length = 0;

  /* USER CODE BEGIN Service6Char1_IS_1*/

  /* USER CODE END Service6Char1_IS_1*/

  if (indication_on_off != Rcf_INDICATION_OFF)
  {
    RCS_UpdateValue(RCS_RCF, &rcs_indication_data);
  }

  /* USER CODE BEGIN Service6Char1_IS_Last*/

  /* USER CODE END Service6Char1_IS_Last*/

  return;
}

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS*/

/* USER CODE END FD_LOCAL_FUNCTIONS*/
