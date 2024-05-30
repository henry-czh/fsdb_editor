
export VERDI_HOME=/store_121/EDA/Verdi_O-2018.09-SP2
commFlags = -I. -I./include -I$(VERDI_HOME)/share/FsdbReader
commFlags += -I./include -I$(VERDI_HOME)/share/FsdbWriter
CFLAGS=${commFlags}
CPPFLAGS=-std=c++11 ${commFlags}

LDFLAGS = -Wall -lz -ldl -lstdc++ -lpthread
LDFLAGS += -L $(VERDI_HOME)share/FsdbReader
LDFLAGS += -L $(VERDI_HOME)share/FsdbWriter

C_NAME = example/zgd
# C_NAME = example/bus_tr_recording

OBJS = $(C_NAME).o
OBJS += $(VERDI_HOME)/share/FsdbReader/linux64/libnsys.so
OBJS += $(VERDI_HOME)/share/FsdbReader/linux64/libnffr.so
OBJS += $(VERDI_HOME)/share/FsdbWriter/linux64/libnffw.so
#OBJS += misc.o

run: $(C_NAME).o
	g++ ${OBJS} -o xxx $(LDFLAGS)


clean:
	rm xxx $(C_NAME).o

