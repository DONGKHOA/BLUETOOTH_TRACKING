/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linked_list.h"

/******************************************************************************
 *   PUBLIC FUNCTION
 *****************************************************************************/

Node_t *
LINKED_LIST_CreateNode (void *p_value, uint8_t u8_dataSize)
{
  Node_t *newNode = (Node_t *)malloc(sizeof(Node_t));
  newNode->p_data = malloc(u8_dataSize); // Allocate memory for the data
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

  Node_t *temp = *p_head;
  while (temp->p_next != NULL)
  {
    temp = temp->p_next;
  }
  temp->p_next = newNode;
}

Node_t *
LINKED_LIST_DeleteNode (Node_t *p_head, uint32_t u32_position)
{
  if (p_head == NULL || u32_position == 0)
  {
    return p_head; // Edge case: empty list or invalid position
  }

  Node_t *temp = p_head;
  Node_t *prev = NULL;

  // Case 1: Head node needs to be deleted
  if (u32_position == 1)
  {
    p_head = temp->p_next;
    free(temp);
    return p_head;
  }

  // Traverse till the given position
  for (uint32_t i = 1; temp != NULL && i < u32_position; i++)
  {
    prev = temp;
    temp = temp->p_next;
  }

  // If position is out of bounds
  if (temp == NULL)
  {
    printf("Error: Position out of range\n");
    return p_head;
  }

  // Unlink the node from the list
  prev->p_next = temp->p_next;
  free(temp);

  return p_head;
}
