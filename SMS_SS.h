/*
SMS_SS v 0.921/20171130 - a-gsmII 2.105/b-gsmgnss 2.105 send/read/list SMS example utility
COPYRIGHT (c) 2014-2017 Dragos Iosub / R&D Software Solutions srl

******************************IMPORTANT NOTICE*******************************************
"agsmII_basic_lbr.h", "agsmII_SMS_lbr.ino" and "agsmII_basic_lbr.h", "agsmII_SMS_lbr.ino"
or,
"bgsmgnss_basic_lbr.h", "bgsmgnss_SMS_lbr.ino" and "bgsmgnss_basic_lbr.h", "bgsmgnss_SMS_lbr.ino"
ARE REQUIERED IN ORDER TO RUN THIS EXAMPLE!!!!!!!!!!!!!!!!!!!!

Download the "a-gsmII kickstart for Arduino"/"b-gsmgnss kickstart for Arduino" from here:
https://itbrainpower.net/downloads
Uncompress the archive and copy the files mentined above in the folder
where is this utility, then you can compile this code.

You may want to modify "message" and "destinationNumber" variables found at lines 55/56
*******************************END of NOTICE*********************************************

You are legaly entitled to use this SOFTWARE ONLY IN CONJUNCTION WITH a-gsmII/b-gsmgnss DEVICES USAGE. Modifications, derivates and redistribution
of this software must include unmodified this COPYRIGHT NOTICE. You can redistribute this SOFTWARE and/or modify it under the terms
of this COPYRIGHT NOTICE. Any other usage may be permited only after written notice of Dragos Iosub / R&D Software Solutions srl.

This SOFTWARE is distributed is provide "AS IS" in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

Dragos Iosub, Bucharest 2017.
http://itbrainpower.net
*/
/*
In order to make your Arduino serial communication reliable (especially for Arduino Uno) with a-gsmII/b-gsmgnss shield, you must edit:
C:\Program Files\Arduino\libraries\SoftwareSerial\SoftwareSerial.h

comment the line 42
#define _SS_MAX_RX_BUFF 64

this, will look after that like:
//#define _SS_MAX_RX_BUFF 64

and add bellow the next line:

#define _SS_MAX_RX_BUFF 128

You just increased the RX buffer size for UNO and other "snails".

Now you can compile and upload your code supporting highier buffered serial input data.
*/

// next 2 definition: leave them commented for standard conectivity over Software serial
// #define usejLader						//un-comment this if you use micro and nano GSM 3G adapter for ArduinoNano --Do not use it with a-gsmII//b-gsmgnss!!!!
// #define HARDWARESERIAL 				//remove comment to use Serial1 for communication on AT MEGA 2560...DUE..

// char message[] = "Hi!\r\nThis message was sent from a-gsmII/b-gsmgnss v2.105 Arduino shield connected with my Arduino board."; // no more than 160 chars -text SMS maximum lenght
// char destinationNumber[] = "+XXxxxxxxxxx"; // +XXxxxxxxxxx

#define SMSLibDebug // use this definition to output the SMS send status messages
#define atDebug						//un-comment this if you want to debug the AT exchange

// /*do not change under this line! Insteed, make one copy for playing with.*/
#define powerPIN 7  // Arduino Digital pin used to power up / power down the modem
#define resetPIN 6  // Arduino Digital pin used to reset the modem
#define statusPIN 5 // Arduino Digital pin used to monitor if modem is powered

#if (ARDUINO >= 100)
#include "Arduino.h"
#if !defined(HARDWARESERIAL)
#include <SoftwareSerial.h>
#endif
#else
#include "WProgram.h"
#if !defined(HARDWARESERIAL)
#include <NewSoftSerial.h>
#endif
#endif

#if defined(HARDWARESERIAL)
#define BUFFDSIZE 1024
#else
#if defined(__AVR_ATmega1280__) /*AT MEGA ADK*/ || defined(__AVR_ATmega2560__) /*AT MEGA 2560*/ || defined(__AVR_ATmega32U4__) /*LEONARDO*/
SoftwareSerial agsmSerial(10, 3); // RX==>10,TX soft==>3...read
#define BUFFDSIZE 1024
#else                 /*UNO*/
#define UNO_MODE      // Arduino UNO
#define BUFFDSIZE 200 // 240
#if defined usejLader
SoftwareSerial agsmSerial(3, 2); // RX==>3 ,TX soft==>2
#else
SoftwareSerial agsmSerial(2, 3); // RX==>2 ,TX soft==>3
#endif
#endif
#endif

#include "agsmII_SMS_lbr.h"

#define printDebugLN(x)    \
    {                      \
        Serial.println(x); \
    }

int state = 0, i = 0, powerState = 0;
char ch;
char buffd[256];
// char IMEI[18];
unsigned long offsetTime;
int totalChars = 0;
// int ready4SMS = 0;
// int	ready4Voice = 0;

char readBuffer[200];

void setupSMS();
void sendMessage(char destinationNumber[], char message[]);