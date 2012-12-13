# Aduino Based RFID Authorization System

This system makes use of a few components.

* An Arduino Uno
* An Arduino Ethernet Shield
* Sonmicro's SM130 RFID module
* Sparkfun's SM130 Breakout Board with Antenna
* Server with Ruby

Also required are the Sinatra and JSON ruby gems.

The Sparkfun board is not required.  Although without it, you will need to create your own 13.56 MHz antenna.

Be advised that these Arduino components use more power than what a standard USB port will provide.  However, you can get away with using the wall wart for a normal USB based phone charger.

To get this up and running, simply edit line 26 of **scanner.ino** to point to your server and upload to the Arduino.

Finally, run:

    ruby rfid.rb

Your server should be configured to reverse proxy requests to port *55555*.  Alternatively, you could change the port specified in **rfid.rb** to *80*, and run the script as root.  (Root access is required for Sinatra to listen on ports under 1000)
