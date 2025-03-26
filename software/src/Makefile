# paths to microchip tools. might have to be changed if other verision is installed
CC      	= "C:\Program Files\Microchip\xc8\v3.00\bin\xc8-cc.exe"
UPLOADER 	= java -jar "C:\Program Files\Microchip\MPLABX\v6.20\mplab_platform\mplab_ipe\ipecmdboost.jar"
LIBM 		= -mdfp="C:\Program Files\Microchip\MPLABX\v6.20\packs\Microchip\ATtiny_DFP\3.1.260\xc8"
OBJECTCOPY 	= "C:\Program Files\Microchip\xc8\v3.00\avr\bin\avr-objcopy.exe"
MCU     	= attiny402

CFLAGS  	= -gdwarf-3 -mconst-data-in-progmem -mno-const-data-in-config-mapped-progmem \
        	  -funsigned-char -funsigned-bitfields -Wall -O2 -fno-common -mmcu=$(MCU) -mcpu=ATtiny402
LDFLAGS 	= $(LIBM) -Wl,--gc-sections -mmcu=$(MCU)

# List your source files here
SRC     	= main.c TWI_host.c
OBJ     	= $(SRC:.c=.o)
TARGET  	= main.elf
HEXFILE 	= firmware.hex
PORTNUMBER 	= 2025
PROGTOOL 	= TPSNAP

all: hex 

$(TARGET): $(OBJ)
	@echo "linking"
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	@echo "compiling"
	$(CC) $(CFLAGS) -c $< -o $@

hex: $(TARGET)
	@echo "converting elf to hex"
	$(OBJECTCOPY) -O ihex $(TARGET) $(HEXFILE)

run_script:
	@echo "running script"
	py ../dataGenerator.py

upload: all
	@echo "Uploading"
	$(UPLOADER) -$(PROGTOOL) -M -ORIUPDI -P$(MCU) -F$(HEXFILE) -OY$(PORTNUMBER)
# see file:///C:/Program%20Files/Microchip/MPLABX/v6.20/docs/Readme%20for%20IPECMD.htm

end_uploader:
	@echo "killing uploader"
	$(UPLOADER) -OY$(PORTNUMBER) -OQ

clean:
	@echo "cleaning"
	rm -f $(OBJ) $(TARGET) $(HEXFILE)

.PHONY: all clean hex run_script
