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

  /**
   * The function `LINKED_LIST_CreateNode` creates a new node for a linked list
   * with the specified data size and copies the provided data into the node.
   *
   * @param p_value The `p_value` parameter is a pointer to the data that you
   * want to store in the node. It is of type `void *`, which means it can be a
   * pointer to any type of data.
   * @param u8_dataSize The `u8_dataSize` parameter in the
   * `LINKED_LIST_CreateNode` function represents the size (in bytes) of the
   * data that will be stored in the node. This parameter is used to allocate
   * memory for the data within the node and to ensure that the correct amount
   * of data is copied
   *
   * @return The function `LINKED_LIST_CreateNode` returns a pointer to a newly
   * created `Node_t` structure.
   */
  Node_t *LINKED_LIST_CreateNode(void *p_value, uint8_t u8_dataSize);

  /**
   * The function `LINKED_LIST_InsertAtHead` inserts a new node at the head of a
   * linked list.
   *
   * @param p_head `p_head` is a pointer to a pointer to the head of a linked
   * list.
   * @param p_value The `p_value` parameter in the `LINKED_LIST_InsertAtHead`
   * function represents the value that you want to insert at the head of the
   * linked list. It is a pointer to the data that you want to store in the
   * linked list node.
   * @param u8_dataSize The `u8_dataSize` parameter in the
   * `LINKED_LIST_InsertAtHead` function represents the size of the data that is
   * being stored in the linked list node. It is used to determine the amount of
   * memory that needs to be allocated for the data being stored in the node.
   */
  void LINKED_LIST_InsertAtHead(Node_t **p_head,
                                void    *p_value,
                                uint8_t  u8_dataSize);

  /**
   * The function LINKED_LIST_InsertAtTail inserts a new node at the end of a
   * linked list.
   *
   * @param p_head `p_head` is a pointer to a pointer to the head of a linked
   * list. This allows the function to modify the head pointer if needed.
   * @param p_value The `p_value` parameter in the `LINKED_LIST_InsertAtTail`
   * function represents the value that you want to insert at the tail of the
   * linked list. It is a pointer to the data that you want to store in the
   * linked list node.
   * @param u8_dataSize The `u8_dataSize` parameter in the
   * `LINKED_LIST_InsertAtTail` function represents the size of the data that is
   * being stored in the linked list node. It is used to allocate memory for the
   * data and ensure that the correct amount of memory is copied when inserting
   * a new node at
   *
   * @return The function `LINKED_LIST_InsertAtTail` returns `void`, which means
   * it does not return any value.
   */
  void LINKED_LIST_InsertAtTail(Node_t **p_head,
                                void    *p_value,
                                uint8_t  u8_dataSize);

  /**
   * The function `LINKED_LIST_DeleteNode` deletes a node at a specified
   * position in a linked list.
   *
   * @param p_head `p_head` is a pointer to the head of a linked list, which is
   * the first node in the list.
   * @param position The `position` parameter in the `LINKED_LIST_DeleteNode`
   * function specifies the position of the node that needs to be deleted from
   * the linked list. The function will delete the node at the specified
   * position in the linked list if it exists.
   *
   * @return The function `LINKED_LIST_DeleteNode` returns a pointer to the head
   * of the linked list after deleting a node at the specified position.
   */
  Node_t *LINKED_LIST_DeleteNode(Node_t *p_head, uint32_t u32_position);

#ifdef __cplusplus
}
#endif

#endif /* LINKED_LIST_H_ */