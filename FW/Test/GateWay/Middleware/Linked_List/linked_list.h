#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *   PUBLIC TYPEDEFS
   ***************************************************************************/

  typedef struct Node
  {
    void        *p_data;
    struct Node *p_next;
  } Node_t;

  /****************************************************************************
   *   PUBLIC FUNCTION
   ***************************************************************************/

  Node_t *LINKED_LIST_CreateNode(void *p_value, uint8_t u8_dataSize);
  void    LINKED_LIST_InsertAtHead(Node_t **p_head,
                                   void    *p_value,
                                   uint8_t  u8_dataSize);
  void    LINKED_LIST_InsertAtTail(Node_t **p_head,
                                   void    *p_value,
                                   uint8_t  u8_dataSize);
  Node_t *LINKED_LIST_DeleteNode(Node_t *p_head, uint32_t u32_position);

#ifdef __cplusplus
}
#endif

#endif /* LINKED_LIST_H_ */