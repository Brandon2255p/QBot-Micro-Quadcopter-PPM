#include "Arduino.h"
#include <stdio.h>

int PPM_Pin = 2;
unsigned long tMark = 0;
enum ePPM_State {WAIT, SPACE, THR, AIL, ELE, RUD, GER, VOID};
unsigned long tPulseStart =0;
char buffer[16];
int counter =0;
int iLoopCout = 0;

struct PPM_State
{
  unsigned long wait;
  ePPM_State eState;
  ePPM_State eNextState;
  int pinState;
} sCurrentState;

struct PPM_Values
{
	int iThrust;
	int iAil;
	int iEle;
	int iRud;
	int iGer;
} sPPM_Values;

void setup() {
	pinMode(PPM_Pin, OUTPUT);  
	pinMode(13, OUTPUT);
	digitalWrite(13, LOW);
	Serial.begin(115200);   
	sCurrentState.eNextState = THR; 
	sCurrentState.eState = SPACE;
	sCurrentState.pinState = LOW;
	sCurrentState.wait = 400;
	tMark = micros();
	tPulseStart = micros();
	SetValues(755, 755, 755, 755, 755);
	Serial.print("hello there");
}

void loop() 
{
	iLoopCout++;
	bool bEnoughData = false;
	if(Serial.available() > 15) //wait for 16 bytes of data {0x0000,0x0000,0x0000,0x0000,0x0000}
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
		sscanf(buffer, "{%[^,],%[^,],%[^,],%[^,],%[^}]}", val1, val2, val3, val4, val5);
		sPPM_Values.iThrust = (*(uint16_t*) &val1[0])&0xBFFF;
		sPPM_Values.iAil 	= (*(uint16_t*) &val2[0])&0xBFFF;
		sPPM_Values.iEle 	= (*(uint16_t*) &val3[0])&0xBFFF;
		sPPM_Values.iRud 	= (*(uint16_t*) &val4[0])&0xBFFF;
		sPPM_Values.iGer	= (*(uint16_t*) &val5[0])&0xBFFF;
	}
	/*if(iLoopCout%10000==0)
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


	SetValues(sPPM_Values.iThrust, sPPM_Values.iAil, sPPM_Values.iEle, sPPM_Values.iRud, sPPM_Values.iGer);
	PPMStateMachine();
}

void SetValues(int Thr, int Ail, int Ele, int Rud, int Ger)
{
	sPPM_Values.iThrust = Thr;
	sPPM_Values.iAil = Ail;
	sPPM_Values.iEle= Ele;
	sPPM_Values.iRud= Rud;
	sPPM_Values.iGer= Ger;
}

void PPMStateMachine()
{
	unsigned long timeDiff = micros() - tMark;
		unsigned tPulseLength = micros() - tPulseStart;
		if(tPulseLength < 20000)
		{
			if(timeDiff >= sCurrentState.wait)
			{
				switch (sCurrentState.eState)
				{
				case WAIT:
					//SetSpace();
				break;
				case SPACE:
					/// Switch for space
					switch(sCurrentState.eNextState)
					{
					case THR:
						SetTHR();
						break;
					case AIL:
						SetAIL();
						break;
					case ELE:
						SetELE();
						break;
					case RUD:
						SetRUD();
						break;
					case GER:
						SetGER();
						break;
					case VOID:
						SetVOID();
						break;
					case WAIT:
						SetWait();
						break;
					default:
						sCurrentState.eState = WAIT;
						sCurrentState.eNextState = SPACE;
					} /// End switch for space
				  break;
				default:
				  SetSpace();
				}
				tMark = micros();
				digitalWrite(PPM_Pin, sCurrentState.pinState);
			}
		}
		else
		{
			tPulseStart = micros();
			SetSpace();
		}
}

void SetSpace()
{
	sCurrentState.eState = SPACE;
	sCurrentState.pinState = LOW;
	sCurrentState.wait = 390;
}

void SetWait()
{
	sCurrentState.eState = WAIT;
	sCurrentState.eNextState = VOID;
	sCurrentState.pinState = HIGH;
	sCurrentState.wait = 7000;
}

void SetTHR()
{
	sCurrentState.eState = THR;
	sCurrentState.eNextState = AIL;
	sCurrentState.pinState = HIGH;
	sCurrentState.wait = sPPM_Values.iThrust;
}
void SetAIL()
{
	sCurrentState.eState = AIL;
	sCurrentState.eNextState = ELE;
	sCurrentState.pinState = HIGH;
	sCurrentState.wait = sPPM_Values.iAil;
}
void SetELE()
{
	sCurrentState.eState = ELE;
	sCurrentState.eNextState = RUD;
	sCurrentState.pinState = HIGH;
	sCurrentState.wait = sPPM_Values.iEle;
}
void SetRUD()
{
	sCurrentState.eState = RUD;
	sCurrentState.eNextState = GER;
	sCurrentState.pinState = HIGH;
	sCurrentState.wait = sPPM_Values.iRud;
}
void SetGER()
{
	sCurrentState.eState = GER;
	sCurrentState.eNextState = WAIT;
	sCurrentState.pinState = HIGH;
	sCurrentState.wait = sPPM_Values.iGer;
}

void SetVOID()
{
	sCurrentState.eState = VOID;
	sCurrentState.eNextState = THR;
	sCurrentState.pinState = HIGH;
	sCurrentState.wait = 500;// Time doesnt do anything
}
