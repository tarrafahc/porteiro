PROG = porteiro

all: $(PROG).hex

CC      = avr-gcc
OBJCOPY = avr-objcopy
CFLAGS  = -mmcu=atmega328p -std=c99 -Wall -O2 -ffreestanding
LDFLAGS = -Wl,--relax

%.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

$(PROG).elf: $(PROG).o fonte.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(PROG).hex: $(PROG).elf
	$(OBJCOPY) -j .text -j .data -O ihex $^ $@

$(PROG)_eeprom.hex: $(PROG).elf
	$(OBJCOPY) -j .eeprom -O ihex $^ $@

clean::
	rm -f *~ $(PROG).hex $(PROG)_eeprom.hex $(PROG).elf $(PROG).o fonte.o

upload: $(PROG).hex
	avrdude -c arduino -p m328p -P /dev/ttyUSB0 -b 57600 -u -U flash:w:$(PROG).hex

upload_eeprom: $(PROG)_eeprom.hex
	avrdude -c arduino -p m328p -P /dev/ttyUSB0 -b 57600 -u -U eeprom:w:$(PROG)_eeprom.hex
