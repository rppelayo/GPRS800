/*
 * GPRS800.h
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
 
 #ifndef __GPRS800_H__
#define __GPRS800_H__

#include "Arduino.h"
#include <SoftwareSerial.h>

#define TRUE                    1
#define FALSE                   0

#define SIM800_TX_PIN           8
#define SIM800_RX_PIN           7
#define SIM800_POWER_PIN        9
#define SIM800_POWER_STATUS     12

#define UART_DEBUG

#ifdef UART_DEBUG
#define ERROR(x)            Serial.println(x)
#define DEBUG(x)            Serial.println(x);
#else
#define ERROR(x)
#define DEBUG(x)
#endif

#define DEFAULT_TIMEOUT     5


enum Protocol {
    CLOSED = 0,
    TCP    = 1,
    UDP    = 2,
};

#define MESSAGE_LENGTH  100

class GPRS800
{
public:
    uint32_t _ip;
    char ip_string[20];

    GPRS800(int baudRate = 9600):serialSIM800(SIM800_TX_PIN,SIM800_RX_PIN){
        powerPin = SIM800_POWER_PIN;
        pinMode(powerPin,OUTPUT);
        serialSIM800.begin(baudRate);
	};

 
    bool init(const char  *apn = 0, const char *userName = 0, const char *passWord = 0);

    uint32_t str_to_ip(const char* str);

    char* getIPAddress();

    bool checkSIMStatus(void);

    bool networkCheck(void);
   
    bool connectTCP(const char* ip, int port);

    bool sendTCP(char* data);

    int closeTCP(void);

    int shutTCP(void);

    int bufferRead(char* buffer,int count, unsigned int timeOut = DEFAULT_TIMEOUT);

    void bufferEmpty(char* buffer, int count);
	
	void showResponse(void);
	
    int sendATTest(void);
    
	void sendCmd(const char* cmd);

	int sendCmdAndWaitForResp(const char* cmd, const char *resp, unsigned timeout);

	int waitForResp(const char *resp, unsigned int timeout);
	
    int powerPin;
    SoftwareSerial serialSIM800;

	
	private:

};
#endif

 
 