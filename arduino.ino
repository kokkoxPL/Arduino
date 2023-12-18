#include "SMS_SS.h"
#include "HX711.h"

const int REED = 13;

unsigned long time = 0;
unsigned long time_number2 = 0;
unsigned long time_alarm = 0;
unsigned long timer = 0;
bool ok = false;
bool sendingSMS1 = false;
bool sendingSMS2 = false;
char number1[] = "+48xxxxxxxxx";
char number2[] = "+48xxxxxxxxx";
char message[] = "Hello World!";


void setup()
{
  pinMode(REED, INPUT_PULLUP);
  Serial.begin(57600);
  setupHX711();
  setupSMS();
}

void loop()
{
  delay(500);
  timer = millis() - timer;

  if (checkLoadCells())
  {
    time += timer;
  }
  else
  {
    reset_timers();
  }

  if (time > 3000 || sendingSMS1 || sendingSMS2)
  {
    if (ok)
    {
      reset_timers();
      sendingSMS1 = false;
      sendingSMS2 = false;
      Serial.println("reset");
    }

    if (time_alarm > 3000)
    {
      Serial.println("alarm");
      ok = true;
    }
    else if (time_number2 > 3000)
    {
      Serial.println("sms2");

      if(!sendingSMS2) {
        sendMessage(number2, message);
        sendingSMS2 = true;
      }

      time_alarm += timer;
    }
    else if (time > 3000)
    {
      Serial.println("sms1");

      if(!sendingSMS1) {
        sendMessage(number1, message);
        sendingSMS1 = true;
      }

      time_number2 += timer;
    }
  }

  Serial.print("licznik: ");
  Serial.print(timer);
  Serial.print(" czas: ");
  Serial.print(time);
  Serial.print(" czas_numer2: ");
  Serial.print(time_number2);
  Serial.print(" czas_alarm: ");
  Serial.println(time_alarm);
  
  timer = millis();
}

void reset_timers()
{
  timer = 0;
  time = 0;
  time_number2 = 0;
  time_alarm = 0;
}
