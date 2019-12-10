#pragma once
#include "backend.h"

typedef struct cart cart_t;
typedef struct merch_in_cart merch_in_cart_t;

/**
 * @file carts.c
 * @author Erik Franck
 * @date 13 November 2019
 * @brief Contains the functions that creates and manipulates carts.
 *  */


/// @brief creates a cart with a number as a name, including its wares list.
/// @param cart_name Name of the cart you want to create in the form of a number.
/// @return elem_t containing the cart.
elem_t ioopm_create_cart(int int_name);

/// @brief Deletes the struct merch_in_cart inside inside a cart. Usefully to apply on cart->wares list to delete whole list.
/// @param elem_merch_in_cart pointer to an elem_t with a merch_in_cart_t inside.
/// @param extra Used to send an extra argument when used together with ioopm_linked_apply_to_all in linked_list.h.
void ioopm_merch_in_cart_delete(elem_t *elem_merch_in_cart, void *extra);

/// @brief Removes a cart, including everything in its wares list using ioopm_merch_in_cart_delete.
/// @param elem_cart pointer to an elem_t with a merch_in_cart_t inside.
/// @param extra Used to send an extra argument when used together with ioopm_linked_apply_to_all in linked_list.h.
void ioopm_remove_cart(elem_t *elem_cart, void *extra);

/// @brief Creates a cart hash table with cart names(numbers) as keys and the cart themselves as values.
/// @return A cart hashtable with cart names(numbers) as keys and the cart themselves as values.
ioopm_hash_table_t *ioopm_create_cart_ht();

/// @brief Prints all carts that have been created and not removed or checked out.
/// @param cart_ht a cart hashtable with cart names(numbers) as keys and the cart themselves as values.
/// @return false if there are no carts, true otherwise.
bool ioopm_print_carts(ioopm_hash_table_t *cart_ht);

/// @brief Retrieves the cart->total_cost inside a cart in the cart hashtable which is the combied cost of all wares inside the cart.
/// @param merch_ht a cart hashtable with cart names(numbers) as keys and the cart themselves as values.
/// @param cart_name A name of a cart(number).
/// @return total cost of all wares in the cart.
int ioopm_cart_total_cost(ioopm_hash_table_t *cart_ht, int cart_name);

/// @brief Prints all the wares that are inside a cart.
/// @param cart_ht a cart hashtable with cart names(numbers) as keys and the cart themselves as values.
/// @param cart_name A name of a cart(number).
/// @return true if there are wares in the cart, false otherwise.
bool ioopm_print_cart_wares(ioopm_hash_table_t *cart_ht, int cart_name);

/// @brief Counts how many of a certain merch is currently inside carts(as in, inside cart->wares list). 
/// @param cart_ht a cart hashtable with cart names(numbers) as keys and the cart themselves as values.
/// @param merch_name The name of the merch.
/// @return Number of a certain merch that are currently inside carts(as in, inside cart->wares list).
int ioopm_total_stock_of_a_merch_in_all_carts(ioopm_hash_table_t *cart_ht, char *merch_name);  

/// @brief Edits the name of a certain merch inside all the carts it is in.
/// @param cart_ht a cart hashtable with cart names(numbers) as keys and the cart themselves as values.
/// @param merch_name The name of the merch you want to edit.
/// @param new_name The new name you want to give the merch.
void ioopm_merch_edit_name_in_carts(ioopm_hash_table_t *cart_ht, char *merch_name, char *new_name);

/// @brief Edits the price of a certain merch inside all the carts it is in.
/// @param cart_ht a cart hashtable with cart names(numbers) as keys and the cart themselves as values.
/// @param merch_name The name of the merch you want to edit.
/// @param new_name The new name you want to give the merch.
void ioopm_merch_edit_price_in_carts(ioopm_hash_table_t *cart_ht, char *merch_name,int old_price, int new_price);

/// @brief Add an amount of a merch to a cart. 
/// @param cart_ht a cart hashtable with cart names(numbers) as keys and the cart themselves as values.
/// @param elem_cart Name of the cart inside an elem_t you want to add merch to.
/// @param elem_merch Name of the merch inside an elem_t you want to add to a cart.
/// @param amount_to_add Amount of a merch you want to add to a cart.
/// @return False if the total stock of the merch is lower than the amount you want to add(cant add merch that doesnt exist).
bool ioopm_add_to_cart(ioopm_hash_table_t *cart_ht, elem_t elem_cart, elem_t elem_merch, int amount_to_add);

/// @brief Remove an amount of a merch to a cart. 
/// @param cart_ht a cart hashtable with cart names(numbers) as keys and the cart themselves as values.
/// @param elem_cart Name of the cart inside an elem_t you want to remove merch from.
/// @param elem_merch Name of the merch inside an elem_t you want to remove from a cart.
/// @param amount_to_add Amount of a merch you want to remove from a cart.
void ioopm_remove_from_cart(ioopm_hash_table_t *cart_ht, elem_t elem_cart, elem_t elem_merch,  int amount_to_remove);

/// @brief Checks if a merch is currently inside any cart in the cart hashtable.
/// @param merch_ht a merch hashtable with merch names(strings) as keys and the merch themselves as values.
/// @param cart_ht a cart hashtable with cart names(numbers) as keys and the cart themselves as values.
/// @param merch_name Name of the merch. 
/// @return True if its in a cart, false otherwise.
bool ioopm_is_merch_in_a_cart(ioopm_hash_table_t *merch_ht,ioopm_hash_table_t *cart_ht, char *merch_name);

/// @brief Checks out a cart, removing the cart from the cart hashtable(NOT memory!) and decreasing the warehouse stock of the merch inside the cart.
/// @param merch_ht a merch hashtable with merch names(strings) as keys and the merch themselves as values.
/// @param shelf_ht a shelf hashtable with shelf names(strings) as keys and the shelf themselves as values.
/// @param cart_ht a cart hashtable with cart names(numbers) as keys and the cart themselves as values.
/// @param cart The cart you want to checkout.
void ioopm_checkout_cart(ioopm_hash_table_t *merch_ht, ioopm_hash_table_t *shelf_ht, ioopm_hash_table_t *cart_ht, cart_t *cart);
