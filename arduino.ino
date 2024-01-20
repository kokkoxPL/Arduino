#include "SMS_SS.h"
#include "HX711.h"

const unsigned int REED = 10;
const unsigned int ALARM = 13;

const unsigned long time_to_count = 500;
unsigned long timer = 0;
unsigned long time_number2 = 0;
unsigned long timer_alarm = 0;
bool ok = false;
bool sendingSMS1 = false;
bool sendingSMS2 = false;
char number1[] = "+48xxxxxxxxx";
char number2[] = "+48xxxxxxxxx";
char message[] = "W aucie zostało dziecko!!! Wróć do auta lub zadzwoń na ten numer aby oznać że wszystko jest dobrze.";

void setup()
{
  pinMode(REED, INPUT_PULLUP);
  pinMode(ALARM, OUTPUT);

  Serial.begin(57600);

  setupHX711();
  setupSMS();
}

void loop()
{
  delay(time_to_count);

  if (checkLoadCells())
  {
    timer += time_to_count;
  }
  else
  {
    reset_timers();
  }

  if (timer > 300000 || sendingSMS1 || sendingSMS2)
  {
    if (getcallStatus() == 4) {
        ok = true;
        hangup();
      }

    if (ok)
    {
      reset_timers();
      sendingSMS1 = false;
      sendingSMS2 = false;
      noTone(ALARM);
      Serial.println("reset");
    }

    if (timer_alarm > 300000)
    {
      Serial.println("alarm");
      tone(ALARM, 1000);
    }
    else if (time_number2 > 300000)
    {
      if(!sendingSMS2) {
        Serial.println("sms2");
        sendMessage(number2, message);
        sendingSMS2 = true;
      }

      timer_alarm += time_to_count;
    }
    else if (timer > 300000)
    {
      if(!sendingSMS1) {
        Serial.println("sms1");
        sendMessage(number1, message);
        sendingSMS1 = true;
      }

      time_number2 += time_to_count;
    }
  }

  Serial.print("licznik: ");
  Serial.print(time_to_count);
  Serial.print(" czas: ");
  Serial.print(timer);
  Serial.print(" czas_numer2: ");
  Serial.print(time_number2);
  Serial.print(" czas_alarm: ");
  Serial.println(timer_alarm);
}

void reset_timers()
{
  timer = 0;
  time_number2 = 0;
  timer_alarm = 0;
}