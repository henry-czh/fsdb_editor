
commFlags=-I. -I./include -I/EDA/verdi-2018.9/share/FsdbReader
CFLAGS=${commFlags}
CPPFLAGS=-std=c++11 ${commFlags}

LDFLAGS= -Wall -Werror -lsqlite3
OBJS=main.o

run: ${OBJS}
	gcc ${OBJS} $(LDFLAGS)


clean:
	rm *.o