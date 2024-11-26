MAKE = make

AVRDUDE_MCU = m328pb
#AVRDUDE_PROGRAMMER = avrispv2
AVRDUDE_PROGRAMMER = usbasp
AVRDUDE_DEVICE = /dev/ttyACM0

all: build

build:
	$(MAKE) -C src

clean:
	$(MAKE) -C src clean

load: build
	avrdude -p $(AVRDUDE_MCU) -P $(AVRDUDE_DEVICE) -c $(AVRDUDE_PROGRAMMER) -B 10 -U lfuse:w:0xFF:m
	avrdude -p $(AVRDUDE_MCU) -P $(AVRDUDE_DEVICE) -c $(AVRDUDE_PROGRAMMER) -B 10 -U flash:w:src/shuttertest.hex

.PHONY: all build clean load
