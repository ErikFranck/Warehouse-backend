# MAKEFILE #

FLAG = gcc

main:  hash_table.c linked_list.c iterator.c utils.c hash_table.h linked_list.h iterator.h common.h utils.h frontend.c frontend.h backend.c backend.h carts.c carts.h
				$(FLAG) hash_table.c linked_list.c iterator.c utils.c frontend.c backend.c carts.c -Wall -g -pedantic

tests:  hash_table.c linked_list.c iterator.c utils.c hash_table.h linked_list.h iterator.h common.h utils.h backend.c backend.h carts.c carts.h
				$(FLAG) hash_table.c linked_list.c iterator.c utils.c backend.c carts.c backend_tests.c -Wall -o tests -lcunit -std=c11

backend: utils.h hash_table.h linked_list.h common.h iterator.h
				$(FLAG) backend.c hash_table.c linked_list.c iterator.c -Wno-unused-function -Wall -c

frontend: hash_table.h common.h iterator.h utils.h 
				$(FLAG) frontend.c hash_table.c iterator.c linked_list.c utils.c -Wall

carts: hash_table.h common.h iterator.h utils.h 
				$(FLAG) carts.c hash_table.c iterator.c linked_list.c utils.c -Wall -c -Wno-unused-function


run_tests: backend_tests  
			        valgrind --leak-check=full ./backend_tests

run: a.out  
			        valgrind --leak-check=full  ./a.out 

runt: a.out  
			        valgrind --leak-check=full ./a.out<test.txt

clear:
				rm -f *.o *.gcov *.gcda *.gcno *.out *tests *.info run profile.txt && rm -R all-lcov
