CFLAGS= -Wall

ftserver: ftserver.c
	gcc -o ftserver ftserver.c $(CFLAGS)
clean:
	rm -f ftserver
