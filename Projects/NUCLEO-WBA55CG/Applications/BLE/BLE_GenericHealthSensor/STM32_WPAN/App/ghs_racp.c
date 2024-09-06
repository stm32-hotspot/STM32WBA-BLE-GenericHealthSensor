/**
  ******************************************************************************
  * @file    GHS_RACP.c
  * @author  MCD Application Team
  * @brief   GHS RACP
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


/* Includes ------------------------------------------------------------------*/
#include "log_module.h"
#include "common_blesvc.h"
#include "ghs.h"
#include "ghs_app.h"
#include "ghs_racp.h"
#include "ghs_db.h"

/* Private typedef -----------------------------------------------------------*/
    
/* typedef used for storing the specific filter type to be used */
typedef __PACKED_STRUCT
{
  uint8_t OpCode;
  uint8_t Operator;
  uint8_t Operand[GHS_RACP_OP_CODE_REQUEST_OPERAND_MAX_LENGTH];
} GHS_RACP_Procedure_t;

typedef __PACKED_STRUCT
{
  uint32_t NumberOfStoredRecords;
} GHS_RACP_NumberOfStoredRecordsResponseOperand;

typedef __PACKED_STRUCT
{
  uint8_t RequestOpCode;
  uint8_t ResponseCodeValue;
} GHS_RACP_ResponseCodeResponseOperand;

typedef __PACKED_STRUCT
{
  uint8_t OpCode;
  const uint8_t Operator; /* always 0 */
  uint8_t Operand[GHS_RACP_OP_CODE_RESPONSE_OPERAND_MAX_LENGTH];
} GHS_RACP_Response_t;

typedef struct
{
  GHS_RACP_Procedure_t Procedure;
  GHS_RACP_Response_t Response;
  GHS_RACP_State_t State;
  uint32_t NumberOfRecord;
} GHS_RACP_Context_t;

/* Private variables ---------------------------------------------------------*/
GHS_RACP_Record_t record;
FlagStatus recordSent = SET;
GHS_SegmentationHeader_t SegmentationHeader;    

/**
* START of Section BLE_DRIVER_CONTEXT
*/
PLACE_IN_SECTION("BLE_DRIVER_CONTEXT") static GHS_RACP_Context_t GHS_RACP_Context;

/* Private functions ---------------------------------------------------------*/
extern uint8_t a_GHS_UpdateCharData[247];

/* RACP response functions */
tBleStatus GHS_RACP_send_number_of_stored_records_response(uint32_t numberOfRecords);

/* RACP filter functions */
uint8_t GHS_RACP_recordnumber_is_greater_or_equal(GHS_RACP_Record_t * record, const GHS_RACP_Filter_t * filter);
uint8_t GHS_RACP_recordnumber_is_less_or_equal(GHS_RACP_Record_t * record, const GHS_RACP_Filter_t * filter);
uint8_t GHS_RACP_recordnumber_is_within_range(GHS_RACP_Record_t * record, const GHS_RACP_Filter_t * filter);

/**
* @brief Check if the record matches greater or equal criteria
* @param record : Record to be checked
* @param filter : Filtering criteria against which the record it tested
* @retval TRUE if matches, FALSE (!TRUE) otherwise
*/
uint8_t GHS_RACP_recordnumber_is_greater_or_equal(GHS_RACP_Record_t * record, const GHS_RACP_Filter_t * filter)
{
  if (record->SHOSegment.RecordNumber >= ((GHS_RACP_RecordNumberFilter_t *)filter)->Minimum)
  {
    return TRUE;
  }
  
  return FALSE;
} /* end of GHS_RACP_recordnumber_is_greater_or_equal() */

/**
* @brief Check if the record matches less or equal criteria
* @param record : Record to be checked
* @param filter : Filtering criteria against which the record it tested
* @retval TRUE if matches, FALSE (!TRUE) otherwise
*/
uint8_t GHS_RACP_recordnumber_is_less_or_equal(GHS_RACP_Record_t * record, const GHS_RACP_Filter_t * filter)
{
  if (record->SHOSegment.RecordNumber <= ((GHS_RACP_RecordNumberFilter_t *)filter)->Maximum)
  {
    return TRUE;
  }
  
  return FALSE;
} /* end of GHS_RACP_recordnumber_is_less_or_equal() */

/**
* @brief Check if the record matches within range criteria
* @param record : Record to be checked
* @param filter : Filtering criteria against which the record it tested
* @retval TRUE if matches, FALSE (!TRUE) otherwise
*/
uint8_t GHS_RACP_recordnumber_is_within_range(GHS_RACP_Record_t * record, const GHS_RACP_Filter_t * filter)
{
  if ((GHS_RACP_recordnumber_is_greater_or_equal(record, filter) == TRUE)
      && (GHS_RACP_recordnumber_is_less_or_equal(record, filter) == TRUE))
  {
    return TRUE;
  }
  
  return FALSE;
} /* end of GHS_RACP_recordnumber_is_within_range() */

/**
* @brief Send a RACP response as consequence of a RACP request
* @param responseCode: RACP response code or number of stored records
* @retval None
*/
tBleStatus GHS_RACP_send_response_code(uint8_t responseCode)
{
  GHS_Data_t ghsData;
  tBleStatus retval = BLE_STATUS_FAILED;
  uint8_t a_data[6];
  
  if((GHS_RACP_Context.Procedure.OpCode == GHS_RACP_OP_CODE_COMBINED_REPORT) &&
     (GHS_RACP_Context.NumberOfRecord > 0))
  {
    GHS_RACP_Context.Response.OpCode = GHS_RACP_OP_CODE_COMBINED_REPORT_RESPONSE;
    a_data[0] = GHS_RACP_Context.Response.OpCode;
    a_data[1] = GHS_RACP_OPERATOR_NULL;
    a_data[2] = (uint8_t)(0x00FF & (GHS_RACP_Context.NumberOfRecord));
    a_data[3] = (uint8_t)(((GHS_RACP_Context.NumberOfRecord) >> 8) & 0xFF);
    a_data[4] = (uint8_t)(((GHS_RACP_Context.NumberOfRecord) >> 16) & 0xFF);
    a_data[5] = (uint8_t)(((GHS_RACP_Context.NumberOfRecord) >> 24) & 0xFF);
    ghsData.Length = 6;
    ghsData.p_Payload = a_data;
  }
  else
  {
    GHS_RACP_Context.Response.OpCode = GHS_RACP_OP_CODE_RESPONSE_CODE;
    a_data[0] = GHS_RACP_Context.Response.OpCode;
    a_data[1] = GHS_RACP_OPERATOR_NULL;
    a_data[2] = GHS_RACP_Context.Procedure.OpCode;
    a_data[3] = responseCode;
    ghsData.Length = 4;
    ghsData.p_Payload = a_data;
  }
  
  /* RACP response is indicated */
  retval = GHS_UpdateValue(GHS_RACP, &ghsData);
  
  if(retval == BLE_STATUS_SUCCESS)
  {
    LOG_INFO_APP("RACP STATE = RACP_PROCEDURE_FINISHED\r\n");
    
    /* unset flag for RACP operation already in progress */
    GHS_RACP_Context.State = GHS_RACP_STATE_IDLE;
  }
  else 
  {
    LOG_INFO_APP("FAILED to send RACP response 0x%02X\r\n", retval);
  }
  
  return retval;
} /* end of GHS_RACP_send_response_code() */

/**
* @brief Send requested number of stored records matching the given criteria
* @param numberOfRecords: number of stored records
* @retval None
*/
tBleStatus GHS_RACP_send_number_of_stored_records_response(uint32_t numberOfRecords)
{
  GHS_Data_t ghsData;
  tBleStatus retval = BLE_STATUS_FAILED;
  uint8_t a_data[6];
  
  GHS_RACP_Context.Response.OpCode = GHS_RACP_OP_CODE_NUMBER_STORED_RECORDS_RESPONSE;
  ((GHS_RACP_NumberOfStoredRecordsResponseOperand *)GHS_RACP_Context.Response.Operand)->NumberOfStoredRecords = numberOfRecords;
  
  a_data[0] = GHS_RACP_Context.Response.OpCode;
  a_data[1] = GHS_RACP_OPERATOR_NULL;
  a_data[2] = (uint8_t)(0x00FF & numberOfRecords);
  a_data[3] = (uint8_t)((numberOfRecords >> 8) & 0xFF);
  a_data[4] = (uint8_t)((numberOfRecords >> 16) & 0xFF);
  a_data[5] = (uint8_t)((numberOfRecords >> 24) & 0xFF);
  
  ghsData.Length = 6;
  ghsData.p_Payload = a_data;
    
  /* RACP response is indicated */
  retval = GHS_UpdateValue(GHS_RACP, &ghsData);
  
  if(retval == BLE_STATUS_SUCCESS)
  {
    LOG_INFO_APP("RACP STATE = RACP_PROCEDURE_FINISHED\r\n");
    
    /* unset flag for RACP operation already in progress */
    GHS_RACP_Context.State = GHS_RACP_STATE_IDLE;
  }
  else 
  {
    LOG_INFO_APP("FAILED to send RACP response 0x%02X\r\n", retval);
  }
  
  return retval;
} /* end of GHS_RACP_send_number_of_stored_records_response() */

/* Public functions ----------------------------------------------------------*/

/**
* @brief Initialize the RACP Context state and configuration
* @param None
* @retval None
*/
void GHS_RACP_Init(void)
{
  GHS_RACP_Context.State = GHS_RACP_STATE_IDLE;
  memset(&GHS_RACP_Context.Procedure, 0x00, sizeof(GHS_RACP_Procedure_t));
  memset(&GHS_RACP_Context.Response, 0x00, sizeof(GHS_RACP_Response_t));

  SegmentationHeader.FirstSegment = 1;
  SegmentationHeader.LastSegment = 1;
  SegmentationHeader.SegmentCounter = 0;  
} /* end of GHS_RACP_Init() */

/**
* @brief RACP Long Procedure processing function 
* @param None
* @retval TRUE if OK and still pending, FALSE if no Long Procedure or not pending anymore
*/
uint8_t GHS_RACP_ProcessReportRecordsProcedure(void)
{
  uint8_t retval = FALSE;
  tBleStatus ble_status;
  GHS_RACP_App_Event_t RACPAppEvent;
  
  if (GHS_RACP_Context.State != GHS_RACP_STATE_IDLE)
  {
    if (GHS_RACP_Context.State != GHS_RACP_STATE_PROCEDURE_ABORT_RECEIVED)
    {
      switch(GHS_RACP_Context.Procedure.OpCode)
      {
        case GHS_RACP_OP_CODE_REPORT_STORED_RECORDS:
        case GHS_RACP_OP_CODE_COMBINED_REPORT:
          {
            if (recordSent == SET)
            {
              if (GHS_DB_GetNextSelectedRecord(&record) == TRUE)
              {
                LOG_INFO_APP("GHS RACP next selected record found in DB\r\n");
                ble_status = GHS_APP_UpdateStoredHealthObservation(&record);
                if (ble_status == BLE_STATUS_SUCCESS)
                {
                  recordSent = SET;
                  LOG_INFO_APP("GHS RACP record %d sent successfully\r\n", record.SHOSegment.RecordNumber);
                  RACPAppEvent.EventCode = GHS_RACP_APP_REPORT_RECORDS_PROCEDURE_NOTIFY_NEXT_RECORD_EVENT;
                  GHS_RACP_APP_EventHandler(&RACPAppEvent);
                  retval = TRUE;
                }
                else 
                {
                  /* Last recording sending failed */
                  recordSent = RESET;
                  retval = FALSE;
                }
              }
              else 
              {
                /* No further record selected */
                LOG_INFO_APP("GHS RACP no more selected record found in DB\r\n");
                RACPAppEvent.EventCode = GHS_RACP_APP_REPORT_RECORDS_PROCEDURE_FINISHED_EVENT;
                GHS_RACP_APP_EventHandler(&RACPAppEvent);
                GHS_RACP_send_response_code(GHS_RACP_RESPONSE_SUCCESS);
                retval = FALSE;
              }
            }
            else 
            {
              ble_status = GHS_APP_UpdateStoredHealthObservation(&record);
              if (ble_status == BLE_STATUS_SUCCESS)
              {
                recordSent = SET;
                LOG_INFO_APP("GHS RACP first record %d sent successfully\r\n", record.SHOSegment.RecordNumber);
                retval = TRUE;
              }
              else 
              {
                /* Last recording sending failed */
                LOG_INFO_APP("GHS RACP last measurement record sent failed\r\n");
                recordSent = RESET;
                retval = FALSE;
              }
            }
          }
          break;
          
        default:
          {
            retval = FALSE;
          }
          break;
      }
    }
    else 
    {
      LOG_INFO_APP("GHS_RACP_STATE_PROCEDURE_ABORT_RECEIVED\r\n");
      /* No further record to be sent */
      RACPAppEvent.EventCode = GHS_RACP_APP_REPORT_RECORDS_PROCEDURE_FINISHED_EVENT;
      GHS_RACP_APP_EventHandler(&RACPAppEvent);
      GHS_RACP_send_response_code(GHS_RACP_RESPONSE_SUCCESS);
    }
  }
  
  return retval;
} /* end of GHS_RACP_ProcessReportRecordsProcedure() */

/**
* @brief RACP request handler 
* @param requestData: pointer to received RACP request data
* @param requestDataLength: received RACP request length
* @retval None
*/
void GHS_RACP_RequestHandler(uint8_t * pRequestData, uint8_t requestDataLength)
{
  GHS_RACP_App_Event_t RACPAppEvent;
  
  GHS_RACP_Context.NumberOfRecord = 0;
  
  LOG_INFO_APP("RACP Request, request data length: %d\r\n", requestDataLength);
  
  if ((GHS_RACP_Context.State != GHS_RACP_STATE_IDLE) && ((pRequestData[GHS_RACP_OP_CODE_POSITION]) != GHS_RACP_OP_CODE_ABORT_OPERATION))
  {
    // STM_TODO : Implement error management, error management to be further extended, no propagation currently
  }
  else if (GHS_APP_GetLHOTimerStarted() == TRUE)
  {
      GHS_RACP_Context.Procedure.OpCode = pRequestData[GHS_RACP_OP_CODE_POSITION];
      GHS_RACP_Context.Procedure.Operator = pRequestData[GHS_RACP_OPERATOR_POSITION];
      /* RACP procedure data must be rescheduled the server is busy */
      GHS_RACP_send_response_code(GHS_RACP_RESPONSE_SERVER_BUSY);
  }
  else 
  {
    if (requestDataLength < GHS_RACP_FILTER_TYPE_NO_OPERAND_LENGTH)
    {
      /* RACP procedure data must be always at least 2 bytes long */
      GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPCODE_NOT_SUPPORTED);
    }
    else 
    {
      GHS_RACP_Context.Procedure.OpCode = pRequestData[GHS_RACP_OP_CODE_POSITION];
      GHS_RACP_Context.Procedure.Operator = pRequestData[GHS_RACP_OPERATOR_POSITION];
      LOG_INFO_APP("RACP Procedure OpCode: 0x%02X, Operator: 0x%02X\r\n", GHS_RACP_Context.Procedure.OpCode, GHS_RACP_Context.Procedure.OpCode);
      memset(&GHS_RACP_Context.Procedure.Operand, 0x00, GHS_RACP_OP_CODE_RESPONSE_OPERAND_MAX_LENGTH);
      
      /* Set in progress flag on reception of a new RACP request */
      GHS_RACP_Context.State = GHS_RACP_STATE_PROCEDURE_IN_PROGRESS;
      LOG_INFO_APP("RACP STATE = RACP_PROCEDURE_IN_PROGRESS\r\n");
      
      /* Check and Process the OpCode */
      switch(GHS_RACP_Context.Procedure.OpCode)
      {
        case GHS_RACP_OP_CODE_REPORT_STORED_RECORDS:
          {
            LOG_INFO_APP("GHS_RACP_REPORT_STORED_RECORDS_OP_CODE\r\n");
            switch(GHS_RACP_Context.Procedure.Operator)
            {
              case GHS_RACP_OPERATOR_ALL_RECORDS:
                {
                  LOG_INFO_APP("GHS_RACP_OPERATOR_ALL_RECORDS\r\n");
                  if (requestDataLength != GHS_RACP_FILTER_TYPE_NO_OPERAND_LENGTH)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else 
                  {
                    GHS_RACP_Context.NumberOfRecord = GHS_DB_SelectRecords(NULL, NULL);
                    if (GHS_RACP_Context.NumberOfRecord > 0)
                    {
                      RACPAppEvent.EventCode = GHS_RACP_APP_REPORT_RECORDS_PROCEDURE_STARTED_EVENT;
                      GHS_RACP_APP_EventHandler(&RACPAppEvent);
                    }
                    else 
                    {
                      GHS_RACP_send_response_code(GHS_RACP_RESPONSE_NO_RECORDS);
                    }
                  }
                }
                break;
                
              case GHS_RACP_OPERATOR_GREATER_EQUAL:
                {
                  LOG_INFO_APP("GHS_RACP_OPERATOR_GREATER_EQUAL\r\n");
                  if (requestDataLength != GHS_RACP_FILTER_TYPE_LESS_GREATER_RECORD_NUMBER_LENGTH)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else if (pRequestData[GHS_RACP_OPERAND_POSITION] != GHS_RACP_FILTER_TYPE_RECORD_NUMBER)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else 
                  {
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->FilterType = pRequestData[GHS_RACP_OPERAND_POSITION];
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Minimum = 
                      (uint32_t)((pRequestData[GHS_RACP_OPERAND_POSITION+1])       | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+2] << 8)  |
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+3] << 16) | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+4] << 24));
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Maximum = 0xFFFFFFFF;
                    GHS_RACP_Context.NumberOfRecord = GHS_DB_SelectRecords(&GHS_RACP_recordnumber_is_greater_or_equal, (const GHS_RACP_Filter_t *)(&(GHS_RACP_Context.Procedure.Operand)));
                    if (GHS_RACP_Context.NumberOfRecord > 0)
                    {     
                      RACPAppEvent.EventCode = GHS_RACP_APP_REPORT_RECORDS_PROCEDURE_STARTED_EVENT;
                      GHS_RACP_APP_EventHandler(&RACPAppEvent);
                    }
                    else 
                    {
                      GHS_RACP_send_response_code(GHS_RACP_RESPONSE_NO_RECORDS);
                    }
                  }
                }
                break;
                
              case GHS_RACP_OPERATOR_LESS_EQUAL:
                {
                  LOG_INFO_APP("BLE_CFG_GHS_RACP_OP_CODE_REPORT_STORED_RECORDS_LESS_EQUAL\r\n");
                  if (requestDataLength != GHS_RACP_FILTER_TYPE_LESS_GREATER_RECORD_NUMBER_LENGTH)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else if (pRequestData[GHS_RACP_OPERAND_POSITION] != GHS_RACP_FILTER_TYPE_RECORD_NUMBER)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else 
                  {
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->FilterType = pRequestData[GHS_RACP_OPERAND_POSITION];
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Minimum = 0x0;
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Maximum = 
                      (uint32_t)((pRequestData[GHS_RACP_OPERAND_POSITION+1])       | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+2] << 8)  |
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+3] << 16) | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+4] << 24));
                      GHS_RACP_Context.NumberOfRecord = GHS_DB_SelectRecords(&GHS_RACP_recordnumber_is_less_or_equal, (GHS_RACP_Filter_t *)(&(GHS_RACP_Context.Procedure.Operand)));
                    if (GHS_RACP_Context.NumberOfRecord > 0)
                    {
                      RACPAppEvent.EventCode = GHS_RACP_APP_REPORT_RECORDS_PROCEDURE_STARTED_EVENT;
                      GHS_RACP_APP_EventHandler(&RACPAppEvent);
                    }
                    else 
                    {
                      GHS_RACP_send_response_code(GHS_RACP_RESPONSE_NO_RECORDS);
                    }
                  }
                }
                break;
                
              case GHS_RACP_OPERATOR_WITHIN_RANGE:
                {
                  LOG_INFO_APP("GHS_RACP_OPERATOR_WITHIN_RANGE\r\n");
                  if (requestDataLength != GHS_RACP_FILTER_TYPE_WITHIN_RANGE_RECORD_NUMBER_LENGTH)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else if (pRequestData[GHS_RACP_OPERAND_POSITION] != GHS_RACP_FILTER_TYPE_RECORD_NUMBER)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else 
                  {
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->FilterType = pRequestData[GHS_RACP_OPERAND_POSITION];
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Minimum = 
                      (uint32_t)((pRequestData[GHS_RACP_OPERAND_POSITION+1])       | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+2] << 8)  |
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+3] << 16) | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+4] << 24));
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Maximum = 
                      (uint32_t)((pRequestData[GHS_RACP_OPERAND_POSITION+5])       | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+6] << 8)  |
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+7] << 16) | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+8] << 24));
                    if (((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Minimum > ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Maximum)
                    {
                      GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                    }
                    else 
                    {
                      GHS_RACP_Context.NumberOfRecord = GHS_DB_SelectRecords(&GHS_RACP_recordnumber_is_within_range, (const GHS_RACP_Filter_t *)(&(GHS_RACP_Context.Procedure.Operand)));
                      if (GHS_RACP_Context.NumberOfRecord > 0)
                      {
                        RACPAppEvent.EventCode = GHS_RACP_APP_REPORT_RECORDS_PROCEDURE_STARTED_EVENT;
                        GHS_RACP_APP_EventHandler(&RACPAppEvent);
                      }
                      else 
                      {
                        GHS_RACP_send_response_code(GHS_RACP_RESPONSE_NO_RECORDS);
                      }
                    }
                  }
                }
                break;
                
              case GHS_RACP_OPERATOR_FIRST_RECORD:
                {
                  LOG_INFO_APP("GHS_RACP_OPERATOR_FIRST_RECORD\r\n");
                  if (requestDataLength != GHS_RACP_FILTER_TYPE_NO_OPERAND_LENGTH)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else 
                  {
                    GHS_RACP_Context.NumberOfRecord = GHS_DB_SelectRecordByIndex(0x0000);
                    if (GHS_RACP_Context.NumberOfRecord > 0)
                    {
                      RACPAppEvent.EventCode = GHS_RACP_APP_REPORT_RECORDS_PROCEDURE_STARTED_EVENT;
                      GHS_RACP_APP_EventHandler(&RACPAppEvent);
                    }
                    else 
                    {
                      GHS_RACP_send_response_code(GHS_RACP_RESPONSE_NO_RECORDS);
                    }
                  }
                }
                break;
                
              case GHS_RACP_OPERATOR_LAST_RECORD:
                {
                  LOG_INFO_APP("GHS_RACP_OPERATOR_LAST_RECORD\r\n");
                  if (requestDataLength != GHS_RACP_FILTER_TYPE_NO_OPERAND_LENGTH)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else 
                  {
                    GHS_RACP_Context.NumberOfRecord = GHS_DB_SelectRecordByIndex(0xFFFF);
                    if (GHS_RACP_Context.NumberOfRecord > 0)
                    {
                      RACPAppEvent.EventCode = GHS_RACP_APP_REPORT_RECORDS_PROCEDURE_STARTED_EVENT;
                      GHS_RACP_APP_EventHandler(&RACPAppEvent);
                    }
                    else 
                    {
                      GHS_RACP_send_response_code(GHS_RACP_RESPONSE_NO_RECORDS);
                    }
                  }
                }
                break;
                
              case GHS_RACP_OPERATOR_NULL:
                {
                  LOG_INFO_APP("GHS_RACP_OPERATOR_NULL\r\n");
                  GHS_RACP_send_response_code(GHS_RACP_RESPONSE_INVALID_OPERATOR);
                }
                break;
                
              default:
                {
                  GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERATOR_NOT_SUPPORTED);
                }
                break;
            }
          }
          break;
        
        case GHS_RACP_OP_CODE_DELETE_STORED_RECORDS:
          {
            LOG_INFO_APP("GHS_RACP_OP_CODE_DELETE_STORED_RECORDS\r\n"); 
            switch(GHS_RACP_Context.Procedure.Operator)
            {
              case GHS_RACP_OPERATOR_ALL_RECORDS:
                {
                  LOG_INFO_APP("GHS_RACP_OPERATOR_ALL_RECORDS\r\n"); 
                  if (requestDataLength != GHS_RACP_FILTER_TYPE_NO_OPERAND_LENGTH)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else 
                  {
                    GHS_RACP_Context.NumberOfRecord = GHS_DB_DeleteRecords(NULL, NULL);
                    if (GHS_RACP_Context.NumberOfRecord > 0)
                    {
                      GHS_APP_UpdateDbRecordCount(GHS_RACP_Context.NumberOfRecord);

                      GHS_RACP_send_response_code(GHS_RACP_RESPONSE_SUCCESS);
                    }
                    else 
                    {
                      GHS_RACP_send_response_code(GHS_RACP_RESPONSE_NO_RECORDS);
                    }
                  }
                }
                break;
                
              case GHS_RACP_OPERATOR_GREATER_EQUAL:
                {
                  LOG_INFO_APP("GHS_RACP_OPERATOR_GREATER_EQUAL\r\n"); 
                  if (requestDataLength != GHS_RACP_FILTER_TYPE_LESS_GREATER_RECORD_NUMBER_LENGTH)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else if (pRequestData[GHS_RACP_OPERAND_POSITION] != GHS_RACP_FILTER_TYPE_RECORD_NUMBER)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else 
                  {
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->FilterType = pRequestData[GHS_RACP_OPERAND_POSITION];
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Minimum = 
                      (uint32_t)((pRequestData[GHS_RACP_OPERAND_POSITION+1])       | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+2] << 8)  |
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+3] << 16) | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+4] << 24));
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Maximum = 0xFFFFFFFF;
                    GHS_RACP_Context.NumberOfRecord = GHS_DB_DeleteRecords(&GHS_RACP_recordnumber_is_greater_or_equal, (const GHS_RACP_Filter_t *)(&(GHS_RACP_Context.Procedure.Operand)));
                    if (GHS_RACP_Context.NumberOfRecord > 0)
                    {
                      GHS_APP_UpdateDbRecordCount(GHS_RACP_Context.NumberOfRecord);

                      GHS_RACP_send_response_code(GHS_RACP_RESPONSE_SUCCESS);
                    }
                    else 
                    {
                      GHS_RACP_send_response_code(GHS_RACP_RESPONSE_NO_RECORDS);
                    }
                  }
                }
                break;
                
              case GHS_RACP_OPERATOR_LESS_EQUAL:
                {
                  LOG_INFO_APP("GHS_RACP_OPERATOR_LESS_EQUAL\r\n"); 
                  if (requestDataLength != GHS_RACP_FILTER_TYPE_LESS_GREATER_RECORD_NUMBER_LENGTH)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else if (pRequestData[GHS_RACP_OPERAND_POSITION] != GHS_RACP_FILTER_TYPE_RECORD_NUMBER)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else 
                  {
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->FilterType = pRequestData[GHS_RACP_OPERAND_POSITION];
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Minimum = 0x0;
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Maximum = 
                      (uint32_t)((pRequestData[GHS_RACP_OPERAND_POSITION+1])       | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+2] << 8)  |
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+3] << 16) | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+4] << 24));
                    GHS_RACP_Context.NumberOfRecord = GHS_DB_DeleteRecords(&GHS_RACP_recordnumber_is_less_or_equal, (const GHS_RACP_Filter_t *)(&(GHS_RACP_Context.Procedure.Operand)));
                    if (GHS_RACP_Context.NumberOfRecord > 0)
                    {
                      GHS_APP_UpdateDbRecordCount(GHS_RACP_Context.NumberOfRecord);

                      GHS_RACP_send_response_code(GHS_RACP_RESPONSE_SUCCESS);
                    }
                    else 
                    {
                      GHS_RACP_send_response_code(GHS_RACP_RESPONSE_NO_RECORDS);
                    }
                  }
                }
                break;
                
              case GHS_RACP_OPERATOR_WITHIN_RANGE:
                {
                  LOG_INFO_APP("GHS_RACP_OPERATOR_WITHIN_RANGE\r\n"); 
                  if (requestDataLength != GHS_RACP_FILTER_TYPE_WITHIN_RANGE_RECORD_NUMBER_LENGTH)
                  {
                    LOG_INFO_APP("GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED\r\n"); 
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else if (pRequestData[GHS_RACP_OPERAND_POSITION] != GHS_RACP_FILTER_TYPE_RECORD_NUMBER)
                  {
                    LOG_INFO_APP("GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED\r\n"); 
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else 
                  {
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->FilterType = pRequestData[GHS_RACP_OPERAND_POSITION];
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Minimum = 
                      (uint32_t)((pRequestData[GHS_RACP_OPERAND_POSITION+1])       | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+2] << 8)  |
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+3] << 16) | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+4] << 24));
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Maximum = 
                      (uint32_t)((pRequestData[GHS_RACP_OPERAND_POSITION+5])       | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+6] << 8)  |
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+7] << 16) | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+8] << 24));
                    if (((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Minimum > ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Maximum)
                    {
                      GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                      LOG_INFO_APP("GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED\r\n"); 
                    }
                    else 
                    {
                      GHS_RACP_Context.NumberOfRecord = GHS_DB_DeleteRecords(&GHS_RACP_recordnumber_is_within_range, (const GHS_RACP_Filter_t *)(&(GHS_RACP_Context.Procedure.Operand)));
                      if (GHS_RACP_Context.NumberOfRecord > 0)
                      {
                        GHS_APP_UpdateDbRecordCount(GHS_RACP_Context.NumberOfRecord);

                        GHS_RACP_send_response_code(GHS_RACP_RESPONSE_SUCCESS);
                        LOG_INFO_APP("GHS_RACP_RESPONSE_SUCCESS\r\n"); 
                      }
                      else 
                      {
                        GHS_RACP_send_response_code(GHS_RACP_RESPONSE_NO_RECORDS);
                        LOG_INFO_APP("GHS_RACP_RESPONSE_NO_RECORDS\r\n"); 
                      }
                    }
                  }
                }
                break;
                
              case GHS_RACP_OPERATOR_FIRST_RECORD:
                {
                  LOG_INFO_APP("GHS_RACP_OPERATOR_FIRST_RECORD\r\n"); 
                  if (requestDataLength != GHS_RACP_FILTER_TYPE_NO_OPERAND_LENGTH)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else 
                  {
                    GHS_RACP_Context.NumberOfRecord = GHS_DB_DeleteRecordByIndex(0x0000);
                    if (GHS_RACP_Context.NumberOfRecord > 0)
                    {
                      GHS_APP_UpdateDbRecordCount(GHS_RACP_Context.NumberOfRecord);

                      GHS_RACP_send_response_code(GHS_RACP_RESPONSE_SUCCESS);
                    }
                    else 
                    {
                      GHS_RACP_send_response_code(GHS_RACP_RESPONSE_NO_RECORDS);
                    }
                  }
                }
                break;
                
              case GHS_RACP_OPERATOR_LAST_RECORD:
                {
                  LOG_INFO_APP("GHS_RACP_OPERATOR_LAST_RECORD\r\n"); 
                  if (requestDataLength != GHS_RACP_FILTER_TYPE_NO_OPERAND_LENGTH)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else 
                  {
                    GHS_RACP_Context.NumberOfRecord = GHS_DB_DeleteRecordByIndex(0xFFFF);
                    if (GHS_RACP_Context.NumberOfRecord > 0)
                    {
                      GHS_APP_UpdateDbRecordCount(GHS_RACP_Context.NumberOfRecord);

                      GHS_RACP_send_response_code(GHS_RACP_RESPONSE_SUCCESS);
                    }
                    else 
                    {
                      GHS_RACP_send_response_code(GHS_RACP_RESPONSE_NO_RECORDS);
                    }
                  }
                }
                break;
                
              case GHS_RACP_OPERATOR_NULL:
                {
                  LOG_INFO_APP("GHS_RACP_OPERATOR_NULL\r\n"); 
                  GHS_RACP_send_response_code(GHS_RACP_RESPONSE_INVALID_OPERATOR);
                }
                break;
                
              default:
                {
                  GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERATOR_NOT_SUPPORTED);
                }
                break;
            }
          }
          break;
        
        case GHS_RACP_OP_CODE_REPORT_NUMBER_STORED_RECORDS:
          {
            LOG_INFO_APP("GHS_RACP_OP_CODE_REPORT_NUMBER_STORED_RECORDS\r\n"); 
            switch(GHS_RACP_Context.Procedure.Operator)
            {
              case GHS_RACP_OPERATOR_ALL_RECORDS:
                {
                  LOG_INFO_APP("GHS_RACP_OPERATOR_ALL_RECORDS\r\n"); 
                  if(requestDataLength != GHS_RACP_FILTER_TYPE_NO_OPERAND_LENGTH)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else 
                  {
                    GHS_RACP_Context.NumberOfRecord = GHS_DB_GetRecordsCount(NULL, NULL);
                    GHS_RACP_send_number_of_stored_records_response(GHS_RACP_Context.NumberOfRecord);
                  }
                }
                break;
                
              case GHS_RACP_OPERATOR_GREATER_EQUAL:
                {
                  LOG_INFO_APP("GHS_RACP_OPERATOR_GREATER_EQUAL\r\n"); 
                  if(requestDataLength != GHS_RACP_FILTER_TYPE_LESS_GREATER_RECORD_NUMBER_LENGTH)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else if (pRequestData[GHS_RACP_OPERAND_POSITION] != GHS_RACP_FILTER_TYPE_RECORD_NUMBER)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else 
                  {
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->FilterType = pRequestData[GHS_RACP_OPERAND_POSITION];
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Minimum = 
                      (uint32_t)((pRequestData[GHS_RACP_OPERAND_POSITION+1])       | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+2] << 8)  |
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+3] << 16) | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+4] << 24));
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Maximum = 0xFFFFFFFF;
                    GHS_RACP_Context.NumberOfRecord = GHS_DB_GetRecordsCount(&GHS_RACP_recordnumber_is_greater_or_equal, (const GHS_RACP_Filter_t *)(&(GHS_RACP_Context.Procedure.Operand)));
                    GHS_RACP_send_number_of_stored_records_response(GHS_RACP_Context.NumberOfRecord);
                  }
                }
                break;
                
              case GHS_RACP_OPERATOR_LESS_EQUAL:
                {
                  LOG_INFO_APP("GHS_RACP_OPERATOR_LESS_EQUAL\r\n"); 
                  if(requestDataLength != GHS_RACP_FILTER_TYPE_LESS_GREATER_RECORD_NUMBER_LENGTH)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else if (pRequestData[GHS_RACP_OPERAND_POSITION] != GHS_RACP_FILTER_TYPE_RECORD_NUMBER)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else 
                  {
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->FilterType = pRequestData[GHS_RACP_OPERAND_POSITION];
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Minimum = 0x0;
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Maximum = 
                      (uint32_t)((pRequestData[GHS_RACP_OPERAND_POSITION+1])       | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+2] << 8)  |
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+3] << 16) | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+4] << 24));
                    GHS_RACP_Context.NumberOfRecord = GHS_DB_GetRecordsCount(&GHS_RACP_recordnumber_is_less_or_equal, (const GHS_RACP_Filter_t *)(&(GHS_RACP_Context.Procedure.Operand)));
                    GHS_RACP_send_number_of_stored_records_response(GHS_RACP_Context.NumberOfRecord);
                  }
                }
                break;
                
              case GHS_RACP_OPERATOR_WITHIN_RANGE:
                {
                  LOG_INFO_APP("GHS_RACP_OPERATOR_WITHIN_RANGE\r\n"); 
                  if(requestDataLength != GHS_RACP_FILTER_TYPE_WITHIN_RANGE_RECORD_NUMBER_LENGTH)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else if (pRequestData[GHS_RACP_OPERAND_POSITION] != GHS_RACP_FILTER_TYPE_RECORD_NUMBER)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else 
                  {
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->FilterType = pRequestData[GHS_RACP_OPERAND_POSITION];
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Minimum = 
                      (uint32_t)((pRequestData[GHS_RACP_OPERAND_POSITION+1])       | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+2] << 8)  |
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+3] << 16) | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+4] << 24));
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Maximum = 
                      (uint32_t)((pRequestData[GHS_RACP_OPERAND_POSITION+5])       | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+6] << 8)  |
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+7] << 16) | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+8] << 24));
                    if (((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Minimum > ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Maximum)
                    {
                      GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                    }
                    else 
                    {
                      GHS_RACP_send_number_of_stored_records_response(GHS_RACP_Context.NumberOfRecord);
                    }
                  }
                }
                break;
                
              case GHS_RACP_OPERATOR_FIRST_RECORD:
                {
                  LOG_INFO_APP("GHS_RACP_OPERATOR_ALL_RECORDS\r\n"); 
                  if(requestDataLength != GHS_RACP_FILTER_TYPE_NO_OPERAND_LENGTH)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else 
                  {
                    GHS_RACP_Context.NumberOfRecord = GHS_DB_GetRecordsCount(NULL, NULL);
                    if (GHS_RACP_Context.NumberOfRecord != 0)
                    {
                      GHS_RACP_Context.NumberOfRecord = 1;
                    }
                    GHS_RACP_send_number_of_stored_records_response(GHS_RACP_Context.NumberOfRecord);
                  }
                }
                break;
                
              case GHS_RACP_OPERATOR_LAST_RECORD:
                {
                  LOG_INFO_APP("GHS_RACP_OPERATOR_ALL_RECORDS\r\n"); 
                  if(requestDataLength != GHS_RACP_FILTER_TYPE_NO_OPERAND_LENGTH)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else 
                  {
                    GHS_RACP_Context.NumberOfRecord = GHS_DB_GetRecordsCount(NULL, NULL);
                    if (GHS_RACP_Context.NumberOfRecord != 0)
                    {
                      GHS_RACP_Context.NumberOfRecord = 1;
                    }
                    GHS_RACP_send_number_of_stored_records_response(GHS_RACP_Context.NumberOfRecord);
                  }
                }
                break;
                
              case GHS_RACP_OPERATOR_NULL:
                {
                  GHS_RACP_send_response_code(GHS_RACP_RESPONSE_INVALID_OPERATOR);
                }
                break;
                
              default:
                {
                  GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERATOR_NOT_SUPPORTED);
                }
                break;
            }
          }
          break;
        
        case GHS_RACP_OP_CODE_ABORT_OPERATION:
          {
            LOG_INFO_APP("GHS_RACP_OP_CODE_ABORT_OPERATION\r\n"); 
            if((requestDataLength != GHS_RACP_FILTER_TYPE_NO_OPERAND_LENGTH) ||
               (GHS_RACP_Context.Procedure.Operator != GHS_RACP_OPERATOR_NULL))
            {
              /* Operator must be 0x00 and NO operand for Abort operation procedure */
              GHS_RACP_send_response_code(GHS_RACP_RESPONSE_INVALID_OPERATOR);
            }
            else 
            {
              GHS_RACP_Context.State = GHS_RACP_STATE_PROCEDURE_ABORT_RECEIVED;
            }
          }
          break;
          
        case GHS_RACP_OP_CODE_COMBINED_REPORT:
          {
            LOG_INFO_APP("GHS_RACP_OP_CODE_COMBINED_REPORT\r\n");
            switch(GHS_RACP_Context.Procedure.Operator)
            {
              case GHS_RACP_OPERATOR_ALL_RECORDS:
                {
                  LOG_INFO_APP("GHS_RACP_OPERATOR_ALL_RECORDS\r\n");
                  if (requestDataLength != GHS_RACP_FILTER_TYPE_NO_OPERAND_LENGTH)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else 
                  {
                    GHS_RACP_Context.NumberOfRecord = GHS_DB_SelectRecords(NULL, NULL);
                    if (GHS_RACP_Context.NumberOfRecord > 0)
                    {
                      RACPAppEvent.EventCode = GHS_RACP_APP_REPORT_RECORDS_PROCEDURE_STARTED_EVENT;
                      GHS_RACP_APP_EventHandler(&RACPAppEvent);
                    }
                    else 
                    {
                      GHS_RACP_send_response_code(GHS_RACP_RESPONSE_NO_RECORDS);
                    }
                  }
                }
                break;
                
              case GHS_RACP_OPERATOR_GREATER_EQUAL:
                {
                  LOG_INFO_APP("GHS_RACP_OPERATOR_GREATER_EQUAL\r\n");
                  LOG_INFO_APP("requestDataLength %d GHS_RACP_FILTER_TYPE_LESS_GREATER_RECORD_NUMBER_LENGTH %d\r\n",
                               requestDataLength, GHS_RACP_FILTER_TYPE_LESS_GREATER_RECORD_NUMBER_LENGTH);
                  LOG_INFO_APP("pRequestData[GHS_RACP_OPERAND_POSITION] %d GHS_RACP_FILTER_TYPE_RECORD_NUMBER %d\r\n",
                               pRequestData[GHS_RACP_OPERAND_POSITION], GHS_RACP_FILTER_TYPE_RECORD_NUMBER);
                  if (requestDataLength != GHS_RACP_FILTER_TYPE_LESS_GREATER_RECORD_NUMBER_LENGTH)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else if (pRequestData[GHS_RACP_OPERAND_POSITION] != GHS_RACP_FILTER_TYPE_RECORD_NUMBER)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else 
                  {
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->FilterType = pRequestData[GHS_RACP_OPERAND_POSITION];
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Minimum = 
                      (uint32_t)((pRequestData[GHS_RACP_OPERAND_POSITION+1])       | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+2] << 8)  |
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+3] << 16) | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+4] << 24));
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Maximum = 0xFFFFFFFF;
                    GHS_RACP_Context.NumberOfRecord = GHS_DB_SelectRecords(&GHS_RACP_recordnumber_is_greater_or_equal, (const GHS_RACP_Filter_t *)(&(GHS_RACP_Context.Procedure.Operand)));
                    if (GHS_RACP_Context.NumberOfRecord > 0)
                    {     
                      RACPAppEvent.EventCode = GHS_RACP_APP_REPORT_RECORDS_PROCEDURE_STARTED_EVENT;
                      GHS_RACP_APP_EventHandler(&RACPAppEvent);
                    }
                    else 
                    {
                      GHS_RACP_send_response_code(GHS_RACP_RESPONSE_NO_RECORDS);
                    }
                  }
                }
                break;
                
              case GHS_RACP_OPERATOR_LESS_EQUAL:
                {
                  LOG_INFO_APP("BLE_CFG_GHS_RACP_OP_CODE_REPORT_STORED_RECORDS_LESS_EQUAL\r\n");
                  LOG_INFO_APP("requestDataLength %d GHS_RACP_FILTER_TYPE_LESS_GREATER_RECORD_NUMBER_LENGTH %d\r\n",
                               requestDataLength, GHS_RACP_FILTER_TYPE_LESS_GREATER_RECORD_NUMBER_LENGTH);
                  LOG_INFO_APP("pRequestData[GHS_RACP_OPERAND_POSITION] %d GHS_RACP_FILTER_TYPE_RECORD_NUMBER %d\r\n",
                               pRequestData[GHS_RACP_OPERAND_POSITION], GHS_RACP_FILTER_TYPE_RECORD_NUMBER);
                  if (requestDataLength != GHS_RACP_FILTER_TYPE_LESS_GREATER_RECORD_NUMBER_LENGTH)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else if (pRequestData[GHS_RACP_OPERAND_POSITION] != GHS_RACP_FILTER_TYPE_RECORD_NUMBER)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else 
                  {
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->FilterType = pRequestData[GHS_RACP_OPERAND_POSITION];
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Minimum = 0x0;
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Maximum = 
                      (uint32_t)((pRequestData[GHS_RACP_OPERAND_POSITION+1])       | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+2] << 8)  |
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+3] << 16) | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+4] << 24));
                      GHS_RACP_Context.NumberOfRecord = GHS_DB_SelectRecords(&GHS_RACP_recordnumber_is_less_or_equal, (GHS_RACP_Filter_t *)(&(GHS_RACP_Context.Procedure.Operand)));
                    if (GHS_RACP_Context.NumberOfRecord > 0)
                    {
                      RACPAppEvent.EventCode = GHS_RACP_APP_REPORT_RECORDS_PROCEDURE_STARTED_EVENT;
                      GHS_RACP_APP_EventHandler(&RACPAppEvent);
                    }
                    else 
                    {
                      GHS_RACP_send_response_code(GHS_RACP_RESPONSE_NO_RECORDS);
                    }
                  }
                }
                break;
                
              case GHS_RACP_OPERATOR_WITHIN_RANGE:
                {
                  LOG_INFO_APP("GHS_RACP_OPERATOR_WITHIN_RANGE\r\n");
                  LOG_INFO_APP("requestDataLength %d GHS_RACP_FILTER_TYPE_WITHIN_RANGE_RECORD_NUMBER_LENGTH %d\r\n",
                               requestDataLength, GHS_RACP_FILTER_TYPE_WITHIN_RANGE_RECORD_NUMBER_LENGTH);
                  LOG_INFO_APP("pRequestData[GHS_RACP_OPERAND_POSITION] %d GHS_RACP_FILTER_TYPE_RECORD_NUMBER %d\r\n",
                               pRequestData[GHS_RACP_OPERAND_POSITION], GHS_RACP_FILTER_TYPE_RECORD_NUMBER);
                  if (requestDataLength != GHS_RACP_FILTER_TYPE_WITHIN_RANGE_RECORD_NUMBER_LENGTH)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else if (pRequestData[GHS_RACP_OPERAND_POSITION] != GHS_RACP_FILTER_TYPE_RECORD_NUMBER)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else 
                  {
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->FilterType = pRequestData[GHS_RACP_OPERAND_POSITION];
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Minimum = 
                      (uint32_t)((pRequestData[GHS_RACP_OPERAND_POSITION+1])       | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+2] << 8)  |
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+3] << 16) | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+4] << 24));
                    ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Maximum = 
                      (uint32_t)((pRequestData[GHS_RACP_OPERAND_POSITION+5])       | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+6] << 8)  |
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+7] << 16) | 
                                 (pRequestData[GHS_RACP_OPERAND_POSITION+8] << 24));
                    if (((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Minimum > ((GHS_RACP_RecordNumberFilter_t *)(&GHS_RACP_Context.Procedure.Operand))->Maximum)
                    {
                      GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                    }
                    else 
                    {
                      GHS_RACP_Context.NumberOfRecord = GHS_DB_SelectRecords(&GHS_RACP_recordnumber_is_within_range, (const GHS_RACP_Filter_t *)(&(GHS_RACP_Context.Procedure.Operand)));
                      if (GHS_RACP_Context.NumberOfRecord > 0)
                      {
                        RACPAppEvent.EventCode = GHS_RACP_APP_REPORT_RECORDS_PROCEDURE_STARTED_EVENT;
                        GHS_RACP_APP_EventHandler(&RACPAppEvent);
                      }
                      else 
                      {
                        GHS_RACP_send_response_code(GHS_RACP_RESPONSE_NO_RECORDS);
                      }
                    }
                  }
                }
                break;
                
              case GHS_RACP_OPERATOR_FIRST_RECORD:
                {
                  LOG_INFO_APP("GHS_RACP_OPERATOR_FIRST_RECORD\r\n");
                  if (requestDataLength != GHS_RACP_FILTER_TYPE_NO_OPERAND_LENGTH)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else 
                  {
                    GHS_RACP_Context.NumberOfRecord = GHS_DB_SelectRecordByIndex(0x0000);
                    if (GHS_RACP_Context.NumberOfRecord > 0)
                    {
                      RACPAppEvent.EventCode = GHS_RACP_APP_REPORT_RECORDS_PROCEDURE_STARTED_EVENT;
                      GHS_RACP_APP_EventHandler(&RACPAppEvent);
                    }
                    else 
                    {
                      GHS_RACP_send_response_code(GHS_RACP_RESPONSE_NO_RECORDS);
                    }
                  }
                }
                break;
                
              case GHS_RACP_OPERATOR_LAST_RECORD:
                {
                  LOG_INFO_APP("GHS_RACP_OPERATOR_LAST_RECORD\r\n");
                  if (requestDataLength != GHS_RACP_FILTER_TYPE_NO_OPERAND_LENGTH)
                  {
                    GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED);
                  }
                  else 
                  {
                    GHS_RACP_Context.NumberOfRecord = GHS_DB_SelectRecordByIndex(0xFFFF);
                    if (GHS_RACP_Context.NumberOfRecord > 0)
                    {
                      RACPAppEvent.EventCode = GHS_RACP_APP_REPORT_RECORDS_PROCEDURE_STARTED_EVENT;
                      GHS_RACP_APP_EventHandler(&RACPAppEvent);
                    }
                    else 
                    {
                      GHS_RACP_send_response_code(GHS_RACP_RESPONSE_NO_RECORDS);
                    }
                  }
                }
                break;
                
              case GHS_RACP_OPERATOR_NULL:
                {
                  LOG_INFO_APP("GHS_RACP_OPERATOR_NULL\r\n");
                  GHS_RACP_send_response_code(GHS_RACP_RESPONSE_INVALID_OPERATOR);
                }
                break;
                
              default:
                {
                  GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPERATOR_NOT_SUPPORTED);
                }
                break;
            }
          }
          break;
        
        default:
          {
            GHS_RACP_send_response_code(GHS_RACP_RESPONSE_OPCODE_NOT_SUPPORTED);
          }
          break;
      }
    }
  }
} /* end of GHS_RACP_RequestHandler() */

/**
* @brief RACP new write request permit check
* @param [in] requestData: pointer to received RACP request data
* @param [in] requestDataLength: received RACP request length
* @retval 0x00 when no error, error code otherwise
*/
uint8_t GHS_RACP_CheckRequestValid(uint8_t * pRequestData, uint8_t requestDataLength)
{
  uint8_t retval = 0x00;
  
  LOG_INFO_APP("RACP Request, request data length: %d\r\n", requestDataLength);
  
  if ((GHS_RACP_Context.State == GHS_RACP_STATE_PROCEDURE_IN_PROGRESS) && 
      (pRequestData[GHS_RACP_OP_CODE_POSITION] != GHS_RACP_OP_CODE_ABORT_OPERATION))
  {
    retval = GHS_ATT_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS;
  }
  else if (GHS_APP_GetRACPCharacteristicIndicationEnabled() == FALSE)
  {
    retval = GHS_ATT_ERROR_CODE_CLIENT_CHAR_CONF_DESC_IMPROPERLY_CONFIGURED;
  }
  return retval;
} /* end of GHS_RACP_CheckRequestValid() */

/**
* @brief Get the flag holding whether any RACP operation is already in progress or not
* @param None
* @retval None
*/
GHS_RACP_State_t GHS_RACP_GetState(void)
{
  return GHS_RACP_Context.State;
} /* end of GHS_RACP_GetState() */
