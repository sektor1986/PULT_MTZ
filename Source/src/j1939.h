#ifndef __j1939_h__
#define __j1939_h__

// J1939 PDU formats
#define J1939_PF_ADDRESS_CLAIMED		238		// With global address
#define J1939_PF_CANNOT_CLAIM_ADDRESS	238		// With null address
#define J1939_PF_REQUEST				234		// ������ PG
#define J1939_PF_ACK					232

#define J1939_PF_BOOTLOADER				1
#define J1939_PF_SETADDRESS				2
#define J1939_PF_DATA					3
#define J1939_PF_START					4
#define J1939_PF_STOP					5
#define J1939_PF_START_READ				6
#define J1939_PF_DATA_REQUEST			7
#define J1939_PF_CORRECT_ADDRESS		8

// J1939 PGN's
#define J1939_PGN2_REQ_ADDRESS_CLAIM		0x00
#define J1939_PGN1_REQ_ADDRESS_CLAIM		0xEA
#define J1939_PGN0_REQ_ADDRESS_CLAIM		0x00

#define J1939_PGN2_BOOTLOADER				0
#define J1939_PGN1_BOOTLOADER				1
#define J1939_PGN0_BOOTLOADER				0

#define J1939_PGN2_SETADDRESS				0
#define J1939_PGN1_SETADDRESS				2
#define J1939_PGN0_SETADDRESS				0

#define J1939_PGN2_DATA					0
#define J1939_PGN1_DATA					3
#define J1939_PGN0_DATA					0

#define J1939_PGN2_START					0
#define J1939_PGN1_START					4
#define J1939_PGN0_START					0

#define J1939_PGN2_STOP					0
#define J1939_PGN1_STOP					5
#define J1939_PGN0_STOP					0

#define J1939_PGN2_START_READ				0
#define J1939_PGN1_START_READ				6
#define J1939_PGN0_START_READ				0

// J1939 Defined Addresses
#define J1939_GLOBAL_ADDRESS				255		// ���������� �����
#define J1939_NULL_ADDRESS				254		// ������� �����


// J1939 Default Priorities
#define J1939_CONTROL_PRIORITY				0x03
#define J1939_INFO_PRIORITY				0x06
#define J1939_PROPRIETARY_PRIORITY			0x06
#define J1939_REQUEST_PRIORITY				0x06
#define J1939_ACK_PRIORITY				0x06
#define J1939_TP_CM_PRIORITY				0x07
#define J1939_TP_DT_PRIORITY				0x07

// Control bytes
#define J1939_ACK						0
#define J1939_NACK						1

// ��������� ��������� J1939
struct J1939_message
{
	unsigned int	priority:3;		// ���������
	unsigned int	r:1;			// ���������������
	unsigned int	dp:1;			// ��������
	unsigned char	PDUformat;		// ������ PDU
	unsigned char	PDUspecific;	// ����� ������ ����������
	unsigned char	sourceAddr;		// ����� ���������

	unsigned char	dataLen;		// ���������� ����
	unsigned char	data[8];		// ����������� ����
};

// ��������� ����� ECU
union J1939_name
{
	unsigned char name[8];
};

// ������� �����
extern unsigned char J1939_Address;
// ��� ECU
extern union  J1939_name Name;

// ������������� J1939. ���� ��������� ����� � ��� ECU
void J1939_init(unsigned char startAddress, unsigned char *name);
// ����� ����
void J1939_poll(unsigned char time);
// �������� ��������� � ������� �����������
unsigned char J1939_Send(struct J1939_message *msg);
// ����� ��������� �� ������� ���������
unsigned char J1939_Receive(struct J1939_message *msg);
// ���������� ��������� � ������� ���������
unsigned char J1939_RXsize(void);
// ���������� ��������� � ������� �����������
unsigned char J1939_TXsize(void);

void J1939_StartFindDevices(void);
void J1939_EndFindDevices(void);

// ������ ��� ������������� � �����
void J1939_SetManufCode(union J1939_name *name, unsigned int code);
// ������ ����������������� ����� � �����
void J1939_SetIdentifer(union J1939_name *name, unsigned long int code);
// �������� ��� ������������� �� �����
void J1939_GetManufCode(union J1939_name *name, unsigned int *code);
// �������� ����������������� ����� �� �����
void J1939_GetIdentifer(union J1939_name *name, unsigned long int *code);

void ECAN_Transmit(void);

#endif