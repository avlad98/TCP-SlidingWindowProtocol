all: send recv

link_emulator/lib.o:
	$(MAKE) -C link_emulator

send: send.o link_emulator/lib.o pack.h
	gcc -g -Werror send.o link_emulator/lib.o -o send

recv: recv.o link_emulator/lib.o pack.h
	gcc -g -Werror recv.o link_emulator/lib.o -o recv

.c.o: pack.h
	gcc -Wall -Werror -g -c $?

clean:
	$(MAKE) -C link_emulator clean
	-rm -f *.o send recv
