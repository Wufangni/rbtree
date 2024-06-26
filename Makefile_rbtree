# Makefile content
CC = gcc
CFLAGS = -Wall -g
OBJ = rbtree.o rbtree-tst.o

all: rbtree-tst

rbtree-tst: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

rbtree.o: rbtree.c rbtree.h
	$(CC) $(CFLAGS) -c rbtree.c

rbtree-tst.o: rbtree-tst.c rbtree.h
	$(CC) $(CFLAGS) -c rbtree-tst.c

clean:
	rm -f *.o rbtree-tst rbtree_performance_time.dat rbtree_performance_rotation.dat rbtree_performance_time.gnuplot rbtree_performance_rotation.gnuplot rbtree_performance_time.png rbtree_performance_rotation.png
