# INA220
Lightweight Arduino library for the `INA220` / `INA220-Q1` / `INA219` current sensor

## Introduction
This library is based off [`SV-Zanshin/INA`](https://github.com/SV-Zanshin/INA) but is rearchitected to be much more lightweight.

Each instance of this library class configures all devices with the same settings, and unlike [`SV-Zanshin/INA`](https://github.com/SV-Zanshin/INA) it doesn't rely on EEPROM to store this information. If you need to configure some sensors differently, create a new instance of this class for each unique configuration.

## Breakout Board
An example breakout board is available under [`breakout-pcb`](breakout-pcb). EAGLE files are included, along with Gerber files and a schematic PDF. The Gerber .zip file includes a Bill of Materials and SMD Pick and Place files.

<img src="https://github.com/nathancheek/INA220/raw/master/breakout-pcb/INA220%20Current%20Sensor%20Breakout%20REV%20A.brd.top.png" width="250" title="Breakout Board Top Image (Render by OSH Park)">

The board can be manufactured by uploading the Gerber .zip file to OSH Park, JLCPCB, or another PCB fabrication website.
