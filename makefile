VPATH=inc:src
CFLAGS=-Wall -O -g
INCLUDES=-I inc
LIBS= -pthread
CC=gcc

# dir for objects
OBJ=obj
# dir for includes
INC=inc
# dir for sources
SRC=src
# dir for bin
BIN=bin

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $(OBJ)/$@

main: hhrt.o req_queue.o main.o protocol.o util.o black_list.o
	cd $(OBJ); \
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o ../$(BIN)/$@ $(LIBS)

clean:
	cd obj && rm *.o
	cd bin && rm main
