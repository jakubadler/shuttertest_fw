MCU = atmega328p
F_CPU = 8000000
FORMAT = ihex 
TARGET = shuttertest

CC = avr-gcc

SRC = main.c measuring.c display.c stlcd.c glcd.c glcdfont.c
OBJ = $(SRC:.c=.o)

CFLAGS = -g -Os -mmcu=$(MCU) \
-funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -ffast-math\
-Wall -Wstrict-prototypes \
-DF_CPU=$(F_CPU)

CFLAGS += -std=gnu99

LDFLAGS = -Wl,-u,vfprintf -lprintf_flt

OBJCOPY        = avr-objcopy
OBJDUMP        = avr-objdump

all: hex

$(TARGET).elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rf *.o $(TARGET).elf $(TARGET).hex

hex:  $(TARGET).hex

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@


