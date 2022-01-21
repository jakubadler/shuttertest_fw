MAKE = make

AVRDUDE_MCU = m328p
AVRDUDE_PROGRAMMER = avrispv2
AVRDUDE_DEVICE = /dev/ttyACM0

all: build

build:
	$(MAKE) -C src

clean:
	$(MAKE) -C src clean

load: build
	avrdude -p $(AVRDUDE_MCU) -P $(AVRDUDE_DEVICE) -c $(AVRDUDE_PROGRAMMER) -B 10 -u -U lfuse:w:0xE2:m
	avrdude -p $(AVRDUDE_MCU) -P $(AVRDUDE_DEVICE) -c $(AVRDUDE_PROGRAMMER) -B 10 -u -U flash:w:src/shuttertest.hex

.PHONY: all build clean load
