#include <stdlib.h>
#include "iterator.h"
#include <stdbool.h>


// STRUCTS AND TYPEDEFS

typedef struct list_iterator ioopm_list_iterator_t;
typedef struct link link_t;
typedef struct list ioopm_list_t;

struct link
{
  elem_t value;
  link_t *next;
};

struct list
{
  link_t *first;
  link_t *last;
  int length;
  ioopm_eq_function compare;
};

struct list_iterator
{
  link_t *current;
  ioopm_list_t *list;
};

// Creates an iterator
ioopm_list_iterator_t *ioopm_list_iterator_create(ioopm_list_t *list)
{
  ioopm_list_iterator_t *iter = calloc(1, sizeof(ioopm_list_iterator_t));
  iter->list = list;
  iter->current = list->first;
  return iter;
}

// Checks if iterator has next
bool ioopm_iterator_has_next(ioopm_list_iterator_t *iter)
{
  link_t *current = iter->current;
  if (current->next)
    {
      return true;
    }
  else
    {
      return false;
    }
}

// Returns value of iterator's next.
ioopm_option_t ioopm_iterator_next(ioopm_list_iterator_t *iter)
{
  if (iter->current && iter->current->next)
    {
      iter->current = iter->current->next;
      return Success(iter->current->value);
    }
  else
    {
      return Failure();
    }
}

// Resets iterator to first element in list
void ioopm_iterator_reset(ioopm_list_iterator_t *iter)
{
  iter->current = iter->list->first;
}

// Returns value of iterator
ioopm_option_t ioopm_iterator_current(ioopm_list_iterator_t *iter)
{
  if (iter->list->length == 0)
    {
      return Failure();
    }
  else
    {
      return Success(iter->current->value);
    }

}

// Destroys iterator
void ioopm_iterator_destroy(ioopm_list_iterator_t *iter)
{
  free(iter);
}
