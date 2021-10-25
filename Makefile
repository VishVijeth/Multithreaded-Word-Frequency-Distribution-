CC = gcc
CFLAGS = -g -std=c99 -Wvla -Wall -fsanitize=address,undefined

main: main.o WFD_JSD.o WFD_rep.o linked_queue.o
	$(CC) $(CFLAGS) -o $@ $^ -lm

main.o: WFD_JSD.h WFD_rep.h linked_queue.h
WFD_rep.o: WFD_JSD.h

clean:
	rm -f *.o