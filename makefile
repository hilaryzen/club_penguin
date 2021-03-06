ifeq ($(DEBUG),true)
	CFLAGS = -g
endif

all: server.o client.o sema.o err.o processor.o render.o windows.o
	gcc $(CFLAGS) -o server server.o sema.o err.o processor.o
	gcc $(CFLAGS) -o client client.o render.o err.o windows.o -lncurses

server.o: server.c server.h sema.h err.h processor.h packets.h
	gcc $(CFLAGS) -c server.c
client.o: client.c client.h packets.h err.h
	gcc $(CFLAGS) -c client.c

processor.o: processor.c processor.h sema.h packets.h windows.h
	gcc $(CFLAGS) -c processor.c
render.o: render.c render.h
	gcc $(CFLAGS) -c render.c
windows.o: windows.c windows.h
	gcc $(CFLAGS) -c windows.c

sema.o: sema.c sema.h
	gcc $(CFLAGS) -c sema.c
err.o: err.c err.h
	gcc $(CFLAGS) -c err.c

clean:
	rm server client *.txt *.o *~ \#* 
