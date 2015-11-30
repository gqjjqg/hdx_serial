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
byte mResponse[255];
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
	
	sbi(UCSR1B, TXEN1); // OPEN WRITE
	cbi(UCSR1B, RXEN1); // CLOSE READ
}

void clear_serial1_received() {
	while (Serial1.available() > 0) {
#ifdef DEBUG
		int read = Serial1.read();
		Serial.print("clear->");
		Serial.println(read, HEX);
#endif
	}
}

void loop()              
{
	// clear serial power up.
	clear_serial1_received();
	
	while (HostSerial.available() > 0) {
		mBuffer[mCurIDX] = HostSerial.read();
#ifdef DEBUG
		Serial.print(mCurIDX);
		Serial.print(":");
		Serial.println(mBuffer[mCurIDX], HEX);
#endif
		switch(mCurIDX) {
		case 0 : {
			if (mBuffer[mCurIDX] == 0xFA ||
				mBuffer[mCurIDX] == 0xFC) {
				mCurIDX++;
			} else {
				mCurIDX = 0;
			}
			break; }
		case 1 : {
			if ((mBuffer[mCurIDX] == 0xAF && mBuffer[mCurIDX - 1] == 0xFA) ||
				(mBuffer[mCurIDX] == 0xCF && mBuffer[mCurIDX - 1] == 0xFC) ) {
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
				int retry = 2;
				do {
#ifdef DEBUG
					Serial.println("HDX START");
#endif
					//Send HDX
					Serial1.write(mBuffer, mCurIDX);
					Serial1.flush();
					cbi(UCSR1B, TXEN1); // CLOSE SEND
					sbi(UCSR1B, RXEN1); // OPEN READ
					delayMicroseconds(400);
					
					int size = 0;
					while (Serial1.available() > 0) {
						mResponse[size] = Serial1.read();
#ifdef DEBUG
						Serial.println(mResponse[size], HEX);
#endif
						size++;
						delayMicroseconds(100);
					}
					if (size != 0) {
						retry = 0;
						
						sbi(UCSR1B, TXEN1); // OPEN WRITE
						cbi(UCSR1B, RXEN1); // CLOSE READ
						
						HostSerial.write(mResponse, size);
						HostSerial.flush();
						
					} else {
						retry--;
						
						sbi(UCSR1B, TXEN1); // OPEN WRITE
						cbi(UCSR1B, RXEN1); // CLOSE READ
					}
#ifdef DEBUG
					Serial.println("HDX END");
#endif
				} while (retry != 0);
			}
			mCurIDX = 0;
			break;}
		default :;
		}
	}
}



















