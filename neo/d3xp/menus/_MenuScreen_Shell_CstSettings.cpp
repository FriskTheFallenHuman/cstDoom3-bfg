/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

//#modified-fva; BEGIN
// code below is adapted from MenuScreen_Shell_Settings.cpp (with changes)

#include "precompiled.h"
#pragma hdrstop
#include "../Game_local.h"
#include "../_CstStrings.h"

enum {
	CMD_CONTROLS,
	CMD_GAME_OPTIONS_DOOM3,
	CMD_GAME_OPTIONS_DOOM_CLASSIC,
	CMD_SYSTEM_1,
	CMD_SYSTEM_2,
	CMD_3D,
	CMD_PRESETS
};

/*
========================
idMenuScreen_Shell_CstSettings::Initialize
========================
*/
void idMenuScreen_Shell_CstSettings::Initialize(idMenuHandler * data) {
	numVisibleOptions = 8;
	menuSpriteName = "cstMenuSettings";
	idMenuHandler_Shell * handler = dynamic_cast<idMenuHandler_Shell *>(data);
	btnBackLabel = (handler != NULL && handler->GetInGame()) ? "#str_swf_pause_menu" : "#str_02305";
	headingLabel = "#str_swf_settings";
	shellArea = SHELL_AREA_SETTINGS;
	goBackShellArea = SHELL_AREA_ROOT;
	showJoySelect = true;

	options = new (TAG_SWF) idMenuWidget_DynamicList();
	idList< idStr > option;

	option.Append("#str_04158");					// Controls
	menuOptions.Append(option);
	option.Clear();
	option.Append(CST_STR_00300125);				// Game Options - Doom 3
	menuOptions.Append(option);
	option.Clear();
	option.Append(CST_STR_00300302);				// Game Options - Doom Classic
	menuOptions.Append(option);
	option.Clear();
	option.Append(CST_STR_00300339);				// System 1
	menuOptions.Append(option);
	option.Clear();
	option.Append(CST_STR_00300155);				// System 2
	menuOptions.Append(option);
	if (renderSystem->IsStereoScopicRenderingSupported()) {
		option.Clear();
		option.Append("#str_swf_stereoscopics");	// Stereoscopic Rendering
		menuOptions.Append(option);
	}
	option.Clear();
	option.Append(CST_STR_00300345);				// Presets
	menuOptions.Append(option);

	for (int i = 0; i < numVisibleOptions; ++i) {
		idMenuWidget_Button * const button = new (TAG_SWF) idMenuWidget_Button();
		button->Initialize(data);
		options->AddChild(button);
	}

	int index = 0;
	idMenuWidget_Button * button = dynamic_cast<idMenuWidget_Button *>(&options->GetChildByIndex(index));
	if (button != NULL) {
		button->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, index, CMD_CONTROLS);
		button->SetDescription("#str_02166");
	}

	++index;
	button = dynamic_cast<idMenuWidget_Button *>(&options->GetChildByIndex(index));
	if (button != NULL) {
		button->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, index, CMD_GAME_OPTIONS_DOOM3);
		button->SetDescription(CST_STR_00300126);
	}

	++index;
	button = dynamic_cast<idMenuWidget_Button *>(&options->GetChildByIndex(index));
	if (button != NULL) {
		button->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, index, CMD_GAME_OPTIONS_DOOM_CLASSIC);
		button->SetDescription(CST_STR_00300303);
	}

	++index;
	button = dynamic_cast<idMenuWidget_Button *>(&options->GetChildByIndex(index));
	if (button != NULL) {
		button->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, index, CMD_SYSTEM_1);
		button->SetDescription("#str_02170");
	}

	++index;
	button = dynamic_cast<idMenuWidget_Button *>(&options->GetChildByIndex(index));
	if (button != NULL) {
		button->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, index, CMD_SYSTEM_2);
		button->SetDescription("#str_02170");
	}

	if (renderSystem->IsStereoScopicRenderingSupported()) {
		++index;
		button = dynamic_cast<idMenuWidget_Button *>(&options->GetChildByIndex(index));
		if (button != NULL) {
			button->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, index, CMD_3D);
			button->SetDescription("#str_swf_customize_3d");
		}
	}

	++index;
	button = dynamic_cast<idMenuWidget_Button *>(&options->GetChildByIndex(index));
	if (button != NULL) {
		button->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, index, CMD_PRESETS);
		button->SetDescription(CST_STR_00300346);
	}

	/*
	helpText.Clear();
	helpText.Append(NULL);		// 0
	helpText.Append(NULL);		// 1
	helpText.Append(NULL);		// 2
	helpText.Append(NULL);		// 3
	helpText.Append(NULL);		// 4
	if (renderSystem->IsStereoScopicRenderingSupported()) {
		helpText.Append(NULL);	// 5
	}
	helpText.Append(NULL);		// 6
	*/

	idMenuScreen_Shell_CstMenuBase::Initialize(data);
}

/*
========================
idMenuScreen_Shell_CstSettings::HandleAction
========================
*/
bool idMenuScreen_Shell_CstSettings::HandleAction(idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled) {
	if (menuData == NULL) {
		return true;
	}
	if (menuData->ActiveScreen() != shellArea) {
		return false;
	}

	widgetAction_t actionType = action.GetType();
	const idSWFParmList & parms = action.GetParms();

	if (actionType == WIDGET_ACTION_COMMAND && parms.Num() == 2) {
		switch (parms[1].ToInteger()) {
			case CMD_CONTROLS: {
				menuData->SetNextScreen(SHELL_AREA_CONTROLS, MENU_TRANSITION_SIMPLE);
				break;
			}
			case CMD_GAME_OPTIONS_DOOM3: {
				menuData->SetNextScreen(CST_SHELL_AREA_GAME_OPTIONS_DOOM3, MENU_TRANSITION_SIMPLE);
				break;
			}
			case CMD_GAME_OPTIONS_DOOM_CLASSIC: {
				menuData->SetNextScreen(CST_SHELL_AREA_GAME_OPTIONS_DOOM_CLASSIC, MENU_TRANSITION_SIMPLE);
				break;
			}
			case CMD_SYSTEM_1: {
				menuData->SetNextScreen(SHELL_AREA_SYSTEM_OPTIONS, MENU_TRANSITION_SIMPLE);
				break;
			}
			case CMD_SYSTEM_2: {
				menuData->SetNextScreen(CST_SHELL_AREA_SYSTEM_OPTIONS_2, MENU_TRANSITION_SIMPLE);
				break;
			}
			case CMD_3D: {
				menuData->SetNextScreen(SHELL_AREA_STEREOSCOPICS, MENU_TRANSITION_SIMPLE);
				break;
			}
			case CMD_PRESETS: {
				menuData->SetNextScreen(CST_SHELL_AREA_PRESETS, MENU_TRANSITION_SIMPLE);
				break;
			}
		}
	}

	return idMenuScreen_Shell_CstMenuBase::HandleAction(action, event, widget, forceHandled);
}
//#modified-fva; END
