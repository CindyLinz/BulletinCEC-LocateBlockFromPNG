all: locate_block

locate_block: locate_block.c
	$(CC) -Wall -O3 -std=gnu99 -o locate_block locate_block.c -lpng
