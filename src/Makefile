MCU = atmega328p
F_CPU = 8000000l
FORMAT = ihex 
TARGET = shuttertest

CC = avr-gcc

HEADER = measuring.h display.h stlcd.h glcd.h
SRC = main.c measuring.c display.c stlcd.c glcd.c glcdfont.c
OBJ = $(SRC:.c=.o)

CFLAGS = -g -Os -mmcu=$(MCU) \
-funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums \
-Wall -Wstrict-prototypes \
-DF_CPU=$(F_CPU)

CFLAGS += -std=gnu99

LDFLAGS =

OBJCOPY        = avr-objcopy
OBJDUMP        = avr-objdump

all: hex

depend: .depend

.depend: ${SRC} ${HEADER}
	rm -rf "$@"
	${CC} ${CFLAGS} -MM $^ > "$@"

include .depend

$(TARGET).elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rf *.o $(TARGET).elf $(TARGET).hex

hex:  $(TARGET).hex

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@


