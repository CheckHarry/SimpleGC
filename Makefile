





gc_hash_table:
	cc -c gc_hash_table.c

hash_table_test:  hash_table_test.c gc_hash_table
	cc hash_table_test.c -o hash_table_test.out gc_hash_table.o

main: main.c gc_hash_table
	cc main.c -o main.out gc_hash_table.o

all : gc_hash_table hash_table_test main

.PHONY : all