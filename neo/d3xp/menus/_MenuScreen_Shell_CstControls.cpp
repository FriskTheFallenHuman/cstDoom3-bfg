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
// code below is adapted from MenuScreen_Shell_Controls.cpp (with changes)

#include "precompiled.h"
#pragma hdrstop
#include "../Game_local.h"
#include "../_CstStrings.h"

enum {
	FIELD_AUTOMAP_INHERIT_BINDS,
	FIELD_GAMEPAD_ENABLED,
	FIELD_INVERT_MOUSE,
	FIELD_MOUSE_SENS,
	MAX_FIELDS
};

enum {
	CMD_BINDINGS_DOOM3,
	CMD_BINDINGS_DOOM_CLASSIC_NORMAL,
	CMD_BINDINGS_DOOM_CLASSIC_AUTOMAP,
	CMD_GAMEPAD,
	CMD_AUTOMAP_INHERIT_BINDS,
	CMD_GAMEPAD_ENABLED,
	CMD_INVERT_MOUSE,
	CMD_MOUSE_SENS
};

/*
========================
idMenuScreen_Shell_CstControls::Initialize
========================
*/
void idMenuScreen_Shell_CstControls::Initialize(idMenuHandler * data) {
	numVisibleOptions = 8;
	menuSpriteName = "cstMenuControls";
	btnBackLabel = "#str_swf_settings";
	headingLabel = "#str_04158";
	shellArea = SHELL_AREA_CONTROLS;
	goBackShellArea = SHELL_AREA_SETTINGS;
	headingLabelToUpper = true;
	showJoySelect = true;

	options = new (TAG_SWF) idMenuWidget_DynamicList();
	idMenuWidget_ControlButton * control;

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_BUTTON_TEXT);
	control->SetLabel(CST_STR_00300117); // Key Bindings - DOOM 3
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, CMD_BINDINGS_DOOM3);
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_BUTTON_TEXT);
	control->SetLabel(CST_STR_00300118); // Key Bindings - DOOM Classic - Normal
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, CMD_BINDINGS_DOOM_CLASSIC_NORMAL);
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_BUTTON_TEXT);
	control->SetLabel(CST_STR_00300119); // Key Bindings - DOOM Classic - Automap
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, CMD_BINDINGS_DOOM_CLASSIC_AUTOMAP);
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_BUTTON_TEXT);
	control->SetLabel("#str_swf_gamepad"); // Gamepad
	control->SetDescription("#str_swf_gamepad_desc");
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, CMD_GAMEPAD);
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300120); // Automap Inherit Binds
	control->SetDataSource(&dataSource, FIELD_AUTOMAP_INHERIT_BINDS);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, CMD_AUTOMAP_INHERIT_BINDS);
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel("#str_swf_gamepad_enabled"); // Gamepad Enabled
	control->SetDataSource(&dataSource, FIELD_GAMEPAD_ENABLED);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, CMD_GAMEPAD_ENABLED);
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel("#str_swf_invert_mouse"); // Invert Mouse
	control->SetDataSource(&dataSource, FIELD_INVERT_MOUSE);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, CMD_INVERT_MOUSE);
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_BAR);
	control->SetLabel("#str_swf_mouse_sens"); // Mouse Sensitivity
	control->SetDataSource(&dataSource, FIELD_MOUSE_SENS);
	control->SetupEvents(2, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, CMD_MOUSE_SENS);
	options->AddChild(control);

	helpText.Clear();
	helpText.Append(CST_STR_00300121);	// 0
	helpText.Append(CST_STR_00300122);	// 1
	helpText.Append(CST_STR_00300123);	// 2
	helpText.Append(NULL);				// 3
	helpText.Append(CST_STR_00300124);	// 4
	helpText.Append(NULL);				// 5
	helpText.Append(NULL);				// 6
	helpText.Append(NULL);				// 7

	idMenuScreen_Shell_CstMenuBase::Initialize(data);
}

/*
========================
idMenuScreen_Shell_CstControls::ShowScreen
========================
*/
void idMenuScreen_Shell_CstControls::ShowScreen(const mainMenuTransition_t transitionType) {
	dataSource.LoadData();
	idMenuScreen_Shell_CstMenuBase::ShowScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_CstControls::HideScreen
========================
*/
void idMenuScreen_Shell_CstControls::HideScreen(const mainMenuTransition_t transitionType) {
	if (dataSource.IsDataChanged()) {
		dataSource.CommitData();
	}
	if (menuData != NULL && menuData->NextScreen() != SHELL_AREA_START) {
		idMenuHandler_Shell * handler = dynamic_cast<idMenuHandler_Shell *>(menuData);
		if (handler != NULL) {
			handler->SetupPCOptions();
		}
	}
	idMenuScreen_Shell_CstMenuBase::HideScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_CstControls::HandleAction
========================
*/
bool idMenuScreen_Shell_CstControls::HandleAction(idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled) {
	if (menuData == NULL) {
		return true;
	}
	if (menuData->ActiveScreen() != shellArea) {
		return false;
	}

	widgetAction_t actionType = action.GetType();
	const idSWFParmList & parms = action.GetParms();

	if (actionType == WIDGET_ACTION_COMMAND) {
		if (options == NULL) {
			return true;
		}
		if (parms.Num() > 0) {
			switch (parms[0].ToInteger()) {
				case CMD_BINDINGS_DOOM3: {
					menuData->SetNextScreen(CST_SHELL_AREA_BINDINGS_DOOM3, MENU_TRANSITION_SIMPLE);
					break;
				}
				case CMD_BINDINGS_DOOM_CLASSIC_NORMAL: {
					menuData->SetNextScreen(CST_SHELL_AREA_BINDINGS_DOOM_CLASSIC_NORMAL, MENU_TRANSITION_SIMPLE);
					break;
				}
				case CMD_BINDINGS_DOOM_CLASSIC_AUTOMAP: {
					menuData->SetNextScreen(CST_SHELL_AREA_BINDINGS_DOOM_CLASSIC_AUTOMAP, MENU_TRANSITION_SIMPLE);
					break;
				}
				case CMD_GAMEPAD: {
					menuData->SetNextScreen(SHELL_AREA_GAMEPAD, MENU_TRANSITION_SIMPLE);
					break;
				}
				case CMD_AUTOMAP_INHERIT_BINDS: {
					dataSource.AdjustField(FIELD_AUTOMAP_INHERIT_BINDS, 1);
					options->Update();
					break;
				}
				case CMD_INVERT_MOUSE: {
					dataSource.AdjustField(FIELD_INVERT_MOUSE, 1);
					options->Update();
					break;
				}
				case CMD_MOUSE_SENS: {
					dataSource.AdjustField(FIELD_MOUSE_SENS, 1);
					options->Update();
					break;
				}
				case CMD_GAMEPAD_ENABLED: {
					dataSource.AdjustField(FIELD_GAMEPAD_ENABLED, 1);
					options->Update();
					break;
				}
			}
		}
	}

	return idMenuScreen_Shell_CstMenuBase::HandleAction(action, event, widget, forceHandled);
}

/////////////////////////////////
// SETTINGS
/////////////////////////////////

extern idCVar in_mouseInvertLook;
extern idCVar in_mouseSpeed;
extern idCVar in_useJoystick;

/*
========================
idMenuScreen_Shell_CstControls::idMenuDataSource_Settings::idMenuDataSource_AudioSettings
========================
*/
idMenuScreen_Shell_CstControls::idMenuDataSource_Settings::idMenuDataSource_Settings() {
	fields.SetNum(MAX_FIELDS);
	originalFields.SetNum(MAX_FIELDS);
}

/*
========================
idMenuScreen_Shell_CstControls::idMenuDataSource_Settings::::LoadData
========================
*/
void idMenuScreen_Shell_CstControls::idMenuDataSource_Settings::LoadData() {
	fields[FIELD_AUTOMAP_INHERIT_BINDS].SetBool(cvarSystem->GetCVarBool("cst_amInheritBinds"));
	fields[FIELD_GAMEPAD_ENABLED].SetBool(in_useJoystick.GetBool());
	fields[FIELD_INVERT_MOUSE].SetBool(in_mouseInvertLook.GetBool());
	float mouseSpeed = ((in_mouseSpeed.GetFloat() - 0.25f) / (4.0f - 0.25)) * 100.0f;
	fields[FIELD_MOUSE_SENS].SetFloat(mouseSpeed);
	originalFields = fields;
}

/*
========================
idMenuScreen_Shell_CstControls::idMenuDataSource_Settings::CommitData
========================
*/
void idMenuScreen_Shell_CstControls::idMenuDataSource_Settings::CommitData() {
	cvarSystem->SetCVarBool("cst_amInheritBinds", fields[FIELD_AUTOMAP_INHERIT_BINDS].ToBool());
	in_useJoystick.SetBool(fields[FIELD_GAMEPAD_ENABLED].ToBool());
	in_mouseInvertLook.SetBool(fields[FIELD_INVERT_MOUSE].ToBool());
	float mouseSpeed = 0.25f + ((4.0f - 0.25) * (fields[FIELD_MOUSE_SENS].ToFloat() / 100.0f));
	in_mouseSpeed.SetFloat(mouseSpeed);

	cvarSystem->SetModifiedFlags(CVAR_ARCHIVE);
	originalFields = fields; // update the backup fields
}

/*
========================
idMenuScreen_Shell_CstControls::idMenuDataSource_Settings::IsDataChanged
========================
*/
bool idMenuScreen_Shell_CstControls::idMenuDataSource_Settings::IsDataChanged() const {
	bool changed = false;
	for (int i = 0; i < fields.Num(); ++i) {
		if (i == FIELD_MOUSE_SENS) {
			if (fields[i].ToFloat() != originalFields[i].ToFloat()) {
				changed = true;
				break;
			}
		} else {
			if (fields[i].ToBool() != originalFields[i].ToBool()) {
				changed = true;
				break;
			}
		}
	}
	return changed;
}

/*
========================
idMenuScreen_Shell_CstControls::idMenuDataSource_Settings::GetField
========================
*/
idSWFScriptVar idMenuScreen_Shell_CstControls::idMenuDataSource_Settings::GetField(const int fieldIndex) const {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return false;
	}
	return fields[fieldIndex];
}

/*
========================
void idMenuScreen_Shell_CstControls::idMenuDataSource_Settings::AdjustField
========================
*/
void idMenuScreen_Shell_CstControls::idMenuDataSource_Settings::AdjustField(const int fieldIndex, const int adjustAmount) {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return;
	}
	if (fieldIndex == FIELD_MOUSE_SENS) {
		float newValue = idMath::ClampFloat(0.0f, 100.0f, fields[fieldIndex].ToFloat() + adjustAmount);
		fields[fieldIndex].SetFloat(newValue);
	} else {
		fields[fieldIndex].SetBool(!fields[fieldIndex].ToBool());
	}
}
//#modified-fva; END
