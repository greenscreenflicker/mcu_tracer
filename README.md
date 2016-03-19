# MCU Tracer

## Overview
MCU Tracer is a software tool for embedded hardware debugging. 
It's ment to replace classical serial/printf debugging. 
MCU Tracer allows to readout variables, but also allows to set variables.
The configuration is initially transmitted by the MCU to the PC.
Also the MCU can send classic strings to the PC with messages.

## Supported data types
The following data types are currently supported
*int32_t
*float
*bool

##Tested OS
*Linux Ubuntu
Windows needs to be adjusted and would require some work.

##Getting Started
###Compiling.
Load the git repository, and compile pc software by typing 'make'
###Arduino
Program the system with the arduino code. Configure A0 to an analog voltage, e.g. 3.3V

##Porting
Arduino is used for demonstration proposes. The code should be easily adoptable
to your specific microcontroller system.

##Todo
*Implement Settings Window for startup options
*Implement About Window
