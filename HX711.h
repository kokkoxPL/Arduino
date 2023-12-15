#include <HX711_ADC.h>
#if defined(ESP8266) || defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

const int calVal_eepromAdress = 0;
unsigned long t = 0;

const int HX711_dout = 11;
const int HX711_sck = 13;

HX711_ADC LoadCell(HX711_dout, HX711_sck);

void setupHX711();
void loopHX711();