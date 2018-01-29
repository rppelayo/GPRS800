/*
 * GPRS800.cpp
 * A library for establishing internet connection via GPRS trough SIM800H
 *
 * by R. Pelayo
 * Copyright (c) January 2018 Teach Me Micro
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include "gprs800.h"

bool GPRS800::init(const char  *apn, const char *userName, const char *passWord)
{
	
	char cmd[64];
    char ipAddr[32];
    char gprsBuffer[32];
	
	int checker, counter = 0;
	
	pinMode(SIM800_POWER_STATUS,INPUT);
    delay(10);
    if(digitalRead(SIM800_POWER_STATUS) == LOW)
    {
        if(sendATTest() != 0)
        {
            delay(800);
            digitalWrite(powerPin,HIGH);
            delay(200);
            digitalWrite(powerPin,LOW);
            delay(2000);
            digitalWrite(powerPin,HIGH);
            delay(3000);  
        }
        while(sendATTest() != 0);                
    }
    else
    {
        Serial.println("Power check failed!");  
    }
	
	
	while(sendCmdAndWaitForResp("AT+CIPSHUT\r\n","OK\r\n",DEFAULT_TIMEOUT)!=0){;}
	delay(1000);
	while(sendCmdAndWaitForResp("AT+CIPMUX=0\r\n","OK\r\n",DEFAULT_TIMEOUT)!=0){;}
	delay(1000);
	
    bufferEmpty(ipAddr,32);
    sendCmd("AT+CIFSR\r\n");    
    bufferRead(ipAddr,32,2);

    // If no IP address feedback than bring up wireless 
    if( NULL != strstr(ipAddr, "ERROR") )
    {
	    if( 0 != sendCmdAndWaitForResp("AT+CSTT?\r\n", apn, DEFAULT_TIMEOUT) )
        {
            sendCmd("AT+CSTT=\"");
            sendCmd(apn);
            sendCmd("\",\"");
            sendCmd(userName);
            sendCmd("\",\"");
            sendCmd(passWord);        
            sendCmdAndWaitForResp("\"\r\n","OK\r\n",DEFAULT_TIMEOUT*3);
        }
		
        //Brings up wireless connection
        while(sendCmdAndWaitForResp("AT+CIICR\r\n","OK\r\n",DEFAULT_TIMEOUT)!=0){
			counter++;
			if(counter > 1000){
				return false;
			}
		}
         
        //Get local IP address
        bufferEmpty(ipAddr,32);
        sendCmd("AT+CIFSR\r\n");
        bufferRead(ipAddr,32,2);    
    }          

    if(NULL != strstr(ipAddr,"AT+CIFSR")) {        
        _ip = str_to_ip(ipAddr+11);
        if(_ip != 0) {
            checker++;
        }
    }
	if(checker == 1){
		return true;
	}else{
		return false;
	}
    
}

int GPRS800::bufferRead(char *buffer,int count, unsigned int timeOut)
{
    int i = 0;
    unsigned long timerStart,timerEnd;
    timerStart = millis();
    while(1) {
        while (serialSIM800.available()) {
            char c = serialSIM800.read();
            if (c == '\r' || c == '\n') c = '$';                            
            buffer[i++] = c;
            if(i > count-1)break;
        }
        if(i > count-1)break;
        timerEnd = millis();
        if(timerEnd - timerStart > 1000 * timeOut) {
            break;
        }
    }
    delay(500);
    while(serialSIM800.available()) {   // display the other thing..
        serialSIM800.read();
    }
    return 0;
}

void GPRS800::bufferEmpty(char *buffer, int count)
{
    for(int i=0; i < count; i++) {
        buffer[i] = '\0';
    }
}

uint32_t GPRS800::str_to_ip(const char* str)
{
    uint32_t ip = 0;
    char *p = (char*)str;
    
    for(int i = 0; i < 4; i++) {
        ip |= atoi(p);
        p = strchr(p, '.');
        if (p == NULL) {
            break;
        }
        if(i < 3) ip <<= 8;
        p++;
    }
    return ip;
}

char* GPRS800::getIPAddress()
{
    uint8_t a = (_ip>>24)&0xff;
    uint8_t b = (_ip>>16)&0xff;
    uint8_t c = (_ip>>8)&0xff;
    uint8_t d = _ip&0xff;

    snprintf(ip_string, sizeof(ip_string), "%d.%d.%d.%d", a,b,c,d);
    return ip_string;
}

bool GPRS800::checkSIMStatus(void)
{
    char gprsBuffer[32];
    int count = 0;
    bufferEmpty(gprsBuffer,32);
    while(count < 3) {
        sendCmd("AT+CPIN?\r\n");
        bufferRead(gprsBuffer,32,DEFAULT_TIMEOUT);
        if((NULL != strstr(gprsBuffer,"+CPIN: READY"))) {
            break;
        }
        count++;
        delay(300);
    }
    if(count == 3) {
        return false;
    }
    return true;
}

bool GPRS800::networkCheck(void)
{
    delay(1000);
    if(0 != sendCmdAndWaitForResp("AT+CGREG?\r\n","+CGREG: 0,1",DEFAULT_TIMEOUT*3)) {
        ERROR("ERROR:CGREG");
        return false;
    }
    delay(1000);
    if(0 != sendCmdAndWaitForResp("AT+CGATT?\r\n","+CGATT: 1",DEFAULT_TIMEOUT)) {
        ERROR("ERROR:CGATT");
        return false;
    }
    return true;
}

bool GPRS800::connectTCP(const char *ip, int port)
{
    char cipstart[50];
    sprintf(cipstart, "AT+CIPSTART=\"TCP\",\"%s\",\"%d\"\r\n", ip, port);
    if(0 != sendCmdAndWaitForResp(cipstart, "CONNECT OK", 2*DEFAULT_TIMEOUT)) {// connect tcp
        ERROR("ERROR:CIPSTART");
        return false;
    }

    return true;
}
bool GPRS800::sendTCP(char *data)
{
    char cmd[32];
    int len = strlen(data); 
    snprintf(cmd,sizeof(cmd),"AT+CIPSEND=%d\r\n",len);
    if(0 != sendCmdAndWaitForResp(cmd,">",2*DEFAULT_TIMEOUT)) {
        ERROR("ERROR:CIPSEND");
        return false;
    }
        
    if(0 != sendCmdAndWaitForResp(data,"SEND OK",2*DEFAULT_TIMEOUT)) {
        ERROR("ERROR:SendTCPData");
        return false;
    }     
    return true;
}

int GPRS800::closeTCP(void)
{
    sendCmd("AT+CIPCLOSE\r\n");
    return 0;
}

int GPRS800::shutTCP(void)
{
    sendCmd("AT+CIPSHUT\r\n");
    return 0;
}

void GPRS800::sendCmd(const char* cmd)
{
    serialSIM800.write(cmd);
}

int GPRS800::sendCmdAndWaitForResp(const char* cmd, const char *resp, unsigned timeout)
{
    sendCmd(cmd);
    return waitForResp(resp,timeout);
}

int GPRS800::waitForResp(const char *resp, unsigned int timeout)
{
    int len = strlen(resp);
    int sum=0;
    unsigned long timerStart,timerEnd;
    timerStart = millis();
    
    while(1) {
        if(serialSIM800.available()) {
            char c = serialSIM800.read();
            sum = (c==resp[sum]) ? sum+1 : 0;
            if(sum == len)break;
        }
        timerEnd = millis();
        if(timerEnd - timerStart > 1000 * timeout) {
            return -1;
        }
    }

    while(serialSIM800.available()) {
        serialSIM800.read();
    }

    return 0;
}

void GPRS800::showResponse(void)
{
    while(1) {
        if(serialSIM800.available()){
            Serial.write(serialSIM800.read());
        }
        if(Serial.available()){     
            serialSIM800.write(Serial.read()); 
        }
    }
}

int GPRS800::sendATTest(void)
{
    int ret = sendCmdAndWaitForResp("AT\r\n","OK",DEFAULT_TIMEOUT);
    return ret;
}