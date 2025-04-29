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
// code below is adapted from MenuScreen_Shell_GameOptions.cpp (with changes)

#include "precompiled.h"
#pragma hdrstop
#include "../Game_local.h"
#include "../_CstStrings.h"

enum {
	FIELD_FOV,
	FIELD_USE_VIEW_NODAL,
	FIELD_CHECKPOINTS,
	FIELD_AUTO_SWITCH,
	FIELD_AUTO_RELOAD,
	FIELD_AIM_ASSIST,
	FIELD_FLASHLIGHT_SHADOWS,
	FIELD_MUZZLE_FLASH,
	MAX_FIELDS
};

const float MIN_FOV = 80.0f;
const float MAX_FOV = 115.0f; // was 100.0f

/*
========================
idMenuScreen_Shell_Cst_GoD3_GeneralOptions1::Initialize
========================
*/
void idMenuScreen_Shell_Cst_GoD3_GeneralOptions1::Initialize(idMenuHandler * data) {
	numVisibleOptions = 8;
	menuSpriteName = "cstMenu_GoD3_GeneralOptions1";
	btnBackLabel = CST_STR_00300001;
	headingLabel = CST_STR_00300129;
	shellArea = CST_SHELL_AREA_GO_D3_GENERAL_OPTIONS_1;
	goBackShellArea = CST_SHELL_AREA_GAME_OPTIONS_DOOM3;
	useSmallHeading = true;

	options = new (TAG_SWF) idMenuWidget_DynamicList();
	idMenuWidget_ControlButton * control;

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TEXT);
	control->SetLabel("#str_swf_fov");
	control->SetDataSource(&dataSource, FIELD_FOV);
	control->SetupEvents(80, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300372);
	control->SetDataSource(&dataSource, FIELD_USE_VIEW_NODAL);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel("#str_swf_checkpoints");
	control->SetDataSource(&dataSource, FIELD_CHECKPOINTS);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel("#str_02135");
	control->SetDataSource(&dataSource, FIELD_AUTO_SWITCH);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel("#str_02134");
	control->SetDataSource(&dataSource, FIELD_AUTO_RELOAD);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel("#str_swf_aim_assist");
	control->SetDataSource(&dataSource, FIELD_AIM_ASSIST);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel("#str_swf_flashlight_shadows");
	control->SetDataSource(&dataSource, FIELD_FLASHLIGHT_SHADOWS);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300130);
	control->SetDataSource(&dataSource, FIELD_MUZZLE_FLASH);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	helpText.Clear();
	helpText.Append(CST_STR_00300344);
	helpText.Append(CST_STR_00300373);

	idMenuScreen_Shell_CstMenuBase::Initialize(data);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_GeneralOptions1::ShowScreen
========================
*/
void idMenuScreen_Shell_Cst_GoD3_GeneralOptions1::ShowScreen(const mainMenuTransition_t transitionType) {
	dataSource.LoadData();
	idMenuScreen_Shell_CstMenuBase::ShowScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_GeneralOptions1::HideScreen
========================
*/
void idMenuScreen_Shell_Cst_GoD3_GeneralOptions1::HideScreen(const mainMenuTransition_t transitionType) {
	if (dataSource.IsDataChanged()) {
		dataSource.CommitData();
	}
	idMenuScreen_Shell_CstMenuBase::HideScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_GeneralOptions1::HandleAction
========================
*/
bool idMenuScreen_Shell_Cst_GoD3_GeneralOptions1::HandleAction(idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled) {
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
		int selectionIndex = options->GetFocusIndex();
		if (parms.Num() > 0) {
			selectionIndex = parms[0].ToInteger();
		}
		dataSource.AdjustField(selectionIndex, 1);
		options->Update();
	}

	return idMenuScreen_Shell_CstMenuBase::HandleAction(action, event, widget, forceHandled);
}

/////////////////////////////////
// SETTINGS
/////////////////////////////////

extern idCVar ui_autoSwitch;
extern idCVar ui_autoReload;
extern idCVar aa_targetAimAssistEnable;
extern idCVar g_checkpoints;
extern idCVar g_weaponShadows;

/*
========================
idMenuScreen_Shell_Cst_GoD3_GeneralOptions1::idMenuDataSource_Settings::idMenuDataSource_Settings
========================
*/
idMenuScreen_Shell_Cst_GoD3_GeneralOptions1::idMenuDataSource_Settings::idMenuDataSource_Settings() {
	fields.SetNum(MAX_FIELDS);
	originalFields.SetNum(MAX_FIELDS);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_GeneralOptions1::idMenuDataSource_Settings::LoadData
========================
*/
void idMenuScreen_Shell_Cst_GoD3_GeneralOptions1::idMenuDataSource_Settings::LoadData() {
	fields[FIELD_FOV].SetInteger(g_fov.GetFloat());
	fields[FIELD_USE_VIEW_NODAL].SetBool(cst_useViewNodal.GetBool());
	fields[FIELD_CHECKPOINTS].SetBool(g_checkpoints.GetBool());
	fields[FIELD_AUTO_SWITCH].SetBool(ui_autoSwitch.GetBool());
	fields[FIELD_AUTO_RELOAD].SetBool(ui_autoReload.GetBool());
	fields[FIELD_AIM_ASSIST].SetBool(aa_targetAimAssistEnable.GetBool());
	fields[FIELD_FLASHLIGHT_SHADOWS].SetBool(g_weaponShadows.GetBool());
	fields[FIELD_MUZZLE_FLASH].SetBool(g_muzzleFlash.GetBool());
	originalFields = fields;
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_GeneralOptions1::idMenuDataSource_Settings::CommitData
========================
*/
void idMenuScreen_Shell_Cst_GoD3_GeneralOptions1::idMenuDataSource_Settings::CommitData() {
	if (fields[FIELD_FOV].ToInteger() != originalFields[FIELD_FOV].ToInteger()) {
		g_fov.SetFloat(fields[FIELD_FOV].ToFloat());

		// new approach to compute g_gun_x
		const float d1 = 1.0f / idMath::Tan(DEG2RAD(80.0f * 0.5f));
		const float gunX_1 = 3.0f;

		const float d2 = 1.0f / idMath::Tan(DEG2RAD(106.0f * 0.5f));
		const float gunX_2 = 0.0f;

		float d = 1.0f / idMath::Tan(DEG2RAD(fields[FIELD_FOV].ToFloat() * 0.5f));
		float f = (d - d1) / (d2 - d1);
		float gunX = Lerp(gunX_1, gunX_2, f);

		g_gun_x.SetFloat(gunX);
	}

	cst_useViewNodal.SetBool(fields[FIELD_USE_VIEW_NODAL].ToBool());
	g_checkpoints.SetBool(fields[FIELD_CHECKPOINTS].ToBool());
	ui_autoSwitch.SetBool(fields[FIELD_AUTO_SWITCH].ToBool());
	ui_autoReload.SetBool(fields[FIELD_AUTO_RELOAD].ToBool());
	aa_targetAimAssistEnable.SetBool(fields[FIELD_AIM_ASSIST].ToBool());
	g_weaponShadows.SetBool(fields[FIELD_FLASHLIGHT_SHADOWS].ToBool());
	g_muzzleFlash.SetBool(fields[FIELD_MUZZLE_FLASH].ToBool());

	cvarSystem->SetModifiedFlags(CVAR_ARCHIVE);
	originalFields = fields; // update the backup fields
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_GeneralOptions1::idMenuDataSource_Settings::IsDataChanged
========================
*/
bool idMenuScreen_Shell_Cst_GoD3_GeneralOptions1::idMenuDataSource_Settings::IsDataChanged() const {
	bool changed = false;
	for (int i = 0; i < fields.Num(); ++i) {
		if (i == FIELD_FOV) {
			if (fields[i].ToInteger() != originalFields[i].ToInteger()) {
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
idMenuScreen_Shell_Cst_GoD3_GeneralOptions1::idMenuDataSource_Settings::GetField
========================
*/
idSWFScriptVar idMenuScreen_Shell_Cst_GoD3_GeneralOptions1::idMenuDataSource_Settings::GetField(const int fieldIndex) const {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return false;
	}
	return fields[fieldIndex];
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_GeneralOptions1::idMenuDataSource_Settings::AdjustField
========================
*/
void idMenuScreen_Shell_Cst_GoD3_GeneralOptions1::idMenuDataSource_Settings::AdjustField(const int fieldIndex, const int adjustAmount) {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return;
	}
	if (fieldIndex == FIELD_FOV) {
		fields[fieldIndex].SetInteger(idMath::ClampInt(MIN_FOV, MAX_FOV, fields[fieldIndex].ToInteger() + adjustAmount));
	} else {
		fields[fieldIndex].SetBool(!fields[fieldIndex].ToBool());
	}
}
//#modified-fva; END
