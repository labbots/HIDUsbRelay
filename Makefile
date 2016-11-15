USBFLAGS=   `libusb-config --cflags`
USBLIBS=    `libusb-config --libs`

CC=	gcc
CFLAGS=	-O -Wall $(USBFLAGS)
LIBS=	$(USBLIBS)

OBJ=	usbrelay.o hiddata.o
PROGRAM=usbrelay

all: $(PROGRAM)

$(PROGRAM): $(OBJ)
	$(CC) -o $(PROGRAM) $(OBJ) $(LIBS)

strip: $(PROGRAM)
	strip $(PROGRAM)

clean:
	rm -f $(OBJ) $(PROGRAM)

.c.o:
	$(CC) $(ARCH_COMPILE) $(CFLAGS) -c $*.c -o $*.o
