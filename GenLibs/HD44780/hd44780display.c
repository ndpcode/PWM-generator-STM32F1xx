
#include "hd44780display.h"

//****************************************************************************************************************//
//****************************************************************************************************************//
//���� ���������� �������, ���������� ������ � ������������ �������.
//������ ���� ������� ����������� �������� �� ������ � ���������� �����,
//�������� ������ ����������� ��������������.
//****************************************************************************************************************//
//****************************************************************************************************************//

static unsigned char HD44780DisplayCheckConfig(HD44780_DISPLAY_STRUCT *displayStruct)
{
	if ( !displayStruct ) return HD44780_DISPLAY_ERROR;
	if ( !displayStruct->portStruct ) return HD44780_DISPLAY_CONFIG_ERROR;
	if ( !displayStruct->delayUSFunc ) return HD44780_DISPLAY_CONFIG_ERROR;	
	return HD44780_DISPLAY_OK;
}

static unsigned char HD44780DisplayResetDataPins(HD44780_DISPLAY_STRUCT *displayStruct)
{
	unsigned char i;
	for ( i = 0; i < 8; i++ )
	{
		HD44780DriverWritePin(displayStruct->portStruct, (HD44780_DISPLAY_PIN_NAME)i, 0);		
	};
	return HD44780_DISPLAY_OK;
}

static unsigned char HD44780DisplayWriteHalfByte(HD44780_DISPLAY_STRUCT *displayStruct, unsigned char halfByte)
{
	unsigned char i;
	for ( i = 4; i < 8; i++ )
	{
		HD44780DriverWritePin(displayStruct->portStruct, (HD44780_DISPLAY_PIN_NAME)i, ( halfByte >> ( i - 4 ) ) & 0x01);
	};
	return HD44780_DISPLAY_OK;
}

static unsigned char HD44780DisplayPulse(HD44780_DISPLAY_STRUCT *displayStruct)
{
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 0); // E = 0
	displayStruct->delayUSFunc(2); //����� 2 ���
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 1); // E = 1
	displayStruct->delayUSFunc(2); //����� 2 ���
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 0); // E = 0
	displayStruct->delayUSFunc(100); //����� 100 ���
	return HD44780_DISPLAY_OK;
}

//****************************************************************************************************************//
//****************************************************************************************************************//
//���� �������� ������� ��� ������ � ������������ �������.
//��� ������� �������� ��� ������� ��� ����������.
//****************************************************************************************************************//
//****************************************************************************************************************//

unsigned char HD44780DisplayInit(HD44780_DISPLAY_STRUCT *displayStruct)
{
	//�������� ������� ������
	if ( HD44780DisplayCheckConfig(displayStruct) != HD44780_DISPLAY_OK ) return HD44780_DISPLAY_CONFIG_ERROR;
	
	//�������������
  HD44780DisplayResetDataPins(displayStruct); //ALL DBx  = 0
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 0); // E = 0
	displayStruct->delayUSFunc(50000); //����� 50000 ���
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_READ_WRITE, 0); //RW = 0
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_REG_SELECT, 0); //A0 = 0
	
	HD44780DisplayWriteHalfByte(displayStruct, 0x03);
	displayStruct->delayUSFunc(2); //����� 2 ���
	
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 1); // E = 1
	displayStruct->delayUSFunc(2); //����� 2 ���
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 0); // E = 0
	displayStruct->delayUSFunc(200); //����� 200 ���
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 1); // E = 1
	displayStruct->delayUSFunc(2); //����� 2 ���
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 0); // E = 0
	displayStruct->delayUSFunc(200); //����� 200 ���
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 1); // E = 1
	displayStruct->delayUSFunc(2); //����� 2 ���
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 0); // E = 0
	displayStruct->delayUSFunc(200); //����� 200 ���
	
  HD44780DisplayResetDataPins(displayStruct); //ALL DBx  = 0
	HD44780DisplayWriteHalfByte(displayStruct, 0x02);
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 1); // E = 1
	displayStruct->delayUSFunc(2); //����� 2 ���
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 0); // E = 0
	displayStruct->delayUSFunc(200); //����� 200 ���
	
	HD44780DisplaySendByte(displayStruct, 0x2A, HD44780_DISPLAY_COMMAND);
	HD44780DisplaySendByte(displayStruct, 0x0C, HD44780_DISPLAY_COMMAND);
	HD44780DisplayClear(displayStruct);
	HD44780DisplaySendByte(displayStruct, 0x06, HD44780_DISPLAY_COMMAND);
	HD44780DisplaySendByte(displayStruct, 0x80, HD44780_DISPLAY_COMMAND);
	
	return HD44780_DISPLAY_OK;
}

unsigned char HD44780DisplayClear(HD44780_DISPLAY_STRUCT *displayStruct)
{
	if ( HD44780DisplaySendByte(displayStruct, 0x01, HD44780_DISPLAY_COMMAND) != HD44780_DISPLAY_OK ) return HD44780_DISPLAY_CONFIG_ERROR;
	displayStruct->delayUSFunc(5000); //����� 5000 ���
	return HD44780_DISPLAY_OK;
}

unsigned char HD44780DisplaySendByte(HD44780_DISPLAY_STRUCT *displayStruct,
                                                 unsigned char displayData,
																				  HD44780_DISPLAY_BYTE_TYPE isData)
{
	//�������� ������� ������
	if ( HD44780DisplayCheckConfig(displayStruct) != HD44780_DISPLAY_OK ) return HD44780_DISPLAY_CONFIG_ERROR;
	if ( ( isData != 0 ) && ( isData != 1 ) ) return HD44780_DISPLAY_ERROR;
	//���������� ������
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 0); // E = 0
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_READ_WRITE, 0); //RW = 0
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_REG_SELECT, isData); //A0 = 0(�������) ��� A0 = 1(������)
	HD44780DisplayResetDataPins(displayStruct); //ALL DBx  = 0
	HD44780DisplayWriteHalfByte(displayStruct, (displayData >> 4 ) & 0x0F); //����� ������� �������� �����
  displayStruct->delayUSFunc(2); //����� 2 ���
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 1); // E = 1
	displayStruct->delayUSFunc(2); //����� 2 ���
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 0); // E = 0
	displayStruct->delayUSFunc(2); //����� 2 ���
	HD44780DisplayResetDataPins(displayStruct); //ALL DBx  = 0
	HD44780DisplayWriteHalfByte(displayStruct, displayData & 0x0F); //����� ������� �������� �����
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 1); // E = 1
	displayStruct->delayUSFunc(2); //����� 2 ���
	HD44780DriverWritePin(displayStruct->portStruct, HD44780_DRIVER_ENABLE, 0); // E = 0
	displayStruct->delayUSFunc(100); //����� 100 ���
	return HD44780_DISPLAY_OK;
}

