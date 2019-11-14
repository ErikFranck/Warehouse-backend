#pragma once
#include <stdbool.h>
#include "common.h"


/**
 * @file iterator.h
 * @author Mark Annala and Hanna Dahl
 * @date Oct 3rd 2019
 * @brief Simple iterator that iterates over a linked list
 *
 * Here's all the functions needed to create and move an iterator over a linked list, provided the linked list that's supposed to be iterated over.
 *
 *
 *
 * @see http://wrigstad.com/ioopm19/assignments/assignment1.html
 */

typedef struct ioopm_option option_t;
typedef struct list_iterator ioopm_list_iterator_t;
typedef struct link link_t;
typedef struct list ioopm_list_t;

/// @brief Create an iterator for a given list
/// @param the list to be iterated over
/// @return an iteration positioned at the start of list
ioopm_list_iterator_t *ioopm_list_iterator_create(ioopm_list_t *list);

/// @brief Checks if there are more elements to iterate over
/// @param iter the iterator
/// @return true if the iterator is not at the end of the underlying list
bool ioopm_iterator_has_next(ioopm_list_iterator_t *iter);

/// @brief Step the iterator forward one step and return the value if the step is valid
/// @param iter the iterator
/// @return the next element if not at end of underlying list
ioopm_option_t ioopm_iterator_next(ioopm_list_iterator_t *iter);

/// @brief Reposition the iterator at the start of the underlying list
/// @param iter the iterator
void ioopm_iterator_reset(ioopm_list_iterator_t *iter);

/// @brief Return the current element from the underlying list if there is one, if the list is empty return false
/// @param iter the iterator
/// @return the current element
ioopm_option_t ioopm_iterator_current(ioopm_list_iterator_t *iter);

/// @brief Destroy the iterator and return its resources
/// @param iter the iterator
void ioopm_iterator_destroy(ioopm_list_iterator_t *iter);

//-------- OPTIONAL TO IMPLEMENT IN FUTURE -----------

/// NOTE: REMOVE IS OPTIONAL TO IMPLEMENT
/// @brief Remove the current element from the underlying list
/// @param iter the iterator
/// @return the removed element
///int ioopm_iterator_remove(ioopm_list_iterator_t *iter);

/// NOTE: INSERT IS OPTIONAL TO IMPLEMENT
/// @brief Insert a new element into the underlying list making the current element it's next
/// @param iter the iterator
/// @param element the element to be inserted
///void ioopm_iterator_insert(ioopm_list_iterator_t *iter, int element);
