CC = gcc
CFLAGS = -Wall -g
OBJ = rbtree.o avlVSrb-tst.o avltree.o

all: avlVSrb-tst

avlVSrb-tst: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

rbtree.o: rbtree.c rbtree.h
	$(CC) $(CFLAGS) -c rbtree.c

avlVSrb-tst.o: avlVSrb-tst.c rbtree.h avltree.h
	$(CC) $(CFLAGS) -c avlVSrb-tst.c

avltree.o: avltree.c avltree.h
	$(CC) $(CFLAGS) -c avltree.c

clean:
	rm -f *.o avlVSrb-tst *.dat *.gnuplot *.png
