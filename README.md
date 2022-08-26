# ARFPC (Arduino Returnless Fuel Pressure Control)
Use Arduino UNO and 1602 keypad Shield with AEM 100 PSI pressure transducer

Motivations:
I've converted my carb engine to Speeduino EFI, I want to keep my original single fuel pipe without the extra fuel return pipe, and learnt returnless fuel control is what I need, there's lack of returnless fuel pressure control soltuions on the market, so I decided to make my own using the off-the-shelf components with little modifications to make it works, has been using it for more than 6 months, no problem so far!

Why returnless? 
Not only it eliminates the need for the fuel return pipe and FPR (Fuel pressure regulator), only spin up when needed, actually in most cases during idle, is needs less then half of the full power, therefore, save power, pump runs cooler, and wears, means happier pump and longer lifetime!
Also the ability to control the target pressure on the fly is good for testing the EFI system.

Arduino library requirements:
1. PID control: https://playground.arduino.cc/Code/PIDLibrary/
2. 1602 LCD screen: https://www.arduino.cc/reference/en/libraries/liquidcrystal/

![Screen Shot 2022-08-26 at 5 29 44 PM](https://user-images.githubusercontent.com/138415/186874065-231bd1b5-342d-4cec-a15f-e32cb5af36ea.jpg)

Hardware preparations:
1. Arduino UNO board
2. 1602 Keypad shield
3. 20A PWM board
4. 100 PSI transducer, AEM or others

![Screen Shot 2022-08-26 at 5 23 38 PM](https://user-images.githubusercontent.com/138415/186872809-51ec57ff-8b16-4b07-a1e6-9b94b17c5a8e.jpg)

Pins:
11-Fuel pump control pin to the 20A PWM board
A1-Pressure sensor pin to the transducer
3-Optional, for ECU logging, minic the transducer input voltage reading

Modify the 20A PWM board refer to Robojax's Hack: modify 20A DC 10-60V PWM Motor Speed Controller and control with Arduino [Youtube](https://www.youtube.com/watch?v=ipFxYQkB5uw&t=21s)

![Screen Shot 2022-08-26 at 5 07 57 PM](https://user-images.githubusercontent.com/138415/186875973-fe351e17-5164-48ed-9325-ffd8a09d074b.jpg)
Main screen: 
Upper left: RAW reading from the pressure transducer
Upper right: Current pressure coverted to PSI
Lower left: ADC to send to the PWM board
Lower right: PSI target

Press SELECT once, adjust the LCD brightness (UP/DOWN)
press SELECT again, adjust the target pressure (UP/DOWN)
press SELECT again to return to the main screen

Override mode: the pump continuously to pump at 130 ADC level regardless what transducer is reading, to enter this mode, press both RIGHT button and RST at the same time, hold RIGHT until the screen displays "Overrided mode", it is usefuel to draw the fuel out of the tank, or in case if the transducer failed, from the test, it is still driveable at low RPM, this allows driver to move vehicle to the safer location.

Failsafe timer: when the pump is continuously pump at the highest level (currently set to 200 ADC) more than 7 seconds, it will stop and show "Check pressure!", this is to prevent when transducer failed to read (always at low), or fuel leaks from accident or untighten pipings.
