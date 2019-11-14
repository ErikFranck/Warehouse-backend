#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "backend.h"

#define int_elem(i) (elem_t) { .in =(i) }
#define str_elem(s) (elem_t) { .str =(s) }

#define value_name(elem) ((char *) ((merch_t *)elem.voi)->name)
#define value_descr(elem) ((char *) ((merch_t *)elem.voi)->descr)
#define value_price(elem) ((int) ((merch_t *)elem.voi)->price)

#define shelf_merch_name(elem) ((char *) ((shelf_t *)elem.voi)->merch_name)
#define cart_merch_name(elem) ((char *) ((merch_in_cart_t *)elem.voi)->name)

// TYPEDEFINITIONS
typedef struct merch merch_t;
typedef struct shelf shelf_t;
typedef struct undo undo_t;
typedef struct cart cart_t;
typedef struct merch_in_cart merch_in_cart_t;

// STRUCTS
struct shelf
{
  char *shelf_name;
  char *merch_name;
  int shelf_stock;
};

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

struct undo
{
  char action;
  char *string_data1; 
  char *string_data2;
  int int_data1;
  int int_data2;
  merch_t *merch_data;
  shelf_t *shelf_data;
  cart_t *cart_data;
  ioopm_list_t *list_data;
};


///////////////////////////////////////////////////////////////////////////
//////////////////HASHING FUNCTIONS///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

// Returns randomized unsigned key.
static unsigned long ioopm_merch_hashfunc(elem_t keyword) //to get key for insertion in hash_table
{
  char *key = keyword.str;// value_name(&keyword);
  int result = 0;
  do
    {
      result = result*31 + *key;
    }
  while (*++key != '\0');
  
  return result;
}

///////////////////////////////////////////////////////////////////////////
//////////////////COMPARE FUNCTIONS///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

// Compare function for comparing the name of the merch_t in elem.
static bool ioopm_merch_value_name_compare(elem_t first, elem_t second)
{
  char *name1 = value_name(&first);
  char *name2 = value_name(&second);
  return (strcmp(name1, name2) == 0);
}

// Compare function for comparing the name of the shelf_t in elem
static bool ioopm_shelf_value_name_compare(elem_t first, elem_t second)
{
  char *merch_name1 = shelf_merch_name(first);
  char *merch_name2 = shelf_merch_name(second);
  return (strcmp(merch_name1, merch_name2) == 0);
}

// Compare function for comparing the key of the elem.
static bool ioopm_merch_key_compare(elem_t first, elem_t second)
{
  return (strcmp(first.str, second.str) == 0);
}


///////////////////////////////////////////////////////////////////////////
//////////////////CREATE AND DELETE FUNCTIONS/////////////////////////////
/////////////////////////////////////////////////////////////////////////

static elem_t ioopm_shelf_create(char *merch_name, char *shelf_name)
{
  elem_t elem;
  shelf_t *shelf = calloc(1, sizeof(shelf_t));
  shelf->merch_name = strdup(merch_name); 
  shelf->shelf_name = shelf_name; 
  shelf->shelf_stock = 0; 
  elem.voi = shelf;
  return elem;
}

elem_t ioopm_merch_create(char *name, char *descr, int price)
{
  elem_t elem;
  merch_t *merch = calloc(1, sizeof(merch_t));
  merch->name = name;
  merch->descr = descr;
  merch->price = price;
  merch->total_stock = 0;
  ioopm_list_t *list = ioopm_linked_list_create(ioopm_shelf_value_name_compare);
  merch->shelves = list;
  elem.voi = merch;
  return elem;
}

static void ioopm_shelf_delete(elem_t *elem_shelf,void *extra)
{
  shelf_t *shelf = (shelf_t *)elem_shelf->voi;
  free(shelf->shelf_name);
  free(shelf->merch_name);
  free(shelf);
  return;
}

///////////////////////////////////////////////////////////////////////////
//////////////////UNDO PUSH FUNCTIONS/////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

//help function for undo push checkout. Finds out which shelves will be affected and by how much when you checkout a cart.
//@param cart_wares_list the list of wares of the cart you want to checkout
//@return affected_shelves_list a list of shelves that represent how the shelves in shelf_ht will be affected
static ioopm_list_t *ioopm_undo_checkout_affected_shelves(ioopm_hash_table_t *merch_ht, ioopm_list_t *cart_wares_list)
{
  ioopm_list_t *affected_shelves_list = ioopm_linked_list_create(ioopm_shelf_value_name_compare);//return list
  for (int i = 0; i < ioopm_linked_list_size(cart_wares_list) ; i++)
    {
      elem_t elem_merch_in_cart = ioopm_linked_list_get(cart_wares_list, i).value;
      merch_in_cart_t *merch_in_cart = (merch_in_cart_t *)elem_merch_in_cart.voi;//current merch_in_cart
      elem_t elem_merch = ioopm_hash_table_lookup(merch_ht, str_elem(merch_in_cart->name)).value;
      merch_t *merch = (merch_t *)elem_merch.voi;
      int amount = merch_in_cart->amount;//amount of stock to reduce 
      for (int k = 0; k < ioopm_linked_list_size(merch->shelves) && amount; k++)
	{
	  shelf_t *shelf = (shelf_t *)ioopm_linked_list_get(merch->shelves, k).value.voi;//current shelf
	  elem_t elem_shelf = ioopm_shelf_create(shelf->merch_name, strdup(shelf->shelf_name));
	  shelf_t *new_shelf = (shelf_t *)elem_shelf.voi;
	  
	  if (shelf->shelf_stock > amount)
	    {
	      new_shelf->shelf_stock = amount;
	      amount = 0;
	    }
	  else{
	    new_shelf->shelf_stock = shelf->shelf_stock;
	    amount -= shelf->shelf_stock;
	  }
	  ioopm_linked_list_append(affected_shelves_list,elem_shelf);
	}
    }
  return affected_shelves_list;
}

static void ioopm_undo_list_delete_17th(ioopm_list_t *undo_list)
{
  ioopm_option_t result = ioopm_linked_list_get(undo_list,16);
  elem_t elem = result.value;
  undo_t *undo = elem.voi;
  switch (undo->action)
    {
    case 'a' :
      free(undo->string_data1);
      free(undo);
      ioopm_linked_list_remove(undo_list,16);
      break;
      
    case 'b' : ;
      
      merch_t *merch = undo->merch_data;
      
      ioopm_linked_apply_to_all(merch->shelves,ioopm_shelf_delete,NULL);
      ioopm_linked_list_clear(merch->shelves);
      free(merch->shelves);
      free(merch->name);
      free(merch->descr);
      free(merch);
      free(undo);
      ioopm_linked_list_remove(undo_list,16);
      break;
      
    case 'c':
      free(undo->string_data1);
      free(undo->string_data2);
      free(undo);
      ioopm_linked_list_remove(undo_list,16);
	  break;
	  
    case 'd':
      free(undo);
      ioopm_linked_list_remove(undo_list,16);
      break;
      
    case 'e':
      free(undo->string_data1);
      free(undo->string_data2);
      free(undo);
      ioopm_linked_list_remove(undo_list,16);
      break;
      
    case 'f':
      free(undo->shelf_data->shelf_name);
      free(undo->shelf_data);
      free(undo->string_data1);
      free(undo);
      ioopm_linked_list_remove(undo_list,16);
      break;
      
    case 'g':
      free(undo);
      ioopm_linked_list_remove(undo_list,16);
      break;
      
    case 'h': ;
      cart_t *cart = undo->cart_data;
      ioopm_linked_apply_to_all(cart->wares, ioopm_merch_in_cart_delete, NULL);
      ioopm_linked_list_clear(cart->wares);
      free(cart->wares);
      free(cart);
      free(undo);
      ioopm_linked_list_remove(undo_list,16);
      break;
      
    case 'i': 
      free(undo->string_data1);
      free(undo);
      ioopm_linked_list_remove(undo_list,16);
      break;
      
    case 'j':
      free(undo->string_data1);
      free(undo);
      ioopm_linked_list_remove(undo_list,16);
      break;
      
    case 'k': 
      ioopm_linked_apply_to_all(undo->cart_data->wares, ioopm_merch_in_cart_delete, NULL);
      ioopm_linked_list_clear(undo->cart_data->wares);
      free(undo->cart_data->wares);
      free(undo->cart_data);
      ioopm_linked_apply_to_all(undo->list_data,ioopm_shelf_delete,NULL);
      ioopm_linked_list_clear(undo->list_data);
      free(undo->list_data);
      free(undo);
      ioopm_linked_list_remove(undo_list,16);
      break;
    }
}

static void ioopm_undo_push_merch_add(ioopm_list_t *undo_list, char *merch_name)
{
  undo_t *undo = calloc(1, sizeof(undo_t));
  undo->action = 'a';
  undo->string_data1 = strdup(merch_name);
  free(merch_name);

  ioopm_linked_list_prepend(undo_list, void_elem(undo));
  if (ioopm_linked_list_size(undo_list) > 16) ioopm_undo_list_delete_17th(undo_list);
}


static void ioopm_undo_push_merch_remove(ioopm_list_t *undo_list, merch_t *merch)
{

  elem_t new_elem_merch = ioopm_merch_create(strdup(merch->name), strdup(merch->descr), merch->price);
  merch_t *new_merch = (merch_t *)new_elem_merch.voi;
  
  new_merch->total_stock = merch->total_stock;
  int y = ioopm_linked_list_size(merch->shelves);
  for  (int i = 0; ioopm_linked_list_size(new_merch->shelves) != y ; i++)
    {
      ioopm_option_t result = ioopm_linked_list_get(merch->shelves, i);
      elem_t old_elem = result.value;
      shelf_t *old_shelf = (shelf_t *)old_elem.voi;

      elem_t new_elem_shelf = ioopm_shelf_create(old_shelf->merch_name, strdup(old_shelf->shelf_name));
      shelf_t *new_shelf = (shelf_t *)new_elem_shelf.voi;
      
      new_shelf->shelf_stock = old_shelf->shelf_stock;
      ioopm_linked_list_append(new_merch->shelves, new_elem_shelf);
    }
  undo_t *undo = calloc(1, sizeof(undo_t));
  undo->action = 'b';
  undo->merch_data = (merch_t *)new_merch;

  ioopm_linked_list_prepend(undo_list, void_elem(undo));
  if (ioopm_linked_list_size(undo_list) > 16) ioopm_undo_list_delete_17th(undo_list);
}


static void ioopm_undo_push_merch_edit_name(ioopm_list_t *undo_list, char *old_name, char *new_name)
{
   undo_t *undo = calloc(1, sizeof(undo_t));
  undo->action = 'c';
  undo->string_data1 = old_name;
  undo->string_data2 = new_name;
 
  ioopm_linked_list_prepend(undo_list, void_elem(undo));
  if (ioopm_linked_list_size(undo_list) > 16) ioopm_undo_list_delete_17th(undo_list);
}

static void ioopm_undo_push_merch_edit_price(ioopm_list_t *undo_list, elem_t elem_merch, int new_price)
{
  merch_t *merch = (merch_t *)elem_merch.voi;
  undo_t *undo = calloc(1, sizeof(undo_t));
  undo->action = 'd';
  undo->string_data1 = merch->name;
  undo->int_data1 = merch->price;
   
  ioopm_linked_list_prepend(undo_list, void_elem(undo));
  if (ioopm_linked_list_size(undo_list) > 16) ioopm_undo_list_delete_17th(undo_list);
}
 
static void ioopm_undo_push_merch_edit_descr(ioopm_list_t *undo_list, char *merch_name, char *old_descr)
{
    undo_t *undo = calloc(1, sizeof(undo_t));
  undo->action = 'e';
  undo->string_data1 = merch_name;
  undo->string_data2 = old_descr;
  
  ioopm_linked_list_prepend(undo_list, void_elem(undo));
  if (ioopm_linked_list_size(undo_list) > 16) ioopm_undo_list_delete_17th(undo_list);
}

static void ioopm_undo_push_merch_stock_increase(ioopm_list_t *undo_list, char *merch_name, char *shelf_name, int amount_increased)
{
  undo_t *undo = calloc(1, sizeof(undo_t));
  shelf_t *shelf = calloc(1, sizeof(shelf_t));
  shelf->shelf_name = shelf_name;
  shelf->shelf_stock = amount_increased;

  undo->action = 'f';
  undo->string_data1 = merch_name;
  undo->shelf_data = shelf;
  
  ioopm_linked_list_prepend(undo_list, void_elem(undo));
  if (ioopm_linked_list_size(undo_list) > 16) ioopm_undo_list_delete_17th(undo_list);
}


static void ioopm_undo_push_create_cart(ioopm_list_t *undo_list, int cart_name)
{
  undo_t *undo = calloc(1, sizeof(undo_t));
  undo->action = 'g';
  undo->int_data1 = cart_name;
  
  ioopm_linked_list_prepend(undo_list, void_elem(undo));
  if (ioopm_linked_list_size(undo_list) > 16) ioopm_undo_list_delete_17th(undo_list);
}

static void ioopm_undo_push_remove_cart(ioopm_list_t *undo_list, ioopm_hash_table_t *cart_ht, elem_t elem_cart)
{
  cart_t *cart = (cart_t *)elem_cart.voi;
  elem_t new_elem_cart = ioopm_create_cart(cart->name);
  cart_t *new_cart = (cart_t *)new_elem_cart.voi;
  new_cart->total_cost = cart->total_cost;
  int y = ioopm_linked_list_size(cart->wares);
  for  (int i = 0; ioopm_linked_list_size(new_cart->wares) != y ; i++)
    {
      elem_t old_elem = ioopm_linked_list_get(cart->wares, 0).value;
      merch_in_cart_t *old_merch_in_cart = (merch_in_cart_t *)old_elem.voi;
      merch_in_cart_t *merch_in_cart = calloc(1, sizeof(merch_in_cart_t));
      merch_in_cart->name = strdup(old_merch_in_cart->name);
      merch_in_cart->amount = old_merch_in_cart->amount;
      elem_t cart_elem;
      cart_elem.voi  = (merch_in_cart_t *)merch_in_cart;
      ioopm_linked_list_append(new_cart->wares, cart_elem);
    }
  undo_t *undo = calloc(1, sizeof(undo_t));
  undo->action = 'h';
  undo->cart_data = new_cart;
  ioopm_linked_list_prepend(undo_list, void_elem(undo));
}

static void ioopm_undo_push_add_to_cart(ioopm_list_t *undo_list, elem_t elem_cart, char *merch_name, int amount_increased)
{
  undo_t *undo = calloc(1, sizeof(undo_t));
  cart_t *cart = (cart_t *)elem_cart.voi;
 
  undo->action = 'i';
  undo->string_data1  = merch_name;
  undo->int_data1 = cart->name;
  undo->int_data2 = amount_increased;
  
  ioopm_linked_list_prepend(undo_list, void_elem(undo));
  if (ioopm_linked_list_size(undo_list) > 16) ioopm_undo_list_delete_17th(undo_list);
}

static void ioopm_undo_push_remove_from_cart(ioopm_list_t *undo_list, elem_t elem_cart, char *merch_name, int amount_decreased)
{
  undo_t *undo = calloc(1, sizeof(undo_t));
  cart_t *cart = (cart_t *)elem_cart.voi;
  
  undo->action = 'j';
  undo->string_data1 = merch_name; //name of merch
  undo->int_data1 = cart->name; //name of cart
  undo->int_data2 = amount_decreased;
  
  ioopm_linked_list_prepend(undo_list, void_elem(undo));
  if (ioopm_linked_list_size(undo_list) > 16) ioopm_undo_list_delete_17th(undo_list);
}


static void ioopm_undo_push_checkout(ioopm_list_t *undo_list, ioopm_hash_table_t *merch_ht, cart_t *cart) 
{
  undo_t *undo = calloc(1, sizeof(undo_t));
  ioopm_list_t *affected_shelves_list = ioopm_undo_checkout_affected_shelves(merch_ht, cart->wares);
  
  undo->action = 'k';
  undo->cart_data = cart;
  undo->list_data = affected_shelves_list;
  
  ioopm_linked_list_prepend(undo_list, void_elem(undo));
  if (ioopm_linked_list_size(undo_list) > 16) ioopm_undo_list_delete_17th(undo_list);
}

///////////////////////////////////////////////////////////////////////////
//////////////////MERCH FUNCTIONS/////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

ioopm_hash_table_t *ioopm_create_merch_ht()
{
 return ioopm_hash_table_create(ioopm_merch_hashfunc, ioopm_merch_key_compare, ioopm_merch_value_name_compare);
}

ioopm_hash_table_t *ioopm_create_shelf_ht()
{
return ioopm_hash_table_create(ioopm_merch_hashfunc, ioopm_merch_key_compare, ioopm_shelf_value_name_compare);
}

 bool ioopm_is_merch_in_ht(ioopm_hash_table_t *merch_ht, char *merch_name)
{
  ioopm_option_t result = ioopm_hash_table_lookup(merch_ht, str_elem(merch_name));
  return result.success;
}

int ioopm_merch_total_stock(ioopm_hash_table_t *merch_ht, char* merch_name)
{
  ioopm_option_t result = ioopm_hash_table_lookup(merch_ht, str_elem(merch_name)); //gives out option_t with value
  if (!(result.success)) return 0;
  elem_t elem = result.value;
  merch_t *merch = (merch_t *)elem.voi;
  return merch->total_stock;
}


bool ioopm_merch_insert(ioopm_hash_table_t *merch_ht, elem_t key, elem_t merch)
{
  ioopm_option_t result = ioopm_hash_table_lookup(merch_ht, key);
  if (strcmp(key.str, value_name(merch)) != 0) return false; //check so key and merch->name is same
  if (result.success)//checks if something with the same key is in the merch_ht. If not, it inserts the merch.
    {
      return false;
    }
  else
    {
      ioopm_hash_table_insert(merch_ht, key, merch);
      return true;
    }
}

void ioopm_print_merch(ioopm_hash_table_t *merch_ht)
{
  if (ioopm_hash_table_is_empty(merch_ht)) return;
  puts("-----------------------");
  puts("   ALL MERCHANDISE:    ");
  puts("-----------------------");
  ioopm_list_t *merch_list = ioopm_hash_table_values(merch_ht);
  ioopm_list_iterator_t *iter_keys = ioopm_list_iterator_create(merch_list);
  bool remains = true;
  while (remains)
    {
      for (int i = 0; i < 20; i++)
	{
	  elem_t elem = ioopm_iterator_current(iter_keys).value;
	  merch_t *merch = (merch_t *)elem.voi;
	  printf("  Name:%s   Description:%s   Price:%d\n",merch->name, merch->descr, merch->price);
	  
	  if (ioopm_iterator_has_next(iter_keys))
	    {
	      ioopm_iterator_next(iter_keys);
	    }
	  else
	    {
	      puts("\n-----------------------\n");
	      Free(iter_keys);
	      ioopm_linked_list_clear(merch_list);
	      Free(merch_list);
	      return;
	    }
 	}
      remains = ask_question_int("Type 1 to continue listing merchandises, 0 to go back to menu.");
    }
  Free(iter_keys);
  ioopm_linked_list_clear(merch_list);
  Free(merch_list);
}
 
//shows which shelves a particular merch is in and the stock at each shelf
bool ioopm_merch_show_stock(ioopm_hash_table_t *merch_ht, elem_t key)
{
  ioopm_option_t result = ioopm_hash_table_lookup(merch_ht, key);
  if (!(result.success)) return false;
  elem_t elem = result.value;
  merch_t *merch = (merch_t *)elem.voi;
  ioopm_list_t *list = merch->shelves;
  if (ioopm_linked_list_size(list) == 0)
    {
      puts ("\n No stock of this merch in the warehouse. \n");
      return false;
    }
  else
    {
      puts("-------------------------------------------");
      printf("       STOCK FOR MERCH %s:               \n", merch->name);
      puts("-------------------------------------------");
      
      ioopm_list_iterator_t *iter_values = ioopm_list_iterator_create(list);
      for (int i = 0; i < ioopm_linked_list_size(list); i++)
	{
	  elem_t elem_shelf = ioopm_iterator_current(iter_values).value; 
	  shelf_t *shelf = (shelf_t *)elem_shelf.voi;
	  printf(" %d: Shelf %s, Stock %d \n", i+1, shelf->shelf_name, shelf->shelf_stock); //prints storage
	  ioopm_iterator_next(iter_values);
	}
      Free(iter_values);
      return true;
    }
}

ioopm_option_t ioopm_merch_stock_increase(ioopm_hash_table_t *merch_ht, ioopm_hash_table_t *shelf_ht, elem_t merch_key, elem_t shelf_key, int amount_increased)
{
  ioopm_option_t merch_ht_result = ioopm_hash_table_lookup(merch_ht, merch_key);
  elem_t elem_merch = merch_ht_result.value;
  merch_t *merch = (merch_t *)elem_merch.voi;
  ioopm_option_t shelf_ht_result = ioopm_hash_table_lookup(shelf_ht, shelf_key);
  if (!(shelf_ht_result.success))
    {
      elem_t elem_shelf = ioopm_shelf_create(merch_key.str, strdup(shelf_key.str)); 
      shelf_t *shelf = (shelf_t *)elem_shelf.voi;
      ioopm_hash_table_insert(shelf_ht, str_elem(shelf->shelf_name) , elem_shelf); 
      ioopm_linked_list_append(merch->shelves, elem_shelf); 
      merch->total_stock += amount_increased;
      shelf->shelf_stock += amount_increased;
      free(merch_key.str);
      free(shelf_key.str);
      return Success(elem_merch);
    }
  else
    {
      elem_t elem_shelf = shelf_ht_result.value;
      shelf_t *shelf = (shelf_t *)elem_shelf.voi;
      if (strcmp(shelf->merch_name, merch->name) == 0) //check if the merch on shelf is the same
	{                                              //kind as the one you want to insert
	  merch->total_stock += amount_increased;
	  shelf->shelf_stock += amount_increased;	      
	  free(merch_key.str);
	  free(shelf_key.str);
	  return Success(elem_merch); 
	}
      free(merch_key.str);
      free(shelf_key.str);
      return Failure();
    }
  free(merch_key.str);
  free(shelf_key.str);
  return Failure();
} 

//checks if a string is a valid shelf
bool is_shelf(char *str)
{
  if (!(str)) return false;
  bool result = false;
  int langd = strlen(str);
  if (isupper(str[0]) && langd > 1)
    {
      result = true;
      for (int x = 1; x < langd; x++)
	{
	  if (isdigit(str[x]) == 0) result = false;
	}
    }
  return result;
}

bool ioopm_merch_stock_decrease(ioopm_hash_table_t *merch_ht, ioopm_hash_table_t *shelf_ht, elem_t merch_key, int amount_decreased, char *shelf_optional)
{
  ioopm_option_t merch_ht_result = ioopm_hash_table_lookup(merch_ht, merch_key);
  elem_t elem_merch = merch_ht_result.value;
  merch_t *merch = (merch_t *)elem_merch.voi;
  ioopm_list_t *list = merch->shelves;
  int leftover = amount_decreased;
  
  if (merch->total_stock < amount_decreased) return false;
  while (leftover) {
    if (is_shelf(shelf_optional)) //optional is a valid shelf means it will remove stock from that shelf
      {
	for (int i = 0; i < ioopm_linked_list_size(list); i++)
	  {
	    ioopm_option_t result = ioopm_linked_list_get(list, i); //gets shelf at place i in merch->shelves list
	    shelf_t *shelf = (shelf_t *)result.value.voi;
	    
	    if (strcmp(shelf_optional,shelf->shelf_name) == 0) //checks if its the shelf you want to remove stock from
	      {
		if (shelf->shelf_stock > leftover)
		  {
		    shelf->shelf_stock -= leftover;
		    merch->total_stock -= leftover;
		    return true;
		  }
		merch->total_stock -= shelf->shelf_stock;
		ioopm_linked_list_remove(list, i);
		ioopm_hash_table_remove(shelf_ht, str_elem(shelf->shelf_name));
		free(shelf->merch_name);
		free(shelf->shelf_name);
		free(shelf);
		return true;
	      }
	  }
      }
    ioopm_option_t result = ioopm_linked_list_get(list, 0);
    shelf_t *shelf = (shelf_t *)result.value.voi;
    if (shelf->shelf_stock > leftover)
      {
	shelf->shelf_stock -= leftover;
	merch->total_stock -= leftover;
	leftover = 0;
	return true;
      }
    leftover -= shelf->shelf_stock;
    merch->total_stock -= shelf->shelf_stock;
    ioopm_linked_list_remove(list, 0);
    ioopm_hash_table_remove(shelf_ht, str_elem(shelf->shelf_name));
    free(shelf->merch_name);
    free(shelf->shelf_name);
    free(shelf);
  }
  return true;
}
    
// Removes an merchandise completely from the warehouse, including stock if it has any.
bool ioopm_merch_remove(ioopm_hash_table_t *merch_ht, ioopm_hash_table_t *shelf_ht, elem_t key)
 {
  ioopm_option_t result = ioopm_hash_table_lookup(merch_ht, key);
  elem_t elem_merch = result.value;
  merch_t *merch = (merch_t *)elem_merch.voi;
  if (result.success)
    {
      if (merch->total_stock)//removes merch from all shelves
	{
	  ioopm_list_iterator_t *iter = ioopm_list_iterator_create(merch->shelves);
	  ioopm_option_t result = ioopm_iterator_current(iter);
	  elem_t elem = result.value;
	  shelf_t *shelf = (shelf_t *)elem.voi;
	  ioopm_hash_table_remove(shelf_ht, str_elem(shelf->shelf_name));
	  while (ioopm_iterator_has_next(iter))
	    {
	      ioopm_iterator_next(iter);
	      ioopm_option_t result = ioopm_iterator_current(iter);
	      elem_t elem = result.value;
	      shelf_t *shelf = (shelf_t *)elem.voi;
	      ioopm_hash_table_remove(shelf_ht, str_elem(shelf->shelf_name));
	    } 
	  free(iter);    
	}
      if (ioopm_linked_list_size(merch->shelves))
	{
	  ioopm_linked_apply_to_all(merch->shelves,ioopm_shelf_delete,NULL);
	}
      ioopm_linked_list_clear(merch->shelves);
      ioopm_hash_table_remove(merch_ht, key);
      free(merch->shelves);
      free(merch->name);
      free(merch->descr);
      free(merch);
      return true;
    }
  else
    {
      return false;
    }
}

bool ioopm_merch_edit_price(ioopm_hash_table_t *merch_ht,ioopm_hash_table_t *cart_ht, elem_t key, int new_price)
{
  ioopm_option_t result = ioopm_hash_table_lookup(merch_ht, key);
  if (result.success)
    {
      elem_t elem = result.value;
      merch_t *merch = (merch_t *)elem.voi;
      ioopm_merch_edit_price_in_carts(cart_ht, merch->name, merch->price, new_price);
      merch->price = new_price;
      return true;
    }
  else
    {
      return false;
    }
}

bool ioopm_merch_edit_descr(ioopm_hash_table_t *merch_ht, elem_t key,char *new_description)
{
  ioopm_option_t result = ioopm_hash_table_lookup(merch_ht, key);
  free(key.str);
  if (result.success)
    {
      elem_t elem = result.value;
      merch_t *merch = (merch_t *)elem.voi;
      free(merch->descr);
      merch->descr = new_description;
      return true;
    }
  else
    {
      free(new_description);
      return false;
    }
}

//this edit func needs insert and remove so it needs them to be above
bool ioopm_merch_edit_name(ioopm_hash_table_t *merch_ht,ioopm_hash_table_t *cart_ht, merch_t *merch_old,char *new_name)
{
  ioopm_option_t result = ioopm_hash_table_lookup(merch_ht, str_elem(merch_old->name));
  if (result.success)
    {
      elem_t elem_new = ioopm_merch_create(strdup(new_name), strdup(merch_old->descr), merch_old->price);
      merch_t *merch_new = (merch_t *)elem_new.voi; // creates a new merch with old merch info
      merch_new->total_stock = merch_old->total_stock;
      free(new_name);
      if (ioopm_total_stock_of_a_merch_in_all_carts(cart_ht, merch_old->name) > 0)
	{
	  ioopm_merch_edit_name_in_carts(cart_ht, strdup(merch_old->name), strdup(merch_new->name)); //edits names of merch in carts
	}
      ioopm_list_t *list = merch_old->shelves;
      ioopm_list_iterator_t *iter = ioopm_list_iterator_create(list);
      for (int i = 0; i < ioopm_linked_list_size(list); i++)
	{
	  ioopm_option_t result = ioopm_iterator_current(iter);
	  elem_t elem = result.value;
	  shelf_t *shelf = (shelf_t *)elem.voi;
	  free(shelf->merch_name);
	  shelf->merch_name = strdup(merch_new->name);
	  ioopm_iterator_next(iter);
	}
      ioopm_iterator_destroy(iter);
      free(merch_new->shelves);
      merch_new->shelves = merch_old->shelves;
      merch_old->shelves = NULL;
      ioopm_hash_table_remove(merch_ht, str_elem(merch_old->name));		  
      free(merch_old->name);
      free(merch_old->descr);
      free(merch_old);
      ioopm_merch_insert(merch_ht, str_elem(merch_new->name), elem_new);
      return true;
    }
  else
    {
      return false;
    }
}

///////////////////////////////////////////////////////////////////////////
//////////////////MAIN FUNCTIONS//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

bool ioopm_undoable_merch_add(ioopm_list_t *undo_list, ioopm_hash_table_t *merch_ht, char *merch_name, char *descr, int price)
{
  ioopm_option_t result = ioopm_hash_table_lookup(merch_ht, str_elem(merch_name));
  if (!(result.success))
    {
      elem_t elem = ioopm_merch_create(strdup(merch_name), strdup(descr), price);
      merch_t *merch = (merch_t *)elem.voi;
      free(merch_name);
      free(descr);
      ioopm_merch_insert(merch_ht, str_elem(merch->name), elem);
      ioopm_undo_push_merch_add(undo_list,strdup(merch->name));
      return true;
    }
  else
    {
      free(merch_name);
      free(descr);
    return false;
    } 
}

void ioopm_undoable_edit_price(ioopm_list_t *undo_list, ioopm_hash_table_t *merch_ht,ioopm_hash_table_t *cart_ht, char *merch_name, int new_price)
{
  ioopm_option_t result = ioopm_hash_table_lookup(merch_ht, str_elem(merch_name));
  elem_t elem_merch = result.value;
  ioopm_undo_push_merch_edit_price(undo_list, elem_merch, new_price);
  ioopm_merch_edit_price(merch_ht,cart_ht, str_elem(merch_name), new_price);
}

void ioopm_undoable_edit_descr(ioopm_list_t *undo_list, ioopm_hash_table_t *merch_ht, char *merch_name, char *new_descr)
{
  ioopm_option_t result = ioopm_hash_table_lookup(merch_ht, str_elem(merch_name));
  elem_t elem_merch = result.value;
  merch_t *merch = (merch_t *)elem_merch.voi;
  ioopm_undo_push_merch_edit_descr(undo_list, strdup(merch->name), strdup(merch->descr));
  ioopm_merch_edit_descr(merch_ht, str_elem(strdup(merch_name)), strdup(new_descr));
}


bool ioopm_undoable_edit_name(ioopm_list_t *undo_list, ioopm_hash_table_t *merch_ht, ioopm_hash_table_t *cart_ht, char *merch_name, char *new_name)
{
  ioopm_option_t result = ioopm_hash_table_lookup(merch_ht, str_elem(merch_name));
  if (!(result.success)) return false;

  elem_t elem_merch = result.value;
  merch_t *merch = (merch_t *)elem_merch.voi;
  char *old_name = merch->name;
  ioopm_undo_push_merch_edit_name(undo_list, strdup(old_name), strdup(new_name));
  ioopm_merch_edit_name(merch_ht, cart_ht, merch, strdup(new_name));
  return true;
}


bool ioopm_merch_undoable_remove(ioopm_list_t *undo_list, ioopm_hash_table_t *merch_ht,ioopm_hash_table_t *shelf_ht, char *merch_name)
{
  ioopm_option_t result = ioopm_hash_table_lookup(merch_ht, str_elem(merch_name));
  if (!(result.success)) return false;

  elem_t elem_merch = result.value;
  merch_t *merch = (merch_t *)elem_merch.voi;
  ioopm_undo_push_merch_remove(undo_list,merch);
  ioopm_merch_remove(merch_ht, shelf_ht, str_elem(merch_name)); 
  return true;
}


bool ioopm_merch_undoable_replenish(ioopm_list_t *undo_list, ioopm_hash_table_t *merch_ht,ioopm_hash_table_t *shelf_ht, char *merch_name, int amount_increased, char *shelf_name)
{
  ioopm_option_t merch_result = ioopm_hash_table_lookup(merch_ht, str_elem(merch_name));
  elem_t elem_merch = merch_result.value; 
  merch_t *merch = (merch_t *)elem_merch.voi;
  ioopm_option_t result = ioopm_merch_stock_increase(merch_ht, shelf_ht, str_elem(strdup(merch_name)), str_elem(strdup(shelf_name)), amount_increased);
  if (!(result.success)) //if stock increase failed it means some other merch was already on the selected shelf
    {
      return false;
    }
  ioopm_undo_push_merch_stock_increase(undo_list,strdup(merch->name), strdup(shelf_name), amount_increased);
  return true;
}

void ioopm_merch_undoable_create_cart(ioopm_list_t *undo_list,ioopm_hash_table_t *cart_ht, int cart_name)
{
  ioopm_undo_push_create_cart(undo_list, cart_name);
  elem_t elem_cart1 = ioopm_create_cart(cart_name);
  ioopm_hash_table_insert(cart_ht, int_elem(cart_name), elem_cart1);
}

bool ioopm_merch_undoable_remove_cart(ioopm_list_t *undo_list,ioopm_hash_table_t *cart_ht, int cart_name)
{
  ioopm_option_t result = ioopm_hash_table_lookup(cart_ht, int_elem(cart_name));
  if (!(result.success)) return false;

  elem_t elem_cart = result.value; 
  ioopm_undo_push_remove_cart(undo_list, cart_ht, elem_cart);
  ioopm_remove_cart(&elem_cart, cart_ht);
  return true;
}

bool ioopm_merch_undoable_add_to_cart(ioopm_list_t *undo_list,ioopm_hash_table_t *merch_ht,ioopm_hash_table_t *cart_ht, int cart_name,char *merch_name, int amount_to_add )
{
  ioopm_option_t cart_result = ioopm_hash_table_lookup(cart_ht, int_elem(cart_name));
  if (!(cart_result.success)) return false;
  elem_t elem_cart = cart_result.value; 

  ioopm_option_t merch_result = ioopm_hash_table_lookup(merch_ht, str_elem(merch_name));
  if (!(merch_result.success)) return false;
  elem_t elem_merch = merch_result.value;
  
  ioopm_undo_push_add_to_cart(undo_list, elem_cart, strdup(merch_name), amount_to_add);
  ioopm_add_to_cart(cart_ht, elem_cart, elem_merch, amount_to_add); 
  return true;
}


bool ioopm_merch_undoable_remove_from_cart(ioopm_list_t *undo_list,ioopm_hash_table_t *merch_ht,ioopm_hash_table_t *cart_ht, int cart_name, char *merch_name, int amount_to_remove )
{
  ioopm_option_t cart_result = ioopm_hash_table_lookup(cart_ht, int_elem(cart_name));
  if (!(cart_result.success)) return false;
  elem_t elem_cart = cart_result.value; 

  ioopm_option_t merch_result = ioopm_hash_table_lookup(merch_ht, str_elem(merch_name));
  if (!(merch_result.success)) return false;
  elem_t elem_merch = merch_result.value;
  
  ioopm_undo_push_remove_from_cart(undo_list, elem_cart, strdup(merch_name), amount_to_remove);
  ioopm_remove_from_cart(cart_ht, elem_cart, elem_merch, amount_to_remove); 
  return true;
}


bool ioopm_undoable_checkout(ioopm_list_t *undo_list, ioopm_hash_table_t *merch_ht,ioopm_hash_table_t *shelf_ht,ioopm_hash_table_t *cart_ht,  int cart_name)
{
  ioopm_option_t cart_result = ioopm_hash_table_lookup(cart_ht, int_elem(cart_name));
  if (!(cart_result.success)) return false;

  elem_t elem_cart = cart_result.value; 
  cart_t *cart = (cart_t *)elem_cart.voi;
  ioopm_undo_push_checkout(undo_list,merch_ht,cart );
  ioopm_checkout_cart(merch_ht,shelf_ht, cart_ht, cart);
  ioopm_hash_table_remove(cart_ht,int_elem(cart->name));   
  return true;
}

///////////////////////////////////////////////////////////////////////////
//////////////////UNDO POP FUNCTIONS//////////////////////////////////////
/////////////////////////////////////////////////////////////////////////



//help function for pop checkout. Increases stocks according to affected_shelves_list.
static void ioopm_undo_pop_checkout_increase(ioopm_hash_table_t *merch_ht, ioopm_hash_table_t *shelf_ht, ioopm_list_t *affected_shelves_list)
{ 
  for (int i = 0; i <ioopm_linked_list_size(affected_shelves_list); i++)
    {
      shelf_t *shelf = (shelf_t *)ioopm_linked_list_get(affected_shelves_list, i).value.voi;
      ioopm_merch_stock_increase(merch_ht, shelf_ht, str_elem(strdup(shelf->merch_name)), str_elem(strdup(shelf->shelf_name)), shelf->shelf_stock);
    }
}

ioopm_list_t *undo_list_create()
{
  return ioopm_linked_list_create(ioopm_merch_key_compare);
}


void ioopm_undo_list_delete(ioopm_list_t *undo_list)
{
  while(ioopm_linked_list_size(undo_list))
    {
      ioopm_option_t result = ioopm_linked_list_get(undo_list,0);
      elem_t elem = result.value;
      undo_t *undo = elem.voi;
      switch (undo->action)
	{
	case 'a' :
	  free(undo->string_data1);
	  free(undo);
	  ioopm_linked_list_remove(undo_list,0);
	  break;
	  
	case 'b' : ;
	  merch_t *merch = undo->merch_data;
	  ioopm_linked_apply_to_all(merch->shelves,ioopm_shelf_delete,NULL);
 	  ioopm_linked_list_clear(merch->shelves);
	  free(merch->shelves);
	  free(merch->name);
	  free(merch->descr);
	  free(merch);
	  free(undo);
	  ioopm_linked_list_remove(undo_list,0);
	  break;
	  
	case 'c':
	  
	  free(undo->string_data1);
	  free(undo->string_data2);
	  free(undo);
	  ioopm_linked_list_remove(undo_list,0);
	  break;
	  
	case 'd':
	  free(undo);
	  ioopm_linked_list_remove(undo_list,0);
	  break;

	case 'e':
	  free(undo->string_data1);
	  free(undo->string_data2);
	  free(undo);
	  ioopm_linked_list_remove(undo_list,0);
	  break;

	case 'f':
	  free(undo->shelf_data->shelf_name);
	  free(undo->shelf_data);
	  free(undo->string_data1);
	  free(undo);
	  ioopm_linked_list_remove(undo_list,0);
	  break;

	case 'g':
	  free(undo);
	  ioopm_linked_list_remove(undo_list,0);
	  break;

	case 'h': ;
	  cart_t *cart = undo->cart_data;
	  ioopm_linked_apply_to_all(cart->wares, ioopm_merch_in_cart_delete, NULL);
	  ioopm_linked_list_clear(cart->wares);
	  free(cart->wares);
	  free(cart);
	  free(undo);
	  ioopm_linked_list_remove(undo_list,0);
	  break;

	case 'i':
	  free(undo->string_data1);
	  free(undo);
	  ioopm_linked_list_remove(undo_list,0);
 	  break;

	case 'j':
	  free(undo->string_data1);
	  free(undo);
	  ioopm_linked_list_remove(undo_list,0);
 	  break;
	  
	case 'k': 
	  ioopm_linked_apply_to_all(undo->cart_data->wares, ioopm_merch_in_cart_delete, NULL);
	  ioopm_linked_list_clear(undo->cart_data->wares);
	  free(undo->cart_data->wares);
	  free(undo->cart_data);
	  ioopm_linked_apply_to_all(undo->list_data,ioopm_shelf_delete,NULL);
 	  ioopm_linked_list_clear(undo->list_data);
	  free(undo->list_data);
	  free(undo);
	  ioopm_linked_list_remove(undo_list,0);
	  break;
	}
    }
  ioopm_linked_list_clear(undo_list);
  free(undo_list);
}

static void ioopm_undo_pop_merch_add(ioopm_list_t *undo_list, ioopm_hash_table_t *merch_ht, ioopm_hash_table_t *shelf_ht)
{
  ioopm_option_t result = ioopm_linked_list_get(undo_list,0);  
  elem_t elem = result.value; 
  undo_t *undo = (undo_t *)elem.voi;
  char *name = strdup(undo->string_data1);
  free(undo->string_data1);
  
  ioopm_merch_remove(merch_ht, shelf_ht, str_elem(name));
  free(name);
  free(undo);
  ioopm_linked_list_remove(undo_list, 0);
}

static void ioopm_undo_pop_merch_remove(ioopm_list_t *undo_list, ioopm_hash_table_t *merch_ht)
{
  ioopm_option_t result = ioopm_linked_list_get(undo_list,0);  
  elem_t elem = result.value; 
  undo_t *undo = (undo_t *)elem.voi;
  merch_t *merch = undo->merch_data;

  ioopm_merch_insert(merch_ht,str_elem(merch->name), void_elem(merch));
  free(undo);
  ioopm_linked_list_remove(undo_list, 0);
}

static void ioopm_undo_pop_merch_edit_name(ioopm_list_t *undo_list, ioopm_hash_table_t *merch_ht, ioopm_hash_table_t *cart_ht)
{
  ioopm_option_t result = ioopm_linked_list_get(undo_list,0);  
  elem_t elem = result.value; 
  undo_t *undo = (undo_t *)elem.voi;
  char *old_name = undo->string_data1;
  char *new_name = undo->string_data2;

  ioopm_option_t merch_result = ioopm_hash_table_lookup(merch_ht, str_elem(new_name));
  elem_t elem_merch = merch_result.value;
  merch_t *merch = (merch_t *)elem_merch.voi;
  free(new_name);
  ioopm_merch_edit_name(merch_ht, cart_ht, merch,strdup(old_name));
  free(old_name);
  free(undo);
  ioopm_linked_list_remove(undo_list, 0);
}

static void ioopm_undo_pop_merch_edit_price(ioopm_list_t *undo_list, ioopm_hash_table_t *merch_ht, ioopm_hash_table_t *cart_ht)
{
  ioopm_option_t result = ioopm_linked_list_get(undo_list,0);  
  elem_t elem = result.value; 
  undo_t *undo = (undo_t *)elem.voi;
  char *name = undo->string_data1;
  int old_price = undo->int_data1;
  
  ioopm_merch_edit_price(merch_ht, cart_ht, str_elem(name),old_price);
  free(undo);
  ioopm_linked_list_remove(undo_list, 0);
}
static void ioopm_undo_pop_merch_edit_descr(ioopm_list_t *undo_list, ioopm_hash_table_t *merch_ht)
{
  ioopm_option_t result = ioopm_linked_list_get(undo_list,0);  
  elem_t elem = result.value; 
  undo_t *undo = (undo_t *)elem.voi;
  char *name = undo->string_data1;
  char *old_descr = undo->string_data2;
    
  ioopm_merch_edit_descr(merch_ht,str_elem(strdup(name)),strdup(old_descr));
  free(name);
  free(old_descr);
  free(undo);
  ioopm_linked_list_remove(undo_list, 0);
}

static void ioopm_undo_pop_merch_stock_increase(ioopm_list_t *undo_list, ioopm_hash_table_t *merch_ht, ioopm_hash_table_t *shelf_ht)
{
  ioopm_option_t result = ioopm_linked_list_get(undo_list,0);  
  elem_t elem = result.value; 
  undo_t *undo = (undo_t *)elem.voi;
  shelf_t *shelf = undo->shelf_data;
  
  ioopm_merch_stock_decrease(merch_ht,shelf_ht, str_elem(undo->string_data1), shelf->shelf_stock,shelf->shelf_name);
  free(undo->string_data1);
  free(undo);
  free(shelf->shelf_name);
  free(shelf);
  ioopm_linked_list_remove(undo_list, 0);
}

static void ioopm_undo_pop_create_cart(ioopm_list_t *undo_list, ioopm_hash_table_t *cart_ht)
{
  ioopm_option_t result = ioopm_linked_list_get(undo_list,0);  
  elem_t elem_undo = result.value; 
  undo_t *undo = (undo_t *)elem_undo.voi;
  elem_t elem_cart = ioopm_hash_table_lookup(cart_ht, int_elem(undo->int_data1)).value;
  ioopm_remove_cart(&elem_cart,cart_ht);
  free(undo);
  ioopm_linked_list_remove(undo_list, 0);
}

static void ioopm_undo_pop_remove_cart(ioopm_list_t *undo_list, ioopm_hash_table_t *cart_ht)
{
  ioopm_option_t result = ioopm_linked_list_get(undo_list,0);  
  elem_t elem_undo = result.value; 
  undo_t *undo = (undo_t *)elem_undo.voi;
  cart_t *cart = undo->cart_data;
  elem_t elem_cart;
  elem_cart.voi = (cart_t *)cart;

  ioopm_hash_table_insert(cart_ht, int_elem(cart->name), elem_cart); 
  free(undo);
  ioopm_linked_list_remove(undo_list, 0);
}

static void ioopm_undo_pop_add_to_cart(ioopm_list_t *undo_list, ioopm_hash_table_t *cart_ht, ioopm_hash_table_t *merch_ht)
{
  ioopm_option_t result = ioopm_linked_list_get(undo_list,0);  
  elem_t elem_undo = result.value; 
  undo_t *undo = (undo_t *)elem_undo.voi;
  elem_t elem_merch = ioopm_hash_table_lookup(merch_ht, str_elem(undo->string_data1)).value;
  elem_t elem_cart = ioopm_hash_table_lookup(cart_ht, int_elem(undo->int_data1)).value;
  int amount_increased = undo->int_data2;
  
  ioopm_remove_from_cart(cart_ht, elem_cart, elem_merch, amount_increased);
  free(undo->string_data1);
  free(undo);
  ioopm_linked_list_remove(undo_list, 0);
}

static void ioopm_undo_pop_remove_from_cart(ioopm_list_t *undo_list, ioopm_hash_table_t *cart_ht, ioopm_hash_table_t *merch_ht)
{
  ioopm_option_t result = ioopm_linked_list_get(undo_list,0);  
  elem_t elem_undo = result.value; 
  undo_t *undo = (undo_t *)elem_undo.voi;
  elem_t elem_merch = ioopm_hash_table_lookup(merch_ht, str_elem(undo->string_data1)).value;
  elem_t elem_cart = ioopm_hash_table_lookup(cart_ht, int_elem(undo->int_data1)).value;
  int amount_decreased = undo->int_data2;
  
  ioopm_add_to_cart(cart_ht, elem_cart, elem_merch, amount_decreased);
  free(undo->string_data1);
  free(undo);
  ioopm_linked_list_remove(undo_list, 0);
}

static void ioopm_undo_pop_checkout(ioopm_list_t *undo_list, ioopm_hash_table_t *cart_ht, ioopm_hash_table_t *merch_ht,ioopm_hash_table_t *shelf_ht)
{
  ioopm_option_t result = ioopm_linked_list_get(undo_list,0);  
  elem_t elem_undo = result.value; 
  undo_t *undo = (undo_t *)elem_undo.voi;
  elem_t elem_cart;
  elem_cart.voi = (cart_t *)undo->cart_data;

  ioopm_undo_pop_checkout_increase(merch_ht, shelf_ht, undo->list_data); //increase according to affected shelves list
  ioopm_hash_table_insert(cart_ht, int_elem(undo->cart_data->name), elem_cart); //returns the cart into cart_ht 
  ioopm_linked_apply_to_all(undo->list_data,ioopm_shelf_delete,NULL);
  ioopm_linked_list_clear(undo->list_data);
  free(undo->list_data);
  free(undo);
  ioopm_linked_list_remove(undo_list, 0);
}

void ioopm_undo_pop(ioopm_list_t *undo_list, ioopm_hash_table_t *merch_ht, ioopm_hash_table_t *shelf_ht, ioopm_hash_table_t *cart_ht)
{
  if (ioopm_linked_list_size(undo_list) == 0) return;
  ioopm_option_t result = ioopm_linked_list_get(undo_list,0);
  elem_t elem = result.value;
  undo_t *undo = elem.voi;
  
  switch (undo->action)
    {
    case 'a':
      ioopm_undo_pop_merch_add(undo_list, merch_ht, shelf_ht);
      return;
      
    case 'b':
      ioopm_undo_pop_merch_remove(undo_list, merch_ht);
      return;
      
    case 'c':
      ioopm_undo_pop_merch_edit_name(undo_list, merch_ht, cart_ht);
      return;
      
    case 'd':
      ioopm_undo_pop_merch_edit_price(undo_list, merch_ht, cart_ht);
      return;
      
    case 'e':
      ioopm_undo_pop_merch_edit_descr(undo_list, merch_ht);
      return;
      
    case 'f':
      ioopm_undo_pop_merch_stock_increase(undo_list, merch_ht, shelf_ht);
      return;
      
    case 'g':
      ioopm_undo_pop_create_cart(undo_list, cart_ht);
      return;
      
    case 'h':
      ioopm_undo_pop_remove_cart(undo_list, cart_ht);
      return;
      
    case 'i':
      ioopm_undo_pop_add_to_cart(undo_list, cart_ht, merch_ht);
      return;
      
    case 'j':
      ioopm_undo_pop_remove_from_cart(undo_list, cart_ht, merch_ht);
      return;
      
    case 'k':
      ioopm_undo_pop_checkout(undo_list, cart_ht, merch_ht, shelf_ht);
      return;
    }
}

void ioopm_destroy_all_hashtables(ioopm_hash_table_t *merch_ht, ioopm_hash_table_t *shelf_ht, ioopm_hash_table_t *cart_ht)
{
  if (!(ioopm_hash_table_is_empty(merch_ht)))
    {
      ioopm_list_t *merch_list = ioopm_hash_table_values(merch_ht);
      ioopm_list_iterator_t *iter = ioopm_list_iterator_create(merch_list);
      for (int i = 0; i < ioopm_linked_list_size(merch_list); i++)
	{
	  elem_t elem_merch = ioopm_iterator_current(iter).value; 
	  merch_t *merch = (merch_t *)elem_merch.voi;
	  ioopm_merch_remove(merch_ht, shelf_ht, str_elem(merch->name));
	  ioopm_iterator_next(iter);
	}
      Free(iter);
      ioopm_linked_list_clear(merch_list);
      free(merch_list);    
    }
  if (!(ioopm_hash_table_is_empty(cart_ht)))
    {
      ioopm_list_t *cart_list = ioopm_hash_table_values(cart_ht);
      ioopm_linked_apply_to_all(cart_list, ioopm_remove_cart, cart_ht);
      ioopm_linked_list_clear(cart_list);    
      free(cart_list);    
    }
  ioopm_hash_table_destroy(merch_ht);
  ioopm_hash_table_destroy(shelf_ht);
  ioopm_hash_table_destroy(cart_ht);
}
