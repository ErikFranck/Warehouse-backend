#include <stdlib.h>
#include <stdio.h>
#include "hash_table.h"
#include "iterator.h"
#include <string.h>
//#include "iterator.h"
#define Loadfactor 0.75


size_t primes[] = {17, 31, 67, 127, 257, 509, 1021, 2053, 4099, 8191, 16381, 31393, 634937, 1010329, 0}; // nr of buckets

// STRUCTS & TYPEDEFS
typedef struct entry entry_t;
typedef struct hash_table ioopm_hash_table_t;
struct entry
{
  elem_t key;
  elem_t value;
  entry_t *next;
};

  typedef struct merch merch_t;
struct hash_table
{
  size_t Nr_buckets;
  entry_t *buckets;
  int index_prime;
  size_t size;
  float load_factor;
  ioopm_hash_function hash_func;
  ioopm_eq_function key_compare;
  ioopm_eq_function value_compare;
};

// Resizing the hashtable
static void resize_hashtable(ioopm_hash_table_t *ht )
{
  ioopm_list_t *values =ioopm_hash_table_values(ht);
  ioopm_list_t *keys =ioopm_hash_table_keys(ht);
  size_t new_size = primes[++ht->index_prime];
  ioopm_hash_table_clear(ht);
  ht->Nr_buckets = new_size;
  ht->buckets =realloc(ht->buckets, sizeof(entry_t)*ht->Nr_buckets);
  for (size_t x = 0; x < ht->Nr_buckets; x++) //all dummys poiting to NULL
    {
      ht->buckets[(int)x].next = NULL;
    }
  ioopm_list_iterator_t *iter_values = ioopm_list_iterator_create(values);
  ioopm_list_iterator_t *iter_keys = ioopm_list_iterator_create(keys);
  for (size_t i = 0; i < ioopm_linked_list_size(values); i++)
    {
      ioopm_hash_table_insert(ht,ioopm_iterator_current(iter_keys).value, ioopm_iterator_current(iter_values).value);
      ioopm_iterator_next(iter_keys);
      ioopm_iterator_next(iter_values);
    }
  ioopm_iterator_destroy(iter_keys);
  ioopm_iterator_destroy(iter_values);
  ioopm_linked_list_clear(values);
  ioopm_linked_list_clear(keys);
  ioopm_linked_list_destroy(values);
  ioopm_linked_list_destroy(keys);
}

// Creates a new entry
static entry_t *entry_create(elem_t key, elem_t value, entry_t *next)
{
  entry_t  *newentry = calloc(1, sizeof(entry_t));
  newentry->key = key;
  newentry->value = value;
  newentry->next = next;
  return newentry;
}

// Creates a new hash table
ioopm_hash_table_t *ioopm_hash_table_create(ioopm_hash_function func, ioopm_eq_function key, ioopm_eq_function value)
{
  ioopm_hash_table_t *result = calloc(1, sizeof(ioopm_hash_table_t));
  result->hash_func = func;
  result->key_compare = key;
  result->value_compare = value;
  result->size = 0;
  result->index_prime = 0;
  result->load_factor = Loadfactor;
  result->Nr_buckets= primes[result->index_prime];
  result->buckets = calloc(result->Nr_buckets, sizeof(entry_t));
  return result;
}

// Finds the previous entry prior to the key searched for
static entry_t *find_previous_entry_for_key(entry_t *current, elem_t key, ioopm_eq_function key_compare)
{


  entry_t *tmp = current;
  entry_t *next = current->next; // if dummy points to NULL, return dummy
  while (next != NULL)
    if (key_compare(next->key, key))
      {
        return tmp;
      }
    else
      {
        tmp  = next;
        next = next->next;

      }
  return current; //returning dummy if no matching keys, IE prepending

}

// Frees memory
static void entry_destroy(entry_t *remove)
{
   free(remove);
}


// Creates and inserts entry into hash table
void ioopm_hash_table_insert(ioopm_hash_table_t *ht, elem_t key, elem_t value)
{

  entry_t *previous =  find_previous_entry_for_key(&ht->buckets[(ht->hash_func(key))%(unsigned long)ht->Nr_buckets], key, ht->key_compare);

  entry_t *next = previous->next;
  if (next != NULL && ht->key_compare(next->key, key))
    {
      next->value = value; //replacing value if duplicate keys
    }
  else
    {
      entry_t *new_value = entry_create(key, value, previous->next); // new value in front, pointing to the second value
      previous->next = new_value;// dummy pointing to the inserted value
      ht->size++;
    }
  if (primes[ht->index_prime +1] && ht->size/ht->Nr_buckets > ht->load_factor)
    {
      resize_hashtable(ht);
    }

}




// Looks up whether key is in hash table
ioopm_option_t ioopm_hash_table_lookup (ioopm_hash_table_t *ht, elem_t key)
{
  entry_t *previous = find_previous_entry_for_key(&ht->buckets[(ht->hash_func(key))%ht->Nr_buckets], key, ht->key_compare);

  entry_t *next = previous->next;


  if (next && ht->key_compare(next->key, key) )
    {

      return Success(next->value);
    }
  else
    {
      return Failure();
    }

}

// Removes a chosen entry
ioopm_option_t ioopm_hash_table_remove(ioopm_hash_table_t *ht, elem_t key)
{
  entry_t *previous = find_previous_entry_for_key(&ht->buckets[(ht->hash_func(key))%ht->Nr_buckets], key, ht->key_compare);
  entry_t *current = previous->next;
  if (current && ht->key_compare(current->key, key))
    {
      entry_t *remove = current;
      ioopm_option_t savedentry = {.success = true, .value = remove->value};
      previous->next = current->next;
      entry_destroy(remove);
      ht->size--;
      return savedentry;
    }
  else
    {
      return Failure();
    }
}

// Erases all entries and frees memory
void ioopm_hash_table_clear(ioopm_hash_table_t *ht)
{
  for(size_t i = 0; i < ht->Nr_buckets; i++)
    {
      entry_t *cursor = &ht->buckets[i]; //poiting at dummy
      entry_t *next = cursor->next; // first element

      while (next)
        {
          entry_t *cursor2 = next;
          next = next->next;
          entry_destroy(cursor2);
        }
      ht->buckets[i].next = NULL; //dummy poiting at NULL again
    }
  ht->size = 0; //set size to 0 when done
}

// Destroys hash table, frees memory
void ioopm_hash_table_destroy(ioopm_hash_table_t *ht)
{
  ioopm_hash_table_clear(ht);
  Free(ht->buckets);
  Free(ht);
}

// Size of hash table
size_t ioopm_hash_table_size(ioopm_hash_table_t *ht)
{
  return ht->size;
}

// Checks whether hash table is empty
bool ioopm_hash_table_is_empty(ioopm_hash_table_t *ht)
{
  return ht->size == 0;
}

//return a linked list of all keys
ioopm_list_t *ioopm_hash_table_keys(ioopm_hash_table_t *ht)
{

  ioopm_list_t *list = ioopm_linked_list_create(ht->key_compare);
  for (int i = 0; i < (int)ht->Nr_buckets; i++)
    {
      entry_t *cursor = &ht->buckets[i];
      entry_t *next = cursor->next;
      while (next)
        {
          ioopm_linked_list_append(list, next->key);
          next = next->next;
        }
    }
  return list;
}

//return a linked list of all values
ioopm_list_t *ioopm_hash_table_values(ioopm_hash_table_t *ht)
{
  ioopm_list_t *list = ioopm_linked_list_create(ht->value_compare);
  for (size_t i = 0; i <ht->Nr_buckets; i++)
    {
      entry_t *cursor = &ht->buckets[i];
      entry_t *next = cursor->next;
      while(next)
        {
          ioopm_linked_list_append(list, next->value);
          next = next->next;
        }
    }
  return list;
}

//checks for a specific key in ht
bool ioopm_hash_table_has_key(ioopm_hash_table_t *ht, elem_t key)
{
  bool result = false;
  for (size_t i = 0; i < ht->Nr_buckets && !result; i++)
    {
      entry_t *cursor = &ht->buckets[i];
      entry_t *next = cursor->next;
      while (next)
        {
          result = ht->key_compare(next->key, key);
          if (result)
            {
              break;
            }
          else
            {
              next = next->next;
            }
        }

    }
  return result;
}

//checks for a specific value in ht
bool ioopm_hash_table_has_value(ioopm_hash_table_t *ht, elem_t value)
{
  bool result = false;
  for (size_t i = 0; i < ht->Nr_buckets && !result; i++)
    {
      entry_t *cursor = &ht->buckets[i];
      entry_t *next = cursor->next;
      while (next)
        {
          result = ht->value_compare(next->value, value);
          if (result)
            {
              break;
            }
          else
            {
              next = next->next;
            }
        }
    }
  return result;
}

//checks if a predicate holds for all values
bool ioopm_hash_table_all (ioopm_hash_table_t *ht, ioopm_predicate func, void *arg)
{
  bool result = true;
  for (size_t i = 0; i < ht->Nr_buckets && result; i++)
    {
      entry_t *cursor = &ht->buckets[i];
      entry_t *next = cursor->next;
      while(next)
        {
          result = func(next->key, next->value, arg);
          if (!result)
            {
              return result;
            }
          else
            {
              next = next->next;
            }
        }
    }
  return result;

}

//checks if a predicate holds for any value
bool ioopm_hash_table_any(ioopm_hash_table_t *ht, ioopm_predicate func, void *arg)
{
  bool result = false;
  for (size_t i = 0; i < ht->Nr_buckets && !result; i++)
    {
      entry_t *cursor = &ht->buckets[i];
      entry_t *next = cursor->next;
      while (next)
        {
          result = func(next->key, next->value, arg);
          if (result)
            {
              return result;
            }
          else
            {
              next = next->next;
            }
        }
    }
  return result;
}

// Applies chosen alteration to every entry in hash table
void ioopm_hash_table_apply_to_all(ioopm_hash_table_t *ht, ioopm_apply_function func, void *arg)
{
  for (size_t i = 0; i < ht->Nr_buckets; i++)
    {
      entry_t *cursor = &ht->buckets[i];
      entry_t *next = cursor->next;
      while (next)
        {
          func(next->key, &next->value, arg);
          next = next->next;
        }
    }
}
