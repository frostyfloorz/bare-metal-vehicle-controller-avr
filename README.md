



# Bare-Metal AVR Vehicle Controller


A bare-metal embedded control project built on the ATmega2560 using direct register-level programming in C.

The system reads a dual-axis joystick, controls a servo for steering, controls a DC motor through an L293D H-bridge, includes a safety interlock, and sends live telemetry to a PC over UART.

The main goal of this project was to build a stronger understanding of low-level embedded systems concepts without using the Arduino framework.



## Features

- Dual-axis joystick input using the ATmega2560 ADC
- Interrupt-driven ADC sampling
- Atomic access to shared ADC data
- Servo position control using Timer1 hardware PWM
- DC motor speed control using Timer2 hardware PWM
- Motor direction control using an L293D H-bridge
- Operator safety interlock
- UART telemetry over USB serial
- Direct register-level hardware configuration
- Joystick scaling and deadzone handling

---

## Hardware

- Arduino Mega 2560 / ATmega2560
- Dual-axis analog joystick
- Servo motor
- DC motor
- L293D H-bridge
- Push button
- Breadboard and jumper wires

---

## Pin Mapping

| Function | Arduino Mega Pin | ATmega2560 Pin / Peripheral |
|---|---:|---|
| Joystick X | A1 | ADC1 |
| Joystick Y | A0 | ADC0 |
| Servo PWM | D11 | PB5 / OC1A |
| Motor PWM | D10 | PB4 / OC2A |
| Motor Direction 1 | D22 | PA0 |
| Motor Direction 2 | D23 | PA1 |
| Safety Button | D12 | PB6 |

---

## System Architecture

The ADC continuously alternates between the joystick X and Y channels.

When a conversion finishes, the ADC interrupt service routine:

1. Reads the ADC result
2. Stores the value in the corresponding shared variable
3. Switches the ADC multiplexer to the other joystick channel
4. Starts the next conversion

The main loop takes an atomic snapshot of both ADC values before processing them.

The joystick values are:

Raw ADC
   ↓
Centered around 0
   ↓
Scaled to -100 to +100
   




   

   ## Project Evolution

This project changed significantly as I learned more about the ATmega2560 and embedded systems.

### Initial Version

The first version used:

- Blocking ADC reads
- Basic joystick scaling
- Servo PWM control
- Motor PWM control
- Direct GPIO control for motor direction
- A simple safety button

The ADC was originally read using polling, which is where the CPU would start a conversion and then wait until it finished before continuing.

### Improvements Made

As the project developed, I changed the design to include:

- Interrupt-driven ADC sampling instead of blocking polling
- Automatic switching between ADC0 and ADC1 inside the ADC ISR
- `volatile` shared variables between the ISR and main loop
- Atomic snapshots of 16-bit ADC values to prevent torn reads
- Hardware PWM using Timer1 for the servo
- Hardware PWM using Timer2 for motor speed control
- L293D H-bridge direction control
- A defined safe state for the motor and servo
- UART serial communication for live telemetry
- Integer and string UART helper functions
- Telemetry rate limiting to avoid constantly blocking the control loop
- Cleaner separation between hardware initialization, input processing, and output control

The final version is much less dependent on blocking CPU operations and makes greater use of the microcontroller's hardware peripherals.

---

## What I Learned

Before this project, I had little understanding of C programming and had even less experience with how software directly controls microcontroller hardware.

Through this project I learned:
- How memory-mapped hardware registers are used to control peripherals
- How bitwise operations are used to configure individual register bits
- How GPIO direction and output registers work
- How an ADC converts an analog voltage into a digital value
- How one ADC can switch between multiple analog input channels using a multiplexer
- The difference between polling and interrupt-driven peripherals
- How interrupt service routines work
- Why variables shared with an ISR may need `volatile`
- Why multi-byte shared values can require atomic access on an 8-bit processor
- How hardware timers count independently of the CPU
- How timer prescalers, TOP values, and compare registers determine PWM behavior
- How PWM can control servo position and DC motor power
- How an H-bridge controls motor direction
- How UART transmits data one byte at a time
- How baud rate determines serial communication speed
- How integers must be converted to characters before being displayed in a serial terminal
- How blocking communication can affect a control loop
- How embedded systems require thinking about both software and physical hardware at the same time
- How important pointers are in C
- How Frustrating it is to read a 200 page document to find one hardware peripheral 

One of the biggest takeaways from the project was learning to think of peripherals such as the ADC, timers, and UART as independent pieces of hardware that the CPU configures and interacts with, rather than as normal C functions.





Quick recording of the test cases: moving the joystick in different directions controlled the hardware while UART telemetry sent the system data to my PC.


https://github.com/user-attachments/assets/0682a205-733a-408a-8a10-6ee822620fb4


Prototype hardware setup for the vehicle controller. The project focuses on the control system—the “brains” of the vehicle so I kept the mechanical build minimal rather than turning it into a full RC car.

<img width="2880" height="2160" alt="image" src="https://github.com/user-attachments/assets/075de384-f06a-4574-ad10-98c87d3162fb" />







Deadzone applied
   ↓
Servo / motor commands


--Side note
This project taught me so much about C ,GPIOs ,Timers , interrupts, bit manipulation etc. It was my first project on embedded systems and it had a ton of ups and downs. But i'm so happy to say I completed it. Programming hardware & software was very rewarding. Thank you for checking out my project and I hope you keep your eyes out for future projects as I love programming so i will always be working on something. 
---
