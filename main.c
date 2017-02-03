
#include "settings.h"
#include "gen_system.h"
#include "gen_flash.h"
#include "gen_ports.h"
#include "gen_timers.h"
#include "gen_controls.h"
//#include "gen_menu.h"
#include "bsc_stm32_delay.h"
#include "hd44780.h"
#include "hd44780_stm32f10x.h"
#include <limits.h>

HD44780 Display;
HD44780_Config DisplayConfig;
HD44780_STM32F10x_GPIO_Driver DisplayGPIODriver;

uint8_t InitDefaults(void);
uint8_t DisplayInit(void);
void valcoderbuttonclick(void)
{
	GPIOA->ODR = GPIOA->ODR ^ GPIO_ODR_ODR0;
}

int main(void)
{
	InitDefaults();
	//��������� ������������� ������� delay ���������� bsc_stm32_delay
	delayEnable();
	
	//����� ��, ��������� �������� ����������� � �����
	GenSystemSet();
	//��������� ������
	GenInitPorts();
	
	//��������� ������ � flash MCU
	if ( FlashAccessEnable() != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	//������ �������� �� flash
	
	//��������� ��������
	if ( GenInitTimers() != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	
	//��������� �������
  if ( DisplayInit() != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	
	//��������� ������
	if ( ControlsRegNewButton('B', 10, 0, EVENT_BUTTON1_CLICK) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('B', 11, 0, EVENT_BUTTON2_CLICK) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('B', 12, 0, EVENT_BUTTON3_CLICK) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('B', 13, 0, EVENT_BUTTON4_CLICK) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('B', 14, 0, EVENT_BUTTON5_CLICK) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('B', 15, 0, EVENT_BUTTON6_CLICK) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	if ( ControlsRegNewButton('A', 3, valcoderbuttonclick, EVENT_VALCODER_BUTTON_CLICK) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	
	//��������� ���������
	if ( ControlsRegNewValcoder('A', 2,
                              'A', 4,
	                             0, EVENT_VALCODER_CCW,
                               0, EVENT_VALCODER_CW) != RESULT_OK ) ErrorHandler(RESULT_FATAL_ERROR);
	
	hd44780_clear(&Display);
  hd44780_write_string(&Display, "Hello test _+");
	//ErrorHandler(RESULT_FATAL_ERROR);
	
	while(1)
	{
		//����� ������
		ControlsUpdateEvents(UPDEVENTS_GETEVENTS | UPDEVENTS_HANDLERS_LAUNCH);
	};
	
	ControlsDataClear();
}

uint8_t InitDefaults(void)
{
	return RESULT_OK;
}

void mydelay(unsigned short iii)
{
}

uint8_t DisplayInit(void)
{

	//��������� ��������� �����
  HD44780_STM32F10x_Pinout display_pins =
  {
    {
      /* RS        */  { GPIOB, GPIO_Pin_3 },
      /* ENABLE    */  { GPIOB, GPIO_Pin_9 },
      /* RW        */  { GPIOB, GPIO_Pin_8 },
      /* Backlight */  { 0, 0 },
      /* DP0       */  { 0, 0 },
      /* DP1       */  { 0, 0 },
      /* DP2       */  { 0, 0 },
      /* DP3       */  { 0, 0 },
      /* DP4       */  { GPIOB, GPIO_Pin_4 },
      /* DP5       */  { GPIOB, GPIO_Pin_5 },
      /* DP6       */  { GPIOB, GPIO_Pin_6 },
      /* DP7       */  { GPIOB, GPIO_Pin_7 },
    }
  };
	//��������� �������� �������
	//����������� ���������
	DisplayGPIODriver.interface = HD44780_STM32F10X_PINDRIVER_INTERFACE;
	//����������� ����������� ��������� �����
	DisplayGPIODriver.pinout = display_pins;
	//���������� ������
  DisplayGPIODriver.assert_failure_handler = 0;

	//��������� display config
	//GPIO �������
  DisplayConfig.gpios = (HD44780_GPIO_Interface*)&DisplayGPIODriver;
	//��������� ������� ��������
	DisplayConfig.delay_microseconds = delayUS;
	//���������� ������
	DisplayConfig.assert_failure_handler = 0;
	//���������� ���� RW ��� ������
	DisplayConfig.options = HD44780_OPT_USE_RW;
  
	//��������� �������������
  hd44780_init(&Display, HD44780_MODE_4BIT, &DisplayConfig, 16, 2, HD44780_CHARSIZE_5x8);
	
	return RESULT_OK;
}

void HardFault_Handler(void)
{
	ErrorHandler(RESULT_IO_ERROR);
}

