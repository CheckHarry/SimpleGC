





gc_hash_table:
	cc -c gc_hash_table.c


main: gc_hash_table
	cc main.c -o main.out gc_hash_table.o