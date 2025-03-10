[comment]: <> "LTeX: language=en-GB"

# NFC message switcher

This project is an implementation of an NFC tag which is able to change the message. It communicates to a host device. It was developed to be quickly assembled and easy to use but still be a bit more than a blinking light. It uses the reference implementation for the [bare metal TWI driver](https://github.com/microchip-pic-avr-examples/attiny1627-bare-metal-twi-mplab).
![Isometric view](/pictures/render_ismoetric.png)

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

> Note a release is not yet available. It will be provided soon.

## Sourcing

The following parts are needed to recreate this Project (as well as the PCB):

| Reference 	|   Value   	| Qty 	|    Digikey number    	|
|:---------:	|:---------:	|:---:	|:-------------------:	|
|   C1,C3   	|   0.1uF   	|  2  	|    1276-1000-1-ND   	|
|     C2    	|    47uF   	|  1  	|    1276-3063-2-ND   	|
|   R1,R2   	|    4k7    	|  2  	|    MMA-4.7KACT-ND   	|
|     U1    	| ATtiny402 	|  1  	| ATTINY402-SSNRCT-ND 	|
|     U2    	| ST25DV04K 	|  1  	|    497-17123-1-ND   	|

## Assembly

There are no special requirements for the assembly of the PCB's.

## Software

You can use the provided released hex file, however it is recommended to customize the software to your own taste. 
The provided jupyter notebook can be used to generate custom messages. 
You then have to add a new case to the main code.
```c
case 4: // "your custum message. increment accordingly"
    pData = &dat5[0]; // main output of the jupyter notebook
    datalen = 34;     // length of message (jupyter notebook tells you this)
    break;
```
To compile this code you need the XC8 compiler from microchip.
A snap programmer or similar is also needed to upload to the ATtiny.

# Caveat

It must be noted that the current design has a (obvious) flaw. 
The ATtiny is in the middle of the coil and *probably* quite affected by the activity of it. 
When combined with the nonideal power delivery method, this can significantly impair functionality at times (The I2C content that is written gets verified but I2C has no error correcting method).
Some more analysis in that regard is not yet made.
