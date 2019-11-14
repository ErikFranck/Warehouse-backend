#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "frontend.h"

#define str_elem(s) (elem_t) { .str =(s) }
//TODO move all hash table lookups into backend, none should be here. Make undoable_warehouse?
//TODO strdup to all backend functions.
void print_menu()
{
  char *A = "[A]Add merchandise";
  char *B = "[R]Remove merchandise";
  char *C = "[L]List merchandise";
  char *D = "[E]Edit merchandise";
  char *E = "[S]Show stock";
  char *F = "[P]Replenish merchandise";
  char *G = "[C]Create cart";
  char *H = "[M]Remove cart";
  char *I = "[D]Add to cart";
  char *J = "[F]Remove from cart";
  char *K = "[T]Calculate cost";
  char *L = "[O]Checkout"; 
  char *M = "[U]Undo";
  char *N = "[Q]Quit";
  
  printf("\n %s \n %s \n %s \n %s \n %s \n %s \n %s \n %s \n %s \n %s \n %s \n %s \n %s \n %s \n \n",A,B,C,D,E,F,G,H,I,J,K,L,M,N);
}

bool valid_menu_action_check(char character)
{
  char answer = character;
  if(answer == 'A' || answer == 'a'
     || answer == 'R' || answer == 'r'
     || answer == 'L' || answer == 'l'
     || answer == 'E' || answer == 'e'
     || answer == 'S' || answer == 's'
     || answer == 'P' || answer == 'p'
     || answer == 'S' || answer == 's'
     || answer == 'C' || answer == 'c'
     || answer == 'M' || answer == 'm'
     || answer == 'D' || answer == 'd'
     || answer == 'F' || answer == 'f'
     || answer == 'T' || answer == 't'
     || answer == 'O' || answer == 'o'
     || answer == 'U' || answer == 'u'
     || answer == 'Q' || answer == 'q'
      )
    {
      return true;
    }
  else
    {
      return false;
    }
}
 
void ask_question_edit_descr(ioopm_list_t *undo_list,ioopm_hash_table_t *merch_ht, char* merch_name)
{
  ioopm_print_merch(merch_ht);
  char *new_descr = ask_question_string("Enter a new description for this merchandise");
  ioopm_undoable_edit_descr(undo_list, merch_ht, merch_name, new_descr);
  free(new_descr);
}

void ask_question_edit_name(ioopm_list_t *undo_list, ioopm_hash_table_t *merch_ht, ioopm_hash_table_t *cart_ht,char* merch_name)
{
  ioopm_print_merch(merch_ht);
  char *new_name = ask_question_string("Enter a new name for this merchandise");
  bool result = ioopm_undoable_edit_name(undo_list, merch_ht, cart_ht, merch_name, new_name);
  free(new_name);
  if (!(result))
    {
      printf("A merchandise called %s already exists\n", merch_name);
      return;
    }
}

void ask_question_edit_price(ioopm_list_t *undo_list, ioopm_hash_table_t *merch_ht,ioopm_hash_table_t *cart_ht, char *merch_name)
{
  ioopm_print_merch(merch_ht);
  int new_price = ask_question_int("Enter a new price for this merchandise");
  ioopm_undoable_edit_price(undo_list, merch_ht, cart_ht,merch_name, new_price); 
}

void ask_question_add_merch(ioopm_list_t *undo_list,ioopm_hash_table_t *merch_ht)
{
  char *name = ask_question_string("\n Please enter the name of the merchandise you want to add: ");
  ioopm_option_t result = ioopm_hash_table_lookup(merch_ht, str_elem(name));
  if (result.success)
  {
    printf("\n Could not insert %s, due to already existing. \n", name);
    return;
  }
  char *descr = ask_question_string("\n Please a description of the merchandise you want to add: ");
  int price = ask_question_int("\n Please enter the price of the merchandise you want to add: ");
  while(price < 0)
    {
      price = ask_question_int("\n Please enter the price of the merchandise you want to add: \n");
    }
  ioopm_undoable_merch_add(undo_list, merch_ht, strdup(name), strdup(descr), price);
  free(name);
  free(descr);
  return;
}

char ask_question_menu()
{
  char *question = "Choose an action: ";
  char *answer = ask_question_string(question);
  char action = answer[0];
  while(!(valid_menu_action_check(action)))
    {
      free(answer);
      answer = ask_question_string(question);
      action = answer[0];
    }
  free(answer);
  return action;
}

int main()
{
  ioopm_hash_table_t *merch_ht = ioopm_create_merch_ht();
  ioopm_hash_table_t *shelf_ht = ioopm_create_shelf_ht();
  ioopm_hash_table_t *cart_ht = ioopm_create_cart_ht();
  ioopm_list_t *undo_list = undo_list_create();
  int carts_created = 1;
  
  bool quit = false;
  while (quit != true)
    {
      print_menu();
      char action = ask_question_menu();
      switch (action)
	{
	case 'A'://ADD MERCH
	case 'a':
	  {
	    ask_question_add_merch(undo_list, merch_ht);
	    break;
	  }
	case 'R'://REMOVE MERCH
	case 'r':
	  {
	    ioopm_print_merch(merch_ht);
	    char *merch_name = ask_question_string("Which merchandise do you want to remove?");
	    
	    if (ioopm_is_merch_in_a_cart(merch_ht,cart_ht, merch_name ))
	      {
		printf("Merchandise is currently used in a cart, cannot remove %s\n", merch_name);
	     }
	    else
	      {
		if (ioopm_merch_undoable_remove(undo_list, merch_ht, shelf_ht, merch_name))
		  {
		    printf("%s removed successfully\n", merch_name);
		  }
		else
		  {
		    printf("Merchandise %s does not exist\n", merch_name);
		  }
	      }
	    free(merch_name);
	  }
	  break;
	  
	case 'L'://PRINT MERCH
	case 'l':
	  ioopm_print_merch(merch_ht);
	  break;
	  
	case 'E'://EDIT MERCH
	case 'e':
	 {
	   char *merch_name = ask_question_string("Enter the name of the merchandise that you want to edit");
	   if (!(ioopm_is_merch_in_ht(merch_ht, merch_name)))
	     {
	       puts("There is no merchandise with that name");
	       free(merch_name);
	     }
	   else
	     {
	       int number = ask_question_int("Enter 1 to edit name, enter 2 to edit description or enter 3 to edit the price of a merchandise: ");
	       if (number == 1)
		 {
		   ask_question_edit_name(undo_list, merch_ht, cart_ht, merch_name);
		 }
	       else if (number == 2)
		 {
		   ask_question_edit_descr(undo_list, merch_ht, merch_name);
		 }
	       else if (number == 3)
		 {
		   ask_question_edit_price(undo_list, merch_ht, cart_ht, merch_name);
		 }
	       else
		 {
		   puts("Not an allowed action");
		 }
	       free(merch_name);
	     }
	 }
	 break;
	 
	case 'S'://SHOW STOCK FOR MERCH
	case 's': 
	  {
	    ioopm_print_merch(merch_ht);
	    char *merch_name = ask_question_string("Which merchandise do you want to see stock for?");	    
	    ioopm_merch_show_stock(merch_ht, str_elem(merch_name));
	    
	    int total_stock = ioopm_merch_total_stock(merch_ht, merch_name);
	    printf("Total stock:%d\n" ,total_stock);
	    free(merch_name);
	  }
	  break;
	  
	case 'P'://REPLENISH MERCH
	case 'p':  
	  {
	    ioopm_print_merch(merch_ht);
	    char *merch_name = ask_question_string("Which merchandise do you want to replenish?");
	    if (!(ioopm_is_merch_in_ht(merch_ht, merch_name)))
	      {
		printf("%s does not exist\n", merch_name);
	      }
	    else
	      {
		int amount = ask_question_int("By what amount?");
		char *shelf_name;
		do
		  {
		    shelf_name = ask_question_string("On which shelf?");
		  }
		while (!(is_shelf(shelf_name)));
		bool result = ioopm_merch_undoable_replenish(undo_list, merch_ht, shelf_ht, merch_name, amount, shelf_name);
		if (!(result))
		  {
		    puts("That shelf already has another merchandise");
		  }
		free(shelf_name);
		int total_stock = ioopm_merch_total_stock(merch_ht, merch_name);
		printf("\nTotal stock of %s is now %d\n", merch_name, total_stock);
	      }
	    free(merch_name);
	  }
	  break;
	 
	case 'C'://CREATE CART
	case 'c': 
	  {
	    ioopm_merch_undoable_create_cart(undo_list, cart_ht, carts_created);
	    ioopm_print_carts(cart_ht);
	    printf("Cart %d created\n", carts_created);
	    carts_created +=1;
	  }
	  break;
	  
	case 'M'://REMOVE CART
	case 'm': 
	  {
	    ioopm_print_carts(cart_ht);
	    int cart_name = ask_question_int("Which cart do you want to remove?");
	    bool result = ioopm_merch_undoable_remove_cart(undo_list, cart_ht, cart_name);
	    if (!(result))
	      {
		printf("Cart %d does not exist\n", cart_name);
	      }
	  }
	  break;
	  
	case 'D'://ADD TO CART
	case 'd': 
	  ioopm_print_carts(cart_ht);
	  int cart_name = ask_question_int("Which cart do you want to add a merchandise to?");
	  ioopm_option_t result = ioopm_hash_table_lookup(cart_ht, int_elem(cart_name));
	  if (!(result.success))
	    {
	      printf("Cart %d does not exist\n", cart_name);
	    }
	  else
	    {
	      ioopm_print_merch(merch_ht);
	      char *merch_name = ask_question_string("Which merchandise do you want to add?");
	      if (!(ioopm_is_merch_in_ht(merch_ht, merch_name)))
		{
		  puts("That merchandise does not exist");
		}
	      else
		{
		  int total_stock = ioopm_merch_total_stock(merch_ht, merch_name);
		  int total_stock_in_carts = ioopm_total_stock_of_a_merch_in_all_carts(cart_ht,merch_name);
		  printf("Total stock of %s is %d\n Amount not in other carts:%d\n", merch_name, total_stock, total_stock-total_stock_in_carts);
		  if (total_stock-total_stock_in_carts <= 0)
		    {																	puts("No available stock of that merchandise");
		    }
		  else
		    {
		      int amount_to_add = ask_question_int("How many do you want to add?");
		      while(amount_to_add > total_stock-total_stock_in_carts)
			{
			  amount_to_add = ask_question_int("How many do you want to add?");
			}
		      ioopm_merch_undoable_add_to_cart(undo_list, merch_ht, cart_ht, cart_name, merch_name, amount_to_add);
		      ioopm_print_cart_wares(cart_ht, cart_name);
		    }
		}
	      free(merch_name);
	    }
	  break;
	  
	case 'F'://REMOVE FROM CART
	case 'f': 
	  {
	    ioopm_print_carts(cart_ht);
	    int cart_name = ask_question_int("Which cart do you want to remove merchandise from?");
	    ioopm_option_t result = ioopm_hash_table_lookup(cart_ht, int_elem(cart_name));
	    if (!(result.success))
	      {
		printf("Cart %d does not exist\n", cart_name);
	      }
	    else
	      {
		ioopm_print_cart_wares(cart_ht, cart_name);
		char *merch_name = ask_question_string("Which merchandise?");
		int amount_to_remove = ask_question_int("How many?");
		ioopm_merch_undoable_remove_from_cart(undo_list, merch_ht, cart_ht, cart_name, merch_name, amount_to_remove);
		ioopm_print_cart_wares(cart_ht, cart_name);
		free(merch_name);
	      }
	  }
	  break;
	 
	case 'T'://PRINT TOTAL CART COST
	case 't':
	  {
	    ioopm_print_carts(cart_ht);
	    int cart_name = ask_question_int("Which cart do you want to see total cost for?");
	    ioopm_option_t result = ioopm_hash_table_lookup(cart_ht, int_elem(cart_name));
	    if (!(result.success))
	      {
		printf("Cart %d does not exist\n", cart_name);
	      }
	    else
	      {
		ioopm_print_cart_wares(cart_ht, cart_name);
		printf("Cart %d total cost is %d\n",cart_name,ioopm_cart_total_cost(cart_ht, cart_name));
	      }
	  }
	  break;
	  
	case 'O'://CHECKOUT CART
	case 'o':
	  {
	    ioopm_print_carts(cart_ht);
	    int cart_name = ask_question_int("Which cart do you want to checkout?");
	    ioopm_option_t result = ioopm_hash_table_lookup(cart_ht, int_elem(cart_name));
	    if (!(result.success))
	      {
		printf("Cart %d does not exist\n", cart_name);
	      }
	    else
	      {
		ioopm_print_cart_wares(cart_ht, cart_name);
		int total_cost = ioopm_cart_total_cost(cart_ht, cart_name);
		ioopm_undoable_checkout(undo_list, merch_ht, shelf_ht, cart_ht, cart_name);
		printf("Cart %d has checked out with total cost:%d\n", cart_name, total_cost);
	      }
	  }
	  break;
	  
	case 'U'://UNDO
	case 'u':
	  puts("Undo!");
	  if (ioopm_linked_list_size(undo_list) == 0)
	    {
	      puts("Nothing to undo");
	    }
	  else
	    {
	      ioopm_undo_pop(undo_list, merch_ht, shelf_ht, cart_ht);
	    }
	  break;
	  
	case 'Q': //QUIT
	case 'q':
	  quit = true;
	  break;
	}
    }
  ioopm_undo_list_delete(undo_list);
  ioopm_destroy_all_hashtables(merch_ht,shelf_ht,cart_ht);
}



