/***************************************************************************************************************//*!
* Contact - Nathan Cheek - ncheek@gatech.edu
*
* This project is based largely off of https://github.com/SV-Zanshin/INA
* Both this project and SV-Zanshin/INA are licensed under GNU General Public License v3.0

* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************************************************/
 
#include "Arduino.h"

#ifndef INA220__Class_h
  /*! Guard code definition to prevent multiple includes */
  #define INA220__Class_h

  /*************************************************************************************************************//*!
  * Enumerated list detailing the operating modes of the INA220
  *****************************************************************************************************************/
  enum ina_Mode
  {
    INA_MODE_SHUTDOWN,         ///< Device powered down
    INA_MODE_TRIGGERED_SHUNT,  ///< Triggered shunt, no bus
    INA_MODE_TRIGGERED_BUS,    ///< Triggered bus, no shunt
    INA_MODE_TRIGGERED_BOTH,   ///< Triggered bus and shunt
    INA_MODE_POWER_DOWN,       ///< Shutdown or power-down
    INA_MODE_CONTINUOUS_SHUNT, ///< Continuous shunt, no bus
    INA_MODE_CONTINUOUS_BUS,   ///< Continuous bus, no shunt
    INA_MODE_CONTINUOUS_BOTH   ///< Both continuous, default value
  }; // of enumerated type

  /*************************************************************************************************************//*!
  * Enumerated list detailing the ADC modes of the INA220
  *****************************************************************************************************************/
  enum ina_Adc_Mode
  {
    INA_ADC_MODE_9BIT   = 0x0,
    INA_ADC_MODE_10BIT  = 0x1,
    INA_ADC_MODE_11BIT  = 0x2,
    INA_ADC_MODE_12BIT  = 0x3,
    INA_ADC_MODE_2AVG   = 0x9,
    INA_ADC_MODE_4AVG   = 0xA,
    INA_ADC_MODE_8AVG   = 0xB,
    INA_ADC_MODE_16AVG  = 0xC,
    INA_ADC_MODE_32AVG  = 0xD,
    INA_ADC_MODE_64AVG  = 0xE,
    INA_ADC_MODE_128AVG = 0xF
  }; // of enumerated type

  /*****************************************************************************************************************
  ** Declare constants used in the class                                                                          **
  *****************************************************************************************************************/
  #ifndef I2C_MODES   // I2C related constants
    #define I2C_MODES // Guard code to prevent multiple definitions
    const uint32_t INA_I2C_STANDARD_MODE        =  100000; ///< Default normal I2C 100KHz speed
    const uint32_t INA_I2C_FAST_MODE            =  400000; ///< Fast mode
    const uint32_t INA_I2C_FAST_MODE_PLUS       = 1000000; ///< Really fast mode
    const uint32_t INA_I2C_HIGH_SPEED_MODE      = 3400000; ///< Turbo mode
  #endif
  const uint8_t  INA_CONFIGURATION_REGISTER     =       0; ///< Configuration Register address
  const uint8_t  INA_BUS_VOLTAGE_REGISTER       =       2; ///< Bus Voltage Register address
  const uint8_t  INA_POWER_REGISTER             =       3; ///< Power Register address
  const uint8_t  INA_CALIBRATION_REGISTER       =       5; ///< Calibration Register address
  const uint16_t INA_RESET_DEVICE               =  0x8000; ///< Write to configuration to reset device
  const uint16_t INA_CONVERSION_READY_MASK      =  0x0080; ///< Bit 4
  const uint16_t INA_CONFIG_MODE_MASK           =  0x0007; ///< Bits 0-3
  const uint8_t  INA_DEFAULT_OPERATING_MODE     =     0x7; ///< Default continuous mode
  const uint8_t  INA220_SHUNT_VOLTAGE_REGISTER  =       1; ///< INA220 Shunt Voltage Register
  const uint8_t  INA220_CURRENT_REGISTER        =       4; ///< INA220 Current Register
  const uint16_t INA220_BUS_VOLTAGE_LSB         =     400; ///< INA220 LSB in uV *100 4.00mV
  const uint16_t INA220_SHUNT_VOLTAGE_LSB       =     100; ///< INA220 LSB in uV *10  10.0uV
  const uint16_t INA220_CONFIG_ADCMODE_MASK     =  0x07F8; ///< INA220 Bits 3-6, 7-10
  const uint16_t INA220_CONFIG_PG_MASK          =  0x1800; ///< INA220 Bits 11-12
  const uint16_t INA220_CONFIG_BADC_MASK        =  0x0780; ///< INA220 Bits 7-10
  const uint16_t INA220_CONFIG_SADC_MASK        =  0x0078; ///< INA220 Bits 3-6
  const uint8_t  INA220_BRNG_BIT                =      13; ///< INA220 Bit for BRNG in config register
  const uint8_t  INA220_PG_FIRST_BIT            =      11; ///< INA220 first bit of Programmable Gain
  const uint8_t  INA220_BADC_FIRST_BIT          =       7; ///< INA220 first bit of Bus ADC configuration
  const uint8_t  INA220_SADC_FIRST_BIT          =       3; ///< INA220 first bit of Shunt ADC configuration
  const uint8_t  I2C_DELAY                      =      10; ///< Microsecond delay on I2C writes
  const uint16_t INA220_CONFIG_DEFAULT          =  0x399F; ///< Default register value after reset

  class INA220 {
    public:
      INA220                                ();
      uint8_t     begin                     (uint8_t maxBusAmps, uint32_t microOhmR, const ina_Adc_Mode busAdcMode, const ina_Adc_Mode shuntAdcMode, const ina_Mode deviceMode, uint8_t* deviceAddresses, uint8_t numDevices);
      void        setI2CSpeed               (const uint32_t i2cSpeed = INA_I2C_STANDARD_MODE);
      void        setMode                   (const uint8_t  mode,     const uint8_t deviceNumber);
      void        setModeAll                (const uint8_t  mode);
      uint16_t    getBusMilliVolts          (const uint8_t  deviceNumber);
      uint16_t    getBusRaw                 (const uint8_t  deviceNumber);
      int32_t     getShuntMicroVolts        (const uint8_t  deviceNumber);
      int16_t     getShuntRaw               (const uint8_t  deviceNumber);
      int32_t     getBusMicroAmps           (const uint8_t  deviceNumber);
      int32_t     getBusMicroWatts          (const uint8_t  deviceNumber);
      void        triggerConversion         (const uint8_t  deviceNumber);
      void        triggerConversionAll      ();
      uint16_t    getMode                   (const uint8_t  deviceNumber);
      uint8_t     getDeviceAddress          (const uint8_t  deviceNumber);
      uint8_t     reset                     (const uint8_t  deviceNumber);
      uint8_t     resetAll                  ();
      bool        conversionFinished        (const uint8_t  deviceNumber);
      bool        waitForConversion         (const uint16_t timeout, const uint8_t  deviceNumber);
      uint8_t     waitForConversionAll      (const uint16_t timeout);
    private:
      void        initDevice                (const uint8_t  deviceNumber);
      int16_t     readWord                  (const uint8_t  addr, const uint8_t  deviceAddress);
      void        writeWord                 (const uint8_t  addr, const uint16_t data, const uint8_t deviceAddress);
      uint8_t*    deviceAddresses;
      uint8_t     numDevices;
      uint32_t    current_LSB;
      uint32_t    power_LSB;
      uint16_t    calibrationRegister;
      uint16_t    configRegister;
  };
#endif