#pragma once
#include <stdlib.h>
#include <stdio.h>
#include "hash_table.h"
#include "iterator.h"
#include "carts.h"

typedef struct merch merch_t;
typedef struct shelf shelf_t;

/// @brief Prints all merch in the store, twenty at a time.
/// @param merch_ht hashtable of merch_t with merch names as keys.
void ioopm_print_merch(ioopm_hash_table_t *merch_ht);

/// @brief Creates a merch hashtable with merch names as keys and the merch themselves as values.
/// @return A merch hashtable.
ioopm_hash_table_t *ioopm_create_merch_ht();

/// @brief Creates a shelf hashtable with shelf names as keys and the shelves themselves as values.
/// @return A shelf hashtable.
ioopm_hash_table_t *ioopm_create_shelf_ht();

/// @brief Creates a list to store user actions in so that they can be undone.
/// @return A list that stores user actions.
ioopm_list_t *undo_list_create();


/// @brief Creates a merch_t with a name, description and price as well as a list of shelves where it shows its stock.
/// @param name the name of the merch.
/// @param descr the description of the merch.
/// @param price the price of the merch.
/// @return the created merch_t inside an elem_t.
elem_t ioopm_merch_create(char *name, char *descr, int price);

/// @brief Inserts a merch into a merch hashtable. Merch name and key must be the same, and cant insert if that key is already in ht.
/// @param key elem_t with a string used as key for the hashtable.
/// @param merch an elem_t with a merch_t inside.
/// @return true if successfully inserted into merch hashtable
bool ioopm_merch_insert(ioopm_hash_table_t *merch_ht, elem_t key, elem_t merch);

/// @brief Removes an merchandise completely from the warehouse, including stock if it has any.
/// @param merch_ht hashtable of merch_t with merch names as keys.
/// @param shelf_ht hashtable of shelf_t with shelf names as keys.
/// @param key the name of the merch you want to remove inside an elem_t which will be used as a key for the hashtables.
/// @return True if the merch existed and was removed, false otherwise.
bool ioopm_merch_remove(ioopm_hash_table_t *merch_ht, ioopm_hash_table_t *shelf_ht, elem_t key);

/// @brief Checks if a certain merch is in the merch hashtable.
/// @param merch_ht hashtable of merch_t with merch names as keys.
/// @param merch_name the name of the merch you will add stocks of.
/// @return True if the merch was inside the hashtable, false otherwise.
bool ioopm_is_merch_in_ht(ioopm_hash_table_t *merch_ht, char *merch_name);

/// @brief Checks what the total stock of a merch is, meaning all stock of the merch on all shelves.
/// @param merch_ht hashtable of merch_t with merch names as keys.
/// @param merch_name the name of the merch.
/// @return The number of stock of the merch on all shelves
int ioopm_merch_total_stock(ioopm_hash_table_t *merch_ht, char* merch_name);

/// @brief Prints the stock of a merchandise and which shelf they are on.
/// @param merch_ht hashtable of merch_t with merch names as keys.
/// @param merch_name the name of the merch.
/// @return True if there are any stock, false otherwise.
bool ioopm_merch_show_stock(ioopm_hash_table_t *merch_ht, elem_t key);

/// @brief removes stock from non specific shelves in the warehouse unless shelf_option is a valid shelf, then it removes from that shelf only.
/// @param merch_ht hashtable of merch_t with merch names as keys.
/// @param shelf_ht hashtable of shelf_t with shelf names as keys.
/// @param merch_key the name of the merch you remove stocks of.
/// @param amount_decreased the amount of stock to remove.
/// @param shelf_optional if its a valid shelf name, removes stock only from that shelf.
/// @return ioopm_option_t
bool ioopm_merch_stock_decrease(ioopm_hash_table_t *merch_ht, ioopm_hash_table_t *shelf_ht, elem_t merch_key, int amount_decreased, char *shelf_optional);

/// @brief Checks if a string is a valid name for a shelf. Should be one big letter then at least one number.
/// @param str the string you want to check.
/// @return True if its a valid shelf name, false otherwise.
bool is_shelf(char *str);

/// @brief add a merch to the warehouse(merch_ht). Can be undone. Cant add if another merch with the same name is already inserted.
/// @param undo_list A list of saved actions that is used when you want to undo an action.
/// @param merch_ht hash table of merch_t.
/// @param merch_name name of merch used as key for hash table.
/// @param descr description of the merch.
/// @param price the price of one merch.
/// @return True if successfully inserted, false otherwise.
bool ioopm_undoable_merch_add(ioopm_list_t *undo_list, ioopm_hash_table_t *merch_ht, char *merch_name, char *descr, int price);

/// @brief Removes a merchandise completely from the warehouse including stock. Can be undone.
/// @param merch_ht hashtable of merch_t with merch names as keys.
/// @param shelf_ht hashtable of shelf_t with shelf names as keys.
/// @param merch_name Name of the merch you want to remove from the warehouse. 
/// @return True if the merch was in the hashtable and successfully removed, false otherwise.
bool ioopm_merch_undoable_remove(ioopm_list_t *undo_list, ioopm_hash_table_t *merch_ht,ioopm_hash_table_t *shelf_ht, char *merch_name);

/// @brief edits the name of a merch, rehashes using new name. Can be undone.
/// @param undo_list A list of saved actions that is used when you want to undo an action.
/// @param merch_ht hashtable of merch_t with merch names as keys.
/// @param cart_ht hashtable of cart_t with cart names as keys.
/// @param new_name New name of the merch.
/// @return true if successfully edited otherwise false.
bool ioopm_undoable_edit_name(ioopm_list_t *undo_list, ioopm_hash_table_t *merch_ht, ioopm_hash_table_t *cart_ht, char *merch_name, char *new_name);

/// @brief edits the description of a merch. Can be undone.
/// @param undo_list A list of saved actions that is used when you want to undo an action.
/// @param merch_ht hashtable of merch_t with merch names as keys.
/// @param merch_name Name of the merch you want to edit.
/// @param new_description string with the new description
void ioopm_undoable_edit_descr(ioopm_list_t *undo_list, ioopm_hash_table_t *merch_ht, char *merch_name, char *new_descr);

/// @brief edits the price of a merch. Can be undone.
/// @param undo_list A list of saved actions that is used when you want to undo an action.
/// @param merch_ht hashtable of merch_t with merch names as keys.
/// @param merch_name Name of the merch you want to edit.
/// @param new_price The new price of the merchadise.
 void ioopm_undoable_edit_price(ioopm_list_t *undo_list, ioopm_hash_table_t *merch_ht,ioopm_hash_table_t *cart_ht, char *merch_name, int new_price);
  
/// @brief adds stock of an existing merch to a shelf in the warehouse. Can be undone.
/// @param undo_list A list of saved actions that is used when you want to undo an action.
/// @param merch_ht hashtable of merch_t with merch names as keys.
/// @param shelf_ht hashtable of shelf_t with shelf names as keys.
/// @param merch_key the name of the merch you will add stocks of.
/// @param amount_increased he amount of stock to add.
/// @param shelf_key shelf name you want to add the stock to.
/// @return True if the shelf was empty or had the merch you want to add already there. False if some other stock was on the shelf.
bool ioopm_merch_undoable_replenish(ioopm_list_t *undo_list, ioopm_hash_table_t *merch_ht,ioopm_hash_table_t *shelf_ht, char *merch_name, int amount_increased, char *shelf_name);

/// @brief Creates a new cart with a name of your choice as long as its a number and inserts it into the cart hashtable. Can be undone.
/// @param undo_list A list of saved actions that is used when you want to undo an action.
/// @param cart_ht hashtable of cart_t with cart names(numbers) as keys.
/// @param cart_name Name of the cart you want to create.
void ioopm_merch_undoable_create_cart(ioopm_list_t *undo_list,ioopm_hash_table_t *cart_ht, int cart_name);

/// @brief Removes a cart from the cart hashtable and deletes it. Can be undone.
/// @param undo_list A list of saved actions that is used when you want to undo an action.
/// @param cart_ht hashtable of cart_t with cart names(numbers) as keys.
/// @param cart_name Name of the cart you want to remove(number).
/// @return True if the cart existed in cart_ht and was removed, false otherwise.
bool ioopm_merch_undoable_remove_cart(ioopm_list_t *undo_list,ioopm_hash_table_t *cart_ht, int cart_name);

/// @brief Adds a certain amount of merchandise to a cart. Cant add more than total stock of a merch that also are not in other carts. Doesnt affect stock. Can be undone.
/// @param undo_list A list of saved actions that is used when you want to undo an action.
/// @param merch_ht hashtable of merch_t with merch names as keys.
/// @param cart_ht hashtable of cart_t with cart names(numbers) as keys.
/// @param cart_name Name of the cart you want to add merch to.
/// @param merch_name the name of the merch.
/// @param amount_increased he amount of stock to add to the cart.
/// @return True if the cart and merch exists in their respective hashtables and the amount to add is less or equal to total stock of the merch that is also not in other carts, false otherwise.
bool ioopm_merch_undoable_add_to_cart(ioopm_list_t *undo_list,ioopm_hash_table_t *merch_ht,ioopm_hash_table_t *cart_ht, int cart_name,char *merch_name, int amount_to_add );

/// @brief Removes a certain amount of a merch from a cart. Cant remove more than is in the cart. Can be undone.
/// @param undo_list A list of saved actions that is used when you want to undo an action.
/// @param merch_ht hashtable of merch_t with merch names as keys.
/// @param cart_ht hashtable of cart_t with cart names(numbers) as keys.
/// @param cart_name Name of the cart you want to remove merch from.
/// @param merch_name the name of the merch.
/// @param amount_to_remove The amount of a merch you want to remove from the cart.
/// @return True if the merch was successfully removed from the cart, false otherwise.
bool ioopm_merch_undoable_remove_from_cart(ioopm_list_t *undo_list,ioopm_hash_table_t *merch_ht,ioopm_hash_table_t *cart_ht, int cart_name, char *merch_name, int amount_to_remove );

/// @brief Checkouts a cart, which removes the cart and reduces stock in the warehouse for all merch inside the cart. Can be undone.
/// @param undo_list A list of saved actions that is used when you want to undo an action.
/// @param merch_ht hashtable of merch_t with merch names as keys.
/// @param shelf_ht hashtable of shelf_t with shelf names as keys.
/// @param cart_ht hashtable of cart_t with cart names(numbers) as keys.
/// @param cart_name Name of the cart you want to checkout.
/// @return True if the cart exists, false otherwise.
bool ioopm_undoable_checkout(ioopm_list_t *undo_list, ioopm_hash_table_t *merch_ht,ioopm_hash_table_t *shelf_ht,ioopm_hash_table_t *cart_ht,  int cart_name);

/// @brief Deletes an undo_list of saved user actions.
/// @param undo_list A list of saved actions that is used when you want to undo an action.
void ioopm_undo_list_delete(ioopm_list_t *undo_list);

/// @brief "Pop" an action from the undo_list and undoes the last user action.
/// @param undo_list A list of saved actions that is used when you want to undo an action.
/// @param merch_ht hashtable of merch_t with merch names as keys.
/// @param shelf_ht hashtable of shelf_t with shelf names as keys.
/// @param cart_ht hashtable of cart_t with cart names(numbers) as keys.
void ioopm_undo_pop(ioopm_list_t *undo_list, ioopm_hash_table_t *merch_ht, ioopm_hash_table_t *shelf_ht, ioopm_hash_table_t *cart_ht);

/// @brief Deletes merch, shelf and cart hashtables.
/// @param merch_ht hashtable of merch_t with merch names as keys.
/// @param shelf_ht hashtable of shelf_t with shelf names as keys.
/// @param cart_ht hashtable of cart_t with cart names(numbers) as keys.
void ioopm_destroy_all_hashtables(ioopm_hash_table_t *merch_ht, ioopm_hash_table_t *shelf_ht, ioopm_hash_table_t *cart_ht);

