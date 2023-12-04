/*
agsmII_basic_lbr.ino v 0.97/20170323 - a-gsmII 2.105 LIBRARY SUPPORT
COPYRIGHT (c) 2017 Dragos Iosub / R&D Software Solutions srl

You are legaly entitled to use this SOFTWARE ONLY IN CONJUNCTION WITH a-gsmII DEVICES USAGE. Modifications, derivates and redistribution 
of this software must include unmodified this COPYRIGHT NOTICE. You can redistribute this SOFTWARE and/or modify it under the terms 
of this COPYRIGHT NOTICE. Any other usage may be permited only after written notice of Dragos Iosub / R&D Software Solutions srl.

This SOFTWARE is distributed is provide "AS IS" in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

Dragos Iosub, Bucharest 2017.
http://itbrainpower.net
***************************************************************************************
SOFTWARE:
This file MUST be present, toghether with other files, inside a folder named 
like your main sketch!
***************************************************************************************
HARDWARE:
Read the readme file(s) inside the arhive/folder.
***************************************************************************************
HEALTH AND SAFETY WARNING!!!!!!!!!!!!!!!!!!!!
High power audio (around 700mW RMS)! You can damage your years! 
Use it with care when headset is connected. USE IT AT YOUR OWN RISK!
We recomend to set the volume at maximum level 20!
***************************************************************************************
*/
/*
In order to make your Arduino serial communication reliable (especially for Arduino Uno) with a-gsm shield, you must edit: 
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


/*do not change under this line! Insteed, make one copy for playing with.*/

/*
Prepare a-gsm for audio part usage
High power audio (around 700mW RMS)! You can damage your years! Use it with care when headset is connected.
We recomend to use AT+CLVL=25, audio setup command in order to limit the output power.
*/
void setAUDIOchannel(int speakerVolume, int micGain){
	if(speakerVolume > 21) speakerVolume = 20;//DO NOT CHANGE THIS LINE, OR DO IT ON YOUR OWN RISK!!! - limit output power for headsets usage 
	agsmSerial.println(F("AT+QAUDCH=2"));//use audio channel(2-standard for a-gsm)
	delay(100);
	agsmSerial.print(F("AT+QMIC=2,"));//set mic channel(2)
	agsmSerial.println(micGain);//set mic gain bw. 0 and 15. default is AGSM_DEFAULT_MIC_GAIN 14
	delay(100);
	agsmSerial.print(F("AT+CLVL="));
	agsmSerial.println(speakerVolume);//set output POWER! Do NOT exceed value of 20 if headset is used!
	delay(100);
	clearagsmSerial();
	ready4Voice = 1;
}

/*
	read GSM signal status
	output on debug some bar graph
	returns levels bw 0(no signal/signal error) and 7...see down there the corresonding levels
*/
int getSignalStatus(){
	char i[3];
	int j=0;
	int h=0;//signal level
	int res = 0;
	char tmpChar[20];//40
	char* pch;
	clearBUFFD();		
	memset(tmpChar,0x00,sizeof(tmpChar));
	sprintf(i, ",\r");
	//res = sendATcommand((char*)F("AT+CSQ"),(char*)F("OK"),(char*)F("ERROR"),10);
	res = fATcmd(F("+CSQ"));
	if(res){
		pch = strtok (buffd," ");
		j=0;
		while (pch != NULL){//parsing received data
		 	if(j==1){
				memset(tmpChar,0x00,sizeof(tmpChar));
				sprintf(tmpChar,"RSSI :%s",pch);
				Serial.println(tmpChar);
				res= atoi(pch);
				if(res==99){
					h=0;
				} else if(res>-1&&res<8){//-113dBm -> -99dBm
					h=1;
				}else if(res>7&&res<13){//-99dBm -> -89dBm
					h=2;
				}else if(res>12&&res<18){//-89dBm -> -79dBm
					h=3;
				}else if(res>17&&res<23){//-79dBm -> -69dBm
					h=4;
				}else if(res>22&&res<28){//-69dBm -> -59dBm
					h=5;
				}else if(res>27&&res<31){//-59dBm -> -53dBm
					h=6;
				}else if(res>=31){//>-53dBm 
					h=7;
				}

			}
		  	pch = strtok (NULL, i);
			j++;
		}
		clearBUFFD();

		//print signal as some graph
		Serial.print(F("Signal : "));
		Serial.flush();
		j=0;
		while(j<h){
			Serial.print(F("#"));
			Serial.flush();
			j++;
		}
		Serial.println("");
		Serial.flush();
	}else Serial.println(F("SIGERR")); Serial.flush();
	clearBUFFD();
	return h;
}

/*clear global buffd preparing it for writing*/
void clearBUFFD(){//just clear the data buffer
  memset(buffd, 0x00, BUFFDSIZE);
}

/*send command -cmd to the modem, waiting Delay_mS msecs after that*/
size_t aGsmCMD(char* cmd, int Delay_mS){
    size_t retv;  
    retv = agsmSerial.println(cmd);
    delay(Delay_mS);
    return retv;
}

/*send string -str to the modem, NOT ADDING \r\n to the line end*/
size_t aGsmWRITE(char* str){
    size_t retv;  
    retv = agsmSerial.print(str);
    return retv;
}

/*return read char from the modem*/
inline char aGsmREAD(void){
    char retv;
    retv = agsmSerial.read();
    return retv;
}

/*
Receive data from serial until event:
SUCCESS string - 1'st argument 
FAILURE string - second argument 
TIMEOUT - third argument in secconds!
rreturn: int 
  -2 buffd BUFFER OVERFLOW (avoided) //New feature, added in this version
  -1 TIMEOUT
  0  FAILURE
  1  SUCCESS
the string collected from modem => buffd
Please see default paramethers definition in ags_basic_lbr.h
*/
int recUARTDATA( char* ok, char* err, int to){
	int res=0;
	char u8_c;
	int run = 1;
	int i=0;
	unsigned long startTime;
	clearBUFFD();
	startTime = millis();	
	//delay(10);
	while(run){
		if(strstr(buffd,ok)) {
			delay(200);
			#if defined(atDebug)
				Serial.println(F("ok"));
			#endif
			clearagsmSerial();
			res=1;
			run=0;
		}
		else if(strstr(buffd,err)) {
			#if defined(atDebug)
				Serial.println(F("err"));
			#endif
			run=0;
			clearagsmSerial();
			//clearBUFFD();
		}
		//delay(1);
		if(millis() - startTime > (unsigned long) to *1000) {
			#if defined(atDebug)
				Serial.println(F("to!"));
			#endif
			//clearagsmSerial();
			run=0;
			res=-1;//timeout!
		}

		while(TXavailable()){
			u8_c = aGsmREAD();
      if(i<BUFFDSIZE-1){ 
        buffd[i]=u8_c;
      }else{
			#if defined(atDebug)
				Serial.println(F("bufover"));
			#endif
        clearagsmSerial();
        return -2;//bufferoverflow
      }
      i++;
		}
	}
	return(res);
}
 
/*
send AT command, looking for SUCCES STRING(1'st) or FAILURE STRING(second), and TIMEOUT(third)
return 1 for succes, 2 for failure, -1 for timeout and
  -2 buffd BUFFER OVERFLOW (avoided) //New feature, added in this version
modem response is loaded in buffd
Please see default paramethers definition in ags_basic_lbr.h
*/ 
int sendATcommand(char* outstr, char* ok, char* err, int to){
	int res=0;
	clearagsmSerial();
	clearBUFFD();	
	#if defined(atDebug)
		Serial.println(outstr);
	#endif
  agsmSerial.print(F("AT"));
  agsmSerial.flush();
  aGsmCMD(outstr,1);
	res = recUARTDATA( ok,  err, to);
	return(res);
}

/*
  same as sendATcommand, but command(second par) is STORED IN FLASH not in the volatile memmory
  328 RAM...
*/
int fATcmd(const __FlashStringHelper *cmd, int to, char* ok, char* err)
{
  const char PROGMEM *p = (const char PROGMEM *)cmd;
  char cmdl[60];//40
  size_t n = 0;
  while (1) {
    char c = pgm_read_byte(p++);
    if (c == 0) break;
    cmdl[n] = c;
    n++;//= write(c);
  }
  cmdl[n] = 0;
  return sendATcommand(cmdl,ok,err,to);
}

/*
  returns TRUE if chars are available in RX SERIAL_BUFFER (some chars has been received)
  check this function before call aGsmREAD()
*/
bool TXavailable(){
    int retv;
		// Serial.println(agsmSerial.available());
    retv = agsmSerial.available();
    return (retv > 0);
}

/*
helper function

global var "buffd" is  the input processed parameter
"ok" => input parameter (USUAL OK) from modem response
"head" => input parameter (USUAL the AT command) from modem response
extract from buffd the chars between "head" and "ok", and process further
looking for (split after) "separator" and returns "index"+1 chunk

parseResponce("OK", "+CBC: ", tmpChar, ",", 2); => "split" looking for separator "," and returns third (2+1) pars
parseResponce("OK", "AT+GSN", tmpChar, "", 2); => returns all

returned value in retChar
global "readBuffer" requiered!
*/
int parseResponce(char* ok, char* head, char* retChar, char* separator , int index){
	char* pch0;
	char* pch1;
	//char tmpChar[180];
	//memset(tmpChar, 0x00, sizeof(tmpChar));
	memset(readBuffer, 0x00, sizeof(readBuffer));
	pch1 = strstr(buffd, ok);
	if (pch1 == 0) return -1;
	pch0 = strstr(buffd, head);
	pch0 = pch0+strlen(head);
	
	if(pch0[2] == 0x0A) 
		//strncpy(tmpChar, pch0+3, pch1 - pch0 - strlen(ok) - 5);
		strncpy(readBuffer, pch0+3, pch1 - pch0 - strlen(ok) - 5);
	else
		//strncpy(tmpChar, pch0, pch1 - pch0 - strlen(ok) - 2);
		strncpy(readBuffer, pch0, pch1 - pch0 - strlen(ok) - 2);

	//pch0 = strtok (tmpChar, separator);
	pch0 = strtok (readBuffer, separator);
	int j=0;
	while (pch0 != NULL){//parsing the message
		if(j <= index){
			if(j == index){
				sprintf(retChar,"%s",pch0);
				return 0;
			}
			j++;
		}
		pch0 = strtok (NULL, separator);
	}
	//sprintf(retChar,"%s",tmpChar);
	sprintf(retChar,"%s",readBuffer);
	return 0;
}

/*
	read voltage...returned in buffd, value in mV
	
*/
void getVoltage(){
	char tmpChar[20];
	memset(tmpChar,0x00, sizeof(tmpChar));
	clearBUFFD();
	aGsmCMD("AT+CBC",1000);
	i = 0;
	while(TXavailable()){
		ch = aGsmREAD();
		buffd[i] = ch;
		i++;
	}  
	memset(tmpChar,0x00, sizeof(tmpChar));
	parseResponce("OK", "+CBC: ", tmpChar, ",", 2);
	clearBUFFD();
	strcpy(buffd,tmpChar);//load value into buffd
}

/*utility that read the modem IMEI value ==> buffd*/
void getIMEI(){//AT+GSN
	char tmpChar[20];
	memset(tmpChar,0x00, sizeof(tmpChar));
	clearBUFFD();
	clearagsmSerial();//add
	fATcmd(F("+GSN"));//add
	parseResponce("OK", "AT+GSN", tmpChar, "", 2);
	clearBUFFD();
	strcpy(buffd, tmpChar);//load value into buffd
}

/*utility that read the SIM IMSI value ==> buffd*/
void getIMSI(){
	char tmpChar[20];
	memset(tmpChar,0x00, sizeof(tmpChar));
	clearBUFFD();
	clearagsmSerial();//add
	fATcmd(F("+CIMI"));//add
	/*i = 0;
	while(TXavailable()){
		ch = aGsmREAD();
		buffd[i] = ch;
		i++;
	} */ 
	parseResponce("OK", "AT+CIMI", tmpChar, "", 2);
	clearBUFFD();
	strcpy(buffd, tmpChar);//load value into buffd
}

/*hang up the call*/
int hangup(){
	if(ready4Voice < 1) setAUDIOchannel(20);
	return fATcmd(F("H"));
}

/*pick up the call*/
int answer(){
	if(ready4Voice < 1) setAUDIOchannel(20);
	return fATcmd(F("A"));
}

/*dial destinationNO*/
int dial(char* destinationNO){
	if(ready4Voice < 1) setAUDIOchannel(25);
	char tmpChar[20];//40
	memset(tmpChar, 0x00, sizeof(tmpChar));
	sprintf(tmpChar,"D%s;",destinationNO);
	return sendATcommand(tmpChar);
}

#if !defined(FREESOMEFLASHMEM)
	/*print modem registration status*/
	int printRegistration(int cmdValue){
		if(cmdValue==1) {Serial.println(F("Registrated (home)")); }
		else if(cmdValue==5) {Serial.println(F("Registrated (roaming)"));}
		else {Serial.println(F("Not registrated"));}
		Serial.flush();
	}
#endif
/*
  read modem registration status, 1 for GSM, 0 for GPRS
  return:
  	-2   cmd error / timeout
        1    register home network
        5    register roaming network
    	0    not register
        2
        3
        
*/
int registration(int type){
	char tmpChar[20];//40
	int res;
	if (type==1) res = fATcmd(F("+CREG?")); 
	else res = fATcmd(F("+CGREG?")); 
	
	if(res==1){
		memset(tmpChar,0x00, sizeof(tmpChar));
		if (type==1) res = parseResponce("OK", "+CREG:", tmpChar, ",", 1);
		else res = parseResponce("OK", "+CGREG:", tmpChar, ",", 1);
		res = atoi(tmpChar);//add
		#if !defined(FREESOMEFLASHMEM)
			printRegistration(res);//add
		#endif
	}
	else {
		#if !defined(FREESOMEFLASHMEM)
			Serial.print(F("CMD timeout/error"));
		#endif
		res= -2;//return cmd error/timeout
	}
	Serial.flush();
	return res;
}

char* FF(const __FlashStringHelper *cmd){
  const char PROGMEM *p = (const char PROGMEM *)cmd;
  char* cmdl;//[40];//40
  size_t n = 0;
  while (1) {
    char c = pgm_read_byte(p++);
    if (c == 0) break;
    cmdl[n] = c;
    n ++;//= write(c);
  }
  cmdl[n++] = 0;
  return cmdl;  
}



/*prepare/check modem is ready for SMS handling*/
void setupMODEMforSMSusage(){
	if(ready4SMS > 0) return;
	int res;
	res = 0; 
	while (res!=1){//wait 
		res = fATcmd(F("+CPBS?"));
		delay(500);
	}
	//set SIMM memory as active
	fATcmd(F("+CPBS=\"SM\""));
	//is MODEM ready for SMS?
	res = 0; 
	while (res!=1){//wait 
		res = fATcmd(F("+CPBS?"));
		delay(500);
	}				
	//set SMS mode TEXT		
	res = fATcmd(F("+CMGF=1"));
	res = fATcmd(F("+CNMI=2,0,0,0,0"));
	ready4SMS = 1;
	delay(500);
	clearagsmSerial();     
}


/*
	just read one line(looks for LF char) and return the line loaded into buffd
	see AGSM_DEFAULT_TIMEOUT_NO in ags_basic.h
	returns: 
		1 	LF found
		-1	timeout 
*/
int readline(unsigned long timeout/* = AGSM_DEFAULT_TIMEOUT_NO*/){
	int cnt=0;
	char c;
	int res=0;
	unsigned long startTime;
	startTime = millis();	
	while(1){
		if(TXavailable()){
			c=agsmSerial.read();
		        res = 1;//return found
			if(c == '\n') break;
			buffd[cnt++] = c;
		}
		if(timeout > 0 && millis() - startTime > timeout){
		        res = -1;//return timeout
		        break;
		}

	}
	buffd[cnt++] = 0x00;//add string terminator
	return res;
}

/*just reset the modem - use it only if modem become freezed*/
void resetMODEM(){
	Serial.println(F("reset Modem")); 
	delay(500);
	digitalWrite(resetPIN, LOW);    
	delay(1000);                  
	digitalWrite(resetPIN, HIGH);    
	powerState=1;
	ready4SMS = 0;
	ready4Voice = 0;
	delay(2000);
}

int getModemState(){return digitalRead(statusPIN);}

void powerOffModem(){
	if(getModemState()){
		Serial.println(F("powerOFF")); delay(500);
		digitalWrite(powerPIN, LOW);    
		delay(1000);                  
		digitalWrite(powerPIN, HIGH);  
		powerState=0;
		delay(8000);
	}
}

void powerOnModem(){
	if(!getModemState()){ // not running
		clearagsmSerial();delay(100);
		Serial.println(F("switching on")); delay(500);
		digitalWrite(powerPIN, LOW);    
		delay(1000);                  
		digitalWrite(powerPIN, HIGH);  
		delay(8000);
		powerState = 1;
		state=1;
		aGsmCMD("ATE1",20);//set proper modem reply mode, do not remove!
		//delay(50);
		aGsmCMD("ATV1",20);//set proper modem reply mode, do not remove!
		delay(50);
		clearagsmSerial();
	}
}

/*just restart the modem*/
void restartMODEM(){
	clearagsmSerial();
	delay(100); 
	powerOffModem();
	powerOnModem();
}


/*just flush remaining chars from software serial (a-gsm)*/
void clearagsmSerial(){
	while(TXavailable()){
		ch = aGsmREAD();
		delay(0.5);
	} 
	delay(100);
	ch=0x00;
} 

/*just flush remaining chars from serial (debug)*/
void clearSerial(){
	while(Serial.available()){
		ch = Serial.read();
		delay(0.5);
	} 
	delay(100);
	ch=0x00;
} 



/*Set the active SIM, Active SIM value will be stored in activeSIM var*/
void setActiveSIM(int SIM){
	if(SIM < 0 || SIM >1) //SIM can be 0(zero)- TOP SIM, or 1(one)- BOTTOM SIM
		return;
	int res;
	res = fATcmd(F("+CFUN=0"),10);//shutdown GSM part of the modem 
	delay(1500);//just delay a while
	agsmSerial.print("+QDSIM=");
	agsmSerial.println(SIM);//execute chance SIM
	delay(1500);//just delay a while
	res = fATcmd(F("+CFUN=1"),10);//wakeup GSM part of the modem 
	delay(1500);//just delay a while
	activeSIM = SIM;//load active SIM value
}

/*prepare the modem for DTMF SEND usage */
void setupMODEMforDTMFSusage(){
	int res;
	res = 0; 
	setupMODEMforSMSusage();

	res = fATcmd(F("+QTONEDET=4,1,3,3,65536"));//some DTMF detection settings
	res = fATcmd(F("+QSFR=7"));//use EFR 
	res = fATcmd(F("+QTDMOD=1,0")); 
	#if !defined(FREESOMEFLASHMEM)
		Serial.println(F("DTMF setup ready"));
		Serial.flush();
	#endif
}

/*prepare the modem for DTMF READ usage with autoanswer enabled*/
void setupMODEMforDTMFRusage(){
	int res;
	res = 0;
	setupMODEMforSMSusage();
	enableAutoanswer(2);//enable Autoanswer at second RING
	Serial.flush();
	res = fATcmd(F("+QTONEDET=1"));//enable DTMF detection 
	#if !defined(FREESOMEFLASHMEM)
		Serial.println(F("Autoanswer at second RING enabled"));
		if(res==1) 
			Serial.println(F("DTMF detection enabled"));
		Serial.flush();
	#endif
}

/*enable autoanswer*/
void enableAutoanswer(int RingNo){
	if(RingNo<0) return;
	agsmSerial.print(F("ATS0="));
	agsmSerial.println(RingNo);
	clearagsmSerial();
	#if !defined(FREESOMEFLASHMEM)
		Serial.print(F("Autoanswer "));
		if(RingNo>0) 
			Serial.println(F("enabled"));
		else 
			Serial.println(F("disabled"));
	#endif
}

/*disable autoanswer*/
void disableAutoanswer(){
	enableAutoanswer(0);
}



/*
int getcallStatus()
#detect if the voice call is CONNECTED
#returns: 
#0 Active   CONNECTED   BOTH
#1 Held                 BOTH
#2 Dialing (MO call)    OUTBOUND
#3 Alerting (MO call)   OUTBOUD
#4 Incoming (MT call)   INBOUD
#5 Waiting (MT call)    INBOUD
*/
int getcallStatus(){
	int res=0;
	char content [40];
	res = fATcmd(F("+CLCC"));//CHECK DIAL STATUS ...RETURN +CLCC: 1,0,0,0,0,"dialed NO", 129 IF ACTIVE CALL 
	if(res==1){
		memset (content,0x00, sizeof(content));
		char * pch0;
		char * pch1;
		char pch;
		if(strstr(buffd,"+CLCC: ")){
			pch0 = strstr(buffd,"+CLCC: ");
			pch1 = strstr(buffd,"OK");
			strncpy(content, pch0+7, 5);
			////Serial.println(content);
			pch=content[4];
			////Serial.println((char *)pch);
			if(pch==0x30) {
				//Serial.println("CONNECTED");
				return 0;
			}else if(pch==0x32) {
				//Serial.println("DIALING");
				return 2;
			}else if(pch==0x33) {
				//Serial.println("RING-OUTBOUND");
				return 3;
			}else if(pch==0x34) {
				//Serial.println("RING-INBOUND");
				return 4;
			}
		}
		else{//NO CONNECTION active
			//Serial.println("NO CONNECTION");
			return -1;
		}
	} 
    //Serial.println("CMD ERROR");
	return -2;
}

int enableagsmClockUpdate(){//autoupdate RTC on module's re-boot, NITZ function must be supported by NMO
  return(fATcmd(F("+CTZU=1")));
}
int disableagsmClockUpdate(){
  return(fATcmd(F("+CTZU=0")));
}

int getagsmClock(char* clock){//returns in clock var the RTC value read (timezone info in quarter of hour)
	int res=0;
	res = fATcmd(F("+CCLK?"));
	if(res==1){
		parseResponce("OK", "+CCLK: \"", clock, "\"", 1);
		//clock[strlen(clock)-4]=0x00;//remove comment in order to return date/time w/o timezone info
	}
	return res; 
}

int setagsmClock(char* clock){
	char tmpChar[40];
	memset(tmpChar,0x00, sizeof(tmpChar));
	sprintf(tmpChar,"+CCLK=\"%s+00\"",clock);
	return(sendATcommand(tmpChar,"OK","ERROR",2));
}

void emergencyReset(){
	state = 0;
	#if defined(agsm_IP)
  	  IP_STATE = 0;
	#endif
	resetMODEM();
}

void modemHWSetup(){
  pinMode(resetPIN, OUTPUT);
  digitalWrite(resetPIN, HIGH);
  pinMode(powerPIN, OUTPUT);
  digitalWrite(powerPIN, HIGH);
  pinMode(statusPIN, INPUT);
  delay(100);  
}
