# INA220
Lightweight Arduino library for the `INA220` / `INA220-Q1` / `INA219` current sensor

## Introduction
This library is based off [`SV-Zanshin/INA`](https://github.com/SV-Zanshin/INA) but is rearchitected to be much more lightweight.

Each instance of this library class configures all devices with the same settings, and unlike [`SV-Zanshin/INA`](https://github.com/SV-Zanshin/INA) it doesn't rely on EEPROM to store this information. If you need to configure some sensors differently, create a new instance of this class for each different configuration.
