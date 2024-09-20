
export VERDI_HOME=/store_121/EDA/Verdi_O-2018.09-SP2
commFlags = -I. -I./include -I$(VERDI_HOME)/share/FsdbReader
commFlags += -I./include -I$(VERDI_HOME)/share/FsdbWriter
CFLAGS=${commFlags}
CPPFLAGS=-std=c++11 ${commFlags}

LDFLAGS = -Wall -lz -ldl -lstdc++ -lpthread -lsqlite3
LDFLAGS += -L $(VERDI_HOME)share/FsdbReader
LDFLAGS += -L $(VERDI_HOME)share/FsdbWriter

#C_NAME = refence
#C_NAME = bus
C_NAME = chi_tr_writer

OBJS = $(C_NAME).o
DLLS += $(VERDI_HOME)/share/FsdbReader/linux64/libnsys.so
DLLS += $(VERDI_HOME)/share/FsdbReader/linux64/libnffr.so
DLLS += $(VERDI_HOME)/share/FsdbWriter/linux64/libnffw.so
#OBJS += misc.o
OBJS += chi_tr_db.o

run: $(OBJS)
	gcc $(OBJS) $(DLLS) -o $(C_NAME) $(LDFLAGS)

clean:
	@-rm  -rf *.o  bus_tr.fsdb* chi_tr_writer novas.* verdiLog
