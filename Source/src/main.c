#define USE_OR_MASKS
#include <p18cxxx.h>
#include "j1939.h"
#include "adc.h"
#include "usart.h"
#include "timers.h"
#include "keyboard.h"
#include "74hc595.h"
#include <delays.h>
#include "lcd.h"
#include <stdlib.h>

#pragma config XINST = OFF
#pragma config OSC=IRCIO67, WDT=OFF, LVP=OFF, MCLRE = OFF, PBADEN = OFF //, PWRT = ON //OSC=HS

#define TRUE  1
#define FALSE 0

#define TYPE_INT 	0
#define TYPE_FLOAT 	1
#define TYPE_UART   2

#define TYPE_J1939_CHAR 	0
#define TYPE_J1939_INT 		1
#define TYPE_J1939_LONG		2

#pragma udata bigdata

const unsigned char units[8][7] = {{"об/мин"}, {"л/ч   "}, {"В     "}, {"%     "}, {"км/ч  "}, {"град. "}, {"кПа   "}, {"      "}};

const unsigned char uart_name[11][2] = {"0", "1", "2", "3", "4", "5", "6", "L", "M" , "H", "N"};
const unsigned char uart_value[11] = {1, 2, 4, 8, 16, 32, 64, 193, 194, 196, 200};


typedef void (*j1939_update_func_ptr_t)(float value);

typedef enum en_gui_mode
{
	modeRPM,
	modeLFE,
	modeVEP,
	modeEFL,
	modeSPD,
	modeTemp,
	modeOilPress,
	modeTRANSM,
	maxMode	
}en_gui_mode_t;

typedef struct stc_message_control
{	
	unsigned char enable;
	unsigned char name[16];
	unsigned char number_unit;
	float resol;
	float value;
	float min_value;
	float max_value;
	float step_value;
	unsigned char type_value;
	unsigned char type_j1939_value;
	unsigned int PGN;
	unsigned char num_byte;
	unsigned int interval;
	unsigned int current_timer;
} stc_message_control_t;

static en_gui_mode_t lcd_mode = modeRPM;			// режим вывода информации на дисплее
static stc_message_control_t messageJ1939[maxMode] = {
		{TRUE, " Об. двигателя ", 0, 8,   2000, 0, 3500, 50,  TYPE_INT,   TYPE_J1939_INT,  0xF004, 3, 50,   0},
		{TRUE, " Расход топлива", 1, 20,  0, 0, 50.0, 0.5, TYPE_FLOAT, TYPE_J1939_INT,  0xFEF2, 0, 100,  0},
		{TRUE, " Напр. батареи ", 2, 20,  0, 0, 50.0, 0.5, TYPE_FLOAT, TYPE_J1939_INT,  0xFEF7, 6, 1000, 0},
		{TRUE, " Ур. охл. жидк.", 3, 2.5, 0, 0, 100,  1,   TYPE_INT,   TYPE_J1939_CHAR, 0xFEEF, 7, 500,  0},
		{TRUE, " Скорость      ", 4, 256, 0, 0, 160,  1,   TYPE_INT,   TYPE_J1939_INT,  0xFE6C, 6, 50,   0},
		{TRUE, " Температура   ", 5, 1  , 70, 0, 140,  5,   TYPE_INT,   TYPE_J1939_CHAR, 0xFEEE, 0, 1000, 0},
		{TRUE, " Давление      ", 6, 0.25,500, 0, 750 , 50,  TYPE_INT,   TYPE_J1939_CHAR, 0xFEEF, 3, 500,  0},
		{TRUE, " Тек. передача ", 7, 1,   0, 0, 10,   1,   TYPE_UART,  TYPE_J1939_INT,  0x0000, 0, 500,  0}};


unsigned char name[8] = {0};
unsigned int TimerValue = 0;
unsigned char flag_interrupt = 0;
unsigned char flag_interrupt2 = 0;

unsigned int ADCResult=0;
float voltage=0;

unsigned int valueRPM = 0;
float valueLFE = 0;
float valueVER = 0;
unsigned char valueEFL  = 0;

void adc_init(void);
void uart_init(void);
void mcu_init(void);
void timer_init(void);
void User_Timer(void);
void J1939_update(void);
// Процедуры для отправки сообщений по CAN
void SendJ1939(unsigned char num);
void LCD_init(void);
void InitModeLCD(en_gui_mode_t mode);
void ValueToLCD(en_gui_mode_t mode);
void DelayMs(unsigned int ms);


 
void SendAirTemperatur(unsigned int value);
void SendFuelLevel(unsigned char value);
void ButtonCallback(unsigned char u16ButtonId, en_button_state_t enState);

void InterruptHandlerHigh (void);

#pragma code InterruptVectorHigh  = 0x08     
void InterruptVectorHigh (void) 
{
	_asm
		goto InterruptHandlerHigh  //jump to interrupt routine
	_endasm
}

#pragma code
#pragma interrupt InterruptHandlerHigh      
void InterruptHandlerHigh () 
{
	// Interrupt Timer 1
	if (PIR1bits.TMR1IF)
	{		
		PIR1bits.TMR1IF = 0; 
		WriteTimer1(0xE0C1);
		User_Timer();
		J1939_update();			
	}	
}

void main(void)
{
	unsigned long int time = 0l;
	unsigned long int address;
	unsigned char status=0;
	unsigned char count = 0,i;
    unsigned int k = 0;

	mcu_init();
	adc_init();
	uart_init();
	
//	for (i = 0; i < maxMode; i++)
//		messageJ1939[i].enable = TRUE;
	DelayMs(50);
	OpenXLCD(FOUR_BIT & LINES_5X7);
	DelayMs(50);
	WriteCmdXLCD (DON & CURSOR_OFF & BLINK_OFF);
	LCD_init();
	RCONbits.IPEN = 1; 
	INTCONbits.GIEH = 1;
	INTCONbits.GIEL = 1;

	J1939_init(0x21,name);	
	Button_init(ButtonCallback);
	timer_init();

	while (1)
	{

	}
	return;
}

void adc_init(void)
{
    unsigned char channel=0x00,config1=0x00,config2=0x00,config3=0x00,portconfig=0x00,i=0;

	//-- clear adc interrupt and turn off adc if in case was on prerviously---
    CloseADC();
    config1 = ADC_FOSC_4 | ADC_RIGHT_JUST | ADC_16_TAD ;
    config2 = ADC_CH0 | ADC_INT_ON | ADC_REF_VDD_VSS;
	portconfig = ADC_10ANA ;

    OpenADC(config1,config2,portconfig);
    ADC_INT_ENABLE();	
}

void uart_init(void)
{
	unsigned char config=0,spbrg=0,baudconfig=0,i=0;

	CloseUSART();

	//-----configure USART -----
    config = USART_TX_INT_OFF | USART_RX_INT_OFF | USART_ASYNCH_MODE | USART_EIGHT_BIT | USART_CONT_RX | USART_BRGH_LOW;
	//-----SPBRG needs to be changed depending upon oscillator frequency-------
    //spbrg = 487;                    //At 32Mhz of oscillator frequency & baud rate of 1024
	spbrg = 121;
	/*
	Asynchronous mode, high speed:
	Fosc / (16 * (spbrg + 1))

	Asynchronous mode, low speed:
	Fosc / (64 * (spbrg + 1))
	
	Synchronous mode:
	Fosc / (4 * (spbrg + 1)).
	*/

    OpenUSART(config, spbrg);        //API configures USART for desired parameters

    baudconfig =  BAUD_8_BIT_RATE | BAUD_AUTO_OFF;
    baudUSART (baudconfig);
	
	
	//------USART Transmission ----
//    while(BusyUSART());             //Check if Usart is busy or not
//    	WriteUSART(0x00);           
}

void mcu_init(void) 
{ 
    // Set the internal clock to 16mHz 

	OSCCONbits.IRCF0 = 1; 
	OSCCONbits.IRCF1 = 1; 
	OSCCONbits.IRCF2 = 1; 
	OSCTUNEbits.PLLEN = 1; //Frequency Multiplier PLL for INTOSC Enable bit
} 

void timer_init(void)
{
	WriteTimer1(0);
	OpenTimer1(T1_16BIT_RW | T1_SOURCE_INT | T1_PS_1_1 | T1_OSC1EN_OFF | T1_SYNC_EXT_OFF | TIMER_INT_ON); 
	WriteTimer1(0xE0C1);  // Reload 1 ms 32MHz
//	WriteTimer1(0xD8F0);  // Reload 1 ms 10MHz
}

void User_Timer(void)
{
	TimerValue++;

	if (TimerValue == 10)
	{
		Button_update();
		TimerValue = 0;
	}	
}

void LCD_init(void)
{
	InitModeLCD(lcd_mode);
	ValueToLCD(lcd_mode);
}

void J1939_update(void)
{
	unsigned char i = 0;
	for (i = 0; i < maxMode; i++)
	{
		if (messageJ1939[i].current_timer == messageJ1939[i].interval)	
		{
			if (messageJ1939[i].enable)
				if (i != modeTRANSM)
					SendJ1939(i);
				else
					WriteUSART(uart_value[(unsigned char)messageJ1939[i].value]);	
				//messageJ1939[i].func_upd_j1939(messageJ1939[i].value);
			messageJ1939[i].current_timer = 0;	
		}
		else
			messageJ1939[i].current_timer++;		
	}
}
/* Событие для кнопки */
void ButtonCallback(unsigned char u16ButtonId, en_button_state_t enState)
{
	unsigned char lcd_buf[4];
	static unsigned int pos = 0;
	
	if (enState == StateHigh)
	{		
		switch (u16ButtonId)
		{
			case 4:
				lcd_mode++;
				if (lcd_mode == maxMode)
					lcd_mode = 0;
				InitModeLCD(lcd_mode);
				ValueToLCD(lcd_mode);
				break;
			case 5:
				if (messageJ1939[lcd_mode].value > messageJ1939[lcd_mode].min_value)
					messageJ1939[lcd_mode].value -= messageJ1939[lcd_mode].step_value;
				ValueToLCD(lcd_mode);
				break;
			case 6:
				if (messageJ1939[lcd_mode].value < messageJ1939[lcd_mode].max_value)
					messageJ1939[lcd_mode].value += messageJ1939[lcd_mode].step_value;
				ValueToLCD(lcd_mode);
				break;
			case 7:
				if (messageJ1939[lcd_mode].enable == TRUE)
					messageJ1939[lcd_mode].enable = FALSE;
				else
					messageJ1939[lcd_mode].enable = TRUE;	
				//messageJ1939[lcd_mode].enable = ~messageJ1939[lcd_mode].enable;
				ValueToLCD(lcd_mode);
				break;
			default:
				break;
		}
		
		TRISBbits.TRISB4 = 0; //HC595_PIN_DATA
		TRISBbits.TRISB5 = 0; //HC595_PIN_CLK
	}
}

void InitModeLCD(en_gui_mode_t mode)
{
	lcd_mode = mode;
	SetDDRamAddr(0x00);
	putsXLCD(messageJ1939[mode].name);
}

void ValueToLCD(en_gui_mode_t mode)
{
	clearBottomLCD();

	SetDDRamAddr(0x37);
	if (messageJ1939[mode].enable)
		WriteDataXLCD(0x2B);
	else
		WriteDataXLCD(0x2D);
	
	//SetDDRamAddr(0x29);	
	if (messageJ1939[mode].type_value == TYPE_INT)
	{
		IntToLCD(messageJ1939[mode].value, 0x29);	
		SetDDRamAddr(0x2F);	
		putsXLCD(units[messageJ1939[mode].number_unit]);			
	}
	else if (messageJ1939[mode].type_value == TYPE_FLOAT)
	{
		FloatToLCD(messageJ1939[mode].value, 0, 0x29);	
		SetDDRamAddr(0x2F);	
		putsXLCD(units[messageJ1939[mode].number_unit]);			
	}
	else if (messageJ1939[mode].type_value == TYPE_UART)
	{
		SetDDRamAddr(0x2F);	
		putsXLCD(uart_name[(unsigned char)messageJ1939[mode].value]);
		//SetDDRamAddr(0x2F);	
		//putsXLCD(units[messageJ1939[mode].number_unit]);	
	}

}

void SendJ1939(unsigned char num)  
{
	unsigned long tempValue = 0;
	struct J1939_message msg;

	msg.PDUformat =  (messageJ1939[num].PGN >> 8) & 0xFF;
	msg.PDUspecific = messageJ1939[num].PGN & 0xFF;
	msg.priority = J1939_INFO_PRIORITY;
	msg.sourceAddr = 0x00;
	msg.dataLen = 8;
	msg.r = 0;
	msg.dp = 0;
	msg.data[0] = 0xFF;
	msg.data[1] = 0xFF;
	msg.data[2] = 0xFF;
	msg.data[3] = 0xFF;
	msg.data[4] = 0xFF;
	msg.data[5] = 0xFF;
	msg.data[6] = 0xFF;
	msg.data[7] = 0xFF;
	
	tempValue = messageJ1939[num].value * messageJ1939[num].resol; 
	if (num == modeTemp)
		tempValue += 40; 
	switch (messageJ1939[num].type_j1939_value)
	{
		case TYPE_J1939_CHAR:
			msg.data[messageJ1939[num].num_byte] = tempValue & 0xFF;		
			break;
		case TYPE_J1939_INT:
			msg.data[messageJ1939[num].num_byte] = tempValue & 0xFF;
			msg.data[messageJ1939[num].num_byte+1] = (tempValue>>8) & 0xFF;	
			break;
		case TYPE_J1939_LONG:
			msg.data[messageJ1939[num].num_byte] = tempValue & 0xFF;
			msg.data[messageJ1939[num].num_byte+1] = (tempValue>>8) & 0xFF;	
			msg.data[messageJ1939[num].num_byte+2] = (tempValue>>16) & 0xFF;	
			msg.data[messageJ1939[num].num_byte+3] = (tempValue>>24) & 0xFF;	
			break;			
		default: 
			break;
	}
	
	J1939_Send(&msg);
	J1939_poll(5);
}


void DelayMs(unsigned int ms)
{
	unsigned int i;
	for (i=0;i<ms;i++)
	{	
  		Delay1KTCYx(8);  // Cycles = (TimeDelay * Fosc) / 4
	}
}

void DelayFor18TCY( void )
{
     char i;
     for (i=0;i<12;i++)
     {
         Nop();
     }
}

void DelayPORXLCD (void)
{
	DelayMs(15);
  return;
}
void DelayXLCD (void)
{
	DelayMs(5);
  return;
}

