
#ifndef BSC_TREE_MENU
#define BSC_TREE_MENU

#include <stdint.h>
#include "bsc_controls.h"

//"virual" struct
typedef struct
{
	uint16_t MenuItemId;
	uint8_t MenuItemIndexInSubmenu;
	uint8_t (*MenuDrawFunc)(const uint8_t /*MenuDrawFrameNumber*/);
	uint8_t (*MenuEventsFunc)(const uint16_t /*MenuEventsFrameNumber*/, SYS_EVENTS_DATA /*SystemEvents*/);
	void *MenuParent;
	void *MenuChild;
	void *MenuPrevItem;
	void *MenuNextItem;
} TREE_MENU_ITEM;

typedef struct
{
	TREE_MENU_ITEM *MenuHeadItem;                   //главный(первый) элемент меню
	TREE_MENU_ITEM *MenuCurrentItem;                //элемент меню, активный сейчас
	SYS_EVENTS_DATA (*ExternalGetEventsFunc)(void); //функция, возвращающая коды событий
	//функция, циклически вызываемая в течение времени MenuTransitionTimeInMS при переходе между пунктами меню
	void (*MenuTransitionFunc)(const uint16_t /*DrawFrameNum*/);
	uint16_t MenuTargetDrawFPS;                     //целевая скорость отрисовки меню (кадров в секунду)
	uint16_t MenuTargetEventsFPS;                   //целевая скорость опроса событий (опросов в секунду)
	uint8_t MenuCurrentDrawFrame;	                  //номер текущего кадра отрисовки меню, 1..MenuTargetFPS
	uint16_t MenuCurrentEventsFrame;                //номер текущей итерации опроса событий
	uint16_t MenuMaxItemId;                         //Id самого последнего пункта меню
	uint32_t MenuDrawClock;                         //время последней отрисовки, мс
	uint32_t MenuEventsClock;                       //время последнего обновления событий, мс
	uint8_t MenuEditAllow;                          //флаг разрешения на добавление/удаление/изменение элементов меню
	uint8_t MenuTransitionState;                    //флаг, устаналивается в процессе смены пунктов меню при MenuTransitionTimeInMS != 0
	uint16_t MenuTransitionTimeInMS;                //время, отводимое на переход между пунктами меню
	//при нулевом времени MenuTransitionTimeInMS, MenuTransitionFunc не вызывается и MenuTransitionState не устаналивается
} TREE_MENU;

TREE_MENU *MenuCreate(uint8_t targetDrawFPS, uint16_t targetEventFPS, SYS_EVENTS_DATA (*_ExternalGetEventsFunc)(void),
	                    void (*_MenuTransitionFunc)(const uint16_t), uint16_t _menuTransitionTimeInMs,
											uint8_t (*HeadMenuDrawFunc)(const uint8_t), uint8_t (*HeadMenuEventsFunc)(const uint16_t, SYS_EVENTS_DATA));
uint16_t MenuAddNextItem(TREE_MENU *menuHead, uint16_t parentItemId, uint8_t (*_MenuDrawFunc)(const uint8_t),
                         uint8_t (*_MenuEventsFunc)(const uint16_t, SYS_EVENTS_DATA));
uint16_t MenuAddSubItem(TREE_MENU *menuHead, uint16_t parentItemId, uint8_t (*_MenuDrawFunc)(const uint8_t),
	                      uint8_t (*_MenuEventsFunc)(const uint16_t, SYS_EVENTS_DATA));
uint8_t MenuGoToHeadItem(TREE_MENU *menuHead);
uint8_t MenuGoToPrevItem(TREE_MENU *menuHead);
uint8_t MenuGoToNextItem(TREE_MENU *menuHead);
uint8_t MenuGoToParentItem(TREE_MENU *menuHead);
uint8_t MenuGoToChildItem(TREE_MENU *menuHead);
uint8_t MenuGoToItemId(TREE_MENU *menuHead, uint16_t MeniItemId);
uint8_t MenuUpdate(TREE_MENU *menuHead, uint16_t ClockSecond, uint16_t ClockMillisecond);

#endif
