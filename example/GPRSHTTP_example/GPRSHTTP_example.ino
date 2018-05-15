/*
Sketch: GPRS Connection Test

by R. Pelayo
Copyright (c) January 2018 Teach Me Micro

Full tutorial: https://www.teachmemicro.com/arduino-gprs

Wiring:

SIM800 TX -> D8
SIM800 RX ->D7
Optional (for SIM800 shields):
 SIM800 POWER PIN -> D9          
 SIM800 POWER STATUS -> D12

*/
#include <gprs800.h>
#include <SoftwareSerial.h>

char http_cmd[] = "GET / HTTP/1.0\r\n\r\n";       //server request
char website[] = "www.teachmemicro.com";          //website domain name or ip address
char apn[] = "internet.globe.com.ph";             //APN of your cellular network provider
char uname[] = "";								  //APN username
char pword[] = "";                                //APN password
int port = 80;                                    //connection port, usually 80 for HTTP

char buffer[512];

GPRS800 gprs;

void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.println("GPRS HTTP Connection Example");  
  Serial.print("Connecting via ");
  Serial.prinln(apn);
  
  while(!gprs.init(apn, uname, pword)) { 
      delay(2000);
  }
  Serial.print("IP Address is ");
  Serial.println(gprs.getIPAddress());
  
  Serial.print("Initialization successful. Connecting to ");
  Serial.println(website);
  
  if(gprs.connectTCP(website, port)) {
      Serial.print("Connected to ");
      Serial.println(website);
  }else{
      Serial.println("Connection error");
      while(1);
  }

  Serial.println("Fetching server response");
  if(gprs.sendTCP(http_cmd))
  {      
    gprs.showResponse();
  }
  gprs.closeTCP();

}

void loop() {    
    
}
