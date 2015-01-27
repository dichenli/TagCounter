all: tweets

tweets: tweets.c hashtable.o
	clang -o tweets tweets.c hashtable.o -lm

hashtable.o: hashtable.c hashtable.h
	clang -c hashtable.c hashtable.h

hashtable-test: hashtable-test.c hashtable.o
	clang -o hashtable-test hashtable-test.c hashtable.o -lm

clean:
	rm -rf *.o
	
clobber: clean
	rm -rf hashtable