
#include "bsc_tree_menu.h"
#include <stdlib.h>

//**********************************************************************************************************************//
//Внутренние определения вспомогательных структур и функций
//**********************************************************************************************************************//

uint8_t menuCheckMainConfig(TREE_MENU *menuHead)
{
	if ( !menuHead ) return 0;
	if ( !menuHead->MenuHeadItem ) return 0;
	if ( !menuHead->MenuCurrentItem ) return 0;
	if ( !menuHead->MenuMaxItemId ) return 0;
	if ( !menuHead->MenuCurrentItem->MenuItemId ) return 0;
	if ( !menuHead->MenuCurrentItem->MenuItemIndexInSubmenu ) return 0;	
	return 1;
}

TREE_MENU_ITEM *getMenuItemById(TREE_MENU_ITEM *menuItem, uint16_t _id)
{
	TREE_MENU_ITEM *result = 0;
	if ( menuItem->MenuItemId == _id ) return menuItem;
	if ( menuItem->MenuNextItem ) result = getMenuItemById(menuItem->MenuNextItem, _id);
	if ( ( !result ) && ( menuItem->MenuChild ) ) result = getMenuItemById(menuItem->MenuChild, _id);
	return result;
}

TREE_MENU_ITEM *getLastMenuItem(TREE_MENU *menuHead)
{
	return getMenuItemById(menuHead->MenuHeadItem, menuHead->MenuMaxItemId);
}

uint8_t menuSwitchCurrentItemTo(TREE_MENU *menuHead, TREE_MENU_ITEM *toThisItem)
{
	//проверка входных данных
	//существует ли переключаемый элемент
	if ( !toThisItem ) return 0;
	
	//блокировка
	menuHead->MenuEditAllow = 0;
	//переключаем текущий элемент на другой
	menuHead->MenuCurrentItem = toThisItem;
	//сброс счетчиков кадров
	menuHead->MenuCurrentDrawFrame = 0;
	menuHead->MenuCurrentEventsFrame = 0;
	menuHead->MenuDrawClock = 9999999;
	menuHead->MenuEventsClock = 9999999;
	//если активирован переход между пукнтами
	if ( menuHead->MenuTransitionTimeInMS )
	{
		//устанавливаем флаг перехода
		menuHead->MenuTransitionState = 1;
		//отрисовываем новый пункт меню с 0 кадров
		if ( menuHead->MenuCurrentItem->MenuDrawFunc )
			menuHead->MenuCurrentItem->MenuDrawFunc(0);
	};
	//снимаем блокировку
	menuHead->MenuEditAllow = 1;
	return 1;
}

//**********************************************************************************************************************//
//Пользовательские функции для использования вне модуля
//**********************************************************************************************************************//

TREE_MENU *MenuCreate(uint8_t targetDrawFPS, uint16_t targetEventFPS, SYS_EVENTS_DATA (*_ExternalGetEventsFunc)(void),
	                    void (*_MenuTransitionFunc)(const uint16_t), uint16_t _menuTransitionTimeInMs,
											uint8_t (*HeadMenuDrawFunc)(const uint8_t), uint8_t (*HeadMenuEventsFunc)(const uint16_t, SYS_EVENTS_DATA))
{
	TREE_MENU *newMenu = 0;
	TREE_MENU_ITEM *headMenuItem = 0;
	//проверка входных данных
	//назначение функций отрисовки меню HeadMenuDrawFunc и событий HeadMenuEventsFunc по необходимости
	//если не указаны время обновления отрисовки и обновления событий, выходим
	if ( ( !targetDrawFPS ) && ( !targetEventFPS ) ) return 0;
	
	//выделяем память под новую структуру
	newMenu = (TREE_MENU*)malloc(sizeof(TREE_MENU));
	if ( !newMenu ) return 0;
	newMenu->MenuEditAllow = 0;
	//выделяем память под головной (HEAD) элемент меню
	headMenuItem = (TREE_MENU_ITEM*)malloc(sizeof(TREE_MENU_ITEM));
	if ( !headMenuItem )
	{
		//память под HEAD не выделена, очищаем данные, выходим
		free(newMenu);
		newMenu = 0;
		return 0;
	};
	//заполняем HEAD меню
	headMenuItem->MenuItemId = 1;
	headMenuItem->MenuItemIndexInSubmenu = 1;
	headMenuItem->MenuDrawFunc = HeadMenuDrawFunc;
	headMenuItem->MenuEventsFunc = HeadMenuEventsFunc;
	headMenuItem->MenuParent = 0;
	headMenuItem->MenuChild = 0;
	headMenuItem->MenuPrevItem = 0;
	headMenuItem->MenuNextItem = 0;
	//заполняем основную структуру
	newMenu->MenuTargetDrawFPS = targetDrawFPS;
	newMenu->MenuTargetEventsFPS = targetEventFPS;
	newMenu->MenuCurrentDrawFrame = 0;
	newMenu->MenuCurrentEventsFrame = 0;
	newMenu->MenuMaxItemId = headMenuItem->MenuItemId;
	newMenu->MenuDrawClock = 9999999;
	newMenu->MenuEventsClock = 9999999;
	newMenu->MenuCurrentItem = headMenuItem;
	newMenu->MenuTransitionFunc = _MenuTransitionFunc;
	newMenu->MenuTransitionTimeInMS = _menuTransitionTimeInMs;
	newMenu->MenuTransitionState = 0;

	//в последнюю очередь назначаем головной элемент меню и обработчик событий
	newMenu->MenuHeadItem = headMenuItem;
	newMenu->ExternalGetEventsFunc = _ExternalGetEventsFunc;
	//разрешаем изменять меню
	newMenu->MenuEditAllow = 1;
	
	return newMenu;
}

uint16_t MenuAddNextItem(TREE_MENU *menuHead, uint16_t parentItemId, uint8_t (*_MenuDrawFunc)(const uint8_t),
                         uint8_t (*_MenuEventsFunc)(const uint16_t, SYS_EVENTS_DATA))
{
	TREE_MENU_ITEM *newMenuItem = 0;
	TREE_MENU_ITEM *lastMenuItem = 0;
	//проверка входных данных
	if ( !menuCheckMainConfig(menuHead) ) return 0;
	//проверка разрешения на изменение
	if ( !menuHead->MenuEditAllow ) return 0;

	//ищем родительский элемент, если указан Id
	if ( parentItemId )
	{
		lastMenuItem = getMenuItemById(menuHead->MenuHeadItem, parentItemId); 
	};
	//ищем последний добавленный элемент, если не найден элемент по parentItemId
	if ( !lastMenuItem )
	{
		lastMenuItem = getLastMenuItem(menuHead);
	};
	if ( !lastMenuItem ) return 0;
	//создаем новый элемент и заполняем
	newMenuItem = (TREE_MENU_ITEM*)malloc(sizeof(TREE_MENU_ITEM));
	if ( !newMenuItem ) return 0;
	//блокировка
	menuHead->MenuEditAllow = 0;
	//заполняем
	menuHead->MenuMaxItemId++;
	newMenuItem->MenuItemId = menuHead->MenuMaxItemId;
	newMenuItem->MenuItemIndexInSubmenu = lastMenuItem->MenuItemIndexInSubmenu + 1;
	newMenuItem->MenuDrawFunc = _MenuDrawFunc;
	newMenuItem->MenuEventsFunc = _MenuEventsFunc;
	newMenuItem->MenuParent = lastMenuItem->MenuParent;
	newMenuItem->MenuChild = 0;
	newMenuItem->MenuPrevItem = lastMenuItem;
	newMenuItem->MenuNextItem = 0;
  //добавляем данные в предыдущий элемент меню
  lastMenuItem->MenuNextItem = newMenuItem;
	//снимаем блокировку
	menuHead->MenuEditAllow = 1;
	
	return newMenuItem->MenuItemId;
}

uint16_t MenuAddSubItem(TREE_MENU *menuHead, uint16_t parentItemId, uint8_t (*_MenuDrawFunc)(const uint8_t),
	                      uint8_t (*_MenuEventsFunc)(const uint16_t, SYS_EVENTS_DATA))
{
	TREE_MENU_ITEM *newMenuItem;
	TREE_MENU_ITEM *parentMenuItem;
	//проверка входных данных
	if ( !menuCheckMainConfig(menuHead) ) return 0;
	//проверка разрешения на изменение
	if ( !menuHead->MenuEditAllow ) return 0;

	//ищем родительский элемент, если указан Id
	if ( parentItemId )
	{
		parentMenuItem = getMenuItemById(menuHead->MenuHeadItem, parentItemId); 
	};
	//ищем последний добавленный элемент, если не найден элемент по parentItemId
	if ( !parentMenuItem )
	{
		parentMenuItem = getLastMenuItem(menuHead);
	};
	if ( !parentMenuItem ) return 0;
	//создаем новый элемент и заполняем
	newMenuItem = (TREE_MENU_ITEM*)malloc(sizeof(TREE_MENU_ITEM));
	if ( !newMenuItem ) return 0;
	//блокировка
	menuHead->MenuEditAllow = 0;
	//заполняем
	menuHead->MenuMaxItemId++;
	newMenuItem->MenuItemId = menuHead->MenuMaxItemId;
	newMenuItem->MenuItemIndexInSubmenu = 1;
	newMenuItem->MenuDrawFunc = _MenuDrawFunc;
	newMenuItem->MenuEventsFunc = _MenuEventsFunc;
	newMenuItem->MenuParent = parentMenuItem;
	newMenuItem->MenuChild = 0;
	newMenuItem->MenuPrevItem = 0;
	newMenuItem->MenuNextItem = 0;
  //добавляем данные в родительский элемент меню
  parentMenuItem->MenuChild = newMenuItem;
	//снимаем блокировку
	menuHead->MenuEditAllow = 1;
	
	return newMenuItem->MenuItemId;
}

uint8_t MenuGoToHeadItem(TREE_MENU *menuHead)
{
	//проверка входных данных
	if ( !menuCheckMainConfig(menuHead) ) return 0;
	if ( !menuHead->MenuEditAllow ) return 0;
	
	//переключаем текущий элемент на главный
	if ( !menuSwitchCurrentItemTo(menuHead, menuHead->MenuHeadItem) ) return 0;
	return 1;
}

uint8_t MenuGoToPrevItem(TREE_MENU *menuHead)
{
	//проверка входных данных
	if ( !menuCheckMainConfig(menuHead) ) return 0;
	if ( !menuHead->MenuEditAllow ) return 0;
	
	//переключаем текущий элемент на предыдущий
	if ( !menuSwitchCurrentItemTo(menuHead, menuHead->MenuCurrentItem->MenuPrevItem) ) return 0;
	return 1;
}

uint8_t MenuGoToNextItem(TREE_MENU *menuHead)
{
	//проверка входных данных
	if ( !menuCheckMainConfig(menuHead) ) return 0;
	if ( !menuHead->MenuEditAllow ) return 0;
	
	//переключаем текущий элемент на следующий
	if ( !menuSwitchCurrentItemTo(menuHead, menuHead->MenuCurrentItem->MenuNextItem) ) return 0;
	return 1;
}

uint8_t MenuGoToParentItem(TREE_MENU *menuHead)
{
	//проверка входных данных
	if ( !menuCheckMainConfig(menuHead) ) return 0;
	if ( !menuHead->MenuEditAllow ) return 0;
	
	//переключаем текущий элемент на родительский
	if ( !menuSwitchCurrentItemTo(menuHead, menuHead->MenuCurrentItem->MenuParent) ) return 0;
	return 1;
}

uint8_t MenuGoToChildItem(TREE_MENU *menuHead)
{
	//проверка входных данных
	if ( !menuCheckMainConfig(menuHead) ) return 0;
	if ( !menuHead->MenuEditAllow ) return 0;
	
	//переключаем текущий элемент на дочерний
	if ( !menuSwitchCurrentItemTo(menuHead, menuHead->MenuCurrentItem->MenuChild) ) return 0;
	return 1;
}

uint8_t MenuGoToItemId(TREE_MENU *menuHead, uint16_t MeniItemId)
{
	//проверка входных данных
	if ( !menuCheckMainConfig(menuHead) ) return 0;
	if ( !menuHead->MenuEditAllow ) return 0;
	
	//переключаем текущий элемент на указанный
	if ( !menuSwitchCurrentItemTo(menuHead, getMenuItemById(menuHead->MenuHeadItem, MeniItemId)) ) return 0;
	return 1;
}

uint8_t MenuUpdate(TREE_MENU *menuHead, uint16_t ClockSecond, uint16_t ClockMillisecond)
{
	uint32_t _deltaT, _deltaTEvents, _nowTime;
	SYS_EVENTS_DATA _events;
	//проверка входных данных
	if ( !menuCheckMainConfig(menuHead) ) return 0;
	if ( !menuHead->MenuEditAllow ) return 0;
	
	_nowTime = ClockSecond*1000 + ClockMillisecond;
	if ( menuHead->MenuDrawClock == 9999999) menuHead->MenuDrawClock = _nowTime;
	if ( menuHead->MenuEventsClock == 9999999) menuHead->MenuEventsClock = _nowTime;
	//вычисление delta t, защита от переполнения в 59с
	_deltaT = ( _nowTime >= menuHead->MenuDrawClock ) ?
		            (_nowTime - menuHead->MenuDrawClock) : (60000 - menuHead->MenuDrawClock + _nowTime);
	_deltaTEvents = ( _nowTime >= menuHead->MenuEventsClock ) ?
		              (_nowTime - menuHead->MenuEventsClock) : (60000 - menuHead->MenuEventsClock + _nowTime);
  //защита от переполнения при зависании - порог 1000 мс
	if ( _deltaT > 1000 )
	{
		_deltaT = 0;
		menuHead->MenuDrawClock = _nowTime;
	};
	if ( _deltaTEvents > 1000 )
	{
		_deltaTEvents = 0;
		menuHead->MenuEventsClock = _nowTime;
	};
	
	//если активирован переход
	if ( ( menuHead->MenuTransitionState ) && ( menuHead->MenuTargetDrawFPS ) )
	{
		if ( (menuHead->MenuCurrentDrawFrame*(1000/menuHead->MenuTargetDrawFPS)) > menuHead->MenuTransitionTimeInMS )
		{
			//сброс счетчиков кадров
	    menuHead->MenuCurrentDrawFrame = 0;
	    menuHead->MenuCurrentEventsFrame = 0;
			menuHead->MenuTransitionState = 0;
			return 1;
		};
		if ( _deltaT >= (1000/menuHead->MenuTargetDrawFPS) )
		{
			menuHead->MenuCurrentDrawFrame += _deltaT / ( 1000 / menuHead->MenuTargetDrawFPS );
			menuHead->MenuDrawClock = _nowTime;
			if ( menuHead->MenuTransitionFunc )
				menuHead->MenuTransitionFunc(menuHead->MenuCurrentDrawFrame);
		};		
		return 1;
	}
	
	//обновление для отрисовки
	if ( ( menuHead->MenuTargetDrawFPS ) && ( _deltaT >= (1000/menuHead->MenuTargetDrawFPS) ) )
	{
		menuHead->MenuCurrentDrawFrame += _deltaT*menuHead->MenuTargetDrawFPS/1000;
		if ( menuHead->MenuCurrentDrawFrame > menuHead->MenuTargetDrawFPS ) menuHead->MenuCurrentDrawFrame = 1;
		if ( menuHead->MenuCurrentItem->MenuDrawFunc ) menuHead->MenuCurrentItem->MenuDrawFunc(menuHead->MenuCurrentDrawFrame);
		menuHead->MenuDrawClock = _nowTime;
	};
	
	//обновление для событий
	if ( ( menuHead->MenuTargetEventsFPS ) && ( _deltaTEvents >= (1000/menuHead->MenuTargetEventsFPS) ) )
	{
		menuHead->MenuCurrentEventsFrame += _deltaTEvents*menuHead->MenuTargetEventsFPS/1000;
		if ( menuHead->MenuCurrentEventsFrame > menuHead->MenuTargetEventsFPS ) menuHead->MenuCurrentEventsFrame = 1;
		//обновляем события
		_events = menuHead->ExternalGetEventsFunc();
		if ( menuHead->MenuCurrentItem->MenuEventsFunc ) menuHead->MenuCurrentItem->MenuEventsFunc(menuHead->MenuCurrentEventsFrame, _events);
		menuHead->MenuEventsClock = _nowTime;
	};
	
	return 1;
}




