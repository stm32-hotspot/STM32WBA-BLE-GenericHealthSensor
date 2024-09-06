/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    service1_app.c
  * @author  MCD Application Team
  * @brief   service1_app application definition.
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
#include "ghs_app.h"
#include "ghs.h"
#include "stm32_seq.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ghs_db.h"
#include "ghs_racp.h"
#include "ghs_cp.h"
#include "stm32_timer.h"
#include "uds_app.h"
#include "ets_app.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

typedef enum
{
  Hsf_INDICATION_OFF,
  Hsf_INDICATION_ON,
  Lho_NOTIFICATION_OFF,
  Lho_NOTIFICATION_ON,
  Lho_INDICATION_OFF,
  Lho_INDICATION_ON,
  Sho_NOTIFICATION_OFF,
  Sho_NOTIFICATION_ON,
  Sho_INDICATION_OFF,
  Sho_INDICATION_ON,
  Racp_INDICATION_OFF,
  Racp_INDICATION_ON,
  Ghscp_INDICATION_OFF,
  Ghscp_INDICATION_ON,
  Osc_INDICATION_OFF,
  Osc_INDICATION_ON,
  /* USER CODE BEGIN Service1_APP_SendInformation_t */

  /* USER CODE END Service1_APP_SendInformation_t */
  GHS_APP_SENDINFORMATION_LAST
} GHS_APP_SendInformation_t;

typedef struct
{
  GHS_APP_SendInformation_t     Hsf_Indication_Status;
  GHS_APP_SendInformation_t     Lho_Notification_Status;
  GHS_APP_SendInformation_t     Lho_Indication_Status;
  GHS_APP_SendInformation_t     Sho_Notification_Status;
  GHS_APP_SendInformation_t     Sho_Indication_Status;
  GHS_APP_SendInformation_t     Racp_Indication_Status;
  GHS_APP_SendInformation_t     Ghscp_Indication_Status;
  GHS_APP_SendInformation_t     Osc_Indication_Status;
  /* USER CODE BEGIN Service1_APP_Context_t */
  GHS_HSF_t HSFChar;
  GHS_LHO_t LHOChar;
  GHS_SHO_t SHOChar;
  GHS_OSC_t OSCChar;
  UTIL_TIMER_Object_t TimerLHO_Id;
  UTIL_TIMER_Object_t TimerRACPProcess_Id;
  uint8_t Db_Record_Count;
  /* USER CODE END Service1_APP_Context_t */
  uint16_t              ConnectionHandle;
} GHS_APP_Context_t;

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define GHS_APP_LHO_INTERVAL                                            (5*1000)
#define GHS_APP_RACP_INTERVAL                                             (1000)

#define GHS_APP_LHO_CHAR_SIZE                                               (80) 
#define GHS_APP_SHO_CHAR_SIZE                                               (80) 
/* USER CODE END PD */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Private macros ------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
static GHS_APP_Context_t GHS_APP_Context;

uint8_t a_GHS_UpdateCharData[247];

/* USER CODE BEGIN PV */
uint8_t GHS_APP_LHOIndex;
static uint32_t PulseOximeter[2] =
{
  /* MDC_PULS_OXIM_SAT_O2 */     0x00024BB8,
  /* MDC_PULS_OXIM_PULS_RATE */  0x0002481A
};

static uint32_t MeasurementPeriod[2] =
{
  0xFF00000A,
  0xFF00000A
};

static uint32_t UpdateInterval[2] =
{
  0xFF00000A,
  0xFF00000A
};

const GHS_OSC_t ObservationSchedule[2] = 
{
  {0x00024BB8, GHS_APP_LHO_INTERVAL, GHS_APP_LHO_INTERVAL},
  {0x0002481A, GHS_APP_LHO_INTERVAL, GHS_APP_LHO_INTERVAL}
};

GHS_ValidRangeAccuracy_t ValidRangeAccuracy[2] =
{
  { 0x01, 0x0220, 0, 100, 10 },
  { 0x01, 0x0AA0, 0, 100, 1 }
};

/* Supported Device Specializations example for a Blood Pressure Monitor version 01 */
static uint32_t BloodPressure = 0x00011007;

static uint8_t NumericOxygenSaturation[] =
{
  0x01,                                 /* Observation Class Type: Numeric Observation */
  0x1B, 0x00,                           /* Length: 27 bytes */
  0x63, 0x00,                           /* Flags:  OBSERVATION_TYPE_PRESENT | TIME_STAMP_PRESENT | PATIENT_PRESENT | SUPPLEMENT_INFORMATION_PRESENT */
  0xB8, 0x4B, 0x02, 0x00,               /* Observation Type: MDC_PULS_OXIM_SAT_O2 */               
  0x22,                                 /* Time Stamp */
  0x72, 0x9D, 0x2B, 0x29, 0x00, 0x00,   
  0x06, 0x00,
  0x01,                                 /* Patient */
  0x01,                                 /* Supplemental Information: Count 1 */
  0x3C, 0x4C, 0x02, 0x00,               /*                           Codes MDC_MODALITY_SPOT */
  0x20, 0x02,                           /* Observation Value: Unit Code MDC_DIM_PER_CENT */
  0x62, 0x00, 0x00, 0x00                /*                    Value 98 */
};

static uint8_t SimpleDiscreteObservation[] =
{
  0x02,                                 /* Observation Class Type: Simple Discrete Observation */
  0x18, 0x00,                           /* Length: 24 bytes */
  0x43, 0x00,                           /* Flags:  OBSERVATION_TYPE_PRESENT | TIME_STAMP_PRESENT | SUPPLEMENT_INFORMATION_PRESENT */
  0xB8, 0x4B, 0x02, 0x00,               /* Observation Type: MDC_PULS_OXIM_SAT_O2 */               
  0x22,                                 /* Time Stamp */
  0x72, 0x9D, 0x2B, 0x29, 0x00, 0x00,   
  0x06, 0x00,                                                  
  0x01,                                 /* Supplemental Information: Count 1 */
  0x3C, 0x4C, 0x02, 0x00,               /*                           Codes MDC_MODALITY_SPOT */
  0x62, 0x00, 0x00, 0x00                /* Value 98 */
};

static uint8_t StringObservation[] =
{
  0x03,                                 /* Observation Class Type: String Observation */
  0x07, 0x00,                           /* Length: 7 bytes */
  0x00, 0x00,                           /* Flags:  No flag */
  0x03, 0x00,                           /* Length: 3 bytes */
  'M', 'a', 'n'                         /* Value 'Man' */
};

static uint8_t SampleArrayObservation[] =
{
  0x04,                                 /* Observation Class Type: Sample array observation */
  0x20, 0x00,                           /* Length: 32 bytes */
  0x00, 0x00,                           /* Flags:  No flag */
  0xB2, 0x10,                           /* Observation Value: Unit Code MDC_DIM_MILLI_VOLT */
  0x4C, 0x06, 0x00, 0x00,               /*                    Scale Factor 1.612 */
  0x00, 0x08, 0x00, 0x00,               /*                    Offset 2048 */
  0x0A, 0x00, 0x00, 0xFD,               /*                    Sample Period 0.010 (10 msec) */
  0x01,                                 /*                    Number of Samples Per Period 1 */
  0x01,                                 /*                    Bytes Per Sample 1 */
  0x0A, 0x00, 0x00, 0x00,               /*                    Number Of Samples 10 */
                                        /*                    Scaled Samples: */
  0xF9, 0x07, 0xFB, 0x07, 0xF5, 
  0x07, 0xFF, 0x07, 0x0C, 0x08
};

static uint8_t TLVEncodedObservation[] =
{
  0x08,                                 /* Observation Class Type: TLV Encoded Observation */
  0x21, 0x00,                           /* Length: 33 bytes */
  0x00, 0x00,                           /* Flags:  No flag */
  0x03,                                 /* Number: 3 */
  0xB8, 0x4B, 0x02, 0x00,               /* Type: MDC Code */
  0x03, 0x00,                           /* Length: 3 */
  0x04,                                 /* Format Type: uint8 */
  0x62, 0x00, 0x00,                     /* Encoded Value 98 */
  0xB8, 0x4B, 0x02, 0x00,               /* Type: MDC Code */
  0x03, 0x00,                           /* Length: 3 */
  0x04,                                 /* Format Type: uint8 */
  0x63, 0x00, 0x00,                     /* Encoded Value 99 */
  0xB8, 0x4B, 0x02, 0x00,               /* Type: MDC Code */
  0x03, 0x00,                           /* Length: 3 */
  0x04,                                 /* Format Type: uint8 */
  0x64, 0x00, 0x00,                     /* Encoded Value 100 */
};

static uint8_t CompoundObservationBloodPressure[] =
{
  0x07,                                 /* Observation Class Type: Compound Observation */
  0x35, 0x00,                           /* Length: 53 bytes */
  0x13, 0x00,                           /* Flags:  OBSERVATION_TYPE_PRESENT | TIME_STAMP_PRESENT | OBSERVATION_ID_PRESENT */
  0x04, 0x4A, 0x02, 0x00,               /* Observation Type: MDC_PRESS_BLD_NONINV */               
  0x22,                                 /* Time Stamp */
  0x72, 0x9D, 0x2B, 0x29, 0x00, 0x00,   
  0x06, 0x00,                                                  
  0x40, 0xE2, 0x01, 0x00,               /* Observation Id: 123456 */
  0x03,                                 /* Number of components: 3 */
  0x05, 0x4A, 0x02, 0x00,               /* Component 1: component type MDC_PRESS_BLD_NONINV_SYS */
  0x01,                                 /*              component value type = 1 (numeric) */
  0x20, 0x0F,                           /*              unit code = MDC_DIM_MMHG */
  0x64, 0x00, 0x00, 0x00,               /*              value = 100 */
  0x06, 0x4A, 0x02, 0x00,               /* Component 2: component type MDC_PRESS_BLD_NONINV_DIA */
  0x01,                                 /*              component value type = 1 (numeric) */
  0x20, 0x0F,                           /*              unit code = MDC_DIM_MMHG */
  0x3C, 0x00, 0x00, 0x00,               /*              value = 60 */
  0x07, 0x4A, 0x02, 0x00,               /* Component 3: component type MDC_PRESS_BLD_NONINV_MEAN */
  0x01,                                 /*              component value type = 1 (numeric) */
  0x20, 0x0F,                           /*              unit code = MDC_DIM_MMHG */
  0x50, 0x00, 0x00, 0x00                /*              value = 80 */
};

static uint8_t CompoundDiscreteBloodPressureStatus[] =
{
  0x05,                                 /* Observation Class Type: Compound discrete observation */
  0x14, 0x00,                           /* Length: 20 bytes */
  0x81, 0x00,                           /* Flags:  OBSERVATION_TYPE_PRESENT | DERIVED_FROM_PRESENT */
  0xF0, 0x55, 0x80, 0x00,               /* Observation Type: MDC_BLP_MEASUREMENT STATUS */               
  0x01, 0x40, 0xE2, 0x01, 0x00,         /* Derived From */
  0x02,                                 /* Observation Value: Number of terms 2 */
  0xF0, 0x00, 0x03, 0x00,               /*                    Term 1 Cuff loose (3::240) */                       
  0xAE, 0x01, 0x03, 0x00                /*                    Term 2 Cuff improperly placed (3::430) */
};
                                        
static uint8_t CompoundDiscreteEvent[] =
{
  0x06,                                 /* Observation Class Type: Compound discrete event observation */
  0x18, 0x00,                           /* Length: 24 bytes */
  0x81, 0x00,                           /* Flags:  OBSERVATION_TYPE_PRESENT | DERIVED_FROM_PRESENT */
  0xF0, 0x55, 0x80, 0x00,               /* Observation Type: MDC_BLP_MEASUREMENT STATUS */               
  0x01, 0x40, 0xE2, 0x01, 0x00,         /* Derived From */
  0x04,                                 /* Observation Value: size 4 */
  0xFF, 0xFF, 0xFF, 0xFF,               /*                    Supported Or Unsupported Mask Bits */
  0xFF, 0xFF, 0xFF, 0xFF,               /*                    State Or Event Mask Bits */
  0xF0, 0x00, 0x03, 0x00,               /*                    Term 1 Cuff loose (3::240) */                       
};
                                        
static uint8_t SimpleArrayECGWaveform[] =
{
  0x04,                                 /* Observation Class Type: Sample array observation */
  0x97, 0x00,                           /* Length: 151 bytes */
  0x03, 0x00,                           /* Flags:  OBSERVATION_TYPE_PRESENT | TIME_STAMP_PRESENT */
  0x01, 0x01, 0x02, 0x00,               /* Observation Type: MDC_ECG_ELEC_POTL_I */               
  0x22,                                 /* Time Stamp */
  0x72, 0x9D, 0x2B, 0x29, 0x00, 0x00,   
  0x06, 0x00,                                                  
  0xB2, 0x10,                           /* Observation Value: Unit Code MDC_DIM_MILLI_VOLT */
  0x4C, 0x06, 0x00, 0x00,               /*                    Scale Factor 1.612 */
  0x00, 0x08, 0x00, 0x00,               /*                    Offset 2048 */
  0x0A, 0x00, 0x00, 0xFD,               /*                    Sample Period 0.010 (10 msec) */
  0x01,                                 /*                    Number of Samples Per Period 1 */
  0x02,                                 /*                    Bytes Per Sample 2 */
  0x74,                                 /*                    Number Of Samples 116 */
                                        /*                    Scaled Samples: */
  0xF9, 0x07, 0xFB, 0x07, 0xF5, 0x07, 0xFF, 0x07, 0x0C, 0x08, 0x0E, 0x08, 0x03, 
  0x08, 0xE7, 0x07, 0xDE, 0x07, 0xEB, 0x07, 0xF2, 0x07, 0xF1, 0x07, 0xF8, 0x07, 
  0xFF, 0x07, 0xFF, 0x07, 0x05, 0x08, 0x0A, 0x08, 0x10, 0x08, 0x0B, 0x08, 0x0F, 
  0x08, 0x0D, 0x08, 0x04, 0x08, 0x05, 0x08, 0xF6, 0x07, 0xAE, 0x07, 0x5D, 0x07, 
  0x5C, 0x07, 0xD9, 0x07, 0x51, 0x08, 0x76, 0x08, 0x59, 0x08, 0x36, 0x08, 0x26, 
  0x08, 0x1D, 0x08, 0x13, 0x08, 0x13, 0x08, 0x0C, 0x08, 0x0B, 0x08, 0x0E, 0x08,
  0x0E, 0x08, 0x0C, 0x08, 0x09, 0x08, 0xFD, 0x07, 0xFF, 0x07, 0x09, 0x08, 0x06, 
  0x08, 0xFA, 0x07, 0xED, 0x07, 0xEB, 0x07, 0xE2, 0x07, 0xD7, 0x07, 0xCB, 0x07, 
  0xD1, 0x07, 0xDC, 0x07, 0xE8, 0x07, 0xF7, 0x07, 0x14, 0x08, 0x2C, 0x08, 0x3F, 
  0x08, 0x4D, 0x08, 0x53, 0x08, 0x64, 0x08, 0x59, 0x08, 0x5A, 0x08, 0x50, 0x08, 
  0x50, 0x08, 0x43, 0x08, 0x33, 0x08, 0x31, 0x08, 0x30, 0x08, 0x35, 0x08, 0x35,
  0x08, 0x2B, 0x08, 0x19, 0x08, 0x1C, 0x08, 0x1D, 0x08, 0x24, 0x08, 0x21, 0x08, 
  0x28, 0x08, 0x2C, 0x08, 0x16, 0x08, 0x15, 0x08, 0x1A, 0x08, 0x1D, 0x08, 0x1B, 
  0x08, 0x14, 0x08, 0x10, 0x08, 0x0C, 0x08, 0x0E, 0x08, 0x1A, 0x08, 0x1B, 0x08, 
  0x1A, 0x08, 0x1B, 0x08, 0x0F, 0x08, 0x0A, 0x08, 0x0A, 0x08, 0x10, 0x08, 0x10,
  0x08, 0x16, 0x08, 0x1A, 0x08, 0x13, 0x08, 0x0C, 0x08, 0x0E, 0x08, 0x0F, 0x08,
  0x0D, 0x08, 0x0B, 0x08, 0x00, 0x08, 0x04, 0x08, 0x01, 0x08, 0x00, 0x08, 0x03, 
  0x08, 0x0B, 0x08, 0x0B, 0x08, 0x12, 0x08, 0x1D, 0x08, 0x19, 0x08
};

static uint8_t ObservationBundle[] =
{
  0xFF,                                 /* Observation Class Type: Observation bundle */
  0x2F, 0x00,                           /* Length: 47 bytes */
  0x42, 0x00,                           /* Flags:  TIME_STAMP_PRESENT | SUPPLEMENT_INFORMATION_PRESENT */
  0x22,                                 /* Time Stamp */               
  0x72, 0x9D, 0x2B, 0x29, 0x00, 0x00,   
  0x06, 0x00,                           
  0x01,                                 /* Supplemental Information: Count 1 */                                                                                         
  0x3C, 0x4C, 0x02, 0x00,               /*                           Codes MDC_MODALITY_SPOT */
  0x02,                                 /* Observation Value: Number of terms 2 */  
  /* Bundle Observation 1 */
  0x01,                                 /* Observation Class Type: Numeric observation */ 
  0x0C, 0x00,                           /* Length: 12 bytes */
  0x01, 0x00,                           /* Flags:  OBSERVATION_TYPE_PRESENT */
  0xB8, 0x4B, 0x02, 0x00,               /* Observation Type: MDC_PULS_OXIM_SAT_O2 */
  0x20, 0x02,                           /* Observation Value: Unit Code MDC_DIM_PER_CENT */
  0x62, 0x00, 0x00, 0x00,               /*                    Value 98 */
  /* Bundle Observation 2 */
  0x01,                                 /* Observation Class Type: Numeric observation */  
  0x0C, 0x00,                           /* Length: 12 bytes */
  0x01, 0x00,                           /* Flags:  OBSERVATION_TYPE_PRESENT */
  0x1A, 0x48, 0x02, 0x00,               /* Observation Type: MDC_PULS_OXIM_PULS_RATE */
  0xA0, 0x0A,                           /* Observation Value: Unit Code MDC_DIM_BEAT_PER_MIN */
  0x62, 0x00, 0x00, 0x00                /*                    Value 98 */
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void GHS_Hsf_SendIndication(void);
static void GHS_Lho_SendNotification(void);
static void GHS_Lho_SendIndication(void);
static void GHS_Sho_SendNotification(void);
static void GHS_Sho_SendIndication(void);
static void GHS_Racp_SendIndication(void);
static void GHS_Ghscp_SendIndication(void);
static void GHS_Osc_SendIndication(void);

/* USER CODE BEGIN PFP */
static void ghsapp_timer_handler_lho_process(void *arg);
static void ghsapp_timer_handler_racp_process(void *arg);
static void ghsapp_task_lho(void);
static void ghsapp_task_racp_process_report_record(void);
static void ghsapp_start_session(void);
static void ghsapp_stop_session(void);
static tBleStatus ghsapp_send_segment(uint8_t segment_size,
                                      uint16_t total_size,
                                      uint8_t * p_Data,
                                      GHS_CharOpcode_t CharOpcode);
/* USER CODE END PFP */

/* Functions Definition ------------------------------------------------------*/
void GHS_Notification(GHS_NotificationEvt_t *p_Notification)
{
  /* USER CODE BEGIN Service1_Notification_1 */

  /* USER CODE END Service1_Notification_1 */
  switch(p_Notification->EvtOpcode)
  {
    /* USER CODE BEGIN Service1_Notification_Service1_EvtOpcode */

    /* USER CODE END Service1_Notification_Service1_EvtOpcode */

    case GHS_HSF_READ_EVT:
      /* USER CODE BEGIN Service1Char1_READ_EVT */
      LOG_INFO_APP("HSF READ\r\n");
      /* USER CODE END Service1Char1_READ_EVT */
      break;

    case GHS_HSF_INDICATE_ENABLED_EVT:
      /* USER CODE BEGIN Service1Char1_INDICATE_ENABLED_EVT */
      LOG_INFO_APP("HSF Indication Enabled\r\n");
      GHS_APP_Context.Hsf_Indication_Status = Hsf_INDICATION_ON;
      if(aci_gatt_store_db() != BLE_STATUS_SUCCESS)
      {
        /* Save the descriptor value in GATT database */
        LOG_INFO_APP("aci_gatt_store_db failed\r\n");
      }
      /* USER CODE END Service1Char1_INDICATE_ENABLED_EVT */
      break;

    case GHS_HSF_INDICATE_DISABLED_EVT:
      /* USER CODE BEGIN Service1Char1_INDICATE_DISABLED_EVT */
      LOG_INFO_APP("HSF Indication Disabled\r\n");
      GHS_APP_Context.Hsf_Indication_Status = Hsf_INDICATION_OFF;
      if(aci_gatt_store_db() != BLE_STATUS_SUCCESS)
      {
        /* Save the descriptor value in GATT database */
        LOG_INFO_APP("aci_gatt_store_db failed\r\n");
      }
      /* USER CODE END Service1Char1_INDICATE_DISABLED_EVT */
      break;

    case GHS_LHO_NOTIFY_ENABLED_EVT:
      /* USER CODE BEGIN Service1Char2_NOTIFY_ENABLED_EVT */
      LOG_INFO_APP("LHO Notification Enabled\r\n");
      GHS_APP_Context.Lho_Notification_Status = Lho_NOTIFICATION_ON;
      if(aci_gatt_store_db() != BLE_STATUS_SUCCESS)
      {
        /* Save the descriptor value in GATT database */
        LOG_INFO_APP("aci_gatt_store_db failed\r\n");
      }
      /* USER CODE END Service1Char2_NOTIFY_ENABLED_EVT */
      break;

    case GHS_LHO_NOTIFY_DISABLED_EVT:
      /* USER CODE BEGIN Service1Char2_NOTIFY_DISABLED_EVT */
      LOG_INFO_APP("LHO Notification Disabled\r\n");
      GHS_APP_Context.Lho_Notification_Status = Lho_NOTIFICATION_OFF;
      if(aci_gatt_store_db() != BLE_STATUS_SUCCESS)
      {
        /* Save the descriptor value in GATT database */
        LOG_INFO_APP("aci_gatt_store_db failed\r\n");
      }
      /* USER CODE END Service1Char2_NOTIFY_DISABLED_EVT */
      break;

    case GHS_LHO_INDICATE_ENABLED_EVT:
      /* USER CODE BEGIN Service1Char2_INDICATE_ENABLED_EVT */
      LOG_INFO_APP("LHO Indication Enabled\r\n");
      GHS_APP_Context.Lho_Indication_Status = Lho_INDICATION_ON;
      if(aci_gatt_store_db() != BLE_STATUS_SUCCESS)
      {
        /* Save the descriptor value in GATT database */
        LOG_INFO_APP("aci_gatt_store_db failed\r\n");
      }
      /* USER CODE END Service1Char2_INDICATE_ENABLED_EVT */
      break;

    case GHS_LHO_INDICATE_DISABLED_EVT:
      /* USER CODE BEGIN Service1Char2_INDICATE_DISABLED_EVT */
      LOG_INFO_APP("LHO Indication Disabled\r\n");
      GHS_APP_Context.Lho_Indication_Status = Lho_INDICATION_OFF;
      if(aci_gatt_store_db() != BLE_STATUS_SUCCESS)
      {
        /* Save the descriptor value in GATT database */
        LOG_INFO_APP("aci_gatt_store_db failed\r\n");
      }
      /* USER CODE END Service1Char2_INDICATE_DISABLED_EVT */
      break;

    case GHS_SHO_NOTIFY_ENABLED_EVT:
      /* USER CODE BEGIN Service1Char3_NOTIFY_ENABLED_EVT */
      LOG_INFO_APP("SHO Notification Enabled\r\n");
      GHS_APP_Context.Sho_Notification_Status = Sho_NOTIFICATION_ON;
      if(aci_gatt_store_db() != BLE_STATUS_SUCCESS)
      {
        /* Save the descriptor value in GATT database */
        LOG_INFO_APP("aci_gatt_store_db failed\r\n");
      }
      /* USER CODE END Service1Char3_NOTIFY_ENABLED_EVT */
      break;

    case GHS_SHO_NOTIFY_DISABLED_EVT:
      /* USER CODE BEGIN Service1Char3_NOTIFY_DISABLED_EVT */
      LOG_INFO_APP("SHO Notification Disabled\r\n");
      GHS_APP_Context.Sho_Notification_Status = Sho_NOTIFICATION_OFF;
      if(aci_gatt_store_db() != BLE_STATUS_SUCCESS)
      {
        /* Save the descriptor value in GATT database */
        LOG_INFO_APP("aci_gatt_store_db failed\r\n");
      }
      /* USER CODE END Service1Char3_NOTIFY_DISABLED_EVT */
      break;

    case GHS_SHO_INDICATE_ENABLED_EVT:
      /* USER CODE BEGIN Service1Char3_INDICATE_ENABLED_EVT */
      LOG_INFO_APP("SHO Indication Enabled\r\n");
      GHS_APP_Context.Sho_Indication_Status = Sho_INDICATION_ON;
      if(aci_gatt_store_db() != BLE_STATUS_SUCCESS)
      {
        /* Save the descriptor value in GATT database */
        LOG_INFO_APP("aci_gatt_store_db failed\r\n");
      }
      /* USER CODE END Service1Char3_INDICATE_ENABLED_EVT */
      break;

    case GHS_SHO_INDICATE_DISABLED_EVT:
      /* USER CODE BEGIN Service1Char3_INDICATE_DISABLED_EVT */
      LOG_INFO_APP("SHO Indication Disabled\r\n");
      GHS_APP_Context.Sho_Indication_Status = Sho_INDICATION_OFF;
      if(aci_gatt_store_db() != BLE_STATUS_SUCCESS)
      {
        /* Save the descriptor value in GATT database */
        LOG_INFO_APP("aci_gatt_store_db failed\r\n");
      }
      /* USER CODE END Service1Char3_INDICATE_DISABLED_EVT */
      break;

    case GHS_RACP_WRITE_EVT:
      /* USER CODE BEGIN Service1Char4_WRITE_EVT */

      /* USER CODE END Service1Char4_WRITE_EVT */
      break;

    case GHS_RACP_INDICATE_ENABLED_EVT:
      /* USER CODE BEGIN Service1Char4_INDICATE_ENABLED_EVT */
      LOG_INFO_APP("RACP Indication Enabled\r\n");
      GHS_APP_Context.Racp_Indication_Status = Racp_INDICATION_ON;
      if(aci_gatt_store_db() != BLE_STATUS_SUCCESS)
      {
        /* Save the descriptor value in GATT database */
        LOG_INFO_APP("aci_gatt_store_db failed\r\n");
      }
      /* USER CODE END Service1Char4_INDICATE_ENABLED_EVT */
      break;

    case GHS_RACP_INDICATE_DISABLED_EVT:
      /* USER CODE BEGIN Service1Char4_INDICATE_DISABLED_EVT */
      LOG_INFO_APP("RACP Indication Disabled\r\n");
      GHS_APP_Context.Racp_Indication_Status = Racp_INDICATION_OFF;
      if(aci_gatt_store_db() != BLE_STATUS_SUCCESS)
      {
        /* Save the descriptor value in GATT database */
        LOG_INFO_APP("aci_gatt_store_db failed\r\n");
      }
      /* USER CODE END Service1Char4_INDICATE_DISABLED_EVT */
      break;

    case GHS_GHSCP_WRITE_EVT:
      /* USER CODE BEGIN Service1Char5_WRITE_EVT */

      /* USER CODE END Service1Char5_WRITE_EVT */
      break;

    case GHS_GHSCP_INDICATE_ENABLED_EVT:
      /* USER CODE BEGIN Service1Char5_INDICATE_ENABLED_EVT */
      LOG_INFO_APP("GHSCP Indication Enabled\r\n");
      GHS_APP_Context.Ghscp_Indication_Status = Ghscp_INDICATION_ON;
      if(aci_gatt_store_db() != BLE_STATUS_SUCCESS)
      {
        /* Save the descriptor value in GATT database */
        LOG_INFO_APP("aci_gatt_store_db failed\r\n");
      }
      /* USER CODE END Service1Char5_INDICATE_ENABLED_EVT */
      break;

    case GHS_GHSCP_INDICATE_DISABLED_EVT:
      /* USER CODE BEGIN Service1Char5_INDICATE_DISABLED_EVT */
      LOG_INFO_APP("GHSCP Indication Disabled\r\n");
      GHS_APP_Context.Ghscp_Indication_Status = Ghscp_INDICATION_OFF;
      if(aci_gatt_store_db() != BLE_STATUS_SUCCESS)
      {
        /* Save the descriptor value in GATT database */
        LOG_INFO_APP("aci_gatt_store_db failed\r\n");
      }
      /* USER CODE END Service1Char5_INDICATE_DISABLED_EVT */
      break;

    case GHS_OSC_INDICATE_ENABLED_EVT:
      /* USER CODE BEGIN Service1Char6_INDICATE_ENABLED_EVT */
      LOG_INFO_APP("OSC Indication Enabled\r\n");
      GHS_APP_Context.Osc_Indication_Status = Osc_INDICATION_ON;
      if(aci_gatt_store_db() != BLE_STATUS_SUCCESS)
      {
        /* Save the descriptor value in GATT database */
        LOG_INFO_APP("aci_gatt_store_db failed\r\n");
      }
      /* USER CODE END Service1Char6_INDICATE_ENABLED_EVT */
      break;

    case GHS_OSC_INDICATE_DISABLED_EVT:
      /* USER CODE BEGIN Service1Char6_INDICATE_DISABLED_EVT */
      LOG_INFO_APP("OSC Indication Disabled\r\n");
      GHS_APP_Context.Osc_Indication_Status = Osc_INDICATION_OFF;
      if(aci_gatt_store_db() != BLE_STATUS_SUCCESS)
      {
        /* Save the descriptor value in GATT database */
        LOG_INFO_APP("aci_gatt_store_db failed\r\n");
      }
      /* USER CODE END Service1Char6_INDICATE_DISABLED_EVT */
      break;

    default:
      /* USER CODE BEGIN Service1_Notification_default */

      /* USER CODE END Service1_Notification_default */
      break;
  }
  /* USER CODE BEGIN Service1_Notification_2 */

  /* USER CODE END Service1_Notification_2 */
  return;
}

void GHS_APP_EvtRx(GHS_APP_ConnHandleNotEvt_t *p_Notification)
{
  /* USER CODE BEGIN Service1_APP_EvtRx_1 */

  /* USER CODE END Service1_APP_EvtRx_1 */

  switch(p_Notification->EvtOpcode)
  {
    /* USER CODE BEGIN Service1_APP_EvtRx_Service1_EvtOpcode */

    /* USER CODE END Service1_APP_EvtRx_Service1_EvtOpcode */
    case GHS_CONN_HANDLE_EVT :
      /* USER CODE BEGIN Service1_APP_CONN_HANDLE_EVT */

      /* USER CODE END Service1_APP_CONN_HANDLE_EVT */
      break;

    case GHS_DISCON_HANDLE_EVT :
      /* USER CODE BEGIN Service1_APP_DISCON_HANDLE_EVT */

      /* USER CODE END Service1_APP_DISCON_HANDLE_EVT */
      break;

    default:
      /* USER CODE BEGIN Service1_APP_EvtRx_default */

      /* USER CODE END Service1_APP_EvtRx_default */
      break;
  }

  /* USER CODE BEGIN Service1_APP_EvtRx_2 */

  /* USER CODE END Service1_APP_EvtRx_2 */

  return;
}

void GHS_APP_Init(void)
{
  UNUSED(GHS_APP_Context);
  GHS_Init();

  /* USER CODE BEGIN Service1_APP_Init */
  GHS_Data_t msg_conf;
  uint8_t length = 0;

  /* Initialize the the number of records in database */
  GHS_APP_Context.Db_Record_Count = 0;

  /* Initialize the Generic Health Service records database */
  GHS_DB_Init();

  /* Initialize the Generic Health Service Record Access Point */
  GHS_RACP_Init();
  
  /* GHS Feature */
  GHS_APP_Context.HSFChar.Flags = SUPPORTED_DEVICE_SPECIALIZATIONS_FIELD_PRESENT;
  GHS_APP_Context.HSFChar.SupportedObservationTypes.Count = 2;
  GHS_APP_Context.HSFChar.SupportedObservationTypes.p_Data = PulseOximeter;
  if((GHS_APP_Context.HSFChar.Flags & SUPPORTED_DEVICE_SPECIALIZATIONS_FIELD_PRESENT) == SUPPORTED_DEVICE_SPECIALIZATIONS_FIELD_PRESENT)
  {  
    GHS_APP_Context.HSFChar.SupportedDeviceSpecializations.Count = 1;
    GHS_APP_Context.HSFChar.SupportedDeviceSpecializations.p_Data = &BloodPressure;
  }
  
  a_GHS_UpdateCharData[length] = GHS_APP_Context.HSFChar.Flags;
  length++;
  a_GHS_UpdateCharData[length] = GHS_APP_Context.HSFChar.SupportedObservationTypes.Count;
  length++;
  for(uint8_t i = 0; i < GHS_APP_Context.HSFChar.SupportedObservationTypes.Count; i++)
  {
    a_GHS_UpdateCharData[length] = (GHS_APP_Context.HSFChar.SupportedObservationTypes.p_Data[i]) & 0xFF ;
    length++;
    a_GHS_UpdateCharData[length] = ((GHS_APP_Context.HSFChar.SupportedObservationTypes.p_Data[i]) >> 8) & 0xFF;
    length++;
    a_GHS_UpdateCharData[length] = ((GHS_APP_Context.HSFChar.SupportedObservationTypes.p_Data[i]) >> 16) & 0xFF;
    length++;
    a_GHS_UpdateCharData[length] = ((GHS_APP_Context.HSFChar.SupportedObservationTypes.p_Data[i]) >> 24) & 0xFF;
    length++;
  }
  if((GHS_APP_Context.HSFChar.Flags & SUPPORTED_DEVICE_SPECIALIZATIONS_FIELD_PRESENT) == SUPPORTED_DEVICE_SPECIALIZATIONS_FIELD_PRESENT)
  {  
    a_GHS_UpdateCharData[length] = GHS_APP_Context.HSFChar.SupportedDeviceSpecializations.Count;
    length++;
    for(uint8_t i = 0; i < GHS_APP_Context.HSFChar.SupportedDeviceSpecializations.Count; i++)
    {
      a_GHS_UpdateCharData[length] = (GHS_APP_Context.HSFChar.SupportedDeviceSpecializations.p_Data[i]) & 0xFF ;
      length++;
      a_GHS_UpdateCharData[length] = ((GHS_APP_Context.HSFChar.SupportedDeviceSpecializations.p_Data[i]) >> 8) & 0xFF;
      length++;
      a_GHS_UpdateCharData[length] = ((GHS_APP_Context.HSFChar.SupportedDeviceSpecializations.p_Data[i]) >> 16) & 0xFF;
      length++;
    }
  }
  
  msg_conf.Length = length;
  msg_conf.p_Payload = a_GHS_UpdateCharData;
  GHS_UpdateValue(GHS_HSF, &msg_conf);
  
  GHS_APP_LHOIndex = 0;
  
  GHS_APP_Context.LHOChar.SegmentationHeader.FirstSegment = 1;
  GHS_APP_Context.LHOChar.SegmentationHeader.LastSegment = 1;
  GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter = 0;

  GHS_APP_Context.SHOChar.SegmentationHeader.FirstSegment = 1;
  GHS_APP_Context.SHOChar.SegmentationHeader.LastSegment = 1;
  GHS_APP_Context.SHOChar.SegmentationHeader.SegmentCounter = 0;
  GHS_APP_Context.SHOChar.StoredHealthObservationSegment.RecordNumber = 0;

  /* Generate some dummy data - just for testing purposes */
#if (BLE_CFG_GHS_DEBUG == 1)
  GHS_APP_PTS_TestDBInit();
#endif  
  
  /* Register tasks for Live Health Observations, report Record processing */
  UTIL_SEQ_RegTask( 1<<CFG_TASK_LHO_ID, UTIL_SEQ_RFU, ghsapp_task_lho );
  UTIL_SEQ_RegTask( 1<<CFG_TASK_RACP_PROCESS_REPORT_RECORD_ID, UTIL_SEQ_RFU, ghsapp_task_racp_process_report_record );

  /* Create timer for Live Health Observations */
  UTIL_TIMER_Create(&(GHS_APP_Context.TimerLHO_Id),
                    GHS_APP_LHO_INTERVAL,
                    UTIL_TIMER_PERIODIC,
                    &ghsapp_timer_handler_lho_process, 
                    0);
  
  /* Create timer for RACP process */
  UTIL_TIMER_Create(&(GHS_APP_Context.TimerRACPProcess_Id),
                    GHS_APP_RACP_INTERVAL,
                    UTIL_TIMER_ONESHOT,
                    &ghsapp_timer_handler_racp_process, 0);
  
  /* USER CODE END Service1_APP_Init */
  return;
}

/* USER CODE BEGIN FD */
void GHS_APP_PTS_TestDBInit(void)
{
  uint16_t index;
  GHS_RACP_Record_t db_record;

  db_record.SHOSegment.HealthObservationBody.ObservationClassType = NumericOxygenSaturation[0];
  db_record.SHOSegment.HealthObservationBody.Length = sizeof(NumericOxygenSaturation) - 3;
  db_record.SHOSegment.HealthObservationBody.p_Data = &(NumericOxygenSaturation[3]);
  for (index = 0; index < MAX_RECORDS_COUNT; index++)
  {
    db_record.SHOSegment.RecordNumber = index + 1;
    if(GHS_DB_AddRecord(&(db_record)) == FALSE)
      {
        LOG_INFO_APP("No more space in DB for record %d \n", index);
      }
    }

  GHS_APP_Context.Db_Record_Count = index + 1;
}

/**
  * @brief Get the flag holding whether GHS RACP characteristic indication is enabled or not
  * @param None
  * @retval None
  */
uint8_t GHS_APP_GetRACPCharacteristicIndicationEnabled(void)
{
  return ((GHS_APP_Context.Racp_Indication_Status == Racp_INDICATION_ON) ? TRUE: FALSE);
} /* end of GHS_APP_GetRACPCharacteristicIndicationEnabled() */

/**
  * @brief Get the flag holding whether GHS CP characteristic indication is enabled or not
  * @param None
  * @retval None
  */
uint8_t GHS_APP_GetGHSCPCharacteristicIndicationEnabled(void)
{
  return ((GHS_APP_Context.Ghscp_Indication_Status == Ghscp_INDICATION_ON) ? TRUE: FALSE);
} /* end of GHS_APP_GetGHSCPCharacteristicIndicationEnabled() */

/**
  * @brief Get the status of LHO timer is started or not
  * @param None
  * @retval None
  */
uint32_t GHS_APP_GetLHOTimerStarted(void)
{
  return (UTIL_TIMER_IsRunning(&(GHS_APP_Context.TimerLHO_Id)));
}

/**
  * @brief Update GHS Feature characteristic
  * @param None
  * @retval None
  */
void GHS_APP_UpdateFeature(void)
{
  GHS_Data_t msg_conf;
  uint8_t length = 0;
  tBleStatus ret;

  /* GHS Feature */
  GHS_APP_Context.HSFChar.Flags = SUPPORTED_DEVICE_SPECIALIZATIONS_FIELD_PRESENT;
  GHS_APP_Context.HSFChar.SupportedObservationTypes.Count = 2;
  GHS_APP_Context.HSFChar.SupportedObservationTypes.p_Data = PulseOximeter;
  if((GHS_APP_Context.HSFChar.Flags & SUPPORTED_DEVICE_SPECIALIZATIONS_FIELD_PRESENT) == SUPPORTED_DEVICE_SPECIALIZATIONS_FIELD_PRESENT)
  {  
    GHS_APP_Context.HSFChar.SupportedDeviceSpecializations.Count = 1;
    GHS_APP_Context.HSFChar.SupportedDeviceSpecializations.p_Data = &BloodPressure;
  }
  
  a_GHS_UpdateCharData[length] = GHS_APP_Context.HSFChar.Flags;
  length++;
  a_GHS_UpdateCharData[length] = GHS_APP_Context.HSFChar.SupportedObservationTypes.Count;
  length++;
  for(uint8_t i = 0; i < GHS_APP_Context.HSFChar.SupportedObservationTypes.Count; i++)
  {
    a_GHS_UpdateCharData[length] = (GHS_APP_Context.HSFChar.SupportedObservationTypes.p_Data[i]) & 0xFF ;
    length++;
    a_GHS_UpdateCharData[length] = ((GHS_APP_Context.HSFChar.SupportedObservationTypes.p_Data[i]) >> 8) & 0xFF;
    length++;
    a_GHS_UpdateCharData[length] = ((GHS_APP_Context.HSFChar.SupportedObservationTypes.p_Data[i]) >> 16) & 0xFF;
    length++;
    a_GHS_UpdateCharData[length] = ((GHS_APP_Context.HSFChar.SupportedObservationTypes.p_Data[i]) >> 24) & 0xFF;
    length++;
  }
  if((GHS_APP_Context.HSFChar.Flags & SUPPORTED_DEVICE_SPECIALIZATIONS_FIELD_PRESENT) == SUPPORTED_DEVICE_SPECIALIZATIONS_FIELD_PRESENT)
  {  
    a_GHS_UpdateCharData[length] = GHS_APP_Context.HSFChar.SupportedDeviceSpecializations.Count;
    length++;
    for(uint8_t i = 0; i < GHS_APP_Context.HSFChar.SupportedDeviceSpecializations.Count; i++)
    {
      a_GHS_UpdateCharData[length] = (GHS_APP_Context.HSFChar.SupportedDeviceSpecializations.p_Data[i]) & 0xFF ;
      length++;
      a_GHS_UpdateCharData[2 + length] = ((GHS_APP_Context.HSFChar.SupportedDeviceSpecializations.p_Data[i]) >> 8) & 0xFF;
      length++;
      a_GHS_UpdateCharData[2 + length] = ((GHS_APP_Context.HSFChar.SupportedDeviceSpecializations.p_Data[i]) >> 16) & 0xFF;
      length++;
    }
  }
  
  msg_conf.Length = length;
  msg_conf.p_Payload = a_GHS_UpdateCharData;
  ret= GHS_UpdateValue(GHS_HSF, &msg_conf);
  if(ret != BLE_STATUS_SUCCESS)
  {
    LOG_INFO_APP("Feature update fails\n");
  }

} /* end of GHS_APP_UpdateFeature() */

/**
  * @brief Update GHS Live Health Observations characteristic
  * @param None
  * @retval None
  */
void GHS_APP_UpdateLiveHealthObservation(void)
{
  uint16_t hob_length;
  uint8_t transmit_size;
  uint8_t length = 0;

  /* Max Number of bytes in transmission */ 
  if(GHS_APP_LHO_CHAR_SIZE <= (CFG_BLE_ATT_MTU_MAX - 3))
  { /* LHO char. size is less than MAX ATT MTU size */ 
    transmit_size = GHS_APP_LHO_CHAR_SIZE;
  }
  else
  { /* MAX ATT MTU size is less than LHO char. size */ 
    transmit_size = CFG_BLE_ATT_MTU_MAX - 3;
  }
  
  if(GHS_APP_LHOIndex == 0)
  {
    uint8_t a_elapsedTime[9];
    
    ETS_APP_GetElapsedTime(a_elapsedTime);
    
    /* Get Timestamp from ETS Elapsed Time */
    memcpy(&(NumericOxygenSaturation[10]), &(a_elapsedTime[1]), sizeof(a_elapsedTime) - 1);
    
    /* Get Patient ID from UDS User ID */
    NumericOxygenSaturation[18] = UDS_App_GetUserID();

    hob_length = (NumericOxygenSaturation[1]) |
                 ((NumericOxygenSaturation[2]) << 8);
    
    /* By defaut only one segment */
    GHS_APP_Context.LHOChar.SegmentationHeader.FirstSegment = 1;
    GHS_APP_Context.LHOChar.SegmentationHeader.LastSegment = 1;
    GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter += 1;
    if(GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter > 63)
    {  
      GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter = 0;
    }
    GHS_APP_Context.LHOChar.HealthObservationBody.ObservationClassType = NumericOxygenSaturation[0];
    GHS_APP_Context.LHOChar.HealthObservationBody.Length = hob_length;
    GHS_APP_Context.LHOChar.HealthObservationBody.p_Data = &(NumericOxygenSaturation[3]);

    a_GHS_UpdateCharData[length] = (GHS_APP_Context.LHOChar.SegmentationHeader.FirstSegment) |
                                   ((GHS_APP_Context.LHOChar.SegmentationHeader.LastSegment) << 1) |
                                   ((GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter)) << 2;
    length++;
    a_GHS_UpdateCharData[length] = NumericOxygenSaturation[0]; /* Class Type */
    length++;
    a_GHS_UpdateCharData[length] = NumericOxygenSaturation[1]; /* Length */
    length++;
    a_GHS_UpdateCharData[length] = NumericOxygenSaturation[2];
    length++;
    memcpy(&(a_GHS_UpdateCharData[length]), &(NumericOxygenSaturation[3]), hob_length);
    
    ghsapp_send_segment(transmit_size,
                        hob_length + 4,
                        a_GHS_UpdateCharData,
                        GHS_LHO);
    
    GHS_APP_LHOIndex = 0;
  }
  else if(GHS_APP_LHOIndex == 1)
  {
    uint8_t a_elapsedTime[9];
    
    ETS_APP_GetElapsedTime(a_elapsedTime);
    
    /* Get Timestamp from ETS Elapsed Time */
    memcpy(&(SimpleDiscreteObservation[10]), &(a_elapsedTime[1]), sizeof(a_elapsedTime) - 1);
    
    hob_length = (SimpleDiscreteObservation[1]) |
                 ((SimpleDiscreteObservation[2]) << 8);
    
    /* By defaut only one segment */
    GHS_APP_Context.LHOChar.SegmentationHeader.FirstSegment = 1;
    GHS_APP_Context.LHOChar.SegmentationHeader.LastSegment = 1;
    GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter += 1;
    if(GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter > 63)
    {  
      GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter = 0;
    }
    GHS_APP_Context.LHOChar.HealthObservationBody.ObservationClassType = SimpleDiscreteObservation[0];
    GHS_APP_Context.LHOChar.HealthObservationBody.Length = hob_length;
    GHS_APP_Context.LHOChar.HealthObservationBody.p_Data = &(SimpleDiscreteObservation[3]);

    a_GHS_UpdateCharData[length] = (GHS_APP_Context.LHOChar.SegmentationHeader.FirstSegment) |
                                   ((GHS_APP_Context.LHOChar.SegmentationHeader.LastSegment) << 1) |
                                   ((GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter)) << 2;
    length++;
    a_GHS_UpdateCharData[length] = SimpleDiscreteObservation[0]; /* Class Type */
    length++;
    a_GHS_UpdateCharData[length] = SimpleDiscreteObservation[1]; /* Length */
    length++;
    a_GHS_UpdateCharData[length] = SimpleDiscreteObservation[2];
    length++;
    memcpy(&(a_GHS_UpdateCharData[length]), &(SimpleDiscreteObservation[3]), hob_length);
    
    ghsapp_send_segment(transmit_size,
                        hob_length + 4,
                        a_GHS_UpdateCharData,
                        GHS_LHO);
    
    GHS_APP_LHOIndex = 1;
  }
  else if(GHS_APP_LHOIndex == 2)
  {
    hob_length = (StringObservation[1]) |
                 ((StringObservation[2]) << 8);
    
    /* By defaut only one segment */
    GHS_APP_Context.LHOChar.SegmentationHeader.FirstSegment = 1;
    GHS_APP_Context.LHOChar.SegmentationHeader.LastSegment = 1;
    GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter += 1;
    if(GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter > 63)
    {  
      GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter = 0;
    }
    GHS_APP_Context.LHOChar.HealthObservationBody.ObservationClassType = StringObservation[0];
    GHS_APP_Context.LHOChar.HealthObservationBody.Length = hob_length;
    GHS_APP_Context.LHOChar.HealthObservationBody.p_Data = &(StringObservation[3]);

    a_GHS_UpdateCharData[length] = (GHS_APP_Context.LHOChar.SegmentationHeader.FirstSegment) |
                                   ((GHS_APP_Context.LHOChar.SegmentationHeader.LastSegment) << 1) |
                                   ((GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter)) << 2;
    length++;
    a_GHS_UpdateCharData[length] = StringObservation[0]; /* Class Type */
    length++;
    a_GHS_UpdateCharData[length] = StringObservation[1]; /* Length */
    length++;
    a_GHS_UpdateCharData[length] = StringObservation[2];
    length++;
    memcpy(&(a_GHS_UpdateCharData[length]), &(StringObservation[3]), hob_length);
    
    ghsapp_send_segment(transmit_size,
                        hob_length + 4,
                        a_GHS_UpdateCharData,
                        GHS_LHO);

    GHS_APP_LHOIndex = 2;
  }
  else if(GHS_APP_LHOIndex == 3)
  {
    hob_length =  (SampleArrayObservation[1]) |
                 ((SampleArrayObservation[2]) << 8);
    
    /* By defaut only one segment */
    GHS_APP_Context.LHOChar.SegmentationHeader.FirstSegment = 1;
    GHS_APP_Context.LHOChar.SegmentationHeader.LastSegment = 1;
    GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter += 1;
    if(GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter > 63)
    {  
      GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter = 0;
    }
    GHS_APP_Context.LHOChar.HealthObservationBody.ObservationClassType = SampleArrayObservation[0];
    GHS_APP_Context.LHOChar.HealthObservationBody.Length = hob_length;
    GHS_APP_Context.LHOChar.HealthObservationBody.p_Data = &(SampleArrayObservation[3]);

    a_GHS_UpdateCharData[length] = (GHS_APP_Context.LHOChar.SegmentationHeader.FirstSegment) |
                                   ((GHS_APP_Context.LHOChar.SegmentationHeader.LastSegment) << 1) |
                                   ((GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter)) << 2;
    length++;
    a_GHS_UpdateCharData[length] = SampleArrayObservation[0]; /* Class Type */
    length++;
    a_GHS_UpdateCharData[length] = SampleArrayObservation[1]; /* Length */
    length++;
    a_GHS_UpdateCharData[length] = SampleArrayObservation[2];
    length++;
    memcpy(&(a_GHS_UpdateCharData[length]), &(SampleArrayObservation[3]), hob_length);
    
    ghsapp_send_segment(transmit_size,
                        hob_length + 4,
                        a_GHS_UpdateCharData,
                        GHS_LHO);

    GHS_APP_LHOIndex = 3;
  }
  else if(GHS_APP_LHOIndex == 4)
  {
    hob_length = (CompoundObservationBloodPressure[1]) |
                 ((CompoundObservationBloodPressure[2]) << 8);
    
    /* By defaut only one segment */
    GHS_APP_Context.LHOChar.SegmentationHeader.FirstSegment = 1;
    GHS_APP_Context.LHOChar.SegmentationHeader.LastSegment = 1;
    GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter += 1;
    if(GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter > 63)
    {  
      GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter = 0;
    }
    GHS_APP_Context.LHOChar.HealthObservationBody.ObservationClassType = CompoundObservationBloodPressure[0];
    GHS_APP_Context.LHOChar.HealthObservationBody.Length = hob_length;
    GHS_APP_Context.LHOChar.HealthObservationBody.p_Data = &(CompoundObservationBloodPressure[3]);

    a_GHS_UpdateCharData[length] = (GHS_APP_Context.LHOChar.SegmentationHeader.FirstSegment) |
                                   ((GHS_APP_Context.LHOChar.SegmentationHeader.LastSegment) << 1) |
                                   ((GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter)) << 2;
    length++;
    a_GHS_UpdateCharData[length] = CompoundObservationBloodPressure[0]; /* Class Type */
    length++;
    a_GHS_UpdateCharData[length] = CompoundObservationBloodPressure[1]; /* Length */
    length++;
    a_GHS_UpdateCharData[length] = CompoundObservationBloodPressure[2];
    length++;
    memcpy(&(a_GHS_UpdateCharData[length]), &(CompoundObservationBloodPressure[3]), hob_length);
    
    ghsapp_send_segment(transmit_size,
                        hob_length + 4,
                        a_GHS_UpdateCharData,
                        GHS_LHO);

    GHS_APP_LHOIndex = 4;
  }
  else if(GHS_APP_LHOIndex == 5)
  {
    hob_length = (CompoundDiscreteBloodPressureStatus[1]) |
                 ((CompoundDiscreteBloodPressureStatus[2]) << 8);
    
    /* By defaut only one segment */
    GHS_APP_Context.LHOChar.SegmentationHeader.FirstSegment = 1;
    GHS_APP_Context.LHOChar.SegmentationHeader.LastSegment = 1;
    GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter += 1;
    if(GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter > 63)
    {  
      GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter = 0;
    }
    GHS_APP_Context.LHOChar.HealthObservationBody.ObservationClassType = CompoundDiscreteBloodPressureStatus[0];
    GHS_APP_Context.LHOChar.HealthObservationBody.Length = hob_length;
    GHS_APP_Context.LHOChar.HealthObservationBody.p_Data = &(CompoundDiscreteBloodPressureStatus[3]);

    a_GHS_UpdateCharData[length] = (GHS_APP_Context.LHOChar.SegmentationHeader.FirstSegment) |
                                   ((GHS_APP_Context.LHOChar.SegmentationHeader.LastSegment) << 1) |
                                   ((GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter)) << 2;
    length++;
    a_GHS_UpdateCharData[length] = CompoundDiscreteBloodPressureStatus[0]; /* Class Type */
    length++;
    a_GHS_UpdateCharData[length] = CompoundDiscreteBloodPressureStatus[1]; /* Length */
    length++;
    a_GHS_UpdateCharData[length] = CompoundDiscreteBloodPressureStatus[2];
    length++;
    memcpy(&(a_GHS_UpdateCharData[length]), &(CompoundDiscreteBloodPressureStatus[3]), hob_length);
    
    ghsapp_send_segment(transmit_size,
                        hob_length + 4,
                        a_GHS_UpdateCharData,
                        GHS_LHO);

    GHS_APP_LHOIndex = 5;
  }
  else if(GHS_APP_LHOIndex == 6)
  {
    hob_length = (CompoundDiscreteEvent[1]) |
                 ((CompoundDiscreteEvent[2]) << 8);
    
    /* By defaut only one segment */
    GHS_APP_Context.LHOChar.SegmentationHeader.FirstSegment = 1;
    GHS_APP_Context.LHOChar.SegmentationHeader.LastSegment = 1;
    GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter += 1;
    if(GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter > 63)
    {  
      GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter = 0;
    }
    GHS_APP_Context.LHOChar.HealthObservationBody.ObservationClassType = CompoundDiscreteEvent[0];
    GHS_APP_Context.LHOChar.HealthObservationBody.Length = hob_length;
    GHS_APP_Context.LHOChar.HealthObservationBody.p_Data = &(CompoundDiscreteEvent[3]);

    a_GHS_UpdateCharData[length] = (GHS_APP_Context.LHOChar.SegmentationHeader.FirstSegment) |
                                   ((GHS_APP_Context.LHOChar.SegmentationHeader.LastSegment) << 1) |
                                   ((GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter)) << 2;
    length++;
    a_GHS_UpdateCharData[length] = CompoundDiscreteEvent[0]; /* Class Type */
    length++;
    a_GHS_UpdateCharData[length] = CompoundDiscreteEvent[1]; /* Length */
    length++;
    a_GHS_UpdateCharData[length] = CompoundDiscreteEvent[2];
    length++;
    memcpy(&(a_GHS_UpdateCharData[length]), &(CompoundDiscreteEvent[3]), hob_length);
    
    ghsapp_send_segment(transmit_size,
                        hob_length + 4,
                        a_GHS_UpdateCharData,
                        GHS_LHO);

    GHS_APP_LHOIndex = 6;
  }  
  else if(GHS_APP_LHOIndex == 7)
  {
    hob_length = (TLVEncodedObservation[1]) |
                 ((TLVEncodedObservation[2]) << 8);
    
    /* By defaut only one segment */
    GHS_APP_Context.LHOChar.SegmentationHeader.FirstSegment = 1;
    GHS_APP_Context.LHOChar.SegmentationHeader.LastSegment = 1;
    GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter += 1;
    if(GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter > 63)
    {  
      GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter = 0;
    }
    GHS_APP_Context.LHOChar.HealthObservationBody.ObservationClassType = TLVEncodedObservation[0];
    GHS_APP_Context.LHOChar.HealthObservationBody.Length = hob_length;
    GHS_APP_Context.LHOChar.HealthObservationBody.p_Data = &(TLVEncodedObservation[3]);

    a_GHS_UpdateCharData[length] = (GHS_APP_Context.LHOChar.SegmentationHeader.FirstSegment) |
                                   ((GHS_APP_Context.LHOChar.SegmentationHeader.LastSegment) << 1) |
                                   ((GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter)) << 2;
    length++;
    a_GHS_UpdateCharData[length] = TLVEncodedObservation[0]; /* Class Type */
    length++;
    a_GHS_UpdateCharData[length] = TLVEncodedObservation[1]; /* Length */
    length++;
    a_GHS_UpdateCharData[length] = TLVEncodedObservation[2];
    length++;
    memcpy(&(a_GHS_UpdateCharData[length]), &(TLVEncodedObservation[3]), hob_length);
    
    ghsapp_send_segment(transmit_size,
                        hob_length + 4,
                        a_GHS_UpdateCharData,
                        GHS_LHO);

    GHS_APP_LHOIndex = 7;
  }
  else if(GHS_APP_LHOIndex == 8)
  {
    hob_length = (ObservationBundle[1]) |
                 ((ObservationBundle[2]) << 8);
    
    /* By defaut only one segment */
    GHS_APP_Context.LHOChar.SegmentationHeader.FirstSegment = 1;
    GHS_APP_Context.LHOChar.SegmentationHeader.LastSegment = 1;
    GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter += 1;
    if(GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter > 63)
    {  
      GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter = 0;
    }
    GHS_APP_Context.LHOChar.HealthObservationBody.ObservationClassType = ObservationBundle[0];
    GHS_APP_Context.LHOChar.HealthObservationBody.Length = hob_length;
    GHS_APP_Context.LHOChar.HealthObservationBody.p_Data = &(ObservationBundle[3]);

    a_GHS_UpdateCharData[length] = (GHS_APP_Context.LHOChar.SegmentationHeader.FirstSegment) |
                                   ((GHS_APP_Context.LHOChar.SegmentationHeader.LastSegment) << 1) |
                                   ((GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter)) << 2;
    length++;
    a_GHS_UpdateCharData[length] = ObservationBundle[0]; /* Class Type */
    length++;
    a_GHS_UpdateCharData[length] = ObservationBundle[1]; /* Length */
    length++;
    a_GHS_UpdateCharData[length] = ObservationBundle[2];
    length++;
    memcpy(&(a_GHS_UpdateCharData[length]), &(ObservationBundle[3]), hob_length);
    
    ghsapp_send_segment(transmit_size,
                        hob_length + 4,
                        a_GHS_UpdateCharData,
                        GHS_LHO);

    GHS_APP_LHOIndex = 8;
  }
  else if(GHS_APP_LHOIndex == 9)
  {
    uint8_t a_elapsedTime[9];
    
    ETS_APP_GetElapsedTime(a_elapsedTime);
    
    /* Get Timestamp from ETS Elapsed Time */
    memcpy(&(SimpleArrayECGWaveform[10]), &(a_elapsedTime[1]), sizeof(a_elapsedTime) - 1);
    
    hob_length = (SimpleArrayECGWaveform[1]) |
                 ((SimpleArrayECGWaveform[2]) << 8);
    
    /* By defaut only one segment */
    GHS_APP_Context.LHOChar.SegmentationHeader.FirstSegment = 1;
    GHS_APP_Context.LHOChar.SegmentationHeader.LastSegment = 1;
    GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter += 1;
    if(GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter > 63)
    {  
      GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter = 0;
    }
    GHS_APP_Context.LHOChar.HealthObservationBody.ObservationClassType = SimpleArrayECGWaveform[0];
    GHS_APP_Context.LHOChar.HealthObservationBody.Length = hob_length;
    GHS_APP_Context.LHOChar.HealthObservationBody.p_Data = &(SimpleArrayECGWaveform[3]);

    a_GHS_UpdateCharData[length] = (GHS_APP_Context.LHOChar.SegmentationHeader.FirstSegment) |
                                   ((GHS_APP_Context.LHOChar.SegmentationHeader.LastSegment) << 1) |
                                   ((GHS_APP_Context.LHOChar.SegmentationHeader.SegmentCounter)) << 2;
    length++;
    a_GHS_UpdateCharData[length] = SimpleArrayECGWaveform[0]; /* Class Type */
    length++;
    a_GHS_UpdateCharData[length] = SimpleArrayECGWaveform[1]; /* Length */
    length++;
    a_GHS_UpdateCharData[length] = SimpleArrayECGWaveform[2];
    length++;
    memcpy(&(a_GHS_UpdateCharData[length]), &(SimpleArrayECGWaveform[3]), hob_length);
    
    ghsapp_send_segment(transmit_size,
                        hob_length + 4,
                        a_GHS_UpdateCharData,
                        GHS_LHO);
    
    GHS_APP_LHOIndex = 9;
  }
  
  return;
} /* end of GHS_APP_UpdateLiveHealthObservation() */

/**
  * @brief Update GHS Observation Schedule Changed characteristic
  * @param None
  * @retval None
  */
void GHS_APP_UpdateObservationScheduleChanged(void)
{
  GHS_Data_t msg_conf;
  uint8_t length;
  tBleStatus ret;

  for(uint8_t i = 0; i < GHS_APP_Context.HSFChar.SupportedObservationTypes.Count; i++)
  {
    /* GHS Observation Schedule Changed */
    GHS_APP_Context.OSCChar.ObservationType = GHS_APP_Context.HSFChar.SupportedObservationTypes.p_Data[i];
    GHS_APP_Context.OSCChar.MeasurementPeriod = MeasurementPeriod[i];
    GHS_APP_Context.OSCChar.UpdateInterval = UpdateInterval[i];
    
    length = 0;
    a_GHS_UpdateCharData[length] = (GHS_APP_Context.OSCChar.ObservationType) & 0xFF ;
    length++;
    a_GHS_UpdateCharData[length] = ((GHS_APP_Context.OSCChar.ObservationType) >> 8) & 0xFF;
    length++;
    a_GHS_UpdateCharData[length] = ((GHS_APP_Context.OSCChar.ObservationType) >> 16) & 0xFF;
    length++;
    a_GHS_UpdateCharData[length] = ((GHS_APP_Context.OSCChar.ObservationType) >> 24) & 0xFF;
    length++;
    a_GHS_UpdateCharData[length] = (GHS_APP_Context.OSCChar.MeasurementPeriod) & 0xFF ;
    length++;
    a_GHS_UpdateCharData[length] = ((GHS_APP_Context.OSCChar.MeasurementPeriod) >> 8) & 0xFF;
    length++;
    a_GHS_UpdateCharData[length] = ((GHS_APP_Context.OSCChar.MeasurementPeriod) >> 16) & 0xFF;
    length++;
    a_GHS_UpdateCharData[length] = ((GHS_APP_Context.OSCChar.MeasurementPeriod) >> 24) & 0xFF;
    length++;
    a_GHS_UpdateCharData[length] = (GHS_APP_Context.OSCChar.UpdateInterval) & 0xFF ;
    length++;
    a_GHS_UpdateCharData[length] = ((GHS_APP_Context.OSCChar.UpdateInterval) >> 8) & 0xFF;
    length++;
    a_GHS_UpdateCharData[length] = ((GHS_APP_Context.OSCChar.UpdateInterval) >> 16) & 0xFF;
    length++;
    a_GHS_UpdateCharData[length] = ((GHS_APP_Context.OSCChar.UpdateInterval) >> 24) & 0xFF;
    length++;
    
    msg_conf.Length = length;
    msg_conf.p_Payload = a_GHS_UpdateCharData;
    ret= GHS_UpdateValue(GHS_OSC, &msg_conf);
    if(ret != BLE_STATUS_SUCCESS)
    {
      LOG_INFO_APP("Observation Schedule Changed update fails\n");
    }
  }

} /* end of GHS_APP_UpdateObservationScheduleChanged() */

/**
  * @brief Update GHS Stored Health Observations characteristic
  * @param pointer on Stored Health Observations
  * @retval None
  */
tBleStatus GHS_APP_UpdateStoredHealthObservation(GHS_RACP_Record_t *pStoredHealthObservation)
{
  GHS_Data_t msg_conf;
  uint8_t length = 0;
  tBleStatus ret = BLE_STATUS_FAILED;
  uint16_t hob_length;
  uint8_t transmit_size;
  uint8_t indication = 2;

  /* Max Number of bytes in transmission */ 
  if(GHS_APP_SHO_CHAR_SIZE <= (CFG_BLE_ATT_MTU_MAX - 3))
  { /* SHO char. size is less than MAX ATT MTU size */ 
    transmit_size = GHS_APP_SHO_CHAR_SIZE;
  }
  else
  { /* MAX ATT MTU size is less than SHO char. size */ 
    transmit_size = CFG_BLE_ATT_MTU_MAX - 3;
  }
  
  if((GHS_APP_Context.Sho_Indication_Status == Sho_INDICATION_ON) &&
     (GHS_APP_Context.Sho_Notification_Status == Sho_NOTIFICATION_ON))
  {
    /* Indication and Notification enables => choose just Indication */
    msg_conf.Length = 1;
    msg_conf.p_Payload = &indication;
    ret = GHS_UpdateValue(GHS_SHO_CCCD, &msg_conf);
    
    if(ret != BLE_STATUS_SUCCESS)
    {
      LOG_INFO_APP("Set SHO CCCD in Indication fails\n");
      return ret;
    }
  }
  
  GHS_APP_Context.SHOChar.StoredHealthObservationSegment.HealthObservationBody.Length = 
    pStoredHealthObservation->SHOSegment.HealthObservationBody.Length;
  hob_length = (GHS_APP_Context.SHOChar.StoredHealthObservationSegment.HealthObservationBody.Length);
  
  /* By defaut only one segment */
  GHS_APP_Context.SHOChar.SegmentationHeader.FirstSegment = 1;
  GHS_APP_Context.SHOChar.SegmentationHeader.LastSegment = 1;
  GHS_APP_Context.SHOChar.SegmentationHeader.SegmentCounter += 1;
  if(GHS_APP_Context.SHOChar.SegmentationHeader.SegmentCounter > 63)
  {  
    GHS_APP_Context.SHOChar.SegmentationHeader.SegmentCounter = 1;
  }
    
  GHS_APP_Context.SHOChar.StoredHealthObservationSegment.RecordNumber = 
    pStoredHealthObservation->SHOSegment.RecordNumber;
  GHS_APP_Context.SHOChar.StoredHealthObservationSegment.HealthObservationBody.ObservationClassType =
    pStoredHealthObservation->SHOSegment.HealthObservationBody.ObservationClassType;
  GHS_APP_Context.SHOChar.StoredHealthObservationSegment.HealthObservationBody.p_Data = 
    pStoredHealthObservation->SHOSegment.HealthObservationBody.p_Data;
    
  a_GHS_UpdateCharData[length] = GHS_APP_Context.SHOChar.SegmentationHeader.FirstSegment |
                                 ((GHS_APP_Context.SHOChar.SegmentationHeader.LastSegment) << 1) |
                                 ((GHS_APP_Context.SHOChar.SegmentationHeader.SegmentCounter) << 2);
  length++;
  a_GHS_UpdateCharData[length] = (GHS_APP_Context.SHOChar.StoredHealthObservationSegment.RecordNumber) & 0xFF;
  length++;
  a_GHS_UpdateCharData[length] = ((GHS_APP_Context.SHOChar.StoredHealthObservationSegment.RecordNumber) >> 8) & 0xFF;
  length++;
  a_GHS_UpdateCharData[length] = ((GHS_APP_Context.SHOChar.StoredHealthObservationSegment.RecordNumber) >> 16) & 0xFF;
  length++;
  a_GHS_UpdateCharData[length] = ((GHS_APP_Context.SHOChar.StoredHealthObservationSegment.RecordNumber) >> 24) & 0xFF;
  length++;
  a_GHS_UpdateCharData[length] = GHS_APP_Context.SHOChar.StoredHealthObservationSegment.HealthObservationBody.ObservationClassType;
  length++;
  a_GHS_UpdateCharData[length] = hob_length & 0xFF;
  length++;
  a_GHS_UpdateCharData[length] = (hob_length >> 8) & 0xFF;
  length++;
  memcpy(&(a_GHS_UpdateCharData[length]), 
         GHS_APP_Context.SHOChar.StoredHealthObservationSegment.HealthObservationBody.p_Data,
         hob_length);
  
  ret = ghsapp_send_segment(transmit_size,
                            hob_length + 8,
                            a_GHS_UpdateCharData,
                            GHS_SHO);
  
  return ret;
} /* end of GHS_APP_UpdateStoredHealthObservation() */

/**
  * @brief Record Access Control Point event handler
  * @param pointer on RACP event
  * @retval None
  */
uint8_t GHS_RACP_APP_EventHandler(GHS_RACP_App_Event_t *pRACPAppEvent)
{
  switch(pRACPAppEvent->EventCode)
  {      
    case GHS_RACP_APP_REPORT_RECORDS_PROCEDURE_STARTED_EVENT:
      {
        LOG_INFO_APP("GHS_RACP_APP_REPORT_RECORDS_PROCEDURE_STARTED_EVENT\r\n"); 

        UTIL_TIMER_Start(&(GHS_APP_Context.TimerRACPProcess_Id));
      }
      break;
      
    case GHS_RACP_APP_REPORT_RECORDS_PROCEDURE_FINISHED_EVENT:
      {
        LOG_INFO_APP("GHS_RACP_APP_REPORT_RECORDS_PROCEDURE_FINISHED_EVENT\r\n");
      }
      break;
      
    case GHS_RACP_APP_REPORT_RECORDS_PROCEDURE_NOTIFY_NEXT_RECORD_EVENT:
      {
        LOG_INFO_APP("GHS_RACP_APP_REPORT_RECORDS_PROCEDURE_NOTIFY_NEXT_RECORD_EVENT\r\n"); 

        UTIL_TIMER_Start(&(GHS_APP_Context.TimerRACPProcess_Id));
      }
      break;
      
    default:
      break;
  }
  
  return TRUE;
} /* end of GHS_RACP_APP_EventHandler() */

/**
  * @brief Control Point event handler
  * @param pointer on CP event
  * @retval None
  */
uint8_t GHS_CP_APP_EventHandler(GHS_CP_App_Event_t *pCPAppEvent)
{
  switch(pCPAppEvent->EventCode)
  {      
    case GHS_CP_START_SENDING_LIVE_OBSERVATIONS_EVENT:
      {
        ghsapp_start_session();
      }
      break;
      
    case GHS_CP_STOP_SENDING_LIVE_OBSERVATIONS_EVENT:
      {
        ghsapp_stop_session();
      }
      break;
      
    default:
      break;
  }
  
  return TRUE;
} /* end of GHS_CP_APP_EventHandler() */

/**
  * @brief Update the number of recors in the data base 
  * @param Number of records
  * @retval None
  */
void GHS_APP_UpdateDbRecordCount(uint8_t count)
{
  GHS_APP_Context.Db_Record_Count -= count;
}
/* USER CODE END FD */

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/
__USED void GHS_Hsf_SendIndication(void) /* Property Indication */
{
  GHS_APP_SendInformation_t indication_on_off = Hsf_INDICATION_OFF;
  GHS_Data_t ghs_indication_data;

  ghs_indication_data.p_Payload = (uint8_t*)a_GHS_UpdateCharData;
  ghs_indication_data.Length = 0;

  /* USER CODE BEGIN Service1Char1_IS_1*/

  /* USER CODE END Service1Char1_IS_1*/

  if (indication_on_off != Hsf_INDICATION_OFF)
  {
    GHS_UpdateValue(GHS_HSF, &ghs_indication_data);
  }

  /* USER CODE BEGIN Service1Char1_IS_Last*/

  /* USER CODE END Service1Char1_IS_Last*/

  return;
}

__USED void GHS_Lho_SendNotification(void) /* Property Notification */
{
  GHS_APP_SendInformation_t notification_on_off = Lho_NOTIFICATION_OFF;
  GHS_Data_t ghs_notification_data;

  ghs_notification_data.p_Payload = (uint8_t*)a_GHS_UpdateCharData;
  ghs_notification_data.Length = 0;

  /* USER CODE BEGIN Service1Char2_NS_1*/

  /* USER CODE END Service1Char2_NS_1*/

  if (notification_on_off != Lho_NOTIFICATION_OFF)
  {
    GHS_UpdateValue(GHS_LHO, &ghs_notification_data);
  }

  /* USER CODE BEGIN Service1Char2_NS_Last*/

  /* USER CODE END Service1Char2_NS_Last*/

  return;
}

__USED void GHS_Lho_SendIndication(void) /* Property Indication */
{
  GHS_APP_SendInformation_t indication_on_off = Lho_INDICATION_OFF;
  GHS_Data_t ghs_indication_data;

  ghs_indication_data.p_Payload = (uint8_t*)a_GHS_UpdateCharData;
  ghs_indication_data.Length = 0;

  /* USER CODE BEGIN Service1Char2_IS_1*/

  /* USER CODE END Service1Char2_IS_1*/

  if (indication_on_off != Lho_INDICATION_OFF)
  {
    GHS_UpdateValue(GHS_LHO, &ghs_indication_data);
  }

  /* USER CODE BEGIN Service1Char2_IS_Last*/

  /* USER CODE END Service1Char2_IS_Last*/

  return;
}

__USED void GHS_Sho_SendNotification(void) /* Property Notification */
{
  GHS_APP_SendInformation_t notification_on_off = Sho_NOTIFICATION_OFF;
  GHS_Data_t ghs_notification_data;

  ghs_notification_data.p_Payload = (uint8_t*)a_GHS_UpdateCharData;
  ghs_notification_data.Length = 0;

  /* USER CODE BEGIN Service1Char3_NS_1*/

  /* USER CODE END Service1Char3_NS_1*/

  if (notification_on_off != Sho_NOTIFICATION_OFF)
  {
    GHS_UpdateValue(GHS_SHO, &ghs_notification_data);
  }

  /* USER CODE BEGIN Service1Char3_NS_Last*/

  /* USER CODE END Service1Char3_NS_Last*/

  return;
}

__USED void GHS_Sho_SendIndication(void) /* Property Indication */
{
  GHS_APP_SendInformation_t indication_on_off = Sho_INDICATION_OFF;
  GHS_Data_t ghs_indication_data;

  ghs_indication_data.p_Payload = (uint8_t*)a_GHS_UpdateCharData;
  ghs_indication_data.Length = 0;

  /* USER CODE BEGIN Service1Char3_IS_1*/

  /* USER CODE END Service1Char3_IS_1*/

  if (indication_on_off != Sho_INDICATION_OFF)
  {
    GHS_UpdateValue(GHS_SHO, &ghs_indication_data);
  }

  /* USER CODE BEGIN Service1Char3_IS_Last*/

  /* USER CODE END Service1Char3_IS_Last*/

  return;
}

__USED void GHS_Racp_SendIndication(void) /* Property Indication */
{
  GHS_APP_SendInformation_t indication_on_off = Racp_INDICATION_OFF;
  GHS_Data_t ghs_indication_data;

  ghs_indication_data.p_Payload = (uint8_t*)a_GHS_UpdateCharData;
  ghs_indication_data.Length = 0;

  /* USER CODE BEGIN Service1Char4_IS_1*/

  /* USER CODE END Service1Char4_IS_1*/

  if (indication_on_off != Racp_INDICATION_OFF)
  {
    GHS_UpdateValue(GHS_RACP, &ghs_indication_data);
  }

  /* USER CODE BEGIN Service1Char4_IS_Last*/

  /* USER CODE END Service1Char4_IS_Last*/

  return;
}

__USED void GHS_Ghscp_SendIndication(void) /* Property Indication */
{
  GHS_APP_SendInformation_t indication_on_off = Ghscp_INDICATION_OFF;
  GHS_Data_t ghs_indication_data;

  ghs_indication_data.p_Payload = (uint8_t*)a_GHS_UpdateCharData;
  ghs_indication_data.Length = 0;

  /* USER CODE BEGIN Service1Char5_IS_1*/

  /* USER CODE END Service1Char5_IS_1*/

  if (indication_on_off != Ghscp_INDICATION_OFF)
  {
    GHS_UpdateValue(GHS_GHSCP, &ghs_indication_data);
  }

  /* USER CODE BEGIN Service1Char5_IS_Last*/

  /* USER CODE END Service1Char5_IS_Last*/

  return;
}

__USED void GHS_Osc_SendIndication(void) /* Property Indication */
{
  GHS_APP_SendInformation_t indication_on_off = Osc_INDICATION_OFF;
  GHS_Data_t ghs_indication_data;

  ghs_indication_data.p_Payload = (uint8_t*)a_GHS_UpdateCharData;
  ghs_indication_data.Length = 0;

  /* USER CODE BEGIN Service1Char6_IS_1*/

  /* USER CODE END Service1Char6_IS_1*/

  if (indication_on_off != Osc_INDICATION_OFF)
  {
    GHS_UpdateValue(GHS_OSC, &ghs_indication_data);
  }

  /* USER CODE BEGIN Service1Char6_IS_Last*/

  /* USER CODE END Service1Char6_IS_Last*/

  return;
}

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS*/
static void ghsapp_timer_handler_lho_process(void *arg)
{
  /**
  * The code shall be executed in the background as aci command may be sent
  * The background is the only place where the application can make sure a new aci command
  * is not sent if there is a pending one
  */
  UTIL_SEQ_SetTask( 1<<CFG_TASK_LHO_ID, CFG_SEQ_PRIO_0);
  
  return;
}

static void ghsapp_timer_handler_racp_process(void *arg)
{
  /**
  * The code shall be executed in the background as aci command may be sent
  * The background is the only place where the application can make sure a new aci command
  * is not sent if there is a pending one
  */
  UTIL_SEQ_SetTask( 1<<CFG_TASK_RACP_PROCESS_REPORT_RECORD_ID, CFG_SEQ_PRIO_0);
}

static void ghsapp_task_lho(void)
{
  GHS_RACP_Record_t db_record;
  
  /* New Live Health Observations */
  GHS_APP_UpdateLiveHealthObservation();
  
  /* New Stored Health Observations saved in DB */
  if(GHS_APP_Context.Db_Record_Count == MAX_RECORDS_COUNT)
  { /* Delete oldest record */
    GHS_APP_Context.Db_Record_Count = MAX_RECORDS_COUNT - 1;
    GHS_DB_DeleteRecordByIndex(0x0000);
  }
  
    GHS_APP_Context.SHOChar.StoredHealthObservationSegment.RecordNumber += 1;
  db_record.SHOSegment.RecordNumber = GHS_APP_Context.SHOChar.StoredHealthObservationSegment.RecordNumber;
  db_record.SHOSegment.HealthObservationBody.ObservationClassType = GHS_APP_Context.LHOChar.HealthObservationBody.ObservationClassType;
  db_record.SHOSegment.HealthObservationBody.Length = GHS_APP_Context.LHOChar.HealthObservationBody.Length;
  db_record.SHOSegment.HealthObservationBody.p_Data = GHS_APP_Context.LHOChar.HealthObservationBody.p_Data;    
  if(GHS_DB_AddRecord(&(db_record)) != TRUE)
  {  
    LOG_INFO_APP("No more space in DB for new record\n");
  }
  else
  {
    GHS_APP_Context.Db_Record_Count++;
    LOG_INFO_APP("Stored Health Observations number %d\n", GHS_APP_Context.Db_Record_Count);
  }
  
  return;
}

static void ghsapp_task_racp_process_report_record(void)
{
  GHS_RACP_ProcessReportRecordsProcedure();
}

static void ghsapp_start_session( void )
{    
  LOG_INFO_APP("ghsapp_start_session\r\n");
  
  UTIL_TIMER_Stop(&(GHS_APP_Context.TimerLHO_Id));
    
  UTIL_TIMER_StartWithPeriod(&(GHS_APP_Context.TimerLHO_Id), 
                             GHS_APP_LHO_INTERVAL);
}

static void ghsapp_stop_session( void )
{
  LOG_INFO_APP("ghsapp_stop_session\r\n");

  UTIL_TIMER_Stop(&(GHS_APP_Context.TimerLHO_Id));
}

static tBleStatus ghsapp_send_segment(uint8_t segment_size,
                                      uint16_t total_size,
                                      uint8_t * p_Data,
                                      GHS_CharOpcode_t CharOpcode)
{
  GHS_Data_t msg_conf;
  tBleStatus ret = BLE_STATUS_SUCCESS;
  uint8_t segment[247];

  if(total_size <= segment_size)
  {
    /* Only one segment */
    msg_conf.Length = total_size;
    msg_conf.p_Payload = p_Data;
    ret = GHS_UpdateValue(CharOpcode, &msg_conf);
    if(ret != BLE_STATUS_SUCCESS)
    {
      if(CharOpcode == GHS_LHO)
      {
        LOG_INFO_APP("Live Health Observations update fails\n");
      }
      else
      {
        LOG_INFO_APP("Stored Health Observations update fails\n");
      }
      return ret;
    }
  }
  else
  {
    /* Multiple segments */
    uint8_t current_index;
    
    /* Update with first segment */
    GHS_APP_Context.SHOChar.SegmentationHeader.FirstSegment = 1;
    GHS_APP_Context.SHOChar.SegmentationHeader.LastSegment = 0;
    GHS_APP_Context.SHOChar.SegmentationHeader.SegmentCounter += 1;
    if(GHS_APP_Context.SHOChar.SegmentationHeader.SegmentCounter > 63)
    {  
      GHS_APP_Context.SHOChar.SegmentationHeader.SegmentCounter = 0;
    }

    p_Data[0] =  (GHS_APP_Context.SHOChar.SegmentationHeader.FirstSegment) |
                ((GHS_APP_Context.SHOChar.SegmentationHeader.LastSegment) << 1) |
                ((GHS_APP_Context.SHOChar.SegmentationHeader.SegmentCounter) << 2);
    current_index = 0;
    memcpy(segment, &(p_Data[current_index]), segment_size);
    current_index += segment_size;
    
    msg_conf.Length = segment_size;
    msg_conf.p_Payload = segment;
    ret = GHS_UpdateValue(CharOpcode, &msg_conf);
    if(ret != BLE_STATUS_SUCCESS)
    {
      if(CharOpcode == GHS_LHO)
      {
        LOG_INFO_APP("Live Health Observations first segment update fails\n");
      }
      else
      {
        LOG_INFO_APP("Stored Health Observations first segment update fails\n");
      }
      return ret;
    }
    else
    {
      if(GHS_APP_Context.Sho_Indication_Status == Sho_INDICATION_ON)
      {  
        UTIL_SEQ_ClrEvt(1U << CFG_IDLEEVT_GATT_INDICATION_COMPLETE);
        UTIL_SEQ_WaitEvt(1U << CFG_IDLEEVT_GATT_INDICATION_COMPLETE);
      }
      if(CharOpcode == GHS_LHO)
      {
        LOG_INFO_APP("Live Health Observations first segment generated\n");
      }
      else
      {
        LOG_INFO_APP("Stored Health Observations first segment generated\n");
      }
    }
    
    total_size -= segment_size - 1;
    while(total_size > (segment_size - 1))
    {
      /* Update with intermediate segments */
      GHS_APP_Context.SHOChar.SegmentationHeader.FirstSegment = 0;
      GHS_APP_Context.SHOChar.SegmentationHeader.LastSegment = 0;
      GHS_APP_Context.SHOChar.SegmentationHeader.SegmentCounter += 1;
      if(GHS_APP_Context.SHOChar.SegmentationHeader.SegmentCounter > 63)
      {  
        GHS_APP_Context.SHOChar.SegmentationHeader.SegmentCounter = 0;
      }
      
      segment[0] =  (GHS_APP_Context.SHOChar.SegmentationHeader.FirstSegment) |
                   ((GHS_APP_Context.SHOChar.SegmentationHeader.LastSegment) << 1) |
                   ((GHS_APP_Context.SHOChar.SegmentationHeader.SegmentCounter) << 2);

      memcpy(&(segment[1]), &(p_Data[current_index]), (segment_size - 1));
      current_index += (segment_size - 1);
      
      msg_conf.Length = segment_size;
      msg_conf.p_Payload = segment;
      ret = GHS_UpdateValue(CharOpcode, &msg_conf);
      if(ret != BLE_STATUS_SUCCESS)
      {
        if(CharOpcode == GHS_LHO)
        {
          LOG_INFO_APP("Live Health Observations intermediate segment update fails\n");
        }
        else
        {
          LOG_INFO_APP("Stored Health Observations intermediate segment update fails\n");
        }
        return ret;
      }
      else
      {
        if(GHS_APP_Context.Sho_Indication_Status == Sho_INDICATION_ON)
        {  
          UTIL_SEQ_ClrEvt(1U << CFG_IDLEEVT_GATT_INDICATION_COMPLETE);
          UTIL_SEQ_WaitEvt(1U << CFG_IDLEEVT_GATT_INDICATION_COMPLETE);
        }
        if(CharOpcode == GHS_LHO)
        {
          LOG_INFO_APP("Live Health Observations intermediate segment generated\n");
        }
        else
        {
          LOG_INFO_APP("Stored Health Observations intermediate segment generated\n");
        }
        total_size -= (segment_size - 1);
      }
    } /* End while(total_size > (segment_size - 1)) */
    
    /* Update with last segment */
    GHS_APP_Context.SHOChar.SegmentationHeader.FirstSegment = 0;
    GHS_APP_Context.SHOChar.SegmentationHeader.LastSegment = 1;
    GHS_APP_Context.SHOChar.SegmentationHeader.SegmentCounter += 1;
    if(GHS_APP_Context.SHOChar.SegmentationHeader.SegmentCounter > 63)
    {  
      GHS_APP_Context.SHOChar.SegmentationHeader.SegmentCounter = 0;
    }
    segment[0] =  (GHS_APP_Context.SHOChar.SegmentationHeader.FirstSegment) |
                 ((GHS_APP_Context.SHOChar.SegmentationHeader.LastSegment) << 1) |
                 ((GHS_APP_Context.SHOChar.SegmentationHeader.SegmentCounter) << 2);
    memcpy(&(segment[1]), &(p_Data[current_index]), total_size);
    current_index += total_size;

    msg_conf.Length = total_size;
    msg_conf.p_Payload = segment;
    ret = GHS_UpdateValue(CharOpcode, &msg_conf);
    if(ret != BLE_STATUS_SUCCESS)
    {
      if(CharOpcode == GHS_LHO)
      {
        LOG_INFO_APP("Live Health Observations last segment update fails\n");
      }
      else
      {
        LOG_INFO_APP("Stored Health Observations last segment update fails\n");
      }
      return ret;
    }
    else
    {
      if(GHS_APP_Context.Sho_Indication_Status == Sho_INDICATION_ON)
      {  
        UTIL_SEQ_ClrEvt(1U << CFG_IDLEEVT_GATT_INDICATION_COMPLETE);
        UTIL_SEQ_WaitEvt(1U << CFG_IDLEEVT_GATT_INDICATION_COMPLETE);
      }
      if(CharOpcode == GHS_LHO)
      {
        LOG_INFO_APP("Live Health Observations last segment generated\n");
      }
      else
      {
        LOG_INFO_APP("Stored Health Observations last segment generated\n");
      }
    }
  } /* End else of if(total_size <= segment_size) */
  
  return ret;
}  

/* USER CODE END FD_LOCAL_FUNCTIONS*/
