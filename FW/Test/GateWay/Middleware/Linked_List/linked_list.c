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
  Node_t *prev = NULL;
  Node_t *temp = p_head;

  // Base case if linked list is empty
  if (temp == NULL)
    return p_head;

  // Case 1: p_head is to be deleted
  if (u32_position == 1)
  {
    // make next node as p_head and free old p_head
    p_head = temp->p_next;
    return p_head;
  }

  // Case 2: Node to be deleted is in the middle
  // Traverse till the given u32_position
  for (uint32_t i = 1; temp != NULL && i < u32_position; i++)
  {
    prev = temp;
    temp = temp->p_next;
  }

  // If the given position is found, delete node
  if (temp != NULL)
  {
    prev->p_next = temp->p_next;
    free(temp);
  }
  // If the given position is not present
  else
  {
    printf("Data not present\n");
  }

  return p_head;
}
