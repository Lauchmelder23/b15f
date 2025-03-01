#ifndef GLOBAL_VARS_H
#define GLOBAL_VARS_H

#include "mcp23s17.h"
#include "tlc5615.h"
#include "adu.h"
#include "usart.h"
#include "pwm.h"
#include "servo.h"


#define WDT_TIMEOUT WDTO_15MS

extern volatile SPI spi;
extern volatile MCP23S17 dio0;
extern volatile MCP23S17 dio1;
extern volatile MCP23S17 dsw;
extern volatile TLC5615 dac0;
extern volatile TLC5615 dac1;
extern volatile ADU adu;
extern volatile USART usart;
extern volatile PWM pwm;
extern volatile bool nextRequest;
extern volatile uint16_t interruptCounters[];
extern volatile Servo servo;

#endif // GLOBAL_VARS_H
