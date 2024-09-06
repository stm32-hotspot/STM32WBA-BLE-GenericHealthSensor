
/**
  ******************************************************************************
  * @file    ghs_racp.h
  * @author  MCD Application Team
  * @brief   Header for ghs_racp.c module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GHS_RACP_H
#define __GHS_RACP_H

#ifdef __cplusplus
extern "C" 
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "ghs.h"
  
/* Exported defines ----------------------------------------------------------*/
/******** Generic Health Record Access Control Point (RACP) characteristic *****/

/* Record Access Control Point: Op Code Values */
/* RACP requests */
/* Mandatory */
#define GHS_RACP_OP_CODE_REPORT_STORED_RECORDS          (0x01)
#define GHS_RACP_OP_CODE_DELETE_STORED_RECORDS          (0x02)
#define GHS_RACP_OP_CODE_ABORT_OPERATION                (0x03)
#define GHS_RACP_OP_CODE_REPORT_NUMBER_STORED_RECORDS   (0x04)
#define GHS_RACP_OP_CODE_COMBINED_REPORT                (0x07)
  
/* RACP responses */
#define GHS_RACP_OP_CODE_NUMBER_STORED_RECORDS_RESPONSE (0x05)
#define GHS_RACP_OP_CODE_RESPONSE_CODE                  (0x06)
#define GHS_RACP_OP_CODE_COMBINED_REPORT_RESPONSE       (0x08)
 
/* Record Access Control Point: Operator Values */
/* Mandatory */
#define GHS_RACP_OPERATOR_NULL          (0x00)
#define GHS_RACP_OPERATOR_ALL_RECORDS   (0x01)  
#define GHS_RACP_OPERATOR_GREATER_EQUAL (0x03)

/* Optional */
#define GHS_RACP_OPERATOR_LESS_EQUAL    (0x02)
#define GHS_RACP_OPERATOR_WITHIN_RANGE  (0x04)
#define GHS_RACP_OPERATOR_FIRST_RECORD  (0x05)
#define GHS_RACP_OPERATOR_LAST_RECORD   (0x06)

/* Record Access Control Point: filter types values */
#define GHS_RACP_FILTER_TYPE_NONE             (0x00)
#define GHS_RACP_FILTER_TYPE_RECORD_NUMBER    (0x01)

/* Record Access Control Point: filter types valid length */
#define GHS_RACP_FILTER_TYPE_NO_OPERAND_LENGTH                    (2)
#define GHS_RACP_FILTER_TYPE_LESS_GREATER_RECORD_NUMBER_LENGTH    (7)
#define GHS_RACP_FILTER_TYPE_WITHIN_RANGE_RECORD_NUMBER_LENGTH    (11)

/* Record Access Control Point: fields position */
#define GHS_RACP_OP_CODE_POSITION       (0)
#define GHS_RACP_OPERATOR_POSITION      (1)
#define GHS_RACP_OPERAND_POSITION       (2)

/* Record Access Control Point: responses values */
#define GHS_RACP_RESPONSE_SUCCESS                                     (0x01)
#define GHS_RACP_RESPONSE_OPCODE_NOT_SUPPORTED                        (0x02)
#define GHS_RACP_RESPONSE_INVALID_OPERATOR                            (0x03)
#define GHS_RACP_RESPONSE_OPERATOR_NOT_SUPPORTED                      (0x04)
#define GHS_RACP_RESPONSE_INVALID_OPERAND                             (0x05)
#define GHS_RACP_RESPONSE_NO_RECORDS                                  (0x06) 
#define GHS_RACP_RESPONSE_ABORT_FAILED                                (0x07) 
#define GHS_RACP_RESPONSE_PROCEDURE_NOT_COMPLETED                     (0x08)
#define GHS_RACP_RESPONSE_OPERAND_NOT_SUPPORTED                       (0x09)
#define GHS_RACP_RESPONSE_SERVER_BUSY                                 (0x0A)

#define GHS_RACP_COMMAND_MAX_LEN                (17) /* TBC (Within Range with Time)*/
#define GHS_RACP_RESPONSE_LENGTH                (4)  /* RACP response length */
#define GHS_RACP_NB_REC_RESPONSE_LENGTH         (7)  /* RACP response length */

#define GHS_RACP_OP_CODE_REQUEST_OPERAND_MAX_LENGTH                        (5)
#define GHS_RACP_OP_CODE_RESPONSE_OPERAND_MAX_LENGTH                       (4)
  
/* Exported types ------------------------------------------------------------*/

typedef __PACKED_STRUCT
{
  uint8_t FilterType;
  uint32_t Minimum;
  uint32_t Maximum;
} GHS_RACP_RecordNumberFilter_t;
  
typedef __PACKED_STRUCT
{
  uint8_t FilterType;
  uint8_t FilterParam[sizeof(GHS_RACP_RecordNumberFilter_t)];
} GHS_RACP_Filter_t;

typedef struct 
{
  GHS_SHOSegment_t SHOSegment;
} GHS_RACP_Record_t;

typedef enum
{
  GHS_RACP_STATE_IDLE,
  GHS_RACP_STATE_PROCEDURE_IN_PROGRESS,
  GHS_RACP_STATE_PROCEDURE_ABORT_RECEIVED
} GHS_RACP_State_t;

typedef enum
{
  GHS_RACP_APP_REPORT_RECORDS_PROCEDURE_STARTED_EVENT,
  GHS_RACP_APP_REPORT_RECORDS_PROCEDURE_FINISHED_EVENT,
  GHS_RACP_APP_REPORT_RECORDS_PROCEDURE_NOTIFY_NEXT_RECORD_EVENT
} GHS_RACP_App_EventCode_t;

typedef struct
{
  GHS_RACP_App_EventCode_t      EventCode;
  GHS_Data_t                    EventData;
  uint16_t                      ConnectionHandle;
  uint8_t                       ServiceInstance;
} GHS_RACP_App_Event_t;

/* Exported constants ------------------------------------------------------*/
/* External variables ------------------------------------------------------*/
/* Exported macros ---------------------------------------------------------*/
/* Exported functions ------------------------------------------------------*/

void GHS_RACP_Init(void);
void GHS_RACP_RequestHandler(uint8_t * pRequestData, uint8_t requestDataLength);
uint8_t GHS_RACP_CheckRequestValid(uint8_t * pRequestData, uint8_t requestDataLength);
void GHS_RACP_AcknowledgeHandler(void);
uint8_t GHS_RACP_ProcessReportRecordsProcedure(void);
uint8_t GHS_RACP_SendNextRecord(void);
GHS_RACP_State_t GHS_RACP_GetState(void);
uint8_t GHS_RACP_APP_EventHandler(GHS_RACP_App_Event_t * pNotification);
tBleStatus GHS_RACP_send_response_code(uint8_t responseCode);

#endif /* __GHS_RACP_H */
