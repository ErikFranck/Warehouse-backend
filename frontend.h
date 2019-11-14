#pragma once
#include "backend.h"






/// @brief Asks the user which merch they want to edit description of and to what.
/// @param undo_list A list of saved actions that is used when you want to undo an action.
/// @param merch_ht hashtable of merch_t with merch names as keys.
/// @param merch_name the name of the merch.
void ask_question_edit_descr(ioopm_list_t *undo_list,ioopm_hash_table_t *merch_ht, char* merch_name);

/// @brief Asks the user which merch they want to edit name of and to what.
/// @param undo_list A list of saved actions that is used when you want to undo an action.
/// @param merch_ht hashtable of merch_t with merch names as keys.
/// @param cart_ht hashtable of cart_t with cart names(numbers) as keys.
/// @param merch_name the name of the merch.
void ask_question_edit_name(ioopm_list_t *undo_list, ioopm_hash_table_t *merch_ht, ioopm_hash_table_t *cart_ht,char* merch_name);

/// @brief Asks the user which merch they want to edit price of and to what.
/// @param undo_list A list of saved actions that is used when you want to undo an action.
/// @param merch_ht hashtable of merch_t with merch names as keys.
/// @param cart_ht hashtable of cart_t with cart names(numbers) as keys.
/// @param merch_name the name of the merch.
void ask_question_edit_price(ioopm_list_t *undo_list, ioopm_hash_table_t *merch_ht,ioopm_hash_table_t *cart_ht, char *merch_name);

/// @brief Asks the user which merch they want to add to the warehouse.
/// @param undo_list A list of saved actions that is used when you want to undo an action.
/// @param merch_ht hashtable of merch_t with merch names as keys.
void ask_question_add_merch(ioopm_list_t *undo_list,ioopm_hash_table_t *merch_ht);


/// @brief Checks if a character is one of the chars that issues a valid menu action.
/// @param character a character that was written by the user when chosing menu action.
/// @return True if it was a valid char, false otherwise.
bool valid_menu_action_check(char character);

/// @brief Asks the user to input an action in the form of a character. Has to be a valid character.
/// @return the character that was put in by the user.
char ask_question_menu();

/// @brief Prints the menu of valid actions a user can do.
void print_menu();

/// @brief A warehouse where you can do various actions from both a customer and company perspective.
int main();


