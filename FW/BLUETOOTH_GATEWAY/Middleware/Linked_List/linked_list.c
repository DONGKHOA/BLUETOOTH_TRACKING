/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linked_list.h"
#include "esp_heap_caps.h"

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

Node_t *
LINKED_LIST_CreateNode (void *p_value, uint8_t u8_dataSize)
{
  Node_t *newNode = (Node_t *)heap_caps_malloc(sizeof(Node_t), MALLOC_CAP_SPIRAM);
  newNode->p_data = heap_caps_malloc(u8_dataSize, MALLOC_CAP_SPIRAM); // Allocate memory for the data
  if (newNode->p_data != NULL)
  {
    memcpy(newNode->p_data, p_value, u8_dataSize); // Copy data to node
  }
  newNode->p_next = NULL;
  return newNode;
}

void
LINKED_LIST_InsertAtHead (Node_t **p_head, void *p_value, uint8_t u8_dataSize)
{
  Node_t *newNode = LINKED_LIST_CreateNode(p_value, u8_dataSize);
  newNode->p_next = *p_head;
  *p_head         = newNode;
}

void
LINKED_LIST_InsertAtTail (Node_t **p_head, void *p_value, uint8_t u8_dataSize)
{
  Node_t *newNode = LINKED_LIST_CreateNode(p_value, u8_dataSize);
  if (*p_head == NULL)
  {
    *p_head = newNode;
    return;
  }

  Node_t *p_temp = *p_head;
  while (p_temp->p_next != NULL)
  {
    p_temp = p_temp->p_next;
  }
  p_temp->p_next = newNode;
}

Node_t *
LINKED_LIST_DeleteNode (Node_t *p_head, uint32_t position)
{
  Node_t *p_temp = p_head;
  Node_t *prev   = NULL;

  // Base case if linked list is empty
  if (p_temp == NULL)
  {
    return p_head;
  }

  // Case 1: Head is to be deleted
  if (position == 1)
  {
    p_head = p_temp->p_next;
    heap_caps_free(p_temp->p_data);
    heap_caps_free(p_temp);
    return p_head;
  }

  // Case 2: Node to be deleted is in middle
  // Traverse till given position
  for (int i = 1; p_temp != NULL && i < position; i++)
  {
    prev   = p_temp;
    p_temp = p_temp->p_next;
  }

  // If given position is found, delete node
  if (p_temp != NULL)
  {
    prev->p_next = p_temp->p_next;
    heap_caps_free(p_temp->p_data);
    heap_caps_free(p_temp);
  }
  else
  {
    printf("Data not present\n");
  }

  return p_head;
}
