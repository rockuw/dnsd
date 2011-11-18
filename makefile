VPATH=inc:src
CFLAGS=-Wall -O -g
INCLUDES=-I inc
LIBS=-lrt -pthread
CC=gcc

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o obj/$@

main: hhrt.o req_queue.o main.o protocol.o util.o black_list.o
	$(CC) $(CFLAGS) $(INCLUDES) obj/hhrt.o obj/req_queue.o obj/main.o obj/protocol.o obj/util.o obj/black_list.o -o bin/$@ $(LIBS)

clean:
	cd obj && rm *.o
	cd bin && rm main
