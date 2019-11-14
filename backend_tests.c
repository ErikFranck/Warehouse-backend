#include <string.h>
#include <stdbool.h>
#include <CUnit/Basic.h>
#include "backend.h"
#include <stdlib.h>
#include <ctype.h>

#define int_elem(i) (elem_t) { .in =(i) }
#define str_elem(s) (elem_t) { .str =(s) }
#define value_name(elem) ((char *) ((merch_t *)elem.voi)->name)
#define value_descr(elem) ((char *) ((merch_t *)elem.voi)->descr)
#define value_price(elem) ((int) ((merch_t *)elem.voi)->price)

#define shelf_merch_name(elem) ((char *) ((shelf_t *)elem.voi)->merch_name)
#define shelf_merch_stock(elem) ((int) ((shelf_t *)elem.voi)->merch_stock)

typedef struct merch merch_t;
typedef struct shelf shelf_t;
typedef struct undo undo_t;
typedef struct cart cart_t;
typedef struct merch_in_cart merch_in_cart_t;

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

//-------------------------------------------------------------
int init_suite(void)
{
  return 0;
}

int clean_suite(void)
{
  return 0;
}

//-------------------------------------------------------------

static void ioopm_merch_delete(elem_t elem_merch)
{
  merch_t *merch = (merch_t *)elem_merch.voi;
  ioopm_linked_list_destroy(merch->shelves);
  free(merch);

  return;
}
//-------------------------------------------------------------

//test if create
void merch_create_test1(void)
{
  elem_t elem_merch = ioopm_merch_create("Erik", "dv", 5);
CU_ASSERT_STRING_EQUAL(value_name(elem_merch),"Erik");
CU_ASSERT_STRING_EQUAL(value_descr(elem_merch), "dv");
CU_ASSERT_EQUAL(value_price(elem_merch), 5);
ioopm_merch_delete(elem_merch);
}


//test if insert thinks it inserts an item
void merch_ht_insert_test1(void)
{
  ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
  elem_t elem_merch = ioopm_merch_create("Erik", "dv", 5);
  
  CU_ASSERT_TRUE(ioopm_merch_insert(merch_ht, str_elem("Erik"), elem_merch));
  
  ioopm_merch_delete(elem_merch);
  ioopm_hash_table_destroy(merch_ht);
}


//test if a merch is inserted correctly on empty list
void merch_ht_insert_test2(void)
{
      ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
    elem_t elem_merch = ioopm_merch_create("Erik", "dv", 5);
    ioopm_merch_insert(merch_ht, str_elem("Erik"), elem_merch);

    CU_ASSERT_STRING_EQUAL(value_name((ioopm_hash_table_lookup(merch_ht, str_elem("Erik"))).value), "Erik");
    CU_ASSERT_STRING_EQUAL(value_descr((ioopm_hash_table_lookup(merch_ht, str_elem("Erik"))).value), "dv");
    CU_ASSERT_EQUAL(value_price((ioopm_hash_table_lookup(merch_ht, str_elem("Erik"))).value), 5);
    ioopm_merch_delete(elem_merch);
    ioopm_hash_table_destroy(merch_ht);
}

//check if values are inserted correctly on list with 1 item
void merch_ht_insert_test3(void)
  {
      ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
    elem_t elem_merch1 = ioopm_merch_create("Erik", "dv", 5);
    elem_t elem_key1 = { .str = "Erik"};

    elem_t elem_merch2 = ioopm_merch_create("Missan", "it", 2);
    elem_t elem_key2 = { .str = "Missan"};

    ioopm_merch_insert(merch_ht, elem_key1, elem_merch1);
    ioopm_merch_insert(merch_ht, elem_key2, elem_merch2);
    
    //check if value name is equal
    CU_ASSERT_STRING_EQUAL(value_name((ioopm_hash_table_lookup(merch_ht, elem_key2)).value), "Missan");
    //check if value description is equal
    CU_ASSERT_STRING_EQUAL(value_descr((ioopm_hash_table_lookup(merch_ht, elem_key2)).value), "it");
    //check if value price is equal
    CU_ASSERT_EQUAL(value_price((ioopm_hash_table_lookup(merch_ht, elem_key2)).value), 2);
    ioopm_merch_delete(elem_merch1);
    ioopm_merch_delete(elem_merch2);
    ioopm_hash_table_destroy(merch_ht);
  }

//try to insert key and value name with different values
void merch_ht_insert_test4(void)
{
    ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
  elem_t elem_merch = ioopm_merch_create("Erik", "bra", 5);
  elem_t elem_key = { .str = "Missan"};

  
  CU_ASSERT_FALSE(ioopm_merch_insert(merch_ht, elem_key, elem_merch));
  ioopm_merch_delete(elem_merch);
  ioopm_hash_table_destroy(merch_ht);
}

//check if values are inserted correctly on list with 1 item
void merch_add_test1(void)
  {
   ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
  ioopm_hash_table_t *shelf_ht = ioopm_create_shelf_ht();
   ioopm_list_t *undo_list = undo_list_create();   

 ioopm_undoable_merch_add(undo_list, merch_ht, strdup("Erik"), strdup("dv"), 5);
 ioopm_undoable_merch_add(undo_list, merch_ht, strdup("Missan"), strdup("it"), 2);
        
    //check if value name is equal
    CU_ASSERT_STRING_EQUAL(value_name((ioopm_hash_table_lookup(merch_ht, str_elem("Missan"))).value), "Missan");
    //check if value description is equal
    CU_ASSERT_STRING_EQUAL(value_descr((ioopm_hash_table_lookup(merch_ht, str_elem("Missan"))).value), "it");
    //check if value price is equal
    CU_ASSERT_EQUAL(value_price((ioopm_hash_table_lookup(merch_ht, str_elem("Missan"))).value), 2);

    ioopm_merch_undoable_remove(undo_list,merch_ht, shelf_ht, "Erik");
    ioopm_merch_undoable_remove(undo_list,merch_ht, shelf_ht, "Missan");
    
    //ioopm_merch_remove(merch_ht, shelf_ht, str_elem("Missan"));
    //ioopm_merch_remove(merch_ht, shelf_ht, str_elem("Erik"));

    ioopm_undo_list_delete(undo_list);
    //    ioopm_undo_pop_merch_add(undo_list, merch_ht, shelf_ht);
    // ioopm_undo_pop_merch_add(undo_list, merch_ht, shelf_ht);
    //
    //ioopm_linked_list_destroy(undo_list);
    ioopm_hash_table_destroy(merch_ht);
    ioopm_hash_table_destroy(shelf_ht);
  }

//check if you can insert a merch that is already added
void merch_add_test2(void)
  {
   ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
   ioopm_hash_table_t *shelf_ht = ioopm_create_shelf_ht();
   ioopm_list_t *undo_list = undo_list_create();   

 ioopm_undoable_merch_add(undo_list, merch_ht, strdup("Erik"), strdup("dv"), 5);
 ioopm_undoable_merch_add(undo_list, merch_ht, strdup("Erik"), strdup("it"), 2);
        
    //check if value name is not changed
    CU_ASSERT_STRING_EQUAL(value_name((ioopm_hash_table_lookup(merch_ht, str_elem("Erik"))).value), "Erik");
    //check if value description is not changed
    CU_ASSERT_STRING_EQUAL(value_descr((ioopm_hash_table_lookup(merch_ht, str_elem("Erik"))).value), "dv");
    //check if value price is not changed
    CU_ASSERT_EQUAL(value_price((ioopm_hash_table_lookup(merch_ht, str_elem("Erik"))).value), 5);

    ioopm_merch_remove(merch_ht, shelf_ht, str_elem("Erik"));
    ioopm_undo_list_delete(undo_list);
    ioopm_hash_table_destroy(merch_ht);
    ioopm_hash_table_destroy(shelf_ht);
  }

//test if remove removes merch from merch_ht
void merch_remove_test1(void)
{
  ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
  ioopm_hash_table_t *shelf_ht = ioopm_create_shelf_ht();
   ioopm_list_t *undo_list = undo_list_create();
 
 elem_t elem_merch = ioopm_merch_create(strdup("Erik"), strdup("dv"), 5);
 ioopm_merch_insert(merch_ht, str_elem("Erik"), elem_merch);

 ioopm_merch_remove( merch_ht, shelf_ht, str_elem("Erik"));

 CU_ASSERT_FALSE((ioopm_hash_table_lookup(merch_ht, str_elem("Erik")).success));

 ioopm_hash_table_destroy(merch_ht);
 ioopm_hash_table_destroy(shelf_ht);
 ioopm_undo_list_delete(undo_list);
}
 
//check if remove also removes all stock
void merch_remove_test2(void)
{
    ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
   ioopm_hash_table_t *shelf_ht = ioopm_create_shelf_ht();
    ioopm_list_t *undo_list = undo_list_create();
  
  elem_t elem_merch = ioopm_merch_create(strdup("Erik"), strdup("dv"), 5);
  
  ioopm_merch_insert(merch_ht, str_elem("Erik"), elem_merch);
  ioopm_merch_stock_increase(merch_ht, shelf_ht, str_elem(strdup("Erik")), str_elem(strdup("K3")), 5763);
  ioopm_merch_stock_increase(merch_ht, shelf_ht, str_elem(strdup("Erik")), str_elem(strdup("B57")), 63);
  ioopm_merch_stock_increase(merch_ht, shelf_ht, str_elem(strdup("Erik")), str_elem(strdup("O21")), 53);
  ioopm_merch_stock_increase(merch_ht, shelf_ht, str_elem(strdup("Erik")), str_elem(strdup("A1")), 53);

  
  ioopm_merch_remove( merch_ht, shelf_ht, str_elem("Erik"));
  ioopm_merch_show_stock(merch_ht, str_elem("Erik"));
  
  CU_ASSERT_FALSE((ioopm_hash_table_lookup(shelf_ht, str_elem("K3")).success));
  CU_ASSERT_FALSE((ioopm_hash_table_lookup(shelf_ht, str_elem("B57")).success));
  CU_ASSERT_FALSE((ioopm_hash_table_lookup(shelf_ht, str_elem("O21")).success));
  CU_ASSERT_FALSE((ioopm_hash_table_lookup(shelf_ht, str_elem("A1")).success));
  
    ioopm_hash_table_destroy(merch_ht);
  ioopm_hash_table_destroy(shelf_ht);
  ioopm_undo_list_delete(undo_list);
}



//test if it goes through a non empty hash table
void merch_print_test1(void)
{
    ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
   ioopm_hash_table_t *shelf_ht = ioopm_create_shelf_ht();
  elem_t elem_merch1 = ioopm_merch_create(strdup("Erik"), strdup("dv"), 5);
  elem_t elem_merch2 = ioopm_merch_create(strdup("Missan"), strdup("it"), 5);


  /*  char erik[1]; */
  /* for (int i = 17 ; i<75; i++) */
  /*   { */
  /*     erik[0] = i+'0'; */
  /*     elem_t elem_merch = ioopm_merch_create(erik, "dv", 5); */
  /*     ioopm_merch_insert(merch_ht, str_elem(erik), elem_merch); */
  /*     printf("%s\n",erik); */
  /*   } */
  
  ioopm_merch_insert(merch_ht, str_elem("Erik"), elem_merch1);
  ioopm_merch_insert(merch_ht, str_elem("Missan"), elem_merch2);
  
  CU_ASSERT_TRUE((ioopm_hash_table_lookup(merch_ht, str_elem("Erik")).success));
  
   ioopm_merch_remove( merch_ht, shelf_ht, str_elem("Erik"));
   ioopm_merch_remove( merch_ht, shelf_ht, str_elem("Missan"));
   
   ioopm_hash_table_destroy(merch_ht);
   ioopm_hash_table_destroy(shelf_ht);
}

//test if it works with empty hash table
void merch_print_test2(void)
{
    ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
  //ioopm_option_t result = ioopm_print_merch(merch_ht);
  //CU_ASSERT_FALSE(result.success);
  ioopm_hash_table_destroy(merch_ht);
}


//test if description is changed
void merch_edit_descr_test1(void)
{
    ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
   ioopm_hash_table_t *shelf_ht = ioopm_create_shelf_ht();
  elem_t elem_merch = ioopm_merch_create(strdup("Erik"), strdup("dv"), 5);
  ioopm_merch_insert(merch_ht, str_elem("Erik"), elem_merch);

  ioopm_merch_edit_descr(merch_ht, str_elem(strdup("Erik")), strdup("dvd"));
  CU_ASSERT_STRING_EQUAL(value_descr((ioopm_hash_table_lookup(merch_ht, str_elem("Erik"))).value), "dvd");

  ioopm_merch_remove( merch_ht,shelf_ht, str_elem("Erik"));
  ioopm_hash_table_destroy(merch_ht);
  ioopm_hash_table_destroy(shelf_ht);
}

//
void merch_edit_price_test1(void)
{
    ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
   ioopm_hash_table_t *shelf_ht = ioopm_create_shelf_ht();
    ioopm_hash_table_t *cart_ht = ioopm_create_cart_ht();
    ioopm_list_t *undo_list = undo_list_create();   

  elem_t elem_merch = ioopm_merch_create(strdup("Erik"), strdup("dv"), 5);
  ioopm_merch_insert(merch_ht, str_elem("Erik"), elem_merch);

  ioopm_undoable_edit_price(undo_list, merch_ht, cart_ht, "Erik" ,3);
  ioopm_undo_pop(undo_list, merch_ht, shelf_ht, cart_ht);
  //check if value price has been changed
  CU_ASSERT_EQUAL(value_price((ioopm_hash_table_lookup(merch_ht, str_elem("Erik"))).value), 5);

  ioopm_merch_remove( merch_ht,shelf_ht, str_elem("Erik"));
  
  ioopm_hash_table_destroy(merch_ht);
  ioopm_hash_table_destroy(shelf_ht);
  ioopm_hash_table_destroy(cart_ht);
  ioopm_undo_list_delete(undo_list);
}


void merch_edit_name_test1(void)
{
    ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
   ioopm_hash_table_t *shelf_ht = ioopm_create_shelf_ht();
    ioopm_hash_table_t *cart_ht = ioopm_create_cart_ht();
    ioopm_list_t *undo_list = undo_list_create();   

  elem_t elem_merch = ioopm_merch_create(strdup("Erik"), strdup("dv"), 5);  
  ioopm_merch_insert(merch_ht, str_elem("Erik"), elem_merch);
  
  ioopm_undoable_edit_name(undo_list, merch_ht,cart_ht, "Erik", "Missan");

  CU_ASSERT_STRING_EQUAL(value_name((ioopm_hash_table_lookup(merch_ht, str_elem("Missan"))).value), "Missan");

  ioopm_merch_remove( merch_ht,shelf_ht, str_elem("Missan"));
  ioopm_hash_table_destroy(merch_ht);
  ioopm_hash_table_destroy(shelf_ht);
  ioopm_hash_table_destroy(cart_ht);
  ioopm_undo_list_delete(undo_list);

}

//test if its possible to do several times
void merch_edit_name_test2(void)
{
    ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
    ioopm_hash_table_t *shelf_ht = ioopm_create_shelf_ht();
    ioopm_hash_table_t *cart_ht = ioopm_create_cart_ht();
    ioopm_list_t *undo_list = undo_list_create();   
    elem_t elem_merch = ioopm_merch_create(strdup("Erik"), strdup("dv"), 5);
    merch_t *merch = (merch_t *)elem_merch.voi;
   
  ioopm_merch_insert(merch_ht, str_elem("Erik"), elem_merch);

  ioopm_undoable_edit_name(undo_list,merch_ht,cart_ht, "Erik", "Korv");
    
  CU_ASSERT_STRING_EQUAL(value_name((ioopm_hash_table_lookup(merch_ht, str_elem("Korv"))).value), "Korv");

  ioopm_merch_remove( merch_ht,shelf_ht, str_elem("Korv"));
  ioopm_hash_table_destroy(merch_ht);
  ioopm_hash_table_destroy(shelf_ht);
  ioopm_hash_table_destroy(cart_ht);
  ioopm_undo_list_delete(undo_list);
}


void merch_edit_name_undo_test1(void)
{
    ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
   ioopm_hash_table_t *shelf_ht = ioopm_create_shelf_ht();
    ioopm_hash_table_t *cart_ht = ioopm_create_cart_ht();
    ioopm_list_t *undo_list = undo_list_create();   
  elem_t elem_merch = ioopm_merch_create(strdup("Erik"), strdup("dv"), 5);
  ioopm_merch_insert(merch_ht, str_elem("Erik"), elem_merch);

  
  ioopm_undoable_edit_name(undo_list,merch_ht,cart_ht, "Erik", "Missan");
  ioopm_undo_pop(undo_list, merch_ht, shelf_ht, cart_ht);
    
  CU_ASSERT_STRING_EQUAL(value_name((ioopm_hash_table_lookup(merch_ht, str_elem("Erik"))).value), "Erik");

  ioopm_merch_remove( merch_ht,shelf_ht, str_elem("Erik"));
  ioopm_hash_table_destroy(merch_ht);
  ioopm_hash_table_destroy(shelf_ht);
   ioopm_hash_table_destroy(cart_ht);
  ioopm_undo_list_delete(undo_list);

}
void merch_stock_increase_test1(void)
{
    ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
   ioopm_hash_table_t *shelf_ht = ioopm_create_shelf_ht();
    ioopm_list_t *undo_list = undo_list_create();   
  elem_t elem_merch = ioopm_merch_create(strdup("Erik"), strdup("dv"), 5);
  ioopm_merch_insert(merch_ht, str_elem("Erik"), elem_merch);
  
  ioopm_merch_stock_increase(merch_ht, shelf_ht, str_elem(strdup("Erik")), str_elem(strdup("A25")), 4);
  ioopm_merch_stock_increase(merch_ht, shelf_ht, str_elem(strdup("Erik")), str_elem(strdup("A25")), 4);
  ioopm_merch_stock_increase(merch_ht, shelf_ht, str_elem(strdup("Erik")), str_elem(strdup("B3")), 4);
  

  ioopm_option_t merch_ht_result = ioopm_hash_table_lookup(merch_ht, str_elem("Erik"));
  elem_t elem_merch_after = merch_ht_result.value;
  merch_t *merch = (merch_t *)elem_merch_after.voi;

  ioopm_option_t shelf_ht_result1 = ioopm_hash_table_lookup(shelf_ht, str_elem("A25"));
  elem_t elem_shelf1 = shelf_ht_result1.value;
  shelf_t *shelf1 = (shelf_t *)elem_shelf1.voi;

  ioopm_option_t shelf_ht_result2 = ioopm_hash_table_lookup(shelf_ht, str_elem("B3"));
  elem_t elem_shelf2 = shelf_ht_result2.value;
  shelf_t *shelf2 = (shelf_t *)elem_shelf2.voi;

  
  CU_ASSERT_EQUAL(shelf1->shelf_stock, 8);
  CU_ASSERT_EQUAL(shelf2->shelf_stock, 4);
  CU_ASSERT_EQUAL(merch->total_stock, 12);

  ioopm_merch_remove( merch_ht,shelf_ht,  str_elem("Erik"));
  ioopm_hash_table_destroy(merch_ht);
  ioopm_hash_table_destroy(shelf_ht);
  ioopm_undo_list_delete(undo_list);
}

//test if it can remove a large number of stock from several shelves
//amount decreased cannot be above total stock
void merch_stock_decrease_test1(void)
{
    ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
   ioopm_hash_table_t *shelf_ht = ioopm_create_shelf_ht();
    ioopm_list_t *undo_list = undo_list_create();   
  elem_t elem_merch = ioopm_merch_create(strdup("Erik"), strdup("dv"), 5);
  ioopm_merch_insert(merch_ht, str_elem("Erik"), elem_merch);

  ioopm_merch_stock_increase(merch_ht, shelf_ht, str_elem(strdup("Erik")), str_elem(strdup("A25")), 5);
  ioopm_merch_stock_increase(merch_ht, shelf_ht, str_elem(strdup("Erik")), str_elem(strdup("C17")), 7);
  ioopm_merch_stock_increase(merch_ht, shelf_ht, str_elem(strdup("Erik")), str_elem(strdup("B3")), 4);

  ioopm_merch_stock_decrease(merch_ht, shelf_ht, str_elem("Erik"), 15, NULL);
      
  ioopm_option_t merch_ht_result = ioopm_hash_table_lookup(merch_ht, str_elem("Erik"));
  elem_t elem_merch_after = merch_ht_result.value;
  merch_t *merch = (merch_t *)elem_merch_after.voi;

  ioopm_option_t shelf_ht_result1 = ioopm_hash_table_lookup(shelf_ht, str_elem("A25"));
  CU_ASSERT_FALSE(shelf_ht_result1.success);
  CU_ASSERT_EQUAL(merch->total_stock, 1);

  ioopm_merch_remove( merch_ht,shelf_ht,  str_elem("Erik"));
  ioopm_hash_table_destroy(merch_ht);
  ioopm_hash_table_destroy(shelf_ht);
  ioopm_undo_list_delete(undo_list);
}

//test if
//amount decreased cannot be above total stock
void merch_stock_decrease_test2(void)
{

    ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
   ioopm_hash_table_t *shelf_ht = ioopm_create_shelf_ht();
    ioopm_list_t *undo_list = undo_list_create();
  
  elem_t elem_merch1 = ioopm_merch_create(strdup("Erik"), strdup("dv"), 5);
  ioopm_merch_insert(merch_ht, str_elem("Erik"), elem_merch1);

  elem_t elem_merch2 = ioopm_merch_create(strdup("Stol"), strdup("gjord av trä"), 220);
  ioopm_merch_insert(merch_ht, str_elem("Stol"), elem_merch2);
  
  ioopm_merch_stock_increase(merch_ht, shelf_ht, str_elem(strdup("Erik")), str_elem(strdup("B3")), 4);
  ioopm_merch_stock_increase(merch_ht, shelf_ht, str_elem(strdup("Erik")), str_elem(strdup("A25")), 5);
  ioopm_merch_stock_increase(merch_ht, shelf_ht, str_elem(strdup("Stol")), str_elem(strdup("K18")), 7);
  ioopm_merch_stock_increase(merch_ht, shelf_ht, str_elem(strdup("Stol")), str_elem(strdup("C7")), 3);
    
  ioopm_merch_stock_decrease(merch_ht, shelf_ht, str_elem("Erik"), 7,NULL);
  ioopm_merch_stock_decrease(merch_ht, shelf_ht, str_elem("Erik"), 1,NULL);
  ioopm_merch_stock_decrease(merch_ht, shelf_ht, str_elem("Stol"), 8,NULL);
      
  ioopm_option_t merch_ht_result = ioopm_hash_table_lookup(merch_ht, str_elem("Stol"));
  elem_t elem_merch_after = merch_ht_result.value;
  merch_t *merch = (merch_t *)elem_merch_after.voi;
  CU_ASSERT_EQUAL(merch->total_stock, 2);//check that total stock of Stol is 2
  
  ioopm_option_t shelf_ht_result1 = ioopm_hash_table_lookup(shelf_ht, str_elem("K18"));
  CU_ASSERT_FALSE(shelf_ht_result1.success);//check that K18 is empty

  ioopm_option_t shelf_ht_result2 = ioopm_hash_table_lookup(shelf_ht, str_elem("A25"));
  elem_t elem_shelf2 = shelf_ht_result2.value;
  shelf_t *shelf2 = (shelf_t *)elem_shelf2.voi;
  CU_ASSERT_EQUAL(shelf2->shelf_stock, 1);//check that A25 has 1 items
  
  ioopm_option_t shelf_ht_result3 = ioopm_hash_table_lookup(shelf_ht, str_elem("C7"));
    elem_t elem_shelf3 = shelf_ht_result3.value;
  shelf_t *shelf3 = (shelf_t *)elem_shelf3.voi;
  CU_ASSERT_EQUAL(shelf3->shelf_stock, 2); //check that C7 has 2 item
    
  ioopm_merch_remove( merch_ht,shelf_ht, str_elem("Erik"));
  ioopm_merch_remove( merch_ht,shelf_ht, str_elem("Stol"));
  ioopm_hash_table_destroy(merch_ht);
  ioopm_hash_table_destroy(shelf_ht);
  ioopm_undo_list_delete(undo_list);
}

//test if you can place a merch on a shelf that is taken by other merch.
//amount decreased cannot be above total stock
void merch_stock_decrease_test3(void)
{
    ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
   ioopm_hash_table_t *shelf_ht = ioopm_create_shelf_ht();
    ioopm_list_t *undo_list = undo_list_create();

  elem_t elem_merch1 = ioopm_merch_create(strdup("Erik"), strdup("dv"), 5);
  ioopm_merch_insert(merch_ht, str_elem("Erik"), elem_merch1);

  elem_t elem_merch2 = ioopm_merch_create(strdup("Stol"), strdup("gjord av trä"), 220);
  ioopm_merch_insert(merch_ht, str_elem("Stol"), elem_merch2);

    ioopm_merch_stock_increase(merch_ht, shelf_ht, str_elem(strdup("Erik")), str_elem(strdup("B3")), 4);
    ioopm_merch_stock_increase(merch_ht, shelf_ht, str_elem(strdup("Stol")), str_elem(strdup("B3")), 3);
          
  ioopm_option_t merch_ht_result = ioopm_hash_table_lookup(merch_ht, str_elem("Erik"));
  elem_t elem_merch_after = merch_ht_result.value;
  merch_t *merch = (merch_t *)elem_merch_after.voi;
  CU_ASSERT_EQUAL(merch->total_stock, 4);//check that total stock of Erik is 4
  
  ioopm_option_t shelf_ht_result = ioopm_hash_table_lookup(shelf_ht, str_elem("B3"));
  elem_t elem_shelf = shelf_ht_result.value;
  shelf_t *shelf = (shelf_t *)elem_shelf.voi;

  CU_ASSERT_STRING_EQUAL(shelf->merch_name, "Erik"); //check that B3 has Erik merch
  CU_ASSERT_EQUAL(shelf->shelf_stock, 4);//check that B3 has 4 stock of Erik
  
  ioopm_merch_remove( merch_ht,shelf_ht, str_elem("Erik"));
  ioopm_merch_remove( merch_ht,shelf_ht, str_elem("Stol"));

  ioopm_hash_table_destroy(merch_ht);
  ioopm_hash_table_destroy(shelf_ht);
  ioopm_undo_list_delete(undo_list);
}
  
//test if show merch stock returns true
void merch_show_stock_test1(void)
{
    ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
   ioopm_hash_table_t *shelf_ht = ioopm_create_shelf_ht();
    ioopm_list_t *undo_list = undo_list_create();     
  elem_t elem_merch = ioopm_merch_create(strdup("Erik"), strdup("dv"), 5);
  ioopm_merch_insert(merch_ht, str_elem("Erik"), elem_merch);
   
  ioopm_merch_stock_increase(merch_ht, shelf_ht, str_elem(strdup("Erik")), str_elem(strdup("K3")), 76);
    
  CU_ASSERT_TRUE(ioopm_merch_show_stock(merch_ht, str_elem("Erik")));

   ioopm_merch_remove( merch_ht, shelf_ht, str_elem("Erik"));
  ioopm_hash_table_destroy(merch_ht);
  ioopm_hash_table_destroy(shelf_ht);
  ioopm_undo_list_delete(undo_list);
}

/* void create_cart_test1(void) */
/* { */
/*   elem_t elem_cart = ioopm_create_cart(1); */
/*   cart_t *cart = (cart_t *)elem_cart.voi; */
/*   CU_ASSERT_EQUAL(cart->total_cost, 0); */
/*   CU_ASSERT_TRUE(ioopm_linked_list_is_empty(cart->wares)); */

/*   ioopm_remove_cart(elem_cart, cart_ht); */
/* } */


void total_stock_of_a_merch_in_all_carts_test1(void)
{
    ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
   ioopm_hash_table_t *shelf_ht = ioopm_create_shelf_ht();
    ioopm_hash_table_t *cart_ht = ioopm_create_cart_ht();
    ioopm_list_t *undo_list = undo_list_create();   

  elem_t elem_cart1 = ioopm_create_cart(1);
  cart_t *cart1 = (cart_t *)elem_cart1.voi;

  elem_t elem_cart2 = ioopm_create_cart(2);
  cart_t *cart2 = (cart_t *)elem_cart2.voi;
  
  elem_t elem_merch = ioopm_merch_create(strdup("Erik"), strdup("dv"), 5);
  ioopm_merch_insert(merch_ht, str_elem("Erik"), elem_merch);

  elem_t elem_merch2 = ioopm_merch_create(strdup("Missan"), strdup("katt"), 3);
  ioopm_merch_insert(merch_ht, str_elem("Missan"), elem_merch2);
  
  ioopm_hash_table_insert(cart_ht, int_elem(cart1->name), elem_cart1);
  ioopm_hash_table_insert(cart_ht, int_elem(cart2->name), elem_cart2);

  ioopm_merch_stock_increase(merch_ht, shelf_ht, str_elem(strdup("Erik")), str_elem(strdup("A25")), 5);
  ioopm_merch_stock_increase(merch_ht, shelf_ht, str_elem(strdup("Missan")), str_elem(strdup("R2")), 7);
  
  ioopm_add_to_cart(cart_ht, elem_cart1, elem_merch2, 3);
  ioopm_add_to_cart(cart_ht, elem_cart1, elem_merch, 2);
  ioopm_add_to_cart(cart_ht, elem_cart1, elem_merch2, 3);
  
  
  CU_ASSERT_EQUAL(ioopm_total_stock_of_a_merch_in_all_carts(cart_ht, "Erik"), 2); //check that the total amount in all carts of Erik is 2
  CU_ASSERT_EQUAL(ioopm_total_stock_of_a_merch_in_all_carts(cart_ht, "Missan"), 6); //check that the total amount in all carts of Missan is 6
  ioopm_remove_cart(&elem_cart1,cart_ht);
  ioopm_remove_cart(&elem_cart2,cart_ht);
  ioopm_merch_remove( merch_ht,shelf_ht, str_elem("Erik"));
  ioopm_merch_remove( merch_ht,shelf_ht, str_elem("Missan"));
    
  ioopm_hash_table_destroy(merch_ht);
  ioopm_hash_table_destroy(shelf_ht);
  ioopm_hash_table_destroy(cart_ht);
  ioopm_undo_list_delete(undo_list);
 }


  void add_to_cart_test1(void)
{
    ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
   ioopm_hash_table_t *shelf_ht = ioopm_create_shelf_ht();
    ioopm_hash_table_t *cart_ht = ioopm_create_cart_ht();
    ioopm_list_t *undo_list = undo_list_create();   
  elem_t elem_cart = ioopm_create_cart(1);
  elem_t elem_merch = ioopm_merch_create(strdup("Erik"), strdup("dv"), 5);
  cart_t *cart = (cart_t *)elem_cart.voi;

  ioopm_merch_insert(merch_ht, str_elem("Erik"), elem_merch);
  ioopm_hash_table_insert(cart_ht, int_elem(cart->name), elem_cart);

  ioopm_merch_stock_increase(merch_ht, shelf_ht, str_elem(strdup("Erik")), str_elem(strdup("A25")), 5);
  ioopm_add_to_cart(cart_ht, elem_cart, elem_merch, 3);

  ioopm_option_t result =  ioopm_linked_list_get(cart->wares,0);
  elem_t elem_merch_in_cart = result.value;
  merch_in_cart_t *merch_in_cart = (merch_in_cart_t *)elem_merch_in_cart.voi;

  CU_ASSERT_STRING_EQUAL(merch_in_cart->name, value_name(elem_merch));
  CU_ASSERT_FALSE(ioopm_linked_list_is_empty(cart->wares));
  CU_ASSERT_EQUAL(merch_in_cart->amount, 3);
  CU_ASSERT_EQUAL(cart->total_cost, 15);
  
  ioopm_remove_cart(&elem_cart, cart_ht);
  ioopm_merch_remove( merch_ht,shelf_ht, str_elem("Erik"));
  
  ioopm_hash_table_destroy(merch_ht);
  ioopm_hash_table_destroy(shelf_ht);
  ioopm_hash_table_destroy(cart_ht);
  ioopm_undo_list_delete(undo_list);
 }

  void add_to_cart_test2(void)
{
    ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
   ioopm_hash_table_t *shelf_ht = ioopm_create_shelf_ht();
    ioopm_hash_table_t *cart_ht = ioopm_create_cart_ht();
    ioopm_list_t *undo_list = undo_list_create();   
  elem_t elem_cart = ioopm_create_cart(1);
  elem_t elem_merch = ioopm_merch_create(strdup("Erik"), strdup("dv"), 5);
  cart_t *cart = (cart_t *)elem_cart.voi;

  ioopm_merch_insert(merch_ht, str_elem("Erik"), elem_merch);
  ioopm_hash_table_insert(cart_ht, int_elem(cart->name), elem_cart);

  ioopm_merch_stock_increase(merch_ht, shelf_ht, str_elem(strdup("Erik")), str_elem(strdup("A25")), 5);

  ioopm_add_to_cart(cart_ht, elem_cart, elem_merch, 3);

  ioopm_option_t result =  ioopm_linked_list_get(cart->wares,0);
  elem_t elem_merch_in_cart = result.value;
  merch_in_cart_t *merch_in_cart = (merch_in_cart_t *)elem_merch_in_cart.voi;

  CU_ASSERT_STRING_EQUAL(merch_in_cart->name, value_name(elem_merch));
  CU_ASSERT_FALSE(ioopm_linked_list_is_empty(cart->wares));
  CU_ASSERT_EQUAL(merch_in_cart->amount, 3);
  
  ioopm_remove_cart(&elem_cart, cart_ht);
  ioopm_merch_remove( merch_ht,shelf_ht, str_elem("Erik"));
  
  ioopm_hash_table_destroy(merch_ht);
  ioopm_hash_table_destroy(shelf_ht);
  ioopm_hash_table_destroy(cart_ht);
  ioopm_undo_list_delete(undo_list);
 }


void remove_from_cart_test1(void)
{
    ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
   ioopm_hash_table_t *shelf_ht = ioopm_create_shelf_ht();
    ioopm_hash_table_t *cart_ht = ioopm_create_cart_ht();
    ioopm_list_t *undo_list = undo_list_create();   
  elem_t elem_cart1 = ioopm_create_cart(1);
  cart_t *cart1 = (cart_t *)elem_cart1.voi;

  elem_t elem_cart2 = ioopm_create_cart(2);
  cart_t *cart2 = (cart_t *)elem_cart2.voi;
  
  elem_t elem_merch = ioopm_merch_create(strdup("Erik"), strdup("dv"), 5);
  ioopm_merch_insert(merch_ht, str_elem("Erik"), elem_merch);

  elem_t elem_merch2 = ioopm_merch_create(strdup("Missan"), strdup("katt"), 3);
  ioopm_merch_insert(merch_ht, str_elem("Missan"), elem_merch2);
  
  ioopm_hash_table_insert(cart_ht, int_elem(cart1->name), elem_cart1);
  ioopm_hash_table_insert(cart_ht, int_elem(cart2->name), elem_cart2);

  ioopm_merch_stock_increase(merch_ht, shelf_ht, str_elem(strdup("Erik")), str_elem(strdup("A25")), 5);
  ioopm_merch_stock_increase(merch_ht, shelf_ht, str_elem(strdup("Missan")), str_elem(strdup("R2")), 7);
  
  ioopm_add_to_cart(cart_ht, elem_cart1, elem_merch2, 3);
  ioopm_add_to_cart(cart_ht, elem_cart1, elem_merch, 4);
  ioopm_add_to_cart(cart_ht, elem_cart1, elem_merch2, 3);

  ioopm_remove_from_cart(cart_ht, elem_cart1, elem_merch, 3);
  ioopm_remove_from_cart(cart_ht, elem_cart1, elem_merch2, 4);
    
  CU_ASSERT_EQUAL(ioopm_total_stock_of_a_merch_in_all_carts(cart_ht, "Erik"), 1); //check that the total amount in all carts of Erik is 1
   CU_ASSERT_EQUAL(ioopm_total_stock_of_a_merch_in_all_carts(cart_ht, "Missan"), 2); //check that the total amount in all carts of Missan is 2
  CU_ASSERT_EQUAL(cart1->total_cost, 11); //check so that the total price is correct
  
  ioopm_remove_cart(&elem_cart1, cart_ht);
  ioopm_remove_cart(&elem_cart2, cart_ht);
  ioopm_merch_remove( merch_ht,shelf_ht, str_elem("Erik"));
  ioopm_merch_remove( merch_ht,shelf_ht, str_elem("Missan"));
    
  ioopm_hash_table_destroy(merch_ht);
  ioopm_hash_table_destroy(shelf_ht);
  ioopm_hash_table_destroy(cart_ht);
  ioopm_undo_list_delete(undo_list);
 }
void checkout_cart_test1(void)
{
    ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
   ioopm_hash_table_t *shelf_ht = ioopm_create_shelf_ht();
    ioopm_hash_table_t *cart_ht = ioopm_create_cart_ht();
    ioopm_list_t *undo_list = undo_list_create();   
  elem_t elem_cart = ioopm_create_cart(1);
  elem_t elem_merch = ioopm_merch_create(strdup("Erik"), strdup("dv"), 5);
  cart_t *cart = (cart_t *)elem_cart.voi;

  ioopm_merch_insert(merch_ht, str_elem("Erik"), elem_merch);
  ioopm_hash_table_insert(cart_ht, int_elem(cart->name), elem_cart);

  ioopm_merch_stock_increase(merch_ht, shelf_ht, str_elem(strdup("Erik")), str_elem(strdup("A25")), 5);
  ioopm_add_to_cart(cart_ht, elem_cart, elem_merch, 3);

  ioopm_checkout_cart(merch_ht, shelf_ht, cart_ht, cart);

  ioopm_hash_table_remove(cart_ht, int_elem(1));
  ioopm_linked_apply_to_all(cart->wares, ioopm_merch_in_cart_delete, NULL);
  ioopm_linked_list_clear(cart->wares);
  free(cart->wares);
  free(cart);
	  
    
  ioopm_option_t merch_ht_result = ioopm_hash_table_lookup(merch_ht, str_elem("Erik"));
  elem_t elem_merch_after = merch_ht_result.value;
  merch_t *merch = (merch_t *)elem_merch_after.voi;
  CU_ASSERT_EQUAL(merch->total_stock, 2);//check that total stock of Erik is 2

  ioopm_option_t shelf_ht_result2 = ioopm_hash_table_lookup(shelf_ht, str_elem("A25"));
  elem_t elem_shelf2 = shelf_ht_result2.value;
  shelf_t *shelf2 = (shelf_t *)elem_shelf2.voi;
  CU_ASSERT_EQUAL(shelf2->shelf_stock, 2);//check that A25 has 2 items
    
  ioopm_merch_remove( merch_ht,shelf_ht, str_elem("Erik"));

  ioopm_hash_table_destroy(merch_ht);
  ioopm_hash_table_destroy(shelf_ht);
  ioopm_hash_table_destroy(cart_ht);
  ioopm_undo_list_delete(undo_list);

 }



//check if you can insert a merch that is already added
void undo_pop_merch_add_test1(void)
  {
   ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
  ioopm_hash_table_t *shelf_ht = ioopm_create_shelf_ht();
   ioopm_hash_table_t *cart_ht = ioopm_create_cart_ht();
   ioopm_list_t *undo_list = undo_list_create();   

 ioopm_undoable_merch_add(undo_list, merch_ht, strdup("Erik"), strdup("dv"), 5);
    ioopm_undo_pop(undo_list, merch_ht, shelf_ht, cart_ht);
    
    ioopm_undoable_merch_add(undo_list, merch_ht, strdup("Missan"), strdup("it"), 2);   
    ioopm_undo_pop(undo_list, merch_ht, shelf_ht, cart_ht);

    //check if the merch is still there added undoing both adds
    CU_ASSERT_FALSE(ioopm_hash_table_lookup(merch_ht, str_elem("Erik")).success);
    CU_ASSERT_FALSE(ioopm_hash_table_lookup(merch_ht, str_elem("Missan")).success);

		       
    ioopm_undo_list_delete(undo_list);
    ioopm_hash_table_destroy(merch_ht);
    ioopm_hash_table_destroy(shelf_ht);
    ioopm_hash_table_destroy(cart_ht);
  }

void undo_pop_merch_remove_test1(void)
  {
   ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
  ioopm_hash_table_t *shelf_ht = ioopm_create_shelf_ht();
   ioopm_hash_table_t *cart_ht = ioopm_create_cart_ht();
   ioopm_list_t *undo_list = undo_list_create();   

 ioopm_undoable_merch_add(undo_list, merch_ht, strdup("Erik"), strdup("dv"), 5);
 ioopm_merch_undoable_remove(undo_list, merch_ht, shelf_ht,"Erik");  
 CU_ASSERT_FALSE(ioopm_hash_table_lookup(merch_ht, str_elem("Erik")).success);

 ioopm_undo_pop(undo_list, merch_ht, shelf_ht, cart_ht);
 ioopm_merch_undoable_remove(undo_list, merch_ht, shelf_ht,"Erik");
 CU_ASSERT_FALSE(ioopm_hash_table_lookup(merch_ht, str_elem("Erik")).success);

 ioopm_undo_pop(undo_list, merch_ht, shelf_ht, cart_ht);
 ioopm_merch_undoable_remove(undo_list, merch_ht, shelf_ht,"Erik");  
 ioopm_undo_pop(undo_list, merch_ht, shelf_ht, cart_ht);

    /* 
    ioopm_undoable_merch_add(undo_list, merch_ht, "Missan", "it", 2);
    elem_t elem2 = ioopm_hash_table_lookup(merch_ht, str_elem("Erik")).value;
    ioopm_merch_undoable_remove(undo_list, merch_ht, shelf_ht,elem2);
    ioopm_undo_pop(undo_list, merch_ht, shelf_ht, cart_ht);
    */
    
    //check if the merch is still there added undoing both adds
    CU_ASSERT_TRUE(ioopm_hash_table_lookup(merch_ht, str_elem("Erik")).success);
    CU_ASSERT_FALSE(ioopm_hash_table_lookup(merch_ht, str_elem("Missan")).success);

    ioopm_merch_remove( merch_ht, shelf_ht,str_elem("Erik"));

    ioopm_undo_list_delete(undo_list);
    ioopm_hash_table_destroy(merch_ht);
    ioopm_hash_table_destroy(shelf_ht);
    ioopm_hash_table_destroy(cart_ht);
  }

void undo_pop_merch_remove_test2(void)
  {
   ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
  ioopm_hash_table_t *shelf_ht = ioopm_create_shelf_ht();
   ioopm_hash_table_t *cart_ht = ioopm_create_cart_ht();
   ioopm_list_t *undo_list = undo_list_create();   

    ioopm_undoable_merch_add(undo_list, merch_ht, strdup("Erik"), strdup("dv"), 5);
    ioopm_undoable_merch_add(undo_list, merch_ht, strdup("Missan"), strdup("katt"), 2);
    ioopm_merch_undoable_remove(undo_list, merch_ht, shelf_ht,"Erik");
    ioopm_undo_pop(undo_list, merch_ht, shelf_ht, cart_ht);

    
    CU_ASSERT_TRUE(ioopm_hash_table_lookup(merch_ht, str_elem("Erik")).success);
    CU_ASSERT_TRUE(ioopm_hash_table_lookup(merch_ht, str_elem("Missan")).success);

    ioopm_merch_remove( merch_ht, shelf_ht,str_elem("Erik"));
    ioopm_merch_remove( merch_ht, shelf_ht,str_elem("Missan"));
    ioopm_undo_list_delete(undo_list);
    ioopm_hash_table_destroy(merch_ht);
    ioopm_hash_table_destroy(shelf_ht);
    ioopm_hash_table_destroy(cart_ht);
  }

void undo_pop_merch_remove_test3(void)
  {
   ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
  ioopm_hash_table_t *shelf_ht = ioopm_create_shelf_ht();
   ioopm_hash_table_t *cart_ht = ioopm_create_cart_ht();
   ioopm_list_t *undo_list = undo_list_create();   

 ioopm_undoable_merch_add(undo_list, merch_ht, strdup("Erik1"), strdup("dv"), 5);
 ioopm_undoable_merch_add(undo_list, merch_ht, strdup("Erik2"), strdup("dv"), 5);
 ioopm_undoable_merch_add(undo_list, merch_ht, strdup("Erik3"), strdup("dv"), 5);
 ioopm_merch_undoable_remove(undo_list, merch_ht, shelf_ht,"Erik1");
 ioopm_merch_undoable_remove(undo_list, merch_ht, shelf_ht,"Erik2");
 ioopm_merch_undoable_remove(undo_list, merch_ht, shelf_ht,"Erik3");  
 
 ioopm_undo_pop(undo_list, merch_ht, shelf_ht, cart_ht);
 ioopm_undo_pop(undo_list, merch_ht, shelf_ht, cart_ht);
 ioopm_undo_pop(undo_list, merch_ht, shelf_ht, cart_ht);
 
 CU_ASSERT_TRUE(ioopm_hash_table_lookup(merch_ht, str_elem("Erik1")).success);
 CU_ASSERT_TRUE(ioopm_hash_table_lookup(merch_ht, str_elem("Erik2")).success);
 CU_ASSERT_TRUE(ioopm_hash_table_lookup(merch_ht, str_elem("Erik3")).success);
    
 ioopm_merch_undoable_remove(undo_list, merch_ht, shelf_ht,"Erik1");
 ioopm_merch_undoable_remove(undo_list, merch_ht, shelf_ht,"Erik2");
 ioopm_merch_undoable_remove(undo_list, merch_ht, shelf_ht,"Erik3");

 ioopm_undo_list_delete(undo_list);
 ioopm_hash_table_destroy(merch_ht);
 ioopm_hash_table_destroy(shelf_ht);
 ioopm_hash_table_destroy(cart_ht);
  }

void access_functions_undo_test1(void)
  {
   ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
  ioopm_hash_table_t *shelf_ht = ioopm_create_shelf_ht();
   ioopm_hash_table_t *cart_ht = ioopm_create_cart_ht();
   ioopm_list_t *undo_list = undo_list_create();   

    ioopm_undoable_merch_add(undo_list, merch_ht, strdup("Erik"), strdup("dv"), 5);

    ioopm_undoable_edit_name(undo_list, merch_ht,cart_ht, "Erik", "Missan");
    CU_ASSERT_STRING_EQUAL(value_name((ioopm_hash_table_lookup(merch_ht, str_elem("Missan"))).value), "Missan");
    
    ioopm_undoable_edit_price(undo_list, merch_ht,cart_ht, "Missan" ,3);
    CU_ASSERT_EQUAL(value_price((ioopm_hash_table_lookup(merch_ht, str_elem("Missan"))).value), 3);
    
    ioopm_undoable_edit_descr(undo_list,merch_ht, strdup("Missan"), strdup("Katt"));
    CU_ASSERT_STRING_EQUAL(value_descr((ioopm_hash_table_lookup(merch_ht, str_elem("Missan"))).value), "Katt");
    
    ioopm_merch_undoable_remove(undo_list, merch_ht, shelf_ht,"Missan");
    CU_ASSERT_FALSE(ioopm_hash_table_lookup(merch_ht, str_elem("Missan")).success);

    ioopm_undo_pop(undo_list, merch_ht, shelf_ht, cart_ht);
    ioopm_undo_pop(undo_list, merch_ht, shelf_ht, cart_ht);
    ioopm_undo_pop(undo_list, merch_ht, shelf_ht, cart_ht);
    ioopm_undo_pop(undo_list, merch_ht, shelf_ht, cart_ht); 

    CU_ASSERT_TRUE(ioopm_hash_table_lookup(merch_ht, str_elem("Erik")).success);
    CU_ASSERT_STRING_EQUAL(value_name((ioopm_hash_table_lookup(merch_ht, str_elem("Erik"))).value), "Erik");
    CU_ASSERT_STRING_EQUAL(value_descr((ioopm_hash_table_lookup(merch_ht, str_elem("Erik"))).value), "dv");
    CU_ASSERT_EQUAL(value_price((ioopm_hash_table_lookup(merch_ht, str_elem("Erik"))).value), 5);

    ioopm_merch_undoable_remove(undo_list, merch_ht, shelf_ht,"Erik");
    
    ioopm_undo_list_delete(undo_list);
    ioopm_hash_table_destroy(merch_ht);
    ioopm_hash_table_destroy(shelf_ht);
    ioopm_hash_table_destroy(cart_ht);
  }

void ioopm_destroy_all_hashtables_test1(void)
  {
   ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
  ioopm_hash_table_t *shelf_ht = ioopm_create_shelf_ht();
   ioopm_hash_table_t *cart_ht = ioopm_create_cart_ht();
   ioopm_list_t *undo_list = undo_list_create();   

    ioopm_undoable_merch_add(undo_list, merch_ht, strdup("Erik"), strdup("dv"), 5);
    
    ioopm_destroy_all_hashtables(merch_ht,shelf_ht,cart_ht);
    
    ioopm_undo_list_delete(undo_list);
  }


int main()
{
  CU_pSuite test_suite1 = NULL;

  if (CUE_SUCCESS != CU_initialize_registry())
    return CU_get_error();

  test_suite1 = CU_add_suite("Suite 1, backend tests", init_suite, clean_suite);
  if (NULL == test_suite1)
    {
      CU_cleanup_registry();
      return CU_get_error();
    }

  if (
      (NULL == CU_add_test(test_suite1, "create test 1", merch_create_test1))                       ||
      (NULL == CU_add_test(test_suite1, "insert test 1", merch_ht_insert_test1))                    ||
      (NULL == CU_add_test(test_suite1, "insert test 2", merch_ht_insert_test2))                    ||
      (NULL == CU_add_test(test_suite1, "insert test 3", merch_ht_insert_test3))                    ||
      (NULL == CU_add_test(test_suite1, "insert test 4", merch_ht_insert_test4))                    ||
      (NULL == CU_add_test(test_suite1, "merch add test 1", merch_add_test1))                       ||
      (NULL == CU_add_test(test_suite1, "merch add test 2", merch_add_test2))                       ||
      (NULL == CU_add_test(test_suite1, "remove test 1", merch_remove_test1))                       ||
      (NULL == CU_add_test(test_suite1, "remove test 2", merch_remove_test2))                       ||
      (NULL == CU_add_test(test_suite1, "print merch test 1", merch_print_test1))           ||
      (NULL == CU_add_test(test_suite1, "print merch test 2", merch_print_test2))           ||
      (NULL == CU_add_test(test_suite1, "edit price test 1",merch_edit_price_test1))                ||
      (NULL == CU_add_test(test_suite1, "edit name test 1",merch_edit_name_test1))                  ||
      (NULL == CU_add_test(test_suite1, "edit name test 2",merch_edit_name_test2))                  ||
      (NULL == CU_add_test(test_suite1, "edit name undo test 1",merch_edit_name_test2))             ||
      (NULL == CU_add_test(test_suite1, "edit descr test 1", merch_edit_descr_test1))               ||
      (NULL == CU_add_test(test_suite1, "merch stock increase test 1", merch_stock_increase_test1)) ||
      (NULL == CU_add_test(test_suite1, "merch stock decrease test 1", merch_stock_decrease_test1)) ||
      (NULL == CU_add_test(test_suite1, "merch stock decrease test 2", merch_stock_decrease_test2)) ||
      (NULL == CU_add_test(test_suite1, "merch stock decrease test 3", merch_stock_decrease_test3)) ||
      (NULL == CU_add_test(test_suite1, "merch show stock test 1", merch_show_stock_test1))         ||
      (NULL == CU_add_test(test_suite1, "create cart test 1", create_cart_test1))                   ||
      (NULL == CU_add_test(test_suite1, "total stock carts test 1", total_stock_of_a_merch_in_all_carts_test1))  ||
      (NULL == CU_add_test(test_suite1, "add to cart test 1", add_to_cart_test1))                   ||
      (NULL == CU_add_test(test_suite1, "add to cart test 2", add_to_cart_test2))                   ||
      (NULL == CU_add_test(test_suite1, "remove from cart test 1", remove_from_cart_test1))         ||
      (NULL == CU_add_test(test_suite1, "checkout cart test 1", checkout_cart_test1))               ||
      (NULL == CU_add_test(test_suite1, "undo pop merch add test 1", undo_pop_merch_add_test1))     ||
      (NULL == CU_add_test(test_suite1, "undo pop merch remove test 1", undo_pop_merch_remove_test1))||
      (NULL == CU_add_test(test_suite1, "undo pop merch remove test 2", undo_pop_merch_remove_test2))||
      (NULL == CU_add_test(test_suite1, "undo pop merch remove test 3", undo_pop_merch_remove_test3))||
      //(NULL == CU_add_test(test_suite1, "access functions undo test 1", access_functions_undo_test1))||
      (NULL == CU_add_test(test_suite1, "destroy all hashtables test 1",ioopm_destroy_all_hashtables_test1))


      )
    {
      CU_cleanup_registry();
      return CU_get_error();
    }

  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  CU_cleanup_registry();
  return CU_get_error();
}
