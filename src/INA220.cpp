#include "INA220.h"
#include <Wire.h>

INA220::INA220() {
}

uint8_t INA220::begin(uint8_t maxBusAmps, uint32_t microOhmR, const ina_Adc_Mode busAdcMode, const ina_Adc_Mode shuntAdcMode, const ina_Mode deviceMode, uint8_t* deviceAddresses, uint8_t numDevices) {
  /*! @brief     Initializes the contents of the class
      @details   Sets INA Configuration registers for the devices at the specified addresses.
      @param[in] maxBusAmps Integer value holding the maximum expected bus amperage, this value is used to
                 compute each device's internal power register
      @param[in] microOhmR Shunt resistance in micro-ohms, this value is used to compute each device's internal
                 power register
      @param[in] busAdcMode 4-bit mode selector for the Bus ADC (see ina_Adc_Mode enum for possible modes)
      @param[in] shuntAdcMode 4-bit mode selector for the Shunt ADC (see ina_Adc_Mode enum for possible modes)
      @param[in] deviceMode 3-bit device mode selector (see ina_Mode enum for possible modes)
      @param[in] deviceAddresses pointer to array of device addresses
      @param[in] numDevices number of devices enumerated in deviceAddresses array
      @return    Number of INA220 devices found on the I2C bus */

  this->deviceAddresses      = deviceAddresses;
  this->numDevices           = numDevices;
  this->configRegister       = INA220_CONFIG_DEFAULT;                     // Initialize to the INA220 default value
  this->current_LSB          = (uint64_t)maxBusAmps * 1000000000 / 32767; // Get the best possible LSB in nA
  this->power_LSB            = (uint32_t)20 * this->current_LSB;          // Fixed multiplier per device
  this->calibrationRegister  = (uint64_t)409600000 / ((uint64_t)this->current_LSB * (uint64_t)microOhmR / (uint64_t)100000); // Compute calibration register

  /* Determine optimal programmable gain so that there is no chance of an overflow yet with maximum accuracy */
  uint8_t programmableGain;                                               // work variable for the programmable gain
  uint16_t maxShuntmV = maxBusAmps * microOhmR / 1000;                    // Compute maximum shunt millivolts
  if      (maxShuntmV <= 40)  programmableGain = 0;                       // gain  1 for +/- 40mV  range
  else if (maxShuntmV <= 80)  programmableGain = 1;                       // gain /2 for +/- 80mV  range
  else if (maxShuntmV <= 160) programmableGain = 2;                       // gain /4 for +/- 160mV range
  else                        programmableGain = 3;                       // gain /8 for +/- 320mV range (default)
  this->configRegister  = INA220_CONFIG_DEFAULT & ~INA220_CONFIG_PG_MASK; // Zero out the programmable gain bits
  this->configRegister |= programmableGain << INA220_PG_FIRST_BIT;        // Overwrite the new values
  bitSet(this->configRegister, INA220_BRNG_BIT);                          // set to 1 for 0-32 volts

  /* Place ADC mode bits in configuration register */
  this->configRegister &= ~INA220_CONFIG_ADCMODE_MASK;                    // Zero out the ADC mode bits
  this->configRegister |= (busAdcMode & 0xF)   << INA220_BADC_FIRST_BIT;  // Mask off unused bits then shift in the bus ADC mode
  this->configRegister |= (shuntAdcMode & 0xF) << INA220_SADC_FIRST_BIT;  // Mask off unused bits then shift in the shunt ADC mode

  /* Place device mode bits in configuration register */
  this->configRegister &= ~INA_CONFIG_MODE_MASK;                          // Zero out the device mode bits
  this->configRegister |= INA_CONFIG_MODE_MASK & deviceMode;              // Mask off unused bits then shift in the mode settings

  Wire.begin();
  uint8_t availableDevices = resetAll(); // Check if communication is working, then write calibration and configuration registers
  return availableDevices;
}

void INA220::setI2CSpeed(const uint32_t i2cSpeed) {
  /*! @brief     Set a new I2C speed
      @details   I2C allows various bus speeds, see the enumerated type I2C_MODES for the standard speeds. The valid
                 speeds are  100KHz, 400KHz, 1MHz and 3.4MHz. Default to 100KHz when not specified. No range checking
                 is done.
      @param[in] i2cSpeed [optional] changes the I2C speed to the rate specified in Hertz */

  Wire.setClock(i2cSpeed);
}

void INA220::setMode(const uint8_t mode, const uint8_t deviceNumber) {
  /*! @brief     Sets the operating mode from the list given in enumerated type "ina_Mode" for a device
      @param[in] mode Mode (see "ina_Mode" enumerated type for list of valid values
      @param[in] deviceNumber Number of device to configure */

  int16_t configRegister;
  configRegister = readWord(INA_CONFIGURATION_REGISTER, getDeviceAddress(deviceNumber)); // Get the current register from device
  configRegister &= ~INA_CONFIG_MODE_MASK;                                               // zero out the mode bits
  configRegister |= INA_CONFIG_MODE_MASK & mode;                                         // Mask off unused bits then shift in the mode settings
  writeWord(INA_CONFIGURATION_REGISTER,configRegister, getDeviceAddress(deviceNumber));  // Save new value
}

void INA220::setModeAll(const uint8_t mode) {
  /*! @brief     Sets the operating mode from the list given in enumerated type "ina_Mode" for all devices
      @param[in] mode Mode (see "ina_Mode" enumerated type for list of valid values */

  for (int i = 0; i < this->numDevices; i++) {
    setMode(mode, i);
  }
}

uint16_t INA220::getBusMilliVolts(const uint8_t deviceNumber) {
  /*! @brief     Returns the bus voltage in millivolts
      @details   The converted millivolt value is returned
      @param[in] deviceNumber to return the device bus millivolts for
      @return    uint16_t unsigned integer for the bus millivoltage */

  uint16_t busVoltage = getBusRaw(deviceNumber);                             // Get raw voltage from device
  busVoltage          = (uint32_t)busVoltage * INA220_BUS_VOLTAGE_LSB / 100; // conversion to get milliVolts
  return busVoltage;
}

uint16_t INA220::getBusRaw(const uint8_t deviceNumber) {
  /*! @brief     Returns the raw unconverted bus voltage reading from the device
      @details   The raw measured value is returned
      @param[in] deviceNumber to return the raw device bus voltage reading
      @return    Raw bus measurement */

  uint16_t raw = readWord(INA_BUS_VOLTAGE_REGISTER, getDeviceAddress(deviceNumber)); // Get the raw value from register
  raw = raw >> 3; // INA220 - the 3 LSB unused, so shift right
  return(raw);
}

int32_t INA220::getShuntMicroVolts(const uint8_t deviceNumber) {
  /*! @brief     Returns the shunt reading converted to microvolts
      @details   The computed microvolts value is returned
      @param[in] deviceNumber to return the value for
      @return    int32_t signed integer for the shunt microvolts */

  int32_t shuntVoltage = getShuntRaw(deviceNumber);
  shuntVoltage = shuntVoltage * INA220_SHUNT_VOLTAGE_LSB / 10; // Convert to microvolts
  return(shuntVoltage);
}

int16_t INA220::getShuntRaw(const uint8_t deviceNumber) {
  /*! @brief     Returns the raw shunt reading
      @details   The raw reading is returned
      @param[in] deviceNumber to return the value for
      @return    Raw shunt reading */

  int16_t raw = readWord(INA220_SHUNT_VOLTAGE_REGISTER, getDeviceAddress(deviceNumber)); // Get the raw value from register
  return(raw);
}

int32_t INA220::getBusMicroAmps(const uint8_t deviceNumber) {
  /*! @brief     Returns the computed microamps measured on the bus for the specified device
      @details   The computed reading is returned
      @param[in] deviceNumber to return the value for
      @return    int32_t signed integer for computed microamps on the bus */

  int32_t microAmps = (int64_t)readWord(INA220_CURRENT_REGISTER, getDeviceAddress(deviceNumber)) * this->current_LSB / 1000;
  return(microAmps);
}

int32_t INA220::getBusMicroWatts(const uint8_t deviceNumber) {
  /*! @brief     Returns the computed microwatts measured on the bus for the specified device
      @details   The computed reading is returned
      @param[in] deviceNumber to return the value for
      @return    int32_t signed integer for computed microwatts on the bus */

  int32_t microWatts = (int64_t)readWord(INA_POWER_REGISTER, getDeviceAddress(deviceNumber)) * this->power_LSB / 1000;
  return(microWatts);
}

void INA220::triggerConversion(const uint8_t deviceNumber) {
  /*! @brief     Starts a new conversion if the device is in triggered mode
      @details   Writes back the configuration register, triggering shunt, bus, or both conversions based on the device mode
      @param[in] deviceNumber to return the value for */

  int16_t configRegister = readWord(INA_CONFIGURATION_REGISTER, getDeviceAddress(deviceNumber)); // Get the current register
  writeWord(INA_CONFIGURATION_REGISTER, configRegister, getDeviceAddress(deviceNumber)); // Write back to trigger next conversion
}

void INA220::triggerConversionAll() {
  /*! @brief     Starts a new conversion for all devices if they are in triggered mode
      @details   Writes back the configuration register, triggering shunt, bus, or both conversions based on the device mode */

  for (int i = 0; i < this->numDevices; i++) {
    triggerConversion(i);
  }
}

uint16_t INA220::getMode(const uint8_t deviceNumber) {
  uint16_t configRegister = readWord(INA_CONFIGURATION_REGISTER, getDeviceAddress(deviceNumber));
  return configRegister & INA_CONFIG_MODE_MASK;
}

uint8_t INA220::getDeviceAddress(const uint8_t deviceNumber) {
  /*! @brief     Returns a I2C address of the device specified in the input parameter
      @details   Return the I2C address of the specified device, if the number is out of range 0 is returned
      @param[in] deviceNumber to return the device name of
      @return    I2C address of the device. Returns 0 if value is out-of-range */

  if (deviceNumber > this->numDevices) return 0;
  return(this->deviceAddresses[deviceNumber]);
}

uint8_t INA220::reset(const uint8_t deviceNumber) {
  /*! @brief     Performs a software reset for the specified device
      @details   Device is reset to default settings along with any parameters given when begin() was last called
      @param[in] deviceNumber to reset
      @return    uint8_t number of devices that identified as INA220 (0 or 1) */

  uint8_t availableDevices = 0;
  Wire.beginTransmission(getDeviceAddress(deviceNumber));
  uint8_t good = Wire.endTransmission();
  if (good == 0) { // If no I2C error
    writeWord(INA_CONFIGURATION_REGISTER, INA_RESET_DEVICE, getDeviceAddress(deviceNumber));          // Forces INAs to reset
    uint16_t tempRegister = readWord(INA_CONFIGURATION_REGISTER, getDeviceAddress(deviceNumber));     // Read the newly reset register
    if (tempRegister == INA220_CONFIG_DEFAULT) {
      availableDevices = 1; // Responded correctly with INA220 default configuration register value
    }
  }
  initDevice(deviceNumber);
  return availableDevices;
}

uint8_t INA220::resetAll() {
  /*! @brief     Performs a software reset for all devices
      @details   Devices are reset to default settings along with any parameters given when begin() was last called
      @return    uint8_t number of devices that identified as INA220 */

  uint8_t availableDevices = 0;
  for (int i = 0; i < this->numDevices; i++) {
    availableDevices += reset(i);
  }
  return availableDevices;
}

bool INA220::conversionFinished(const uint8_t deviceNumber) {
  /*! @brief     Returns whether or not the conversion has completed
      @details   The device's conversion ready bit is read and returned. "true" denotes finished conversion.
      @param[in] deviceNumber to check */

  uint16_t cvBits = readWord(INA_BUS_VOLTAGE_REGISTER, getDeviceAddress(deviceNumber)) & 2; // Bit 2 set denotes ready
  readWord(INA_POWER_REGISTER, getDeviceAddress(deviceNumber));                             // Resets the "ready" bit
  if (cvBits != 0) return(true); else return(false);
}

bool INA220::waitForConversion(const uint16_t timeout, const uint8_t deviceNumber) {
  /*! @brief     Waits until the conversion for the specified device is finished, or timeout is exceeded
      @param[in] timeout Time to wait in ms before timing out
      @param[in] deviceNumber to reset
      @return    true if successful, false if timeout was exceeded */

  long startMs = millis();
  uint16_t cvBits = 0;
  while (cvBits == 0) {                                                              // Loop until the value is set or timeout is exceeded
    cvBits = readWord(INA_BUS_VOLTAGE_REGISTER, getDeviceAddress(deviceNumber)) & 2; // Bit 2 set denotes ready
    readWord(INA_POWER_REGISTER, getDeviceAddress(deviceNumber));                    // Resets the "ready" bit
    if (millis() - startMs < timeout) {
      return false;
    }
  }
  return true;
}


uint8_t INA220::waitForConversionAll(const uint16_t timeout) {
  /*! @brief     Waits until the conversion for all devices is finished, or timeout is exceeded
      @param[in] timeout Time to wait for each device in ms before timing out
      @return    number of completed conversions */

  uint8_t successCount = 0;
  for (int i = 0; i < this->numDevices; i++) {
    successCount += waitForConversion(timeout, i);
  }
  return successCount;
}

void INA220::initDevice(const uint8_t deviceNumber) {
  /*! @brief     Initializes the given devices using the settings configured with begin()
      @param[in] deviceNumber Device number to explicitly initialize. */

  writeWord(INA_CALIBRATION_REGISTER, this->calibrationRegister, getDeviceAddress(deviceNumber)); // Write calibration value to device register
  writeWord(INA_CONFIGURATION_REGISTER, this->configRegister, getDeviceAddress(deviceNumber));    // Write new value to config register
}

int16_t INA220::readWord(const uint8_t addr, const uint8_t deviceAddress) {
  /*! @brief     Read one word (2 bytes) from the specified I2C address
      @details   Standard I2C protocol is used, but a delay of I2C_DELAY microseconds has been added to let the INAxxx
                 devices have sufficient time to get the return data ready.
      @param[in] addr I2C address to read from
      @param[in] deviceAddress Address on the I2C device to read from
      @return    integer value read from the I2C device */

  Wire.beginTransmission(deviceAddress);       // Address the I2C device
  Wire.write(addr);                            // Send register address to read
  Wire.endTransmission();                      // Close transmission
  delayMicroseconds(I2C_DELAY);                // Delay required for sync
  Wire.requestFrom(deviceAddress, (uint8_t)2); // Request 2 consecutive bytes
  int16_t returnData  = Wire.read();           // Read the msb
  returnData  = returnData << 8;               // Shift the data over 8 bits
  returnData |= Wire.read();                   // Read the lsb
  return returnData;
}

void INA220::writeWord(const uint8_t addr, const uint16_t data, const uint8_t deviceAddress) {
  /*! @brief     Write 2 bytes to the specified I2C address
      @details   Standard I2C protocol is used, but a delay of I2C_DELAY microseconds has been added to let the INAxxx
                 devices have sufficient time to process the data
      @param[in] addr I2C address to write to
      @param[in] data 2 Bytes to write to the device
      @param[in] deviceAddress Address on the I2C device to write to */

  Wire.beginTransmission(deviceAddress); // Address the I2C device
  Wire.write(addr);                      // Send register address to write
  Wire.write((uint8_t)(data >> 8));      // Write the first (MSB) byte
  Wire.write((uint8_t)data);             // And then the second
  Wire.endTransmission();                // Close transmission and actually send data
  delayMicroseconds(I2C_DELAY);          // Delay required for sync
}