#include <stdio.h>
#include <INA220.h>

/******************
 * Begin Configure
 ******************/

const uint8_t NUM_INA = 4; // 4 INA devices
const uint8_t MAX_CUR = 4; // 4 Amps minimum with 0.010 ohm shunt to avoid overflow in calibration register!
const uint32_t SHUNT_R = 10000; // 10 mOhm, expressed in microOhms
const uint8_t V12_1 = 0x40;
const uint8_t V12_2 = 0x41;
const uint8_t V5_1 = 0x42;
const uint8_t V5_2 = 0x43;

uint8_t ina_addresses[NUM_INA] = {V12_1,V12_2,V5_1,V5_2}; // INA I2C addresses

/******************
 * End Configure
 ******************/

INA220 ina220;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  delay(1000);
  uint8_t availableDevices = ina220.begin(MAX_CUR, SHUNT_R, INA_ADC_MODE_128AVG, INA_ADC_MODE_128AVG, INA_MODE_CONTINUOUS_BOTH, ina_addresses, NUM_INA);
  Serial.print("Configured "); Serial.print(availableDevices); Serial.print(" of "); Serial.print(NUM_INA); Serial.println(" INA220 current sensors");
 
 delay(1000);
}
  
int blink = 0; 
uint16_t registers[6];

#define BITSIZEOF(x)    (sizeof(x)*8)

char *int_to_bitstring_static(int x, int count)
{
    static char bitbuf[BITSIZEOF(x)+1];
    count = (count<1 || count>BITSIZEOF(x)) ? BITSIZEOF(x) : count;
    for(int i = 0; i<count; i++)
        bitbuf[i] = '0' | ((x>>(count-1-i))&1);
    bitbuf[count]=0;
    return bitbuf;
}


void dumpAll()
{
  for( int i = 0; i < NUM_INA ;i++)  
  {
    ina220.dumpRegisters(registers,ina220.getDeviceAddress(i));  
    for(int j = 0; j < 6; j++ ) 
      {
        Serial.print("Device "); Serial.print(i); Serial.print( " Register ");Serial.print(j);Serial.print(" = " );Serial.print(registers[j],DEC);Serial.print(" ");
         Serial.print(registers[j],HEX); Serial.print(" "); Serial.println(int_to_bitstring_static(registers[j],16));

      }
  }
}



void loop() {
  for (int i = 0; i < NUM_INA; i++) {
    float vol = ina220.getBusMilliVolts(i) / 1000.0;
    float shunt_vol = ina220.getShuntMicroVolts(i)/1000.0;
    float cur = ina220.getBusMicroAmps(i) / 1000.0;
    float power = ina220.getBusMicroWatts(i) / 1000.0;
    Serial.print("INA at 0x"); Serial.print(ina_addresses[i], HEX); Serial.print(" measures "); Serial.print(vol); Serial.print(" V, ");
    Serial.print(shunt_vol); Serial.print(" mV, ");
    Serial.print(cur); Serial.print(" mA, and "); Serial.print(power); Serial.println(" mW");
 }
  Serial.println();
  digitalWrite(LED_BUILTIN, (blink++ & 0x01)); 
  // dumpAll();
  delay(1000);
}
