//***********************  main.c  ***********************
// Program written by:
// - Steven Prickett  steven.prickett@gmail.com
//
// Brief desicription of program:
// - Initializes an ESP8266 module to act as a WiFi client
//   and fetch weather data from openweathermap.org
//
//*********************************************************
/* Modified by Mairo Leier
 Oct 06, 2016
 Out of the box: to make this work you must
 Step 1) Set parameters of your AP in lines 59-60 of esp8266.c
 Step 2) Change line 39 with directions in lines 40-42
 Step 3) Run a terminal emulator like Putty or TExasDisplay at
         115200 bits/sec, 8 bit, 1 stop, no flow control
 Step 4) Set line 50 to match baud rate of your ESP8266 (9600 or 115200)
 */
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "tm4c123gh6pm.h"

#include "pll.h"
#include "UART.h"
#include "esp8266.h"
#include "LED.h"
#include "Nokia5110.h"


// prototypes for functions defined in startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

char Fetch[] = "GET /data/2.5/weather?q=tallinn&APPID=792cdad65f6b9e690fe4454312c5cd33&units=metric HTTP/1.1\r\nHost:api.openweathermap.org\r\n\r\n";
// 1) go to http://openweathermap.org/appid#use 
// 2) Register on the Sign up page
// 3) get an API key (APPID) replace the 1234567890abcdef1234567890abcdef with your APPID
// 588409 -> Tallinn



#define MAXLEN 100
char City[MAXLEN];
char Temperature[MAXLEN];
char Wind[MAXLEN];
char Pressure[MAXLEN];
char Humidity[MAXLEN];

void ParseResponse(void){ 
  char *pt; int i;
	
/* find City name in response*/
  pt = strstr(ServerResponseBuffer, "\"name\"");
  i = 0; 
  if( NULL != pt ){
    pt = pt + 8; // skip over "name":"
    while((i<MAXLEN)&&(*pt)&&(*pt!='\"')){
      City[i] = *pt; // copy into string
      pt++; i++;    
    }
  }
  City[i] = 0;
	
	/* find Wind speed in response */
  pt = strstr(ServerResponseBuffer, "\"speed\"");
  i = 0; 
  if( NULL != pt ){
    pt = pt + 8; // skip over "speed":"
    while((i<MAXLEN)&&(*pt)&&(*pt!=',')){
      Wind[i] = *pt; // copy into string
      pt++; i++;    
    }
  }
  Wind[i] = 0;

/* find Temperature value in response */
  pt = strstr(ServerResponseBuffer, "\"temp\"");
  i = 0; 
  if( NULL != pt ){
    pt = pt + 7; // skip over "temp":
    while((i<MAXLEN)&&(*pt)&&(*pt!=',')){
      Temperature[i] = *pt; // copy into string
      pt++; i++;    
    }
  }
  Temperature[i] = 0;
	
	/* find Pressure value in response */
  pt = strstr(ServerResponseBuffer, "\"pressure\"");
  i = 0; 
  if( NULL != pt ){
    pt = pt + 11; // skip over "pressure":
    while((i<MAXLEN)&&(*pt)&&(*pt!=',')){
      Pressure[i] = *pt; // copy into string
      pt++; i++;    
    }
  }
  Pressure[i] = 0;
	
	/* find Humidity value in response */
  pt = strstr(ServerResponseBuffer, "\"humidity\"");
  i = 0; 
  if( NULL != pt ){
    pt = pt + 11; // skip over "humidity":
    while((i<MAXLEN)&&(*pt)&&(*pt!=',')){
      Humidity[i] = *pt; // copy into string
      pt++; i++;    
    }
  }
  Humidity[i] = 0;

  Nokia5110_SetCursor(0,1);
	Nokia5110_OutString(City);
	Nokia5110_SetCursor(0,2);
	Nokia5110_OutString("W:");
	Nokia5110_OutString(Wind);
	Nokia5110_OutString(" m/s");
	Nokia5110_SetCursor(0,3);
	Nokia5110_OutString("T:");
  Nokia5110_OutString(Temperature); 
  Nokia5110_OutString(" C");
	Nokia5110_SetCursor(0,4);
	Nokia5110_OutString("P:");
  Nokia5110_OutString(Pressure); 
  Nokia5110_OutString(" Pa");
	Nokia5110_SetCursor(0,5);
	Nokia5110_OutString("H:");
  Nokia5110_OutString(Humidity); 
  Nokia5110_OutString(" %");
}



// Inputs: Number of msec to delay
// Outputs: None
void delay(unsigned long msec){ 
  unsigned long count;
  while(msec > 0 ) {  // repeat while there are still delay
    count = 16000;    // about 1ms
    while (count > 0) { 
      count--;
    } // This while loop takes approximately 3 cycles
    msec--;
  }
}


int main(void){  
  DisableInterrupts();
  PLL_Init(Bus80MHz);

	// Initialize Nokia5110 LCD
	Nokia5110_Init();             
	Nokia5110_Clear();
	Nokia5110_SetCursor(0, 0);
	Nokia5110_OutString((unsigned char*)"************IoT Weather * Lab num: **    16    **          *************");
	delay(1000);
	Nokia5110_Clear();
	Nokia5110_OutString((unsigned char*)"WIFI:conn...");
	delay(100);
	
	Output_Init();       // UART0 only used for debugging
	printf("\n\r-----------\n\rSystem starting...\n\r");
	ESP8266_Init(115200);      // connect to access point, set up as client
	ESP8266_GetVersionNumber();
	
  while(1){
		ESP8266_GetStatus();
    if(ESP8266_MakeTCPConnection("api.openweathermap.org")){ // open socket in server
			Nokia5110_SetCursor(5, 0);
			Nokia5110_OutString((unsigned char*)"OK     ");
			
      if (ESP8266_SendTCP(Fetch) == 1) {
				Nokia5110_OutString((unsigned char*)"NOK   ");
			} else {
				Nokia5110_OutString((unsigned char*)"OK     ");
			}
			// Add your code here
			ParseResponse();
			delay(600000);
		}

		ESP8266_CloseTCPConnection();
    while(Board_Input()==0){// wait for touch
    }; 
		
  }
	
}


// transparent mode for testing
void ESP8266SendCommand(char *);
int main2(void){  char data;
  DisableInterrupts();
  PLL_Init(Bus80MHz);
  LED_Init();  
  Output_Init();       // UART0 as a terminal
  printf("\n\r-----------\n\rSystem starting at 115200 baud...\n\r");
//  ESP8266_Init(115200);
  ESP8266_InitUART(115200,true);
  ESP8266_EnableRXInterrupt();
  EnableInterrupts();
  ESP8266SendCommand("AT+RST\r\n");
  data = UART_InChar();
//  ESP8266SendCommand("AT+UART=115200,8,1,0,3\r\n");
//  data = UART_InChar();
//  ESP8266_InitUART(115200,true);
//  data = UART_InChar();
  
  while(1){
// echo data back and forth
    data = UART_InCharNonBlock();
    if(data){
      ESP8266_PrintChar(data);
    }
  }
}



