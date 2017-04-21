#include "Infrared.h"
#include "MainProc.h"
#include "gpio.h"

static uint8_t InfraredDecode(uint8_t * code);
static void InfraredRecvInt(uint32_t pinArray);
static void InfraredExecute(uint8_t* code);

void InfraredInit()
{
    GPIO_QuickInit(IR_PORT, IR_PIN, kGPIO_Mode_IPU);
    GPIO_CallbackInstall(IR_PORT, InfraredRecvInt);
    GPIO_ITDMAConfig(IR_PORT, IR_PIN, kGPIO_IT_FallingEdge, true);
}

uint8_t InfraredDecode(uint8_t * code)
{
	uint8_t i,j;
	uint32_t time_out = 0;
	while((IR_READ == 0) && (time_out < 30*10))
	{
		time_out++;
		DelayUs(100);
	}
	if(time_out > 92 || time_out < 87) return 1;
	time_out = 0;
	while((IR_READ == 1) && (time_out < 30*10))
	{
		time_out++;
		DelayUs(100);
	}
	if((time_out > 47) || (time_out < 43)) return 2;

	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 8; j++)
		{
			while(IR_READ == 0);
			time_out = 0;
			while(IR_READ == 1)
			{
				DelayUs(100);
				time_out++;
			}
			code[i] = code[i]>>1;
			if(time_out > 9) code[i] |= 0x80;
		}
	}
	return 0;
}

void InfraredRecvInt(uint32_t pinArray)
{
    GPIO_ITDMAConfig(IR_PORT, IR_PIN, kGPIO_IT_FallingEdge, false);
    uint8_t code[4];
    if(!InfraredDecode(code))
    {
		InfraredExecute(code + 2);
        BUZZLE_ON;
        DelayMs(100);
        BUZZLE_OFF;
    }
    GPIO_ITDMAConfig(IR_PORT, IR_PIN, kGPIO_IT_FallingEdge, true);
}

void InfraredExecute(uint8_t* code)
{
	switch(*code)
	{
		case 0x0: motor_on = false; break; // stop
		case 0x1: motor_on = true; break; // start
    }
}
