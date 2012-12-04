#ifndef __74hc595_H
#define __74hc595_H

//74hc595
#define HC595_PIN_DATA		LATBbits.LATB4
#define HC595_PIN_CLK		LATBbits.LATB5
#define HC595_PIN_STROBE	LATCbits.LATC5
#define HC595_PIN_OE        LATAbits.LATA5

#define HC595_DATA_ON		HC595_PIN_DATA = 1;
#define HC595_DATA_OFF		HC595_PIN_DATA = 0;
#define HC595_CLK			{HC595_PIN_CLK = 1; HC595_PIN_CLK = 0;}
#define HC595_STROBE		{HC595_PIN_STROBE = 1; HC595_PIN_STROBE = 0;}
#define HC595_ENABLE        HC595_PIN_OE = 0;
#define HC595_DISABLE       HC595_PIN_OE = 1;

void Init74hc595(void);
void Sethc595(unsigned int data74hc595);

#endif // __74hc595_H
