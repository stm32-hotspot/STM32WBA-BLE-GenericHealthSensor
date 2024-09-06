/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    service4_app.c
  * @author  MCD Application Team
  * @brief   service4_app application definition.
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
#include "uds_app.h"
#include "uds.h"
#include "stm32_seq.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define MAX_SIZE_USER_DATA             0xFE

typedef struct 
{
  uint8_t size;
  char * p_string;
} UDS_APP_Char_String_t;

typedef struct 
{
  uint8_t user_index;
  uint16_t consent_code;
  UDS_APP_Char_String_t first_name;
  UDS_APP_Char_String_t last_name;
} UDSAPP_UserData_t;

typedef struct 
{
  uint32_t tick;
  uint16_t consent_code;
} UCP_Buffer_RegisterNewUser_t;

typedef struct 
{
  uint32_t tick;
  uint8_t user_index;
  uint16_t consent_code;
  uint8_t tries;
} UCP_Buffer_Consent_t;
/* USER CODE END PTD */

typedef enum
{
  Dci_NOTIFICATION_OFF,
  Dci_NOTIFICATION_ON,
  Ucp_INDICATION_OFF,
  Ucp_INDICATION_ON,
  /* USER CODE BEGIN Service4_APP_SendInformation_t */

  /* USER CODE END Service4_APP_SendInformation_t */
  UDS_APP_SENDINFORMATION_LAST
} UDS_APP_SendInformation_t;

typedef struct
{
  UDS_APP_SendInformation_t     Dci_Notification_Status;
  UDS_APP_SendInformation_t     Ucp_Indication_Status;
  /* USER CODE BEGIN Service4_APP_Context_t */
  UDSAPP_UserData_t user_data[MAX_SIZE_USER_DATA];

  /* buffer for User Control Point */
  UCP_Buffer_RegisterNewUser_t buf_register_new_user;
  UCP_Buffer_Consent_t buf_consent;

  UDS_APP_Char_String_t UDS_Char_FirstName;
  UDS_APP_Char_String_t UDS_Char_LastName;

  uint8_t user_data_access_permitted;

  UDS_ProcedureComplete_t UDS_ErrorMessage;

  uint32_t StartTick;
  
  uint32_t changeIncrement;
  
  uint8_t Procedure_In_Progress;
  /* USER CODE END Service4_APP_Context_t */
  uint16_t              ConnectionHandle;
} UDS_APP_Context_t;

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAX_CONSENT_CODE               0x270F /*9999*/
#define MAX_USER_NUMBER                (2)
#define INTERVAL_REGISTER_NEW_USER     (1000)  /**< 1s */
#define INTERVAL_CONSENT               INTERVAL_REGISTER_NEW_USER
#define INTERVAL_DELETE_USER_DATA      INTERVAL_REGISTER_NEW_USER

#define MAXIMUM_CONSENT_TRIES          3

/* USER CODE END PD */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Private macros ------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
static UDS_APP_Context_t UDS_APP_Context;

uint8_t a_UDS_UpdateCharData[247];

/* USER CODE BEGIN PV */
static char a_FirstName[] = { 'J', 'o', 'h', 'n' };
static char a_LastName[] = { 'M', 'o', 'o', 'd', 'y' };
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void UDS_Dci_SendNotification(void);
static void UDS_Ucp_SendIndication(void);

/* USER CODE BEGIN PFP */
static void UDSAPP_UserControlPoint_Error_Message(void);
static void UDS_App_Notif_UserControlPoint(UDS_App_Notification_UCP_t *data);

static void procedure_complete_error(uint8_t request_op_code, uint8_t error_code);
static void register_new_user(void);
static void consent(void);
static void delete_user_data(void);
static void mark_invalid_data(uint8_t index);
/* USER CODE END PFP */

/* Functions Definition ------------------------------------------------------*/
void UDS_Notification(UDS_NotificationEvt_t *p_Notification)
{
  /* USER CODE BEGIN Service4_Notification_1 */

  /* USER CODE END Service4_Notification_1 */
  switch(p_Notification->EvtOpcode)
  {
    /* USER CODE BEGIN Service4_Notification_Service4_EvtOpcode */
    case UDS_UCP_CONFIRMATION_EVT:
      {
        UDS_APP_Context.Procedure_In_Progress = 0;
      }
      break;
    /* USER CODE END Service4_Notification_Service4_EvtOpcode */

    case UDS_FIN_READ_EVT:
      /* USER CODE BEGIN Service4Char1_READ_EVT */

      /* USER CODE END Service4Char1_READ_EVT */
      break;

    case UDS_FIN_WRITE_EVT:
      /* USER CODE BEGIN Service4Char1_WRITE_EVT */

      /* USER CODE END Service4Char1_WRITE_EVT */
      break;

    case UDS_LANA_READ_EVT:
      /* USER CODE BEGIN Service4Char2_READ_EVT */

      /* USER CODE END Service4Char2_READ_EVT */
      break;

    case UDS_LANA_WRITE_EVT:
      /* USER CODE BEGIN Service4Char2_WRITE_EVT */

      /* USER CODE END Service4Char2_WRITE_EVT */
      break;

    case UDS_DCI_READ_EVT:
      /* USER CODE BEGIN Service4Char3_READ_EVT */

      /* USER CODE END Service4Char3_READ_EVT */
      break;

    case UDS_DCI_WRITE_EVT:
      /* USER CODE BEGIN Service4Char3_WRITE_EVT */

      /* USER CODE END Service4Char3_WRITE_EVT */
      break;

    case UDS_DCI_NOTIFY_ENABLED_EVT:
      /* USER CODE BEGIN Service4Char3_NOTIFY_ENABLED_EVT */
      UDS_APP_Context.Dci_Notification_Status = Dci_NOTIFICATION_ON;
      UDS_APP_Context.changeIncrement += 1;
      UDS_Dci_SendNotification();     /* DataBase Change Increment update */
      /* USER CODE END Service4Char3_NOTIFY_ENABLED_EVT */
      break;

    case UDS_DCI_NOTIFY_DISABLED_EVT:
      /* USER CODE BEGIN Service4Char3_NOTIFY_DISABLED_EVT */
      UDS_APP_Context.Dci_Notification_Status = Dci_NOTIFICATION_OFF;
      /* USER CODE END Service4Char3_NOTIFY_DISABLED_EVT */
      break;

    case UDS_USI_READ_EVT:
      /* USER CODE BEGIN Service4Char4_READ_EVT */

      /* USER CODE END Service4Char4_READ_EVT */
      break;

    case UDS_UCP_WRITE_EVT:
      /* USER CODE BEGIN Service4Char5_WRITE_EVT */
      {
        UDS_App_Notification_UCP_t notification;
        
        notification.op_code = p_Notification->DataTransfered.p_Payload[0];
        notification.parameter_length = p_Notification->DataTransfered.Length - 1;
        if(notification.parameter_length > 0)
        {
          memcpy((void*)notification.parameter, (void *)&(p_Notification->DataTransfered.p_Payload[1]), notification.parameter_length);
        }

        UDS_App_Notif_UserControlPoint(&notification);
      }
      /* USER CODE END Service4Char5_WRITE_EVT */
      break;

    case UDS_UCP_INDICATE_ENABLED_EVT:
      /* USER CODE BEGIN Service4Char5_INDICATE_ENABLED_EVT */
      UDS_APP_Context.Ucp_Indication_Status = Ucp_INDICATION_ON;
      /* USER CODE END Service4Char5_INDICATE_ENABLED_EVT */
      break;

    case UDS_UCP_INDICATE_DISABLED_EVT:
      /* USER CODE BEGIN Service4Char5_INDICATE_DISABLED_EVT */
      UDS_APP_Context.Ucp_Indication_Status = Ucp_INDICATION_OFF;
      /* USER CODE END Service4Char5_INDICATE_DISABLED_EVT */
      break;

    default:
      /* USER CODE BEGIN Service4_Notification_default */

      /* USER CODE END Service4_Notification_default */
      break;
  }
  /* USER CODE BEGIN Service4_Notification_2 */

  /* USER CODE END Service4_Notification_2 */
  return;
}

void UDS_APP_EvtRx(UDS_APP_ConnHandleNotEvt_t *p_Notification)
{
  /* USER CODE BEGIN Service4_APP_EvtRx_1 */

  /* USER CODE END Service4_APP_EvtRx_1 */

  switch(p_Notification->EvtOpcode)
  {
    /* USER CODE BEGIN Service4_APP_EvtRx_Service4_EvtOpcode */

    /* USER CODE END Service4_APP_EvtRx_Service4_EvtOpcode */
    case UDS_CONN_HANDLE_EVT :
      /* USER CODE BEGIN Service4_APP_CONN_HANDLE_EVT */

      /* USER CODE END Service4_APP_CONN_HANDLE_EVT */
      break;

    case UDS_DISCON_HANDLE_EVT :
      /* USER CODE BEGIN Service4_APP_DISCON_HANDLE_EVT */
      UDS_APP_Context.user_data_access_permitted = 0;  /* permitted access disable */
      /* USER CODE END Service4_APP_DISCON_HANDLE_EVT */
      break;

    default:
      /* USER CODE BEGIN Service4_APP_EvtRx_default */

      /* USER CODE END Service4_APP_EvtRx_default */
      break;
  }

  /* USER CODE BEGIN Service4_APP_EvtRx_2 */

  /* USER CODE END Service4_APP_EvtRx_2 */

  return;
}

void UDS_APP_Init(void)
{
  UNUSED(UDS_APP_Context);
  UDS_Init();

  /* USER CODE BEGIN Service4_APP_Init */
  APP_DBG_MSG("UDS_APP_Init\n\r");
  
  /*
   * Initialize Application Context
   */
  UDS_APP_Context.Ucp_Indication_Status = Ucp_INDICATION_OFF;  /* disable */
  UDS_APP_Context.user_data_access_permitted = 0; /* disable */
  UDS_APP_Context.UDS_Char_FirstName.p_string = a_FirstName;
  UDS_APP_Context.UDS_Char_FirstName.size = strlen(a_FirstName);
  UDS_APP_Context.UDS_Char_LastName.p_string = a_LastName;
  UDS_APP_Context.UDS_Char_LastName.size = strlen(a_LastName);
  UDS_APP_Context.StartTick = HAL_GetTick();
  UDS_APP_Context.changeIncrement = 0;
  UDS_APP_Context.Procedure_In_Progress = 0;
  
  /* Initialisation of user_data table */
  for(uint8_t i = 0; i < MAX_USER_NUMBER; i++)
  {
    mark_invalid_data(i);
  }

  /*
   * Register task for User Control Point
   */
  UTIL_SEQ_RegTask( 1<< CFG_TASK_UDS_CRL_MSG_ID, UTIL_SEQ_RFU, UDSAPP_UserControlPoint_Error_Message );
  UTIL_SEQ_RegTask( 1<< CFG_TASK_UDS_REG_NEW_ID, UTIL_SEQ_RFU, register_new_user );
  UTIL_SEQ_RegTask( 1<< CFG_TASK_UDS_CONSENT_ID, UTIL_SEQ_RFU, consent );
  UTIL_SEQ_RegTask( 1<< CFG_TASK_UDS_DEL_USER_ID, UTIL_SEQ_RFU, delete_user_data );
  /* USER CODE END Service4_APP_Init */
  return;
}

/* USER CODE BEGIN FD */
void UDS_App_SetAccessPermitted(void)
{
  UDS_APP_Context.user_data_access_permitted = 1;
}

uint8_t UDS_App_AccessPermitted(void)
{
  return UDS_APP_Context.user_data_access_permitted != 0;
}

uint8_t UDS_App_UcpCCCDEnabled(void)
{
  return UDS_APP_Context.Ucp_Indication_Status == Ucp_INDICATION_ON;
}

uint8_t UDS_App_GetUserID(void)
{
  return UDS_APP_Context.buf_consent.user_index;
}

uint8_t UDS_App_ProcedureInProgress(void)
{
  return UDS_APP_Context.Procedure_In_Progress;
}

/* USER CODE END FD */

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/
__USED void UDS_Dci_SendNotification(void) /* Property Notification */
{
  UDS_APP_SendInformation_t notification_on_off = Dci_NOTIFICATION_OFF;
  UDS_Data_t uds_notification_data;

  uds_notification_data.p_Payload = (uint8_t*)a_UDS_UpdateCharData;
  uds_notification_data.Length = 0;

  /* USER CODE BEGIN Service4Char3_NS_1*/
  notification_on_off = UDS_APP_Context.Dci_Notification_Status;
  a_UDS_UpdateCharData[(uds_notification_data.Length)++] = (UDS_APP_Context.changeIncrement) & 0xFF;
  a_UDS_UpdateCharData[(uds_notification_data.Length)++] = ((UDS_APP_Context.changeIncrement) >> 8) & 0xFF;
  a_UDS_UpdateCharData[(uds_notification_data.Length)++] = ((UDS_APP_Context.changeIncrement) >> 16) & 0xFF;
  a_UDS_UpdateCharData[(uds_notification_data.Length)++] = ((UDS_APP_Context.changeIncrement) >> 24) & 0xFF;
  /* USER CODE END Service4Char3_NS_1*/

  if (notification_on_off != Dci_NOTIFICATION_OFF)
  {
    UDS_UpdateValue(UDS_DCI, &uds_notification_data);
  }

  /* USER CODE BEGIN Service4Char3_NS_Last*/

  /* USER CODE END Service4Char3_NS_Last*/

  return;
}

__USED void UDS_Ucp_SendIndication(void) /* Property Indication */
{
  UDS_APP_SendInformation_t indication_on_off = Ucp_INDICATION_OFF;
  UDS_Data_t uds_indication_data;

  uds_indication_data.p_Payload = (uint8_t*)a_UDS_UpdateCharData;
  uds_indication_data.Length = 0;

  /* USER CODE BEGIN Service4Char5_IS_1*/

  /* USER CODE END Service4Char5_IS_1*/

  if (indication_on_off != Ucp_INDICATION_OFF)
  {
    UDS_UpdateValue(UDS_UCP, &uds_indication_data);
  }

  /* USER CODE BEGIN Service4Char5_IS_Last*/

  /* USER CODE END Service4Char5_IS_Last*/

  return;
}

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS*/
static void UDSAPP_UserControlPoint_Error_Message(void)
{
  UDS_Data_t msg_conf;
  uint8_t length = 0;

  UDS_APP_Context.UDS_ErrorMessage.ResponseCodeOpCode = UDS_UCP_OPCODE_RESPONSE_CODE;
  UDS_APP_Context.UDS_ErrorMessage.ResponseParameterLength = 0;

  a_UDS_UpdateCharData[length++] = UDS_APP_Context.UDS_ErrorMessage.ResponseCodeOpCode;
  a_UDS_UpdateCharData[length++] = UDS_APP_Context.UDS_ErrorMessage.RequestOpCode;
  a_UDS_UpdateCharData[length++] = UDS_APP_Context.UDS_ErrorMessage.ResponseValue;
  msg_conf.Length = length;
  msg_conf.p_Payload = a_UDS_UpdateCharData;
  UDS_UpdateValue(UDS_UCP, &msg_conf);
}

static void procedure_complete_error(uint8_t request_op_code, uint8_t error_code)
{
  UDS_APP_Context.UDS_ErrorMessage.ResponseValue = error_code;
  UDS_APP_Context.UDS_ErrorMessage.RequestOpCode = request_op_code;
  UTIL_SEQ_SetTask( 1<<CFG_TASK_UDS_CRL_MSG_ID, CFG_SEQ_PRIO_0);
}

static void register_new_user(void)
{
  uint16_t consent_code = UDS_APP_Context.buf_register_new_user.consent_code;
  uint8_t index = 0;
  UDS_ProcedureComplete_t response;
  UDS_Data_t msg_conf;
  uint8_t length = 0;

  APP_DBG_MSG("Register New User procedure [tick = %ld]", UDS_APP_Context.buf_register_new_user.tick);

  response.ResponseCodeOpCode = UDS_UCP_OPCODE_RESPONSE_CODE;
  response.RequestOpCode      = UDS_UCP_OPCODE_REGISTER_NEW_USER;

  if (consent_code > MAX_CONSENT_CODE) 
  {
    LOG_INFO_APP("UDS_RESPONSE_VALUE_INVALID_PARAMETER\n");
    procedure_complete_error(response.RequestOpCode, UDS_RESPONSE_VALUE_INVALID_PARAMETER);

    return;
  }

  while((index < MAX_USER_NUMBER) &&
        (UDS_APP_Context.user_data[index].user_index != UDS_USER_INDEX_UNKNOW))
  {
    index++;
  };

  if((index == MAX_USER_NUMBER) ||
     (UDS_APP_Context.user_data[index].consent_code <= MAX_CONSENT_CODE))
  {
    /* arrive the range of User Index */
    procedure_complete_error(response.RequestOpCode, UDS_RESPONSE_VALUE_OPERATION_FAILED);

    return;
  }

  /* procedure */
  UDS_APP_Context.user_data[index].user_index = index;
  UDS_APP_Context.user_data[index].consent_code = consent_code;
  UDS_APP_Context.user_data[index].first_name.p_string = NULL;
  UDS_APP_Context.user_data[index].first_name.p_string = NULL;
  UDS_APP_Context.user_data[index].last_name.size = 0;
  UDS_APP_Context.user_data[index].first_name.size = 0;
  
  /* reset the counter for consent tries */
  UDS_APP_Context.buf_consent.tries = 0;

  UDS_APP_Context.user_data_access_permitted = 0; /* disable */

  /* procedure complete message */
  response.ResponseValue = UDS_RESPONSE_VALUE_SUCCESS;
  response.ResponseParameter[0] = index;
  response.ResponseParameterLength = 1;

  a_UDS_UpdateCharData[length++] = response.ResponseCodeOpCode;
  a_UDS_UpdateCharData[length++] = response.RequestOpCode;
  a_UDS_UpdateCharData[length++] = response.ResponseValue;
  a_UDS_UpdateCharData[length++] = response.ResponseParameter[0];
  msg_conf.Length = length;
  msg_conf.p_Payload = a_UDS_UpdateCharData;
  UDS_UpdateValue(UDS_UCP, &msg_conf);

  if(index < 0xFF)
  {
    mark_invalid_data(index + 1);
  }
  UDS_APP_Context.Procedure_In_Progress = 1; /* wait for indication confirmation */

}

static void consent(void)
{
  uint16_t consent_code = UDS_APP_Context.buf_consent.consent_code;
  uint8_t user_index = UDS_APP_Context.buf_consent.user_index; /* User Index starts from 1 */
  UDS_ProcedureComplete_t response;
  UDS_Data_t msg_conf;
  uint8_t length = 0;

  APP_DBG_MSG("Consent procedure [tick = %ld]", UDS_APP_Context.buf_consent.tick);

  response.ResponseCodeOpCode = UDS_UCP_OPCODE_RESPONSE_CODE;
  response.RequestOpCode      = UDS_UCP_OPCODE_CONSENT;

  if (consent_code > MAX_CONSENT_CODE) 
  {
    procedure_complete_error(response.RequestOpCode, UDS_RESPONSE_VALUE_INVALID_PARAMETER);

    return;
  }

  if (user_index == MAX_USER_NUMBER)
  {
    LOG_INFO_APP("UDS_RESPONSE_VALUE_INVALID_PARAMETER, user_index %d\n",
                 user_index);
    procedure_complete_error(response.RequestOpCode, UDS_RESPONSE_VALUE_INVALID_PARAMETER);

    return;
  }

  if (UDS_APP_Context.user_data[user_index].consent_code != consent_code)
  {
    if(UDS_APP_Context.buf_consent.tries < MAXIMUM_CONSENT_TRIES)
    {
      procedure_complete_error(response.RequestOpCode, UDS_RESPONSE_VALUE_USER_NOT_AUTHORIZED);
    } 
    else 
    {
      procedure_complete_error(response.RequestOpCode, UDS_RESPONSE_VALUE_OPERATION_FAILED);
    }

    UDS_APP_Context.buf_consent.tries += 1;

    return;
  }

  /* procedure complete message */
  response.ResponseValue = UDS_RESPONSE_VALUE_SUCCESS;
  response.ResponseParameterLength = 0;

  a_UDS_UpdateCharData[length++] = response.ResponseCodeOpCode;
  a_UDS_UpdateCharData[length++] = response.RequestOpCode;
  a_UDS_UpdateCharData[length++] = response.ResponseValue;
  msg_conf.Length = length;
  msg_conf.p_Payload = a_UDS_UpdateCharData;
  UDS_UpdateValue(UDS_UCP, &msg_conf);

  /* populate User Index */
  length = 0;
  a_UDS_UpdateCharData[length++] = user_index;
  msg_conf.Length = length;
  msg_conf.p_Payload = a_UDS_UpdateCharData;
  UDS_UpdateValue(UDS_USI, &msg_conf);

  /* populate current User Data */
  if(UDS_APP_Context.user_data[user_index].first_name.p_string != NULL)
  {
    length = 0;
    for(uint8_t i = 0; 
        i < UDS_APP_Context.user_data[user_index].first_name.size;
        i++)
    {
      a_UDS_UpdateCharData[length++] = UDS_APP_Context.user_data[user_index].first_name.p_string[i];
    }

    msg_conf.Length = length;
    msg_conf.p_Payload = a_UDS_UpdateCharData;
    UDS_UpdateValue(UDS_FIN, &msg_conf);
  }
  if(UDS_APP_Context.user_data[user_index].last_name.p_string != NULL)
  {
    length = 0;
    for(uint8_t i = 0; 
        i < UDS_APP_Context.user_data[user_index].last_name.size;
        i++)
    {
      a_UDS_UpdateCharData[length++] = UDS_APP_Context.user_data[user_index].last_name.p_string[i];
    }

    msg_conf.Length = length;
    msg_conf.p_Payload = a_UDS_UpdateCharData;
    UDS_UpdateValue(UDS_LANA, &msg_conf);
  }
  UDS_APP_Context.user_data_access_permitted = 1; /* enable */
}

static void delete_user_data(void)
{
  uint8_t index;
  UDS_ProcedureComplete_t response;
  UDS_Data_t msg_conf;
  uint8_t length = 0;

  APP_DBG_MSG("Delete User Data procedure [tick = %ld]", UDS_APP_Context.buf_consent.tick);

  response.ResponseCodeOpCode = UDS_UCP_OPCODE_RESPONSE_CODE;
  response.RequestOpCode      = UDS_UCP_OPCODE_DELETE_USER_DATA;

  index = UDS_APP_Context.buf_consent.user_index;
  
  if (UDS_APP_Context.user_data[index].user_index == UDS_USER_INDEX_UNKNOW)
  {
    procedure_complete_error(response.RequestOpCode, UDS_RESPONSE_VALUE_USER_NOT_AUTHORIZED);

    return;
  }

  if (index == MAX_USER_NUMBER)
  {
    /* arrive the range of User Index */
    procedure_complete_error(response.RequestOpCode, UDS_RESPONSE_VALUE_OPERATION_FAILED);

    return;
  }

  /* mark current data as invalid */
  mark_invalid_data(index);

  UDS_APP_Context.user_data_access_permitted = 0; /* disable */

  /* procedure complete message */
  response.ResponseValue = UDS_RESPONSE_VALUE_SUCCESS;
  response.ResponseParameterLength = 0;

  a_UDS_UpdateCharData[length++] = response.ResponseCodeOpCode;
  a_UDS_UpdateCharData[length++] = response.RequestOpCode;
  a_UDS_UpdateCharData[length++] = response.ResponseValue;
  msg_conf.Length = length;
  msg_conf.p_Payload = a_UDS_UpdateCharData;
  UDS_UpdateValue(UDS_UCP, &msg_conf);
}

static void mark_invalid_data(uint8_t index)
{
  /* mark current data as invalid */
  UDS_APP_Context.user_data[index].user_index = UDS_USER_INDEX_UNKNOW;
  UDS_APP_Context.user_data[index].consent_code = (MAX_CONSENT_CODE + 1);
  UDS_APP_Context.user_data[index].first_name.p_string = NULL;
  UDS_APP_Context.user_data[index].first_name.size = 0;
  UDS_APP_Context.user_data[index].last_name.p_string = NULL;
  UDS_APP_Context.user_data[index].last_name.size = 0;
}

static void UDS_App_Notif_UserControlPoint(UDS_App_Notification_UCP_t *data)
{
  uint32_t tick = HAL_GetTick();
  APP_DBG_MSG("[enabled = %d] UDS_App_Notif_UserControlPoint, code = %d (tick = %ld)\n\r",
              UDS_APP_Context.Ucp_Indication_Status, data->op_code, tick);

  if(UDS_APP_Context.Ucp_Indication_Status == Ucp_INDICATION_OFF)
  {
    return;
  }
  
  switch(data->op_code)
  {
    case 0:
      {
        /* Reserved for future use */
        procedure_complete_error(0, UDS_RESPONSE_VALUE_OP_CODE_NOT_SUPPORTED);
      }
      break;
      
    case UDS_UCP_OPCODE_REGISTER_NEW_USER:
      {
        /* Register New User */
        LOG_INFO_APP("UDS_UCP_OPCODE_REGISTER_NEW_USER\n");
        UDS_APP_Context.buf_register_new_user.tick = tick;
        UDS_APP_Context.buf_register_new_user.consent_code = *((uint16_t*)data->parameter);
        UTIL_SEQ_SetTask( 1<<CFG_TASK_UDS_REG_NEW_ID, CFG_SEQ_PRIO_0);
      }
      break;
      
    case UDS_UCP_OPCODE_CONSENT:
      {
        /* Consent */
        UDS_APP_Context.buf_consent.tick = tick;
        UDS_APP_Context.buf_consent.user_index = data->parameter[0];
        UDS_APP_Context.buf_consent.consent_code = *((uint16_t*)(data->parameter + 1));
        LOG_INFO_APP("UDS_UCP_OPCODE_CONSENT for index: %d\n", UDS_APP_Context.buf_consent.user_index);

        UTIL_SEQ_SetTask( 1<<CFG_TASK_UDS_CONSENT_ID, CFG_SEQ_PRIO_0);
      }
      break;
      
    case UDS_UCP_OPCODE_DELETE_USER_DATA:
      {
        LOG_INFO_APP("UDS_UCP_OPCODE_DELETE_USER_DATA index: %d\n", UDS_APP_Context.buf_consent.user_index);
        /* Delete User Data */
        UTIL_SEQ_SetTask( 1<<CFG_TASK_UDS_DEL_USER_ID, CFG_SEQ_PRIO_0);
      }
      break;
      
    case UDS_UCP_OPCODE_LIST_ALL_USERS:
      {
        /* List All Users */
        procedure_complete_error(UDS_UCP_OPCODE_LIST_ALL_USERS, UDS_RESPONSE_VALUE_OP_CODE_NOT_SUPPORTED);
      }
      break;
      
    case UDS_UCP_OPCODE_DELETE_USER:
      {
        /* Delete User(s) */
        procedure_complete_error(UDS_UCP_OPCODE_DELETE_USER, UDS_RESPONSE_VALUE_OP_CODE_NOT_SUPPORTED);
      }
      break;
      
    default:
      {
        procedure_complete_error(data->op_code, UDS_RESPONSE_VALUE_OP_CODE_NOT_SUPPORTED);
      }
      break;
  }

  return;
}
/* USER CODE END FD_LOCAL_FUNCTIONS*/
