#include <HX711_ADC.h>
#if defined(ESP8266) || defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

const int calVal_eepromAdress_1 = 0;
const int calVal_eepromAdress_2 = 4;
const unsigned int HX711_dout_1 = 11;
const unsigned int HX711_sck_1 = 12;
const unsigned int HX711_dout_2 = 8;
const unsigned int HX711_sck_2 = 9;
unsigned long t = 0;

HX711_ADC LoadCell_1(HX711_dout_1, HX711_sck_1);
HX711_ADC LoadCell_2(HX711_dout_2, HX711_sck_2);

void setupHX711();
bool checkLoadCells();