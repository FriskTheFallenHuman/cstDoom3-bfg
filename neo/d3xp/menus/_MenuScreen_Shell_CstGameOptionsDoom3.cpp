//#modified-fva; BEGIN
#include "precompiled.h"
#pragma hdrstop
#include "../Game_local.h"
#include "../_CstStrings.h"

enum {
	CMD_GENERAL_OPTIONS_1,
	CMD_GENERAL_OPTIONS_2,
	CMD_GENERAL_OPTIONS_3,
	CMD_RUN_CROUCH_ZOOM,
	CMD_MAP_OPTIONS,
	CMD_WEAPON_OPTIONS,
	CMD_WEAPON_SMOKE_SELF,
	CMD_WEAPON_SMOKE_OTHERS_MP,
	CMD_PROJECTILE_SMOKE_SELF,
	CMD_PROJECTILE_SMOKE_OTHERS_MP,
	CMD_DAMAGE_FEEDBACK,
	CMD_SERVER_OPTIONS
};

/*
========================
idMenuScreen_Shell_CstGameOptionsDoom3::Initialize
========================
*/
void idMenuScreen_Shell_CstGameOptionsDoom3::Initialize(idMenuHandler * data) {
	numVisibleOptions = 14;
	menuSpriteName = "cstMenuGameOptionsDoom3";
	btnBackLabel = "#str_swf_settings";
	headingLabel = CST_STR_00300127;
	shellArea = CST_SHELL_AREA_GAME_OPTIONS_DOOM3;
	goBackShellArea = SHELL_AREA_SETTINGS;
	showJoySelect = true;

	options = new (TAG_SWF) idMenuWidget_DynamicList();
	idList< idStr > option;

	option.Append(CST_STR_00300128);	// General Options 1
	menuOptions.Append(option);
	option.Clear();
	option.Append(CST_STR_00300131);	// General Options 2
	menuOptions.Append(option);
	option.Clear();
	option.Append(CST_STR_00300405);	// General Options 3
	menuOptions.Append(option);
	option.Clear();
	option.Append(CST_STR_00300176);	// Run, Crouch, Zoom
	menuOptions.Append(option);
	option.Clear();
	option.Append(CST_STR_00300194);	// Map Options
	menuOptions.Append(option);
	option.Clear();
	option.Append(CST_STR_00300204);	// Weapon Options
	menuOptions.Append(option);
	option.Clear();
	option.Append(CST_STR_00300238);	// Weapon Smoke - Self
	menuOptions.Append(option);
	option.Clear();
	option.Append(CST_STR_00300250);	// Weapon Smoke - Others MP
	menuOptions.Append(option);
	option.Clear();
	option.Append(CST_STR_00300253);	// Projectile Smoke - Self
	menuOptions.Append(option);
	option.Clear();
	option.Append(CST_STR_00300265);	// Projectile Smoke - Others MP
	menuOptions.Append(option);
	option.Clear();
	option.Append(CST_STR_00300268);	// Damage Feedback
	menuOptions.Append(option);
	option.Clear();
	option.Append(CST_STR_00300285);	// Server Options
	menuOptions.Append(option);

	for (int i = 0; i < numVisibleOptions; ++i) {
		idMenuWidget_Button * const button = new (TAG_SWF) idMenuWidget_Button();
		button->Initialize(data);
		int commandID = i;
		button->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, commandID);
		options->AddChild(button);
	}

	helpText.Clear();
	helpText.Append(NULL);				// 0
	helpText.Append(NULL);				// 1
	helpText.Append(NULL);				// 2
	helpText.Append(NULL);				// 3
	helpText.Append(NULL);				// 4
	helpText.Append(NULL);				// 5
	helpText.Append(CST_STR_00300239);	// 6
	helpText.Append(CST_STR_00300251);	// 7
	helpText.Append(CST_STR_00300254);	// 8
	helpText.Append(CST_STR_00300266);	// 9
	helpText.Append(CST_STR_00300269);	// 10
	helpText.Append(CST_STR_00300286);	// 11

	idMenuScreen_Shell_CstMenuBase::Initialize(data);
}

/*
========================
idMenuScreen_Shell_CstGameOptionsDoom3::HandleAction
========================
*/
bool idMenuScreen_Shell_CstGameOptionsDoom3::HandleAction(idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled) {
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
				menuData->SetNextScreen(CST_SHELL_AREA_GO_D3_GENERAL_OPTIONS_1, MENU_TRANSITION_SIMPLE);
				break;
			}
			case CMD_GENERAL_OPTIONS_2: {
				menuData->SetNextScreen(CST_SHELL_AREA_GO_D3_GENERAL_OPTIONS_2, MENU_TRANSITION_SIMPLE);
				break;
			}
			case CMD_GENERAL_OPTIONS_3: {
				menuData->SetNextScreen(CST_SHELL_AREA_GO_D3_GENERAL_OPTIONS_3, MENU_TRANSITION_SIMPLE);
				break;
			}
			case CMD_RUN_CROUCH_ZOOM: {
				menuData->SetNextScreen(CST_SHELL_AREA_GO_D3_RUN_CROUCH_ZOOM, MENU_TRANSITION_SIMPLE);
				break;
			}
			case CMD_MAP_OPTIONS: {
				menuData->SetNextScreen(CST_SHELL_AREA_GO_D3_MAP_OPTIONS, MENU_TRANSITION_SIMPLE);
				break;
			}
			case CMD_WEAPON_OPTIONS: {
				menuData->SetNextScreen(CST_SHELL_AREA_GO_D3_WEAPON_OPTIONS, MENU_TRANSITION_SIMPLE);
				break;
			}
			case CMD_WEAPON_SMOKE_SELF: {
				menuData->SetNextScreen(CST_SHELL_AREA_GO_D3_WEAPON_SMOKE_SELF, MENU_TRANSITION_SIMPLE);
				break;
			}
			case CMD_WEAPON_SMOKE_OTHERS_MP: {
				menuData->SetNextScreen(CST_SHELL_AREA_GO_D3_WEAPON_SMOKE_OTHERS_MP, MENU_TRANSITION_SIMPLE);
				break;
			}
			case CMD_PROJECTILE_SMOKE_SELF: {
				menuData->SetNextScreen(CST_SHELL_AREA_GO_D3_PROJECTILE_SMOKE_SELF, MENU_TRANSITION_SIMPLE);
				break;
			}
			case CMD_PROJECTILE_SMOKE_OTHERS_MP: {
				menuData->SetNextScreen(CST_SHELL_AREA_GO_D3_PROJECTILE_SMOKE_OTHERS_MP, MENU_TRANSITION_SIMPLE);
				break;
			}
			case CMD_DAMAGE_FEEDBACK: {
				menuData->SetNextScreen(CST_SHELL_AREA_GO_D3_DAMAGE_FEEDBACK, MENU_TRANSITION_SIMPLE);
				break;
			}
			case CMD_SERVER_OPTIONS: {
				menuData->SetNextScreen(CST_SHELL_AREA_GO_D3_SERVER_OPTIONS, MENU_TRANSITION_SIMPLE);
				break;
			}
		}
	}

	return idMenuScreen_Shell_CstMenuBase::HandleAction(action, event, widget, forceHandled);
}
//#modified-fva; END
