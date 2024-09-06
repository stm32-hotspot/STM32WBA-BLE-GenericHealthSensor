
/**
  ******************************************************************************
  * @file    ghs_db.h
  * @author  MCD Application Team
  * @brief   Header for ghs_db.c module
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
#ifndef __GHS_DB_H
#define __GHS_DB_H

#ifdef __cplusplus
extern "C" 
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "ghs.h"
#include "ghs_racp.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void GHS_DB_Init(void);
uint8_t GHS_DB_AddRecord(GHS_RACP_Record_t * pSource);
uint8_t GHS_DB_GetNextSelectedRecord(GHS_RACP_Record_t * pTarget);
uint32_t GHS_DB_GetRecordsCount( uint8_t (*Arbiter)(GHS_RACP_Record_t *, const GHS_RACP_Filter_t *), const GHS_RACP_Filter_t * filter);
uint16_t GHS_DB_SelectRecords( uint8_t (*Arbiter)(GHS_RACP_Record_t *, const GHS_RACP_Filter_t *), const GHS_RACP_Filter_t * filter);
uint16_t GHS_DB_DeleteRecords( uint8_t (*Arbiter)(GHS_RACP_Record_t *, const GHS_RACP_Filter_t *), const GHS_RACP_Filter_t * filter);
uint8_t GHS_DB_GetRecordByIndex(uint16_t index, GHS_RACP_Record_t * pTarget);
uint8_t GHS_DB_SelectRecordByIndex(uint16_t index);
uint8_t GHS_DB_DeleteRecordByIndex( uint16_t index );
uint16_t GHS_DB_ResetRecordsSelection( void );
  
#endif /* __GHS_DB_H */
