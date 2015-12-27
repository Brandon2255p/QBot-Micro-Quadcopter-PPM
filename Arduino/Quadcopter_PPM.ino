#include "Arduino.h"
#include <stdio.h>

#include "Arduino.h"
#include "TimerOne.h"
#include <stdint.h>
#include <stdio.h>

int32_t iPulseCount=0, iPeriodCount =0;

#define PPMPin 2

enum PPMState {prePulse0=0, P1, prePulse1, P2, prePulse2, P3, prePulse3, P4, prePulse4, P5, prePulse5};
PPMState eCurrentState;

char buffer[16];
int counter =0;
int iLoopCout = 0;

struct PPM_Values
{
	int iThrust;
	int iAil;
	int iEle;
	int iRud;
	int iGer;
};
static struct PPM_Values sPPM_Values;
void setup() {
	//Setup PPM
	pinMode(PPMPin, OUTPUT);
	eCurrentState = prePulse0;

	Timer1.initialize(20); // set a timer of length 10 microseconds
	Timer1.attachInterrupt( timerIsr ); // attach the service routine here

	sPPM_Values.iThrust = 1500;
	sPPM_Values.iAil 	= 750;
	sPPM_Values.iEle 	= 750;
	sPPM_Values.iRud 	= 750;
	sPPM_Values.iGer	= 750;

	//Setup Serial Comms
	Serial.begin(115200);

	Serial.print("hello there");

}

void loop() 
{
	iLoopCout++;
	bool bEnoughData = false;
	if(Serial.available() > 15) //wait for 16 bytes of data {0x00 00,0x00 00,0x00 00,0x00 00,0x00 00}
		bEnoughData = true;
	while((bEnoughData)&&(Serial.available() > 0))
	{
		buffer[counter] = Serial.read();
		counter++;
	}
	if(counter>0)
		Serial.write(buffer, counter);

	char val1[3], val2[3],val3[3], val4[3],val5[3];
	if (bEnoughData)
	{
		sPPM_Values.iThrust = (*(uint16_t*) &buffer[1])&0xBFFF;
		sPPM_Values.iAil 	= (*(uint16_t*) &buffer[4])&0xBFFF;
		sPPM_Values.iEle 	= (*(uint16_t*) &buffer[7])&0xBFFF;
		sPPM_Values.iRud 	= (*(uint16_t*) &buffer[10])&0xBFFF;
		sPPM_Values.iGer	= (*(uint16_t*) &buffer[13])&0xBFFF;
	}
/*	if(iLoopCout%10==0)
	{
		Serial.print(sPPM_Values.iThrust);
		Serial.print(" ");
		Serial.print(sPPM_Values.iAil);
		Serial.print(" ");
		Serial.print(sPPM_Values.iEle);
		Serial.print(" ");
		Serial.print(sPPM_Values.iRud);
		Serial.print(" ");
		Serial.print(sPPM_Values.iGer);
		Serial.println(" ");
	}*/
	counter=0;
	StateMachine();
}
/// --------------------------
/// Custom ISR Timer Routine
/// --------------------------
void timerIsr()
{
	iPeriodCount++;
	iPulseCount++;


}

void StateMachine()
{
	if(iPeriodCount>=900)
		{
			iPeriodCount =0;
			iPulseCount =0;
			eCurrentState = prePulse0;//begin the pulse
			digitalWrite(PPMPin, 0);//Set low to start pulse
		}

	switch(eCurrentState)
	{
		case prePulse0:
			if(iPulseCount >= 20)
			{
				iPulseCount =0;
				eCurrentState = P1;//After waiting 400 us go to next pulse
				digitalWrite(PPMPin, 1);
			}
		break;

		case P1:
			if(iPulseCount >= sPPM_Values.iThrust/20)
			{
				iPulseCount =0;
				eCurrentState = prePulse1;//After waiting 400 us go to next pulse
				digitalWrite(PPMPin, 0);
			}
		break;

		case prePulse1:
			if(iPulseCount >= 20)
			{
				iPulseCount =0;
				eCurrentState = P2;//After waiting 400 us go to next pulse
				digitalWrite(PPMPin, 1);
			}
		break;

		case P2:
			if(iPulseCount >= sPPM_Values.iAil/20)
			{
				iPulseCount =0;
				eCurrentState = prePulse2;//After waiting 400 us go to next pulse
				digitalWrite(PPMPin, 0);
			}
		break;

		case prePulse2:
			if(iPulseCount >= 20)
			{
				iPulseCount =0;
				eCurrentState = P3;//After waiting 400 us go to next pulse
				digitalWrite(PPMPin, 1);
			}
		break;

		case P3:
			if(iPulseCount >= sPPM_Values.iEle/20)
			{
				iPulseCount =0;
				eCurrentState = prePulse3;//After waiting 400 us go to next pulse
				digitalWrite(PPMPin, 0);
			}
		break;

		case prePulse3:
			if(iPulseCount >= 20)
			{
				iPulseCount =0;
				eCurrentState = P4;//After waiting 400 us go to next pulse
				digitalWrite(PPMPin, 1);
			}
		break;

		case P4:
			if(iPulseCount >= sPPM_Values.iRud/20)
			{
				iPulseCount =0;
				eCurrentState = prePulse4;//After waiting 400 us go to next pulse
				digitalWrite(PPMPin, 0);
			}
		break;

		case prePulse4:
			if(iPulseCount >= 20)
			{
				iPulseCount =0;
				eCurrentState = P5;//After waiting 400 us go to next pulse
				digitalWrite(PPMPin, 1);
			}
		break;

		case P5:
			if(iPulseCount >= sPPM_Values.iGer/20)
			{
				iPulseCount =0;
				eCurrentState = prePulse5;//After waiting 400 us go to next pulse
				digitalWrite(PPMPin, 0);
			}
		break;

		case prePulse5:
			if(iPulseCount >= 20)
			{
				digitalWrite(PPMPin, 1);
			}
		break;

	}
}
