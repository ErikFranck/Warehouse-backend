#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "carts.h"

#define value_name(elem) ((char *) ((merch_t *)elem.voi)->name)
#define cart_merch_name(elem) ((char *) ((merch_in_cart_t *)elem.voi)->name)

typedef struct merch merch_t;
typedef struct cart cart_t;
typedef struct merch_in_cart merch_in_cart_t;

struct merch
{
  char *name;
  char *descr;
  int price;
  ioopm_list_t *shelves;
  int total_stock; 
};

struct cart
{
  int name;
  size_t total_cost;
  ioopm_list_t *wares; 
};
struct merch_in_cart
{
  char *name;
  size_t amount;
};


///////////////////////////////////////////////////////////////////////////
//////////////////CART FUNCTIONS//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


static bool ioopm_cart_key_compare(elem_t first, elem_t second)
{
  return (first.in == second.in);
}

static unsigned long ioopm_cart_hashfunc(elem_t keyword)
{
  int result = keyword.in;
  return result;
}

void ioopm_merch_in_cart_delete(elem_t *elem_merch_in_cart, void *extra)
{
  merch_in_cart_t *merch_in_cart = (merch_in_cart_t *)elem_merch_in_cart->voi;
  free(merch_in_cart->name);
  free(merch_in_cart);
  return;
}

static bool ioopm_merch_in_cart_compare(elem_t first, elem_t second)
{
  char *merch_name1 = cart_merch_name(first);
  char *merch_name2 = cart_merch_name(second);
  return (strcmp(merch_name1, merch_name2) == 0);
}

ioopm_hash_table_t *ioopm_create_cart_ht()
{
return ioopm_hash_table_create(ioopm_cart_hashfunc, ioopm_cart_key_compare, ioopm_merch_in_cart_compare);
}

elem_t ioopm_create_cart(int cart_name)
{
  cart_t *cart = calloc(1, sizeof(cart_t));
  ioopm_list_t *list = ioopm_linked_list_create(ioopm_merch_in_cart_compare);
  
  cart->name = cart_name;
  cart->total_cost = 0;
  cart->wares = list;
  elem_t elem;
  elem.voi = cart;
  return elem;
}

void ioopm_remove_cart(elem_t *elem_cart, void *extra)
{
  cart_t *cart = (cart_t *)elem_cart->voi;
  ioopm_hash_table_t *cart_ht = (ioopm_hash_table_t *)extra;
  ioopm_hash_table_remove(cart_ht, int_elem(cart->name));
  ioopm_linked_apply_to_all(cart->wares, ioopm_merch_in_cart_delete, NULL);
  ioopm_linked_list_clear(cart->wares);
  free(cart->wares);
  free(cart);
}

bool ioopm_print_carts(ioopm_hash_table_t *cart_ht)
{
  puts("-----------------------");
  puts("      ALL CARTS:       ");
  puts("-----------------------");
  ioopm_list_t *cart_list = ioopm_hash_table_values(cart_ht);
  if (!(ioopm_linked_list_size(cart_list)))
    {
      ioopm_linked_list_clear(cart_list);
      free(cart_list);
      return false;
    }
  ioopm_list_iterator_t *iter = ioopm_list_iterator_create(cart_list);
  for (int i = 0; i < ioopm_linked_list_size(cart_list); i++)
    {
      elem_t elem = ioopm_iterator_current(iter).value;
      cart_t *cart = (cart_t *)elem.voi;
      printf("Cart %d\n", cart->name);
      if (ioopm_iterator_has_next(iter))
	{
	  ioopm_iterator_next(iter);
	}
      else
	{
	  puts("\n-----------------------\n");
	  Free(iter);
	  ioopm_linked_list_clear(cart_list);
	  Free(cart_list);
	  return true;
	}
    }
  Free(iter);
  ioopm_linked_list_clear(cart_list);
  Free(cart_list);
  return true;
}

int ioopm_cart_total_cost(ioopm_hash_table_t *cart_ht, int cart_name)
{
  puts("\n-----------------------\n");
  elem_t elem = ioopm_hash_table_lookup(cart_ht, int_elem(cart_name)).value;
  cart_t *cart = (cart_t *)elem.voi;
  puts("\n-----------------------\n");
  return cart->total_cost;
}

bool ioopm_print_cart_wares(ioopm_hash_table_t *cart_ht, int cart_name)
{
  puts("-------------------------------------------");
  printf("      HERE ARE THE WARES IN CART %d:     \n", cart_name);
  puts("-------------------------------------------");
  cart_t *cart = (cart_t *)(ioopm_hash_table_lookup(cart_ht, int_elem(cart_name)).value.voi);
  if (!(ioopm_linked_list_size(cart->wares))) return false;
  ioopm_list_iterator_t *iter = ioopm_list_iterator_create(cart->wares);
  for (int i = 0; i < ioopm_linked_list_size(cart->wares); i++)
    {
      elem_t elem = ioopm_iterator_current(iter).value;
      merch_in_cart_t *merch_in_cart = (merch_in_cart_t *)elem.voi;
      printf("Ware - %s        Amount - %zu\n", merch_in_cart->name, merch_in_cart->amount);
      if (ioopm_iterator_has_next(iter))
	{
	  ioopm_iterator_next(iter);
	}
      else
	{
	  puts("\n--------------------------------------\n");
	  Free(iter);
	  return true;
	}
    }
  return true;
}


//help function for adding merch to a cart by checking so that the amount of a merch in all carts is <= total stock of that merch.
int ioopm_total_stock_of_a_merch_in_all_carts(ioopm_hash_table_t *cart_ht, char *merch_name)
{
  ioopm_list_t *cart_list = ioopm_hash_table_values(cart_ht);
  elem_t counter;
  counter.in = 0;
  ioopm_list_iterator_t *iter_carts = ioopm_list_iterator_create(cart_list);
  for (int i = 0; i < ioopm_linked_list_size(cart_list); i++)
    {
      elem_t elem_cart = ioopm_iterator_current(iter_carts).value;
      cart_t *cart = (cart_t *)elem_cart.voi;
      ioopm_list_t *merch_in_cart_list = cart->wares;
      ioopm_list_iterator_t *iter_merch_in_cart = ioopm_list_iterator_create(merch_in_cart_list);
      for (int i = 0; i < ioopm_linked_list_size(merch_in_cart_list); i++)
	{
	  elem_t elem_merch_in_cart = ioopm_iterator_current(iter_merch_in_cart).value;
	  merch_in_cart_t *merch_in_cart = (merch_in_cart_t *)elem_merch_in_cart.voi;
	  if (ioopm_iterator_has_next(iter_merch_in_cart)) ioopm_iterator_next(iter_merch_in_cart);
	  if (strcmp(merch_in_cart->name, merch_name) == 0)
	    {
	      counter.in += merch_in_cart->amount;
	    }
	}
      free(iter_merch_in_cart);
      if (ioopm_iterator_has_next(iter_carts)) ioopm_iterator_next(iter_carts);
    }
  ioopm_linked_list_clear(cart_list);
  free(cart_list);
  free(iter_carts);
  return counter.in;
}

//help function to edit merch name, edits the name of all merch in carts
void ioopm_merch_edit_name_in_carts(ioopm_hash_table_t *cart_ht, char *merch_name, char *new_name)
{
  ioopm_list_t *cart_list = ioopm_hash_table_values(cart_ht);
  ioopm_list_iterator_t *iter_carts = ioopm_list_iterator_create(cart_list);
  for (int i = 0; i < ioopm_linked_list_size(cart_list); i++)
    {
      elem_t elem_cart = ioopm_iterator_current(iter_carts).value;
      cart_t *cart = (cart_t *)elem_cart.voi;
      ioopm_list_t *merch_in_cart_list = cart->wares;
      ioopm_list_iterator_t *iter_merch_in_cart = ioopm_list_iterator_create(merch_in_cart_list);
      for (int i = 0; i < ioopm_linked_list_size(merch_in_cart_list); i++)
	{
	  elem_t elem_merch_in_cart = ioopm_iterator_current(iter_merch_in_cart).value;
	  merch_in_cart_t *merch_in_cart = (merch_in_cart_t *)elem_merch_in_cart.voi;
	  if (ioopm_iterator_has_next(iter_merch_in_cart)) ioopm_iterator_next(iter_merch_in_cart);
	  if (strcmp(merch_in_cart->name, merch_name) == 0)
	    {
	      free(merch_in_cart->name);
	      merch_in_cart->name = new_name;
	    }
	}
      free(iter_merch_in_cart);
      if (ioopm_iterator_has_next(iter_carts)) ioopm_iterator_next(iter_carts);
    }
  free(merch_name);
  ioopm_linked_list_clear(cart_list);
  free(cart_list);
  free(iter_carts);
  return;
}

//help function to edit merch price, edits the price of all merch in carts
void ioopm_merch_edit_price_in_carts(ioopm_hash_table_t *cart_ht, char *merch_name,int old_price, int new_price)
{
  ioopm_list_t *cart_list = ioopm_hash_table_values(cart_ht);
  ioopm_list_iterator_t *iter_carts = ioopm_list_iterator_create(cart_list);
  for (int i = 0; i < ioopm_linked_list_size(cart_list); i++)
    {
      elem_t elem_cart = ioopm_iterator_current(iter_carts).value;
      cart_t *cart = (cart_t *)elem_cart.voi;
      ioopm_list_t *merch_in_cart_list = cart->wares;
      ioopm_list_iterator_t *iter_merch_in_cart = ioopm_list_iterator_create(merch_in_cart_list);
      for (int i = 0; i < ioopm_linked_list_size(merch_in_cart_list); i++)
	{
	  elem_t elem_merch_in_cart = ioopm_iterator_current(iter_merch_in_cart).value;
	  merch_in_cart_t *merch_in_cart = (merch_in_cart_t *)elem_merch_in_cart.voi;
	  if (ioopm_iterator_has_next(iter_merch_in_cart)) ioopm_iterator_next(iter_merch_in_cart);
	  if (strcmp(merch_in_cart->name, merch_name) == 0)
	    {
	      cart->total_cost -= merch_in_cart->amount * old_price;
	      cart->total_cost += merch_in_cart->amount * new_price;
	    }
	}
      free(iter_merch_in_cart);
      if (ioopm_iterator_has_next(iter_carts)) ioopm_iterator_next(iter_carts);
    }
  ioopm_linked_list_clear(cart_list);
  free(cart_list);
  free(iter_carts);
  return;
}

//cant add more to the cart than the total stock of the merch in the warehouse
bool ioopm_add_to_cart(ioopm_hash_table_t *cart_ht, elem_t elem_cart, elem_t elem_merch, int amount_to_add)
{
  cart_t *cart = (cart_t *)elem_cart.voi;
  merch_t *merch = (merch_t *)elem_merch.voi;
  int stock_result = ioopm_total_stock_of_a_merch_in_all_carts(cart_ht, merch->name);
  if( (stock_result + amount_to_add) > merch->total_stock )
    {
      return false;
    }
  ioopm_list_t *list = cart->wares;
  ioopm_list_iterator_t *iter_values = ioopm_list_iterator_create(list);
  for (int i = 0; i < ioopm_linked_list_size(list); i++)
    {
      elem_t elem_merch_in_cart = ioopm_iterator_current(iter_values).value;
      merch_in_cart_t *merch_in_cart = (merch_in_cart_t *)elem_merch_in_cart.voi;
      if (strcmp(merch_in_cart->name, value_name(elem_merch)) == 0)
	{
	  merch_in_cart->amount += amount_to_add;
	  cart->total_cost += (amount_to_add * merch->price);
	  free(iter_values);
	  return true;
	}
      if (ioopm_iterator_has_next(iter_values)) ioopm_iterator_next(iter_values);
    }
  merch_in_cart_t  *merch_in_cart = calloc(1, sizeof(merch_in_cart_t));
  merch_in_cart->amount += amount_to_add;
  merch_in_cart->name = strdup(value_name(elem_merch));
  cart->total_cost += (amount_to_add * merch->price);
  elem_t elem;
  elem.voi = merch_in_cart;
  ioopm_linked_list_append(cart->wares,elem);
  free(iter_values);
  return true;
}

//cant remove more from cart than there is in it
void ioopm_remove_from_cart(ioopm_hash_table_t *cart_ht, elem_t elem_cart, elem_t elem_merch,  int amount_to_remove)
{
  cart_t *cart = (cart_t *)elem_cart.voi;
  merch_t *merch = (merch_t *)elem_merch.voi;
  ioopm_list_t *list = cart->wares;
  ioopm_list_iterator_t *iter_values = ioopm_list_iterator_create(list);
  for (int i = 0; i < ioopm_linked_list_size(list); i++)
    {
      elem_t elem_merch_in_cart = ioopm_iterator_current(iter_values).value;
      merch_in_cart_t *merch_in_cart = (merch_in_cart_t *)elem_merch_in_cart.voi;
      if (strcmp(merch_in_cart->name, merch->name) == 0)
	{
	  if (merch_in_cart->amount < amount_to_remove)
	    {
	      merch_in_cart->amount = 0;
	      free(iter_values);
	      return;
	    }
	  merch_in_cart->amount -= amount_to_remove;
	  cart->total_cost -= (amount_to_remove * merch->price);
	  if (merch_in_cart->amount == 0)
	    {
	      free(merch_in_cart->name);
	      free(merch_in_cart);
	      ioopm_linked_list_remove(list, i);
	    }
	  free(iter_values);
	  return;
	}
      if (ioopm_iterator_has_next(iter_values)) ioopm_iterator_next(iter_values);
    }
  free(iter_values);
  return;
}

static bool ioopm_is_merch_in_cart_wares_list(ioopm_list_t *list, char *merch_name)
{
  ioopm_list_iterator_t *iter_values = ioopm_list_iterator_create(list);
  for (int i = 0; i < ioopm_linked_list_size(list); i++)
    {
      elem_t elem_merch_in_cart = ioopm_iterator_current(iter_values).value;
      merch_in_cart_t *merch_in_cart = (merch_in_cart_t *)elem_merch_in_cart.voi;
      if (strcmp(merch_name, merch_in_cart->name) == 0)
	{
	  free(iter_values);
	  return true;
	}
      if (ioopm_iterator_has_next(iter_values)) ioopm_iterator_next(iter_values);
    }
  free(iter_values);
  return false;
}

bool ioopm_is_merch_in_a_cart(ioopm_hash_table_t *merch_ht,ioopm_hash_table_t *cart_ht, char *merch_name)
{
  ioopm_list_t *list = ioopm_hash_table_values(cart_ht);
  ioopm_list_iterator_t *iter_values = ioopm_list_iterator_create(list);
  for (int i = 0; i < ioopm_linked_list_size(list); i++)
    {
      cart_t *cart = (cart_t *)(ioopm_iterator_current(iter_values).value.voi);
      if (ioopm_is_merch_in_cart_wares_list(cart->wares,merch_name))
	{
	  ioopm_linked_list_clear(list);
	  free(list);
	  free(iter_values);
	  return true;
	}
      if (ioopm_iterator_has_next(iter_values)) ioopm_iterator_next(iter_values);
    }
  ioopm_linked_list_clear(list);
  free(list);
  free(iter_values);
  return false;
}

void ioopm_checkout_cart(ioopm_hash_table_t *merch_ht, ioopm_hash_table_t *shelf_ht, ioopm_hash_table_t *cart_ht, cart_t *cart)
{
  ioopm_list_t *list = cart->wares;
  ioopm_list_iterator_t *iter_values = ioopm_list_iterator_create(list);
  for (int i = 0; i < ioopm_linked_list_size(list); i++)
    {
      elem_t elem_merch_in_cart = ioopm_iterator_current(iter_values).value;
      merch_in_cart_t *merch_in_cart = (merch_in_cart_t *)elem_merch_in_cart.voi;
      ioopm_merch_stock_decrease(merch_ht, shelf_ht, str_elem(merch_in_cart->name), merch_in_cart->amount, NULL);
      if (ioopm_iterator_has_next(iter_values)) ioopm_iterator_next(iter_values);
    }
  free(iter_values);
  return;
}

