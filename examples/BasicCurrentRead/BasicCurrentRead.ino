#include <INA220.h>

/******************
 * Begin Configure
 ******************/
const uint8_t NUM_INA = 2; // 2 INA devices
const uint8_t MAX_CUR = 5; // 5 Amps
const uint16_t SHUNT_R = 20000; // 20 mOhm
uint8_t ina_addresses[NUM_INA] = {0x40, 0x41}; // INA I2C addresses
/******************
 * End Configure
 ******************/

INA220 ina220;

void setup() {
  Serial.begin(115200);
  uint8_t availableDevices = ina220.begin(MAX_CUR, SHUNT_R, INA_ADC_MODE_128AVG, INA_ADC_MODE_128AVG, INA_MODE_CONTINUOUS_BOTH, ina_addresses, NUM_INA);
  Serial.print("Configured "); Serial.print(availableDevices); Serial.print(" of "); Serial.print(NUM_INA); Serial.println(" INA220 current sensors");
  delay(100);
}

void loop() {
  for (int i = 0; i < NUM_INA; i++) {
    float vol = ina220.getBusMilliVolts(i) / 1000.0;
    float cur = ina220.getBusMicroAmps(i) / 1000.0;
    float power = ina220.getBusMicroWatts(i) / 1000.0;
    Serial.print("INA at 0x"); Serial.print(ina_addresses[i], HEX); Serial.print(" measures "); Serial.print(vol); Serial.print(" V, ");
    Serial.print(cur); Serial.print(" mA, and "); Serial.print(power); Serial.println(" mW");
  }
  Serial.println();
  delay(1000);
}