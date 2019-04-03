#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include "global_vars.h"
#include "selftest.h"
#include "requests.h"


void initAll()
{
	// deactivate WDT ENTIRELY!
	MCUSR &= ~_BV(WDRF);
	WDTCSR = 0;
	wdt_disable();
	
	// enable interrupts before configuring io
	sei();

	spi.init();
	
	dio0.setDirA(0x00); // alle Ausgang
	dio0.setDirB(0xFF); // alle Eingang
	dio1.setDirA(0x00); // alle Ausgang
	dio1.setDirB(0xFF); // alle Eingang
	dsw.setDirB(0xFF); // alle Eingang
	adu.init();
	
	usart.init();
	usart.initRX();
}

void handleRequest()
{
	const uint8_t req = usart.readByte();
	
	wdt_reset();
	
	switch(req)
	{
		case RQ_DISC:
			break;

		case RQ_TEST:
			rqTestConnection();
			break;
		
		case RQ_INFO:
			rqBoardInfo();
			break;
		
		case RQ_INT:
			rqTestIntConv();
			break;

		case RQ_ST:
			rqSelfTest();
			break;
			
		case RQ_BA0:
			rqDigitalWrite0();
			break;

		case RQ_BA1:
			rqDigitalWrite1();
			break;

		case RQ_BE0:
			rqDigitalRead0();
			break;

		case RQ_BE1:
			rqDigitalRead1();
			break;
			
		case RQ_DSW:
			rqReadDipSwitch();
			break;

		case RQ_AA0:
			rqAnalogWrite0();
			break;

		case RQ_AA1:
			rqAnalogWrite1();
			break;

		case RQ_ADC:
			rqAnalogRead();
			break;
			
		case RQ_ADC_DAC_STROKE:
			rqAdcDacStroke();
			break;

		default:
			break;
	}
	
	usart.initRX();
	
	wdt_disable();
}

int main()
{
	initAll();

	while(1)
	{
		if(nextRequest)
		{
			nextRequest = false;
			handleRequest();
		}
		_delay_us(1);
	}

	return 0;
}
