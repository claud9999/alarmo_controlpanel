# Alarmo Control Panel

This project is Arduino source code for programming an Inkplate 6 Plus to be
a control panel to arm/disarm your Home Assistant Alarmo system.

## What is Arduino?

[Arduino](https://arduino.cc) is an IDE and a C++-like programming language for
microcontrollers. This project uses the Arduino IDE to compile and deploy
the firmware on your Inkplate 6 Plus. See [the instructions](https://inkplate.readthedocs.io/en/latest/get-started.html#arduino).

## What is Inkplate 6 Plus?

[Inkplate 6 Plus](https://www.crowdsupply.com/soldered/inkplate-6plus)
is an open hardware eInk board with an ESP32 microcontroller
and a recycled eInk touchscreen display (alas, the display hardware is *not*
open design.)

## What is Home Assistant?

[Home Assistant](hass.io) is an open source home automation suite.

## What is Alarmo for Home Assistant?

[Alarmo](https://github.com/nielsfaber/alarmo) is an integration for Home
Assistant that provides a "software" alarm system.

## How to Use?

Load the Arduino IDE on your computer. Follow the Inkplate instructions to
install the board definition and library. Open this project in the Arduino
IDE, configure the Wifi and MQTT settings  and compile and install on your
Inkplate.

## Future features

* Pick random image from folder
* Portrait orientation
* make display into a clock/ rtc and ntp?
* Wifi network selection?
