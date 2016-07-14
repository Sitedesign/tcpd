# For debugging...
#CFLAGS=-D_REENTRANT -Wall -ggdb -DDEBUG

CFLAGS32=-D_REENTRANT -Wall -O6 -m32
CFLAGS64=-D_REENTRANT -Wall -O6 -m64
LIBS=-lpthread

all: tcpd32 tcpd64

OBJECTS32 = $(patsubst %.c, %.o32, $(wildcard *.c))
OBJECTS64 = $(patsubst %.c, %.o64, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o32: %.c $(HEADERS)
	gcc $(CFLAGS32) -c $< -o $@

%.o64: %.c $(HEADERS)
	gcc $(CFLAGS64) -c $< -o $@

tcpd32: $(OBJECTS32)
	gcc $(CFLAGS32) $(OBJECTS32) $(LIBS) -o $@
	strip $@
	rm -f *~ *.o32

tcpd64: $(OBJECTS64)
	gcc $(CFLAGS64) $(OBJECTS64) $(LIBS) -o $@
	strip $@
	rm -f *~ *.o64

clean:
	rm -f *~ *.o32 *.o64 tcpd32 tcpd64
