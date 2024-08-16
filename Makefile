CC = gcc
CFLAGS = -Wall -pthread
LIBS = -L. -lmsocket

libmsocket.a: msocket.o
	ar rcs libmsocket.a msocket.o

msocket.o: msocket.c msocket.h
	$(CC) $(CFLAGS) -c msocket.c

initmsocket: initmsocket.o libmsocket.a
	$(CC) $(CFLAGS) -o initmsocket initmsocket.o $(LIBS)

initmsocket.o: initmsocket.c msocket.h
	$(CC) $(CFLAGS) -c initmsocket.c

user1: user1.o libmsocket.a
	$(CC) $(CFLAGS) -o user1 user1.o $(LIBS)

user1.o: user1.c msocket.h
	$(CC) $(CFLAGS) -c user1.c

user2: user2.o libmsocket.a
	$(CC) $(CFLAGS) -o user2 user2.o $(LIBS)

user2.o: user2.c msocket.h
	$(CC) $(CFLAGS) -c user2.c


clean:
	rm -f libmsocket.a msocket.o initmsocket initmsocket.o user1 user1.o user2 user2.o
