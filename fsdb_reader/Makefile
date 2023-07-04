
commFlags=-I. -I./include -I/EDA/verdi-2018.9/share/FsdbReader
CFLAGS=${commFlags}
CPPFLAGS=-std=c++11 ${commFlags}

LDFLAGS=-L /EDA/verdi-2018.9/share/FsdbReader -Wall -lz -ldl -lstdc++ -lpthread

#C_NAME = free_api 
C_NAME = fsdb_reader

OBJS = main.o $(C_NAME).o /EDA/verdi-2018.9/share/FsdbReader/linux64/libnffr.so /EDA/verdi-2018.9/share/FsdbReader/linux64/libnsys.so
OBJS += misc.o

run:main.o $(C_NAME).o misc.o
	gcc ${OBJS} -o $(C_NAME) $(LDFLAGS)


clean:
	rm *.o