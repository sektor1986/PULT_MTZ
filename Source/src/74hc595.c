#include <p18cxxx.h>
#include "74hc595.h"

void Init74hc595(void)
{
	TRISBbits.TRISB4 = 0; //HC595_PIN_DATA
	LATBbits.LATB4 =  0;
	TRISBbits.TRISB5 = 0; //HC595_PIN_CLK
	LATBbits.LATB5 = 0;
	TRISCbits.TRISC5 = 0; //HC595_PIN_STROBE
	LATCbits.LATC5 = 0;
	TRISAbits.TRISA5 = 0; //OE 
	HC595_ENABLE
	Sethc595(0);
}

void Sethc595(unsigned int data74hc595)
{
	char i;

	for(i=16;i>0;i--)
	{
	
		if(data74hc595 & 0x8000)
			HC595_DATA_ON 
		else 
			HC595_DATA_OFF
		    
		data74hc595 <<= 1;
		HC595_CLK		
	}	
	HC595_STROBE

}
