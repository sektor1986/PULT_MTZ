#include "lcd.h"

char simbolsLatin[] = { 0x41,0xa0,0x42,0xa1,0xe0,0x45,0xa3,0xa4,0xa5,0xa6,0x4b,0xa7,0x4d,0x48,0x4f,0xa8,
                                       0x50,0x43,0x54,0xa9,0xaa,0x58,0xe1,0xab,0xac,0xe2,0xad,0xae,0x62,0xaf,0xb0,0xb1,
                                       0x61,0xb2,0xb3,0xb4,0xe3,0x65,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0x6f,0xbe,
                                       0x70,0x63,0xbf,0x79,0xe4,0x78,0xe5,0xc0,0xc1,0xe6,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7 }; 

void OpenXLCD(unsigned char lcdtype)
{
// 4-bit mode
#ifdef UPPER                            // Upper 4-bits of the port
	DATA_PORT &= 0x0f;
	TRIS_DATA_PORT &= 0x0F;
#else                                   // Lower 4-bits of the port
	DATA_PORT &= 0xf0;
	TRIS_DATA_PORT &= 0xF0;
#endif
	TRIS_RW = 0;                    // All control signals made outputs
	TRIS_RS = 0;
	TRIS_E = 0;
	RW_PIN = 0;                     // R/W pin made low
	RS_PIN = 0;                     // Register select pin made low
	E_PIN = 0;                      // Clock pin made low

	// Delay for 15ms to allow for LCD Power on reset
	DelayPORXLCD();
 //-------------------reset procedure through software----------------------       
	WriteCmdXLCD(0x30);
	Delay1KTCYx(33);

	WriteCmdXLCD(0x30);
	Delay1KTCYx(0x01);


	WriteCmdXLCD(0x32);
	while( BusyXLCD() );
//------------------------------------------------------------------------------------------

	// Set data interface width, # lines, font
	while(BusyXLCD());              // Wait if LCD busy
	WriteCmdXLCD(lcdtype);          // Function set cmd

	// Turn the display on then off
	while(BusyXLCD());              // Wait if LCD busy
	WriteCmdXLCD(DOFF&CURSOR_OFF&BLINK_OFF);        // Display OFF/Blink OFF
	while(BusyXLCD());              // Wait if LCD busy
	WriteCmdXLCD(DON&CURSOR_ON&BLINK_ON);           // Display ON/Blink ON

	// Clear display
	while(BusyXLCD());              // Wait if LCD busy
	WriteCmdXLCD(0x01);             // Clear display

	// Set entry mode inc, no shift
	while(BusyXLCD());              // Wait if LCD busy
	WriteCmdXLCD(SHIFT_CUR_LEFT);   // Entry Mode

	// Set DD Ram address to 0
	while(BusyXLCD());              // Wait if LCD busy
	SetDDRamAddr(0x80);                // Set Display data ram address to 0

	return;
}

void WriteCmdXLCD(unsigned char cmd)
{
	// 4-bit interface
#ifdef UPPER                            // Upper nibble interface
        TRIS_DATA_PORT &= 0x0f;
        DATA_PORT &= 0x0f;
        DATA_PORT |= cmd&0xf0;
#else                                   // Lower nibble interface
        TRIS_DATA_PORT &= 0xf0;
        DATA_PORT &= 0xf0;
        DATA_PORT |= (cmd>>4)&0x0f;
#endif
        RW_PIN = 0;                     // Set control signals for command
        RS_PIN = 0;
        DelayFor18TCY();
        E_PIN = 1;                      // Clock command in
        DelayFor18TCY();
        E_PIN = 0;
#ifdef UPPER                            // Upper nibble interface
        DATA_PORT &= 0x0f;
        DATA_PORT |= (cmd<<4)&0xf0;
#else                                   // Lower nibble interface
        DATA_PORT &= 0xf0;
        DATA_PORT |= cmd&0x0f;
#endif
        DelayFor18TCY();
        E_PIN = 1;                      // Clock command in
        DelayFor18TCY();
        E_PIN = 0;
#ifdef UPPER                            // Make data nibble input
        TRIS_DATA_PORT |= 0xf0;
#else
        TRIS_DATA_PORT |= 0x0f;
#endif

	return;
}

void WriteDataXLCD(unsigned char data)
{
#ifdef UPPER                            // Upper nibble interface
	TRIS_DATA_PORT &= 0x0f;
	DATA_PORT &= 0x0f;
	DATA_PORT |= data&0xf0;
#else                                   // Lower nibble interface
	TRIS_DATA_PORT &= 0xf0;
	DATA_PORT &= 0xf0;
	DATA_PORT |= ((data>>4)&0x0f);
#endif
	RS_PIN = 1;                     // Set control bits
	RW_PIN = 0;
	DelayFor18TCY();
	E_PIN = 1;                      // Clock nibble into LCD
	DelayFor18TCY();
	E_PIN = 0;
#ifdef UPPER                            // Upper nibble interface
	DATA_PORT &= 0x0f;
	DATA_PORT |= ((data<<4)&0xf0);
#else                                   // Lower nibble interface
	DATA_PORT &= 0xf0;
	DATA_PORT |= (data&0x0f);
#endif
	DelayFor18TCY();
	E_PIN = 1;                      // Clock nibble into LCD
	DelayFor18TCY();
	E_PIN = 0;
#ifdef UPPER                            // Upper nibble interface
	TRIS_DATA_PORT |= 0xf0;
#else                                   // Lower nibble interface
	TRIS_DATA_PORT |= 0x0f;
#endif

	return;
}

unsigned char BusyXLCD(void)
{
	RW_PIN = 1;                     // Set the control bits for read
	RS_PIN = 0;
	DelayFor18TCY();
	E_PIN = 1;                      // Clock in the command
	DelayFor18TCY();
                                   // 4-bit interface
#ifdef UPPER                            // Upper nibble interface
	if(DATA_PORT&0x80)
#else                                   // Lower nibble interface
	if(DATA_PORT&0x08)
#endif
	{
		E_PIN = 0;              // Reset clock line
		DelayFor18TCY();
		E_PIN = 1;              // Clock out other nibble
		DelayFor18TCY();
		E_PIN = 0;
		RW_PIN = 0;             // Reset control line
		return 1;               // Return TRUE
	}
	 else                            // Busy bit is low
	{
		E_PIN = 0;              // Reset clock line
		DelayFor18TCY();
		E_PIN = 1;              // Clock out other nibble
		DelayFor18TCY();
		E_PIN = 0;
		RW_PIN = 0;             // Reset control line
		return 0;               // Return FALSE
	}
}

void putrsXLCD(const rom char *buffer)
{
        while(*buffer)                  // Write data to LCD up to null
        {
                while(BusyXLCD());      // Wait while LCD is busy
                WriteDataXLCD(*buffer); // Write character to LCD
                buffer++;               // Increment buffer
        }
        return;
}

void putsXLCD(unsigned char *buffer)
{
        while(*buffer)                  // Write data to LCD up to null
        {
//                while(BusyXLCD());      // Wait while LCD is busy
			if (buffer[0] > 191)
				WriteDataXLCD(simbolsLatin[buffer[0] - 192]);
			else
				WriteDataXLCD(*buffer); // Write character to LCD
			buffer++;               // Increment buffer
        }
        return;
}

unsigned char ReadAddrXLCD(void)
{
	char data;                      // Holds the data retrieved from the LCD

	RW_PIN = 1;                     // Set control bits for the read
	RS_PIN = 0;
	DelayFor18TCY();
	E_PIN = 1;  

#ifdef UPPER                            // Upper nibble interface
	data = DATA_PORT&0xf0;          // Read the nibble into the upper nibble of data
#else                                   // Lower nibble interface
	data = (DATA_PORT<<4)&0xf0;     // Read the nibble into the upper nibble of data
#endif
	E_PIN = 0;                      // Reset the clock
	DelayFor18TCY();
	E_PIN = 1;                      // Clock out the lower nibble
	DelayFor18TCY();
#ifdef UPPER                            // Upper nibble interface
	data |= (DATA_PORT>>4)&0x0f;    // Read the nibble into the lower nibble of data
#else                                   // Lower nibble interface
	data |= DATA_PORT&0x0f;         // Read the nibble into the lower nibble of data
#endif
	E_PIN = 0;
	RW_PIN = 0;                     // Reset the control lines

	return (data&0x7f);             // Return the address, Mask off the busy bit
}

char ReadDataXLCD(void)
{
        char data;

	// 4-bit interface
	RW_PIN = 1;
	RS_PIN = 1;
	DelayFor18TCY();
	E_PIN = 1;                      // Clock the data out of the LCD
	DelayFor18TCY();
#ifdef UPPER                            // Upper nibble interface
	data = DATA_PORT&0xf0;          // Read the upper nibble of data
#else                                   // Lower nibble interface
	data = (DATA_PORT<<4)&0xf0;     // read the upper nibble of data
#endif
	E_PIN = 0;                      // Reset the clock line
	DelayFor18TCY();
	E_PIN = 1;                      // Clock the next nibble out of the LCD
	DelayFor18TCY();
#ifdef UPPER                            // Upper nibble interface
	data |= (DATA_PORT>>4)&0x0f;    // Read the lower nibble of data
#else                                   // Lower nibble interface
	data |= DATA_PORT&0x0f;         // Read the lower nibble of data
#endif
	E_PIN = 0;                                      
	RS_PIN = 0;                     // Reset the control bits
	RW_PIN = 0;

	return(data);                   // Return the data byte
}

void SetCGRamAddr(unsigned char CGaddr)
{
	// 4-bit interface
#ifdef UPPER                                    // Upper nibble interface
	TRIS_DATA_PORT &= 0x0f;                 // Make nibble input
	DATA_PORT &= 0x0f;                      // and write upper nibble
	DATA_PORT |= ((CGaddr | 0b01000000) & 0xf0);
#else                                           // Lower nibble interface
	TRIS_DATA_PORT &= 0xf0;                 // Make nibble input
	DATA_PORT &= 0xf0;                      // and write upper nibble
	DATA_PORT |= (((CGaddr |0b01000000)>>4) & 0x0f);
#endif
	RW_PIN = 0;                             // Set control signals
	RS_PIN = 0;
	DelayFor18TCY();
	E_PIN = 1;                              // Clock cmd and address in
	DelayFor18TCY();
	E_PIN = 0;
#ifdef UPPER                                    // Upper nibble interface
	DATA_PORT &= 0x0f;                      // Write lower nibble
	DATA_PORT |= ((CGaddr<<4)&0xf0);
#else                                           // Lower nibble interface
	DATA_PORT &= 0xf0;                      // Write lower nibble
	DATA_PORT |= (CGaddr&0x0f);
#endif
	DelayFor18TCY();
	E_PIN = 1;                              // Clock cmd and address in
	DelayFor18TCY();
	E_PIN = 0;
#ifdef UPPER                                    // Upper nibble interface
	TRIS_DATA_PORT |= 0xf0;                 // Make inputs
#else                                           // Lower nibble interface
	TRIS_DATA_PORT |= 0x0f;                 // Make inputs
#endif

	return;
}

void SetDDRamAddr(unsigned char DDaddr)
{
                                          // 4-bit interface
#ifdef UPPER                                    // Upper nibble  interface
	TRIS_DATA_PORT &= 0x0f;                 // Make port output
	DATA_PORT &= 0x0f;                      // and write upper nibble
	DATA_PORT |= ((DDaddr | 0b10000000) & 0xf0);
#else                                           // Lower nibble interface
	TRIS_DATA_PORT &= 0xf0;                 // Make port output
	DATA_PORT &= 0xf0;                      // and write upper nibble
	DATA_PORT |= (((DDaddr | 0b10000000)>>4) & 0x0f);
#endif
	RW_PIN = 0;                             // Set control bits
	RS_PIN = 0;
	DelayFor18TCY();
	E_PIN = 1;                              // Clock the cmd and address in
	DelayFor18TCY();
	E_PIN = 0;
#ifdef UPPER                                    // Upper nibble interface
	DATA_PORT &= 0x0f;                      // Write lower nibble
	DATA_PORT |= ((DDaddr<<4)&0xf0);
#else                                           // Lower nibble interface
	DATA_PORT &= 0xf0;                      // Write lower nibble
	DATA_PORT |= (DDaddr&0x0f);
#endif
	DelayFor18TCY();
	E_PIN = 1;                              // Clock the cmd and address in
	DelayFor18TCY();
	E_PIN = 0;
#ifdef UPPER                                    // Upper nibble interface
	TRIS_DATA_PORT |= 0xf0;                 // Make port input
#else                                           // Lower nibble interface
	TRIS_DATA_PORT |= 0x0f;                 // Make port input
#endif

	return;
}

void clearTopLCD(void)
{
	unsigned char data[] = "                ";
	SetDDRamAddr(0x00);
	putsXLCD(data);		
}

void clearBottomLCD(void)
{
	unsigned char data[] = "                ";
	SetDDRamAddr(0x28);
	putsXLCD(data);		
}

void IntToLCD(unsigned int data, unsigned char pos)
{
	unsigned char buf[5];
	itoa(data, buf);

	pos += 4;
	if (data > 9)
		pos--;
	if (data > 99)
		pos--;
	if (data > 999)
		pos--;	
	SetDDRamAddr(pos);
	putsXLCD(buf);		
}

void FloatToLCD(float data, unsigned char countDs, unsigned char pos)
{
	unsigned int tempSt = 0;
	unsigned int tempDec = 0;
	unsigned char buf[5];
	
	tempSt = (unsigned int)data;
	tempDec = (unsigned int)((data - tempSt)*10);
	
	pos += 4;
	if (tempSt > 9)
		pos--;
	if (tempSt > 99)
		pos--;
	if (tempSt > 999)
		pos--;
	pos -= 2;		
	SetDDRamAddr(pos);
	itoa(tempSt, buf);
	putsXLCD(buf);
	pos++;
	if (tempSt > 9)
		pos++;
	if (tempSt > 99)
		pos ++;
	if (tempSt > 999)
		pos ++;	
	SetDDRamAddr(pos);
	WriteDataXLCD(0x2E);
	pos++;
	SetDDRamAddr(pos);
	itoa(tempDec, buf);
	putsXLCD(buf);			
}