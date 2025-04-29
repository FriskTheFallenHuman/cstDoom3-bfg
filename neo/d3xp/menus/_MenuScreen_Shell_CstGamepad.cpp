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
// code below is adapted from MenuScreen_Shell_Gamepad.cpp (with changes)

#include "precompiled.h"
#pragma hdrstop
#include "../Game_local.h"
#include "../_CstStrings.h"

enum {
	FIELD_LEFTY,
	FIELD_INVERT,
	FIELD_VIBRATE,
	FIELD_HOR_SENS,
	FIELD_VERT_SENS,
	FIELD_ACCELERATION,
	FIELD_THRESHOLD,
	MAX_FIELDS
};

enum {
#ifndef ID_PC
	CMD_CONFIG,
#endif
	CMD_LEFTY,
	CMD_INVERT,
	CMD_VIBRATE,
	CMD_HOR_SENS,
	CMD_VERT_SENS,
	CMD_ACCELERATION,
	CMD_THRESHOLD,
};

/*
========================
idMenuScreen_Shell_CstGamepad::Initialize
========================
*/
void idMenuScreen_Shell_CstGamepad::Initialize(idMenuHandler * data) {
	numVisibleOptions = 8;
	menuSpriteName = "cstMenuGamepad";
	btnBackLabel = "#str_04158";
	headingLabel = "#str_swf_gamepad_heading";
	shellArea = SHELL_AREA_GAMEPAD;
	goBackShellArea = SHELL_AREA_CONTROLS;
	btnBackLabelToUpper = true;
#ifndef ID_PC
	showJoySelect = true;
#endif

	options = new (TAG_SWF) idMenuWidget_DynamicList();
	idMenuWidget_ControlButton * control;

#ifndef ID_PC
	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_BUTTON_TEXT);
	control->SetLabel("#str_swf_gamepad_config");	// Gamepad Configuration
	control->SetDescription("#str_swf_config_desc");
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, CMD_CONFIG);
	options->AddChild(control);
#endif

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel("#str_swf_lefty_flip");
	control->SetDataSource(&dataSource, FIELD_LEFTY);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, CMD_LEFTY);
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel("#str_swf_invert_gamepad");
	control->SetDataSource(&dataSource, FIELD_INVERT);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, CMD_INVERT);
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel("#str_swf_vibration");
	control->SetDataSource(&dataSource, FIELD_VIBRATE);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, CMD_VIBRATE);
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_BAR);
	control->SetLabel("#str_swf_hor_sens");
	control->SetDataSource(&dataSource, FIELD_HOR_SENS);
	control->SetupEvents(2, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, CMD_HOR_SENS);
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_BAR);
	control->SetLabel("#str_swf_vert_sens");
	control->SetDataSource(&dataSource, FIELD_VERT_SENS);
	control->SetupEvents(2, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, CMD_VERT_SENS);
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel("#str_swf_joy_gammaLook");
	control->SetDataSource(&dataSource, FIELD_ACCELERATION);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, CMD_ACCELERATION);
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel("#str_swf_joy_mergedThreshold");
	control->SetDataSource(&dataSource, FIELD_THRESHOLD);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, CMD_THRESHOLD);
	options->AddChild(control);

	/*
	helpText.Clear();
#ifndef ID_PC
	helpText.Append(NULL); // 0
#endif
	helpText.Append(NULL); // 1
	helpText.Append(NULL); // 2
	helpText.Append(NULL); // 3
	helpText.Append(NULL); // 4
	helpText.Append(NULL); // 5
	helpText.Append(NULL); // 6
	helpText.Append(NULL); // 7
	*/

	idMenuScreen_Shell_CstMenuBase::Initialize(data);
}

/*
========================
idMenuScreen_Shell_CstGamepad::ShowScreen
========================
*/
void idMenuScreen_Shell_CstGamepad::ShowScreen(const mainMenuTransition_t transitionType) {
	dataSource.LoadData();
	idMenuScreen_Shell_CstMenuBase::ShowScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_CstGamepad::HideScreen
========================
*/
void idMenuScreen_Shell_CstGamepad::HideScreen(const mainMenuTransition_t transitionType) {
	if (dataSource.IsDataChanged()) {
		dataSource.CommitData();
	}
	/*
	if (menuData != NULL) {
		idMenuHandler_Shell * handler = dynamic_cast<idMenuHandler_Shell *>(menuData);
		if (handler != NULL) {
			handler->SetupPCOptions();
		}
	}
	*/
	idMenuScreen_Shell_CstMenuBase::HideScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_CstGamepad::HandleAction
========================
*/
bool idMenuScreen_Shell_CstGamepad::HandleAction(idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled) {
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
#ifndef ID_PC
				case CMD_CONFIG: {
					menuData->SetNextScreen(SHELL_AREA_CONTROLLER_LAYOUT, MENU_TRANSITION_SIMPLE);
					break;
				}
#endif
				case CMD_INVERT: {
					dataSource.AdjustField(FIELD_INVERT, 1);
					options->Update();
					break;
				}
				case CMD_LEFTY: {
					dataSource.AdjustField(FIELD_LEFTY, 1);
					options->Update();
					break;
				}
				case CMD_VIBRATE: {
					dataSource.AdjustField(FIELD_VIBRATE, 1);
					options->Update();
					break;
				}
				case CMD_HOR_SENS: {
					dataSource.AdjustField(FIELD_HOR_SENS, 1);
					options->Update();
					break;
				}
				case CMD_VERT_SENS: {
					dataSource.AdjustField(FIELD_VERT_SENS, 1);
					options->Update();
					break;
				}
				case CMD_ACCELERATION: {
					dataSource.AdjustField(FIELD_ACCELERATION, 1);
					options->Update();
					break;
				}
				case CMD_THRESHOLD: {
					dataSource.AdjustField(FIELD_THRESHOLD, 1);
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

extern idCVar in_invertLook;
extern idCVar in_joystickRumble;
extern idCVar joy_pitchSpeed;
extern idCVar joy_yawSpeed;
extern idCVar joy_gammaLook;
extern idCVar joy_mergedThreshold;

/*
========================
idMenuScreen_Shell_CstGamepad::idMenuDataSource_Settings::idMenuDataSource_Settings
========================
*/
idMenuScreen_Shell_CstGamepad::idMenuDataSource_Settings::idMenuDataSource_Settings() {
	fields.SetNum(MAX_FIELDS);
	originalFields.SetNum(MAX_FIELDS);
}

/*
========================
idMenuScreen_Shell_CstGamepad::idMenuDataSource_Settings::LoadData
========================
*/
void idMenuScreen_Shell_CstGamepad::idMenuDataSource_Settings::LoadData() {
	fields[FIELD_LEFTY].SetBool(cvarSystem->GetCVarBool("cst_joyLeftyFlip"));
	fields[FIELD_INVERT].SetBool(in_invertLook.GetBool());
	fields[FIELD_VIBRATE].SetBool(in_joystickRumble.GetBool());
	fields[FIELD_HOR_SENS].SetFloat(100.0f * ((joy_yawSpeed.GetFloat() - 100.0f) / 300.0f));
	fields[FIELD_VERT_SENS].SetFloat(100.0f * ((joy_pitchSpeed.GetFloat() - 60.0f) / 200.0f));
	fields[FIELD_ACCELERATION].SetBool(joy_gammaLook.GetBool());
	fields[FIELD_THRESHOLD].SetBool(joy_mergedThreshold.GetBool());
	originalFields = fields;
}

/*
========================
idMenuScreen_Shell_CstGamepad::idMenuDataSource_Settings::CommitData
========================
*/
void idMenuScreen_Shell_CstGamepad::idMenuDataSource_Settings::CommitData() {
	if (fields[FIELD_LEFTY].ToBool() != originalFields[FIELD_LEFTY].ToBool()) {
		// see the original idPlayerProfile::ExecConfig
		bool leftyFlip = fields[FIELD_LEFTY].ToBool();
		cvarSystem->SetCVarBool("cst_joyLeftyFlip", leftyFlip);
		if (leftyFlip) {
			cmdSystem->BufferCommandText(CMD_EXEC_NOW, "exec joy_lefty.cfg\n");
		} else {
			cmdSystem->BufferCommandText(CMD_EXEC_NOW, "exec joy_righty.cfg\n");
		}
	}
	in_invertLook.SetBool(fields[FIELD_INVERT].ToBool());
	in_joystickRumble.SetBool(fields[FIELD_VIBRATE].ToBool());
	joy_yawSpeed.SetFloat(((fields[FIELD_HOR_SENS].ToFloat() / 100.0f) * 300.0f) + 100.0f);
	joy_pitchSpeed.SetFloat(((fields[FIELD_VERT_SENS].ToFloat() / 100.0f) * 200.0f) + 60.0f);
	joy_gammaLook.SetBool(fields[FIELD_ACCELERATION].ToBool());
	joy_mergedThreshold.SetBool(fields[FIELD_THRESHOLD].ToBool());

	cvarSystem->SetModifiedFlags(CVAR_ARCHIVE);
	originalFields = fields; // update the backup fields
}

/*
========================
idMenuScreen_Shell_CstGamepad::idMenuDataSource_Settings::IsDataChanged
========================
*/
bool idMenuScreen_Shell_CstGamepad::idMenuDataSource_Settings::IsDataChanged() const {
	bool changed = false;
	for (int i = 0; i < fields.Num(); ++i) {
		if (i == FIELD_HOR_SENS || i == FIELD_VERT_SENS) {
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
idMenuScreen_Shell_CstGamepad::idMenuDataSource_Settings::GetField
========================
*/
idSWFScriptVar idMenuScreen_Shell_CstGamepad::idMenuDataSource_Settings::GetField(const int fieldIndex) const {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return false;
	}
	return fields[fieldIndex];
}

/*
========================
idMenuScreen_Shell_CstGamepad::idMenuDataSource_Settings::AdjustField
========================
*/
void idMenuScreen_Shell_CstGamepad::idMenuDataSource_Settings::AdjustField(const int fieldIndex, const int adjustAmount) {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return;
	}
	if (fieldIndex == FIELD_HOR_SENS || fieldIndex == FIELD_VERT_SENS) {
		float newValue = idMath::ClampFloat(0.0f, 100.0f, fields[fieldIndex].ToFloat() + adjustAmount);
		fields[fieldIndex].SetFloat(newValue);
	} else {
		fields[fieldIndex].SetBool(!fields[fieldIndex].ToBool());
	}
}
//#modified-fva; END
