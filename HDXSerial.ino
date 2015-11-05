#include <SoftwareSerial.h>

#include "Arduino.h"
#include "HardwareSerial.h"
#include "HardwareSerial_private.h"

#define DEBUG

/**
 * Not all pins on the Leonardo support change interrupts,
 so only the following can be used for RX:
 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).
 */
SoftwareSerial HostSerial(10, 11); // RX, TX

byte mBuffer[255];
byte mSumVerify;
int mCurIDX;

void setup()               
{
#ifdef DEBUG
	Serial.begin(19200); 
	while (!Serial) {
		; // wait for serial port to connect. Needed for native USB port only
	}
	Serial.println("debug");
#endif

	Serial1.begin(115200); 
  while (!Serial1) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
	HostSerial.begin(9600);
  while (!HostSerial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  HostSerial.flush();
	mCurIDX = 0;
}

void loop()              
{
	while (HostSerial.available() > 0) {
		mBuffer[mCurIDX] = HostSerial.read();
#ifdef DEBUG
		Serial.print(mCurIDX);
		Serial.print(":");
		Serial.println(mBuffer[mCurIDX], HEX);
#endif
		switch(mCurIDX) {
		case 0 : {
			if (mBuffer[mCurIDX] == 0xFA) {
				mCurIDX++;
			} else {
				mCurIDX = 0;
			}
			break; }
		case 1 : {
			if (mBuffer[mCurIDX] == 0xAF) {
				mSumVerify = 0;
				mCurIDX++;
			} else {
				mCurIDX = 0;
			}
			break; }
		case 2 :
		case 3 : 
		case 4 : 
		case 5 : 
		case 6 : 
		case 7 :{
			mSumVerify = (0xFF & (mSumVerify + mBuffer[mCurIDX]));
			mCurIDX++;
			break; }
		case 8 : {
			if (mBuffer[mCurIDX] == mSumVerify) {
				mCurIDX++;
			} else {
				mCurIDX = 0;
			}
			break; }
		case 9 : {
			if (mBuffer[mCurIDX] == 0xED) {
				mCurIDX++;
#ifdef DEBUG
				Serial.println("HDX START");
#endif
				//Send HDX
				cbi(UCSR1B, RXEN1); // CLOSE READ
				Serial1.write(mBuffer, mCurIDX);
				Serial1.flush();
				cbi(UCSR1B, TXEN1); // CLOSE SEND
				sbi(UCSR1B, RXEN1); // OPEN READ
				delayMicroseconds(400);
				while (Serial1.available() > 0) {
#ifdef DEBUG
					int read = Serial1.read();
					HostSerial.write(read);
					Serial.println(read, HEX);
#else
					HostSerial.write(Serial1.read());
#endif
				}
				HostSerial.flush();
        sbi(UCSR1B, TXEN1);
#ifdef DEBUG
				Serial.println("HDX END");
#endif
			}
			mCurIDX = 0;
			break;}
		default :;
		}
	}
}












