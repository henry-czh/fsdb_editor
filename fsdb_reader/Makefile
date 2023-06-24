
commFlags=-I. -I/EDA/verdi-2018.9/share/FsdbReader
CFLAGS=${commFlags}
CPPFLAGS=-std=c++11 ${commFlags}

LDFLAGS=-L /EDA/verdi-2018.9/share/FsdbReader -Wall -lz -ldl -lstdc++ -lpthread

#C_NAME = free_api 
C_NAME = read_verilog

OBJS = $(C_NAME).o /EDA/verdi-2018.9/share/FsdbReader/linux64/libnffr.so /EDA/verdi-2018.9/share/FsdbReader/linux64/libnsys.so

run:$(C_NAME).o
	gcc ${OBJS} -o $(C_NAME) $(LDFLAGS)