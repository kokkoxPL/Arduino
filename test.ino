unsigned long czas = 0;
unsigned long czasNumer2 = 0;
unsigned long czasAlarm = 0;
unsigned long add = 0;
unsigned long addNumer2 = 0;
unsigned long addAlarm = 0;
bool dziecko = false;
bool dorosly = false;
char numer1[];
char numer2[];

void setup() {
  Serial.begin(9600);
  add = millis();
}

void loop() {
  add = millis();
  addNumer2 = millis();
  addAlarm = millis();
  
  if(dziecko && !dorosly) {
    czas += add;
  } else {
    czas = 0;
  }

  if(czas > 30000) {
    ok = checkSMS();

    if(ok) {
      czas = 0;
      czasNumer2 = 0;
      czasAlarm = 0;
    }

    if(czasAlarm > 30000) {
      alarm();
    }
    else if(czasNumer2 > 30000) {
      sendSMS(numer2);
      czasAlarm += addAlarm;
    }
    else {
      sendSMS(numer1);
      czasNumer2 += addNumer2;
    }
  }

  add = millis() - add;
  addNumer2 = millis() - addAlarm;
  addAlarm = millis() - addAlarm;

  Serial.println(add);
}
