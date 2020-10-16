
# Arduino code for the Flux Capacitor

This code is running on an Arduino Pro Mini 168, 16MHz, 5V
It should also work on Arduino Uno or Nano


How the Arduino pins are connected:

Pin(s)       connected to: 

TX/RX        RX/TX of DFPlayer Mini (via 1k resistors)

2            "busy" signal from DFPlayer Mini

3,5,6,9      transistors to Y-bar LEDs (from outer to inner) via R=2k2

10           transistor to the 4x2 LEDs in the sides of the enclosure

11           transistor to the center LED

A2,A3        push buttons to set volume and mode of operation


uses sound files on microSD card in folders 01,02,03,07,08,09

1:  8 clips   constant noise  (4s - 39s) 

2: 14 clips   longer sparking noise (aggressive)   (4-33s)   

3: 18 clips   short noise (1-3s)

7:  4 clips   BTTF  3x"bling" + "88mph, serious..."

8:  4 clips   announce 4 modes

9:  4 clips   announce 4 volume levels

The first push button is used to set the volume in four levels [0-3]
(level 0 is "silent")

Easter egg: if both push buttons are pressed at startup, it plays
the sound clip "... 88 miles per hour, you see some serious shit"


