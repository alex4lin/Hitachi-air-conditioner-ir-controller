#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "irslinger.h"

int main(int argc, char *argv[])
{
	uint32_t outPin = 17;           // The Broadcom pin number the signal will be sent on
	int frequency = 38000;          // The frequency of the IR signal in Hz
	double dutyCycle = 0.5;         // The duty cycle of the IR signal. 0.5 means for every cycle,
	                                // the LED will turn on for half the cycle time, and off the other half
        
	int leadingPulseDuration = 3392;
	int leadingGapDuration = 1695;
	int onePulse = 446;
	int zeroPulse = 446;
	int oneGap = 1240;
	int zeroGap = 483;
	int sendTrailingPulse = 1;
	
	int key;
        int temperature; // setting temperature, range 16~32
	int wind; // 1 silent, 2 slight, 3 weak, 4 strong, 5 auto
        int mode; // 1 air supply, 3 cold, 5 dehumidify, 7 auto control, 10 warm
	int power; // 1:on 0:off
	int daily;
        int sleepHours;
	int timerOn, timerOff;
	int timerOffMin, timerOnMin;
        
	int i, j, index;
	int setCode[20];
	int setCode2[43];
	int codes[691];
	
	//get date and time
	int c_month;
	int c_day;
	int c_hour;
	int c_min;
        time_t timep;
	struct tm *p;
	time(&timep);
	p=localtime(&timep);
        c_month = 1 + p->tm_mon;
	c_day = p->tm_mday;
	c_hour = p->tm_hour;
	c_min = p->tm_min;
        
	printf("Current time %d/%d %d:%d\n", c_month,c_day,c_hour,c_min);

	if (argc != 6) {
	    printf("Usage: Hitachi key temperature wind mode power\n");
            printf("       key: 0x13 start/stop, 0x22 reserve, 0x24 cancel, 0x31 sleep,\n");
            printf("            0x41 mode, 0x42 wind speed, 0x43 temp down, 0x44 temp up,\n");
            printf("            0x71 dehumidify, 0x81 auto wind direction, 0x89 quick\n");
            printf("       temperature: 16~32 degree\n");
            printf("       wind: 1 silent, 2 slight, 3 weak, 4 strong, 5 auto \n");
            printf("       mode: 1 air supply, 3 cold, 5 dehumidify, 6 warm,  7 auto control\n");
            printf("       power: 1 on, 0 off \n");
	    exit(1);
	}
	
	//collect arguments
	sscanf(argv[1],"%x",&key);
        sscanf(argv[2],"%i",&temperature);
        sscanf(argv[3],"%i",&wind);
        sscanf(argv[4],"%i",&mode);
        sscanf(argv[5],"%i",&power);

	daily = 0; // 0 false
	sleepHours = 8;
	timerOn = 0; //0 false
	timerOff = 0; // 0 falses
	timerOffMin = 0;
	timerOnMin = 0;

	//compile arguments into setCode
	setCode[0] = 0x40;
	setCode[1] = 0xff;
	setCode[2] = 0xcc;

	// printf("key = 0x%2x\n",key);
	
	switch(key) {
            case 0x13: setCode[3] = 0x92; break;
            case 0x22: if (daily != 0) {setCode[3] = 0x98;} else {setCode[3] = 0x92;} break;
	    case 0x24: setCode[3] = 0x92; break;
	    case 0x31: setCode[3] = 0x92; break;
            case 0x41: setCode[3] = 0x92; break;
            case 0x42: if (wind==1) {setCode[3] = 0x98;} else {setCode[3] = 0x92;} break;
            case 0x43: setCode[3] = 0x92; break;
            case 0x44: setCode[3] = 0x92; break;
	    case 0x71: setCode[3] = 0x94; break;
	    case 0x81: setCode[3] = 0x92; break;
	    case 0x89: setCode[3] = 0x96; break;
	    default: printf("Unknown key!\n"); exit(-1); 
	}	

	setCode[4] = key; // press key

	if (mode == 7) {
            setCode[5] = 0x04;
	} else {
	    setCode[5] = temperature << 2; // set temperature
	};
	
	// clear the timer region
	setCode[6] = 0;
	setCode[7] = 0;
	setCode[8] = 0;
	setCode[9] = 0;
	setCode[10] = 0;

	if (key==0x31) {
   	    setCode[6] = (sleepHours * 60) & 0xff;
	    setCode[7] = ((sleepHours * 60) >> 8) & 0x0f;
	};

	if (key==0x22) {
	    setCode[7] = (timerOffMin & 0x0f) << 4;
            setCode[8] = (timerOffMin >> 4);
	    setCode[9] = (timerOnMin & 0xff);
            setCode[10] = (timerOnMin >> 8) & 0x0f;
	    if (daily != 0) setCode[10] |= 0x80;
	    if (key==0x31) setCode[10] |= 0x40;
	    if (timerOn != 0) setCode[10] |= 0x20;
	    if (timerOff != 0) setCode[10] |= 0x10;
	};    
        
	// printf("wind = 0x%02x\n",wind);
	
	if (wind >= 1 && wind <= 5) {
	    setCode[11] = wind << 4;
	} else { printf("Wrong wind!"); exit(-1); }
	
	// printf("mode = 0x%02x\n",mode);
	
	
	switch(mode) {
	    case 3: //cold
	    case 6: //warm
		    setCode[11] |= mode;
                    break;

            case 1: //air supply   
	            if (wind<=4) { setCode[11] |= mode;
	            } else { printf("Wrong wind!\n"); exit(-1); }
	            break;

            case 5: //dehumidify	
                    if (wind<=2) { setCode[11] |= mode;
	            } else { printf("Wrong wind!\n"); exit(-1); }
                    break;

            case 7: //auto control
	            if ((wind <=2)||(wind==5)) { setCode[11] |= mode;
	            } else { printf("Wrong wind!\n"); exit(-1); }
                    break;
	    
	    default: printf("Unknown mode!\n"); exit(-1); 
        }

	if (power==1) {
            setCode[12] = 0xf1; // power ON
	} else setCode[12]= 0xe1; //power OFF

	//set date and time
	setCode[13] = c_day;
	setCode[14] = c_month;
	setCode[15] = 0x80;
	setCode[16] = 0x03;
	setCode[17] = 0;
	setCode[18] = (c_hour * 60 + c_min) >> 8;
	setCode[19] = (c_hour * 60 + c_min) & 0xff;
	
	printf("setCode = ");
	for (i=0;i<20;i++) printf(" 0x%02X", setCode[i]);
	printf("\n");
        
        setCode2[0] = 0x01;
	setCode2[1] = 0x10;
	setCode2[2] = 0;
	for (i=0;i<20;i++) {
	    setCode2[i*2+3] = setCode[i];
	    setCode2[i*2+4] = (setCode[i] ^ 0xff) & 0xff;
	};
/*
	printf("setCode2 = ");
	for (i=0;i<43;i++) printf(" 0x%02X", setCode2[i]);
	printf("\n");
*/

	//set leading pulse/gap
        codes[0] = leadingPulseDuration;
        codes[1] = leadingGapDuration;
	codes[690] = onePulse; //sending trailing pulse

	for (i=0;i<43;i++) {
	    for (j=0;j<8;j++) {
		index = i*16+j*2+2;
                if ((setCode2[i] >> j) & 1) {
		    codes[index]=onePulse;
	            codes[index+1]=oneGap;	    
		}else {
		    codes[index]=zeroPulse;
	            codes[index+1]=zeroGap;	    

		};
	    };
	};
/*	
	printf("codes = ");
	for (i=0;i<691;i++) {
	    if (i%16==2) printf("\n");
	    printf(" %i",codes[i]);
	}
        printf("\n");
*/	

	int result = irSlingRaw(
		outPin,
		frequency,
		dutyCycle,
		codes,
		sizeof(codes) / sizeof(int));
	
	return result; 
}
