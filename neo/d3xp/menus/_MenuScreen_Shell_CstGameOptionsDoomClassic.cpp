//#modified-fva; BEGIN
#include "precompiled.h"
#pragma hdrstop
#include "../Game_local.h"
#include "../_CstStrings.h"

enum {
	CMD_GENERAL_OPTIONS_1,
	CMD_AUTOMAP_OPTIONS,
	CMD_RUN_OPTIONS,
	CMD_SCREEN_OPTIONS
};

/*
========================
idMenuScreen_Shell_CstGameOptionsDoomClassic::Initialize
========================
*/
void idMenuScreen_Shell_CstGameOptionsDoomClassic::Initialize(idMenuHandler * data) {
	numVisibleOptions = 8;
	menuSpriteName = "cstMenuGameOptionsDoomClassic";
	btnBackLabel = "#str_swf_settings";
	headingLabel = CST_STR_00300304;
	shellArea = CST_SHELL_AREA_GAME_OPTIONS_DOOM_CLASSIC;
	goBackShellArea = SHELL_AREA_SETTINGS;
	showJoySelect = true;

	options = new (TAG_SWF) idMenuWidget_DynamicList();
	idList< idStr > option;

	option.Append(CST_STR_00300414);	// General Options
	menuOptions.Append(option);
	option.Clear();
	option.Append(CST_STR_00300305);	// Automap Options
	menuOptions.Append(option);
	option.Clear();
	option.Append(CST_STR_00300315);	// Run Options
	menuOptions.Append(option);
	option.Clear();
	option.Append(CST_STR_00300320);	// Screen Options
	menuOptions.Append(option);

	for (int i = 0; i < numVisibleOptions; ++i) {
		idMenuWidget_Button * const button = new (TAG_SWF) idMenuWidget_Button();
		button->Initialize(data);
		int commandID = i;
		button->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, commandID);
		options->AddChild(button);
	}

	idMenuScreen_Shell_CstMenuBase::Initialize(data);
}

/*
========================
idMenuScreen_Shell_CstGameOptionsDoomClassic::HandleAction
========================
*/
bool idMenuScreen_Shell_CstGameOptionsDoomClassic::HandleAction(idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled) {
	if (menuData == NULL) {
		return true;
	}
	if (menuData->ActiveScreen() != shellArea) {
		return false;
	}

	widgetAction_t actionType = action.GetType();
	const idSWFParmList & parms = action.GetParms();

	if (actionType == WIDGET_ACTION_COMMAND && parms.Num() > 0) {
		switch (parms[0].ToInteger()) {
			case CMD_GENERAL_OPTIONS_1: {
				menuData->SetNextScreen(CST_SHELL_AREA_GO_DC_GENERAL_OPTIONS_1, MENU_TRANSITION_SIMPLE);
				break;
			}
			case CMD_AUTOMAP_OPTIONS: {
				menuData->SetNextScreen(CST_SHELL_AREA_GO_DC_AUTOMAP_OPTIONS, MENU_TRANSITION_SIMPLE);
				break;
			}
			case CMD_RUN_OPTIONS: {
				menuData->SetNextScreen(CST_SHELL_AREA_GO_DC_RUN_OPTIONS, MENU_TRANSITION_SIMPLE);
				break;
			}
			case CMD_SCREEN_OPTIONS: {
				menuData->SetNextScreen(CST_SHELL_AREA_GO_DC_SCREEN_OPTIONS, MENU_TRANSITION_SIMPLE);
				break;
			}
		}
	}

	return idMenuScreen_Shell_CstMenuBase::HandleAction(action, event, widget, forceHandled);
}
//#modified-fva; END
