/**
  ******************************************************************************
  * @file    ghs_db.c
  * @author  MCD Application Team
  * @brief   GHS Records Database
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
#include "common_blesvc.h"
#include "ghs_db.h"

/* Private typedef -----------------------------------------------------------*/

typedef struct GHS_DB_Item
{
  GHS_RACP_Record_t Record;
  FlagStatus IsSelected;
  struct GHS_DB_Item * NextItem;
} GHS_DB_Item_t;

typedef struct
{
  GHS_DB_Item_t * Head;
} GHS_DB_Context_t;

/* Private variables ---------------------------------------------------------*/
/* Private functions definition ----------------------------------------------*/
static uint8_t ghs_db_delete_last_item(GHS_DB_Item_t ** head);
static uint8_t ghs_db_delete_first_item(GHS_DB_Item_t ** head);

/* START of Section BLE_DRIVER_CONTEXT
 */
PLACE_IN_SECTION("BLE_DRIVER_CONTEXT") static GHS_DB_Context_t GHS_DB_Context = {0};

/* Private Functions ---------------------------------------------------------*/

/**
* @brief  Delete last item of the GHS Database
* @param  head : pointer to pointer of the first DB item
* @retval TRUE if item exists and deleted, FALSE (!TRUE) otherwise
*/
static uint8_t ghs_db_delete_last_item(GHS_DB_Item_t ** head)
{
  GHS_DB_Item_t * nextTail = NULL;
  
  if (*head != NULL)
  {
    if ((*head)->NextItem != NULL)
    {
      nextTail = (*head);
      while (nextTail->NextItem->NextItem != NULL)
      {
        nextTail = nextTail->NextItem;
      }
      
      free(nextTail->NextItem);
      nextTail->NextItem = NULL;
    }
    else 
    {
      free(GHS_DB_Context.Head);
    }
    return TRUE;
  }
   
  return FALSE;
} /* end of ghs_db_delete_last_item() */

/**
* @brief  Delete first item of the GHS Database
* @param  head : pointer to pointer of the first DB item
* @retval TRUE if item exists and deleted, FALSE (!TRUE) otherwise
*/
static uint8_t ghs_db_delete_first_item(GHS_DB_Item_t ** head)
{
  GHS_DB_Item_t * nextHead = NULL;
  
  if (*head != NULL)
  {
    nextHead = (*head)->NextItem;
    free(*head);
    *head = nextHead;
    
    return TRUE;
  }
   
  return FALSE;
} /* end of ghs_db_delete_first_item() */

/* Public Functions ----------------------------------------------------------*/

/**
* @brief  Initialize the GHS database
* @param  UUID: UUID of the characteristic
* @param  pPayload: Payload of the characteristic
* @retval None
*/
void GHS_DB_Init(void)
{
  GHS_DB_Context.Head = NULL;
} /* end of GHS_DB_Init() */

/**
* @brief  Get a specified record from the GHS database
* @param  pTarget: Pointer to the memory to copy the requested record data
* @retval TRUE if any selected record found, FALSE (!TRUE) otherwise
*/
uint8_t GHS_DB_GetNextSelectedRecord(GHS_RACP_Record_t * pTarget)
{
  GHS_DB_Item_t * currentItem = GHS_DB_Context.Head;
  
  while (currentItem != NULL)
  {
    if (currentItem->IsSelected == SET)
    {
      memcpy(pTarget, &(currentItem->Record), sizeof(GHS_RACP_Record_t));
      currentItem->IsSelected = RESET;
      return TRUE;
    }
    currentItem = currentItem->NextItem;
  }
  
  return FALSE;
} /* end of GHS_DB_GetRecord() */

/**
* @brief  Deselect (mark) all items in the database
* @param  None
* @retval Count of the items found selected
*/
uint16_t GHS_DB_ResetRecordsSelection( void )
{
  uint16_t deselectedItemsCount = 0;
  GHS_DB_Item_t * currentItem = GHS_DB_Context.Head;
  
  while (currentItem != NULL)
  {
    if (currentItem->IsSelected == SET)
    {
      currentItem->IsSelected = RESET;
      deselectedItemsCount++;
    }
    currentItem = currentItem->NextItem;
  }
  
  return deselectedItemsCount;
} /* end of GHS_DB_ResetRecordsSelection() */

/**
* @brief  Select (mark) all items in the database matching the filtering criteria
* @param  Arbiter: Pointer to function implementing the requested select logic
* @retval Count of the items selected
*/
uint16_t GHS_DB_SelectRecords( uint8_t (*FilterFunc)(GHS_RACP_Record_t *, const GHS_RACP_Filter_t *), const GHS_RACP_Filter_t * filter)
{
  uint16_t selectedItemsCount = 0;
  GHS_DB_Item_t * currentItem = GHS_DB_Context.Head;
    
  while (currentItem != NULL)
  {
    if (FilterFunc == NULL)
    {
      currentItem->IsSelected = SET;
      selectedItemsCount++;
    }
    else 
    {
      if (FilterFunc(&(currentItem->Record), filter) == TRUE)
      {
        currentItem->IsSelected = SET;
        selectedItemsCount++;
      }
      else 
      {
        currentItem->IsSelected = RESET;
      }
    }
    currentItem = currentItem->NextItem;
  }
  
  return selectedItemsCount;
} /* end of GHS_DB_SelectRecords() */

/**
* @brief  Delete all items in the database matching the filtering criteria
* @param  Arbiter: Pointer to function implementing the requested delete logic
* @retval Count of the items selected
*/
uint16_t GHS_DB_DeleteRecords( uint8_t (*FilterFunc)(GHS_RACP_Record_t *, const GHS_RACP_Filter_t *), const GHS_RACP_Filter_t * filter)
{
  uint16_t deletedItemsCount = 0;
  uint16_t index = 0;
  GHS_DB_Item_t * currentItem = GHS_DB_Context.Head;
  
  if (FilterFunc == NULL)
  {
    while (GHS_DB_Context.Head != NULL)
    {
      if (ghs_db_delete_first_item(&(GHS_DB_Context.Head)) != FALSE)
      {
        deletedItemsCount++;
      }
    }
  }
  else 
  {
    while (currentItem != NULL)
    {
      if (FilterFunc(&(currentItem->Record), filter) == TRUE)
      {
        if (GHS_DB_DeleteRecordByIndex(index) != FALSE)
        {
          currentItem = GHS_DB_Context.Head;
          index = 0;
          deletedItemsCount++;
        }
      }
      else 
      {
        /* We go next, item doesn't match the criteria */
        index++;
        currentItem = currentItem->NextItem;
      }
    }
  }
    
  return deletedItemsCount;
} /* end of GHS_DB_DeleteRecords */

/**
* @brief  Delete the last record stored in the GHS database
* @param  index: 0xFFFF (remove last), 0x0000 (remove first), other (remove exact)
* @retval FALSE if no record, TRUE (!FALSE) otherwise
*/
uint8_t GHS_DB_DeleteRecordByIndex( uint16_t index )
{
  GHS_DB_Item_t * currentItem;
  GHS_DB_Item_t * temp = NULL;
  uint16_t n;
  
  if (GHS_DB_Context.Head != NULL)
  {
    if (index == 0x0000)
    {
      return ghs_db_delete_first_item(&(GHS_DB_Context.Head));
    }
    else if (index == 0xFFFF)
    {
      return ghs_db_delete_last_item(&(GHS_DB_Context.Head));
    }
    else 
    {
      currentItem = GHS_DB_Context.Head;
      for (n = 0; n < (index-1); n++)
      {
        if (currentItem->NextItem == NULL)
        {
          return FALSE;
        }
        currentItem = currentItem->NextItem;
      }
      
      temp = currentItem->NextItem;
      currentItem->NextItem = temp->NextItem;
      free(temp);
      return TRUE;
    }
  }
   
  return FALSE;
} /* end of GHS_DB_DeleteRecordByIndex() */

/**
* @brief  Add a new record to the GHS database
* @param  pNewRecords: Pointer to the new record to be added
* @retval FALSE if failed, TRUE (!FALSE) otherwise
*/
uint8_t GHS_DB_AddRecord(GHS_RACP_Record_t * pNewRecord)
{
  GHS_DB_Item_t * currentItem = GHS_DB_Context.Head;
  
  if (currentItem == NULL)
  {
    GHS_DB_Context.Head = malloc(sizeof(GHS_DB_Item_t));
    currentItem = GHS_DB_Context.Head;
  }
  else 
  {
    while (currentItem->NextItem != NULL)
    {
      currentItem = currentItem->NextItem;
    }
     
    currentItem->NextItem = malloc(sizeof(GHS_DB_Item_t));
    currentItem = currentItem->NextItem;
  }
  
  if(currentItem != NULL)
  {
    memcpy(&(currentItem->Record), pNewRecord, sizeof(GHS_RACP_Record_t));
    currentItem->IsSelected = RESET;
    currentItem->NextItem = NULL;
    return TRUE;
  }
  else
  {
    return FALSE;
  }
} /* end of GHS_DB_AddRecord() */

/**
* @brief  Get the count of records stored in the GHS database
* @param  Arbiter: Pointer to function implementing the requested compare logic
* @retval Number of records
*/
uint32_t GHS_DB_GetRecordsCount( uint8_t (*FilterFunc)(GHS_RACP_Record_t *, const GHS_RACP_Filter_t *), const GHS_RACP_Filter_t * filter)
{
  uint16_t foundItemsCount = 0;
  GHS_DB_Item_t * currentItem = GHS_DB_Context.Head;
    
  if (currentItem == NULL)
  {
    return 0;
  }
  
  while (currentItem != NULL)
  {
    if (FilterFunc == NULL)
    {
      foundItemsCount++;
    }
    else 
    {
      if (FilterFunc(&(currentItem->Record), filter) == TRUE)
      {
        foundItemsCount++;
      }
      else 
      {
        /* Do nothing - item doesn't match the criteria */
      }
    }
    currentItem = currentItem->NextItem;
  }
  
  return foundItemsCount;
} /* end of GHS_DB_GetRecordsNum() */

/**
* @brief  Get the last record stored in the GHS database
* @param  index: 0xFFFF (get last), 0x0000 (get first), other (get exact)
* @param  pTarget: Pointer to the memory to copy the requested record data
* @retval FALSE if no record, TRUE (!FALSE) otherwise
*/
uint8_t GHS_DB_GetRecordByIndex(uint16_t index, GHS_RACP_Record_t * pTarget)
{
  uint16_t n;
  GHS_DB_Item_t * currentItem;
  
  if (GHS_DB_Context.Head != NULL)
  {
    currentItem = GHS_DB_Context.Head;
    
    if (index == 0x0000)
    {
      memcpy(pTarget, &(currentItem->Record), sizeof(GHS_RACP_Record_t));
      return TRUE;
    }
    else if (index == 0xFFFF)
    {
      while(currentItem->NextItem != NULL)
      {
        currentItem = currentItem->NextItem;
      }
      memcpy(pTarget, &(currentItem->Record), sizeof(GHS_RACP_Record_t));
      return TRUE;
    }
    else 
    {
      for (n = 0; n < index; n++)
      {
        if (currentItem->NextItem != NULL)
        {
          return FALSE;
        }
        currentItem = currentItem->NextItem;
      }
      memcpy(pTarget, &(currentItem->Record), sizeof(GHS_RACP_Record_t));
      return TRUE;
    }
  }
   
  return FALSE;
} /* end of GHS_DB_GetRecordByIndex() */

/**
* @brief  Select record stored in the GHS database by its index
* @param  index: 0xFFFF (get last), 0x0000 (get first), other (get exact)
* @param  pTarget: Pointer to the memory to copy the requested record data
* @retval FALSE if no record, TRUE (!FALSE) otherwise
*/
uint8_t GHS_DB_SelectRecordByIndex(uint16_t index)
{
  uint16_t n;
  GHS_DB_Item_t * currentItem;
  
  if (GHS_DB_Context.Head != NULL)
  {
    currentItem = GHS_DB_Context.Head;
    
    if (index == 0x0000)
    {
      currentItem->IsSelected = SET;
      return TRUE;
    }
    else if (index == 0xFFFF)
    {
      while(currentItem->NextItem != NULL)
      {
        currentItem = currentItem->NextItem;
      }
      currentItem->IsSelected = SET;
      return TRUE;
    }
    else 
    {
      for (n = 0; n < index; n++)
      {
        if (currentItem->NextItem != NULL)
        {
          return FALSE;
        }
        currentItem = currentItem->NextItem;
      }
      currentItem->IsSelected = SET;
      return TRUE;
    }
  }
   
  return FALSE;
} /* end of GHS_DB_SelectRecordByIndex() */
