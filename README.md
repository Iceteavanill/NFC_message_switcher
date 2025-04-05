[comment]: <> "LTeX: language=en-GB"

# NFC message switcher

This project is an implementation of an NFC tag which is able to change the message. It communicates to a host device. It was developed to be quickly assembled and easy to use but still be a bit more than a blinking light. It uses the reference implementation for the [bare metal TWI driver](https://github.com/microchip-pic-avr-examples/attiny1627-bare-metal-twi-mplab).
![PCB render](/pictures/render_blue.png)

## Main principle of operation

The main device is the [ST25DV04K](https://www.st.com/resource/en/datasheet/st25dv04k.pdf). 
It can communicate via NFC and I2C. 
When an NFC capable device is held near the PCB, the ST25DV04K starts to receive power and starts communicating with the device. 
The Power that's transmitted is shared (energy harvesting mode) with a [ATtiny402](http://ww1.microchip.com/downloads/en/DeviceDoc/ATtiny202-402-AVR-MCU-with-Core-Independent-Peripherals_and-picoPower-40001969A.pdf). 
When powered it starts communicating via I2C and writes to the user accessible memory. 
When the written data has been confirmed, it powers down.

## Details

### Power

Power provider is the energy harvesting mode of the ST25DV04K, the power source is a NFC capable device that is in close proximity of the PCB.
It is buffered via a 47μF capacitor.
Because of that, it is essential to keep the power consumption of the ATtiny as low as possible.
Consult the source code for more details.

### Link arbitration

The ST25DV04K does only support I2C *or* NFC active at the same time. 
However, it has an open drain output that can be used to sense the current activity. 
Low, signals that the device is currently busy. 
The ATtiny then waits for the device to be available, by waiting in sleep. 
A hardware interrupt is not yet implemented.

### Device configuration

The ST25DV04K needs some configuration for correct operation. 
The energy harvesting mode must be enabled/forced at startup. 
Also, the GPO mode must be set to RF_ACTIVITY_EN to enable better link arbitration.
The configuration happens after the ATtiny has been programmed once.

### Antenna

The antenna / coil design is based on a [paper](https://www.researchgate.net/publication/339137261_Inductance_Formula_for_Rectangular_Planar_Spiral_Inductors_with_Rectangular_Conductor_Cross_Section) from Hubert Aebischer. 
The ST25DV04K has an internal tuning capacitor which should be assumed to be 29pF.
Based on that, the inductance should be 4.75μH. 

### Formfactor

The PCB dimensions are the same as a credit card.
It is however quite a bit thicker.

# Building your own

All required files to recreate this project can be found [here](https://github.com/Iceteavanill/NFC_message_switcher/releases/tag/1.0.0). 


## Sourcing

The following parts are needed to recreate this Project (as well as the PCB):

| Reference 	|   Value   	| Qty 	|    Digikey number    	|
|:---------:	|:---------:	|:---:	|:-------------------:	|
|   C1, C3   	|   0.1uF   	|  2  	|    1276-1000-1-ND   	|
|     C2    	|    47uF   	|  1  	|    1276-3063-2-ND   	|
|   R1, R2   	|    4k7    	|  2  	|    MMA-4.7KACT-ND   	|
|     U1    	| ATtiny402 	|  1  	| ATTINY402-SSNRCT-ND 	|
|     U2    	| ST25DV04K 	|  1  	|    497-17123-1-ND   	|

## Assembly

There are no special requirements for the assembly of the PCB's. 
If you are used to soldering SMD components it shouldn0t be a problem. 

## Software

You can use the provided released hex file, however it is recommended to customize the software to your own taste. 

There are multiple options to customize the software

### Overview

Customizing the software always requires a rebuild of the firmware of the Attiny. 
The XC8 compiler from microchip is recommended. 
The following flags should be used:

| Flags    | commands                                                                   |
|----------|----------------------------------------------------------------------------|
| Global   | -gdwarf-3 -mconst-data-in-progmem -mno-const-data-in-config-mapped-progmem |
| compiler | -funsigned-char -funsigned-bitfields -Wall -O2 -fno-common                 |
| linker   | "libm" -Wl,--gc-sections                                                   |

A makefile is already prepared with all the required flags. 
If you install a later version you might have to alter the paths a little.

For customization, only a header file has to be altered (```nfc_messages.h```).  
There are multiple ways of accomplishing that:

 * editing the header directly (not recommended)
 * using the python script
 * using the gui (recommended)

### Editing the header directly

Editing the header directly is not recommended because the required data must be in the NDEF format. 
Manually generating that is very tedious. 
If you have special needs however, like encoding special data types, you have to do it this way because anything else has not been implemented.

### Using the pyton script

The ```dataGenerator.py``` python script can be used to quickly generate custom messages. 
It requires the NDEFLib python library. 
Edit the ```message_as_text_list``` list with your messages and then run the script:
```py
message_as_text_list = ["Test 1",
                        "Test 2 a bit longer",
                        "Test 3 even looooooooooooooooooooooooooonger",
                        "https://www.youtube.com/watch?v=dQw4w9WgXcQ",
                        "spotify:track:4cOdK2wGLETKBW3PvgPWqT"
]
```
### Using the gui

A gui has been made to quickly program multiple NFC tags. 
It requires WXpython and uses the previously mentioned script and the ```message_as_text_list```.

Programming steps

1. Step:

Enter your desired message(s) in the tabele at the top. 
Links to websites videos or Spotify songs are also allowed.

![picture of the table](/pictures/manual_table.png)

2. Step:

If your main device is an iPhone check the ```iPhone compatability mode```. 
IPhones are unable to read text from a NFC device, and the text messages have to be encoded as a website.

3. Step:

Some default messages are added to the list automatically. If you dont want that, you can disable that with the checkbox ```Remove predefined texts```. 
The ```message_as_text_list``` is used as a source.

4. Step:

Connect your programmer to the tag and press program device. 
The build and upload steps are automatically executed.

A snap programmer or similar is needed to upload to the ATtiny.

## Batch programming

Many devices in a row can be programmed with the programming jig:

![programming jig](/pictures/programming%20jig.png)

It was designed in Onshape. 
I used pogopins that I had on hand. 
You probably want to customize it to your current stock. 
You find the Onshape document [here](https://cad.onshape.com/documents/450623688917edcaa975b9e7/w/39588c936aa9381f65fac7c0/e/be57d568f84f27791dc539d2?renderMode=0&uiState=67cfd948ca833f6780730f6a).

# Caveat

It must be noted that the current design has a (obvious) flaw. 
The ATtiny is in the middle of the coil and *probably* quite affected by the activity of it. 
When combined with the nonideal power delivery method, this can significantly impair functionality at times (The I2C content that is written gets verified but I2C has no error correcting method).
Some more analysis in that regard is not yet made.
