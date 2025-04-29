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
#include "precompiled.h"
#pragma hdrstop
#include "../Game_local.h"
#include "../_CstStrings.h"

enum {
	FIELD_TAKE_NO_AMMO_WEAPONS,
	FIELD_ARMOR_PROTECTION_SP,
	FIELD_OLD_HEALTH_STATION,
	FIELD_OLD_PLAYER_SPEED_SP,
	FIELD_OLD_AMMO_D3,
	FIELD_OLD_AMMO_ROE,
	FIELD_OLD_AMMO_LM,
	MAX_FIELDS
};

/*
========================
idMenuScreen_Shell_Cst_GoD3_GeneralOptions3::Initialize
========================
*/
void idMenuScreen_Shell_Cst_GoD3_GeneralOptions3::Initialize(idMenuHandler * data) {
	numVisibleOptions = 8;
	menuSpriteName = "cstMenu_GoD3_GeneralOptions3";
	btnBackLabel = CST_STR_00300001;
	headingLabel = CST_STR_00300406;
	shellArea = CST_SHELL_AREA_GO_D3_GENERAL_OPTIONS_3;
	goBackShellArea = CST_SHELL_AREA_GAME_OPTIONS_DOOM3;
	useSmallHeading = true;

	options = new (TAG_SWF) idMenuWidget_DynamicList();
	idMenuWidget_ControlButton * control;

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300140);
	control->SetDataSource(&dataSource, FIELD_TAKE_NO_AMMO_WEAPONS);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TEXT);
	control->SetLabel(CST_STR_00300407);
	control->SetDataSource(&dataSource, FIELD_ARMOR_PROTECTION_SP);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300141);
	control->SetDataSource(&dataSource, FIELD_OLD_HEALTH_STATION);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300143);
	control->SetDataSource(&dataSource, FIELD_OLD_PLAYER_SPEED_SP);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300142);
	control->SetDataSource(&dataSource, FIELD_OLD_AMMO_D3);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300374);
	control->SetDataSource(&dataSource, FIELD_OLD_AMMO_ROE);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300376);
	control->SetDataSource(&dataSource, FIELD_OLD_AMMO_LM);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	helpText.Clear();
	helpText.Append(CST_STR_00300151); // 0
	helpText.Append(CST_STR_00300408); // 1
	helpText.Append(CST_STR_00300152); // 2
	helpText.Append(CST_STR_00300154); // 3
	helpText.Append(CST_STR_00300153); // 4
	helpText.Append(CST_STR_00300375); // 5
	helpText.Append(CST_STR_00300377); // 6

	idMenuScreen_Shell_CstMenuBase::Initialize(data);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_GeneralOptions3::ShowScreen
========================
*/
void idMenuScreen_Shell_Cst_GoD3_GeneralOptions3::ShowScreen(const mainMenuTransition_t transitionType) {
	dataSource.LoadData();
	idMenuScreen_Shell_CstMenuBase::ShowScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_GeneralOptions3::HideScreen
========================
*/
void idMenuScreen_Shell_Cst_GoD3_GeneralOptions3::HideScreen(const mainMenuTransition_t transitionType) {
	if (dataSource.IsDataChanged()) {
		dataSource.CommitData();
	}
	idMenuScreen_Shell_CstMenuBase::HideScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_GeneralOptions3::HandleAction
========================
*/
bool idMenuScreen_Shell_Cst_GoD3_GeneralOptions3::HandleAction(idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled) {
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

/*
========================
idMenuScreen_Shell_Cst_GoD3_GeneralOptions3::idMenuDataSource_Settings::idMenuDataSource_Settings
========================
*/
idMenuScreen_Shell_Cst_GoD3_GeneralOptions3::idMenuDataSource_Settings::idMenuDataSource_Settings() {
	fields.SetNum(MAX_FIELDS);
	originalFields.SetNum(MAX_FIELDS);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_GeneralOptions3::idMenuDataSource_Settings::LoadData
========================
*/
void idMenuScreen_Shell_Cst_GoD3_GeneralOptions3::idMenuDataSource_Settings::LoadData() {
	fields[FIELD_TAKE_NO_AMMO_WEAPONS].SetBool(cst_takeNoAmmoWeapons.GetBool());
	fields[FIELD_ARMOR_PROTECTION_SP].SetFloat(cst_armorProtectionSP.GetFloat());
	fields[FIELD_OLD_HEALTH_STATION].SetBool(cst_oldHealthStation.GetBool());
	fields[FIELD_OLD_PLAYER_SPEED_SP].SetBool(cst_oldPlayerSpeedSP.GetBool());
	fields[FIELD_OLD_AMMO_D3].SetBool(cst_oldAmmoD3.GetBool());
	fields[FIELD_OLD_AMMO_ROE].SetBool(cst_oldAmmoRoE.GetBool());
	fields[FIELD_OLD_AMMO_LM].SetBool(cst_oldAmmoLM.GetBool());
	originalFields = fields;
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_GeneralOptions3::idMenuDataSource_Settings::CommitData
========================
*/
void idMenuScreen_Shell_Cst_GoD3_GeneralOptions3::idMenuDataSource_Settings::CommitData() {
	cst_takeNoAmmoWeapons.SetBool(fields[FIELD_TAKE_NO_AMMO_WEAPONS].ToBool());
	cst_armorProtectionSP.SetFloat(fields[FIELD_ARMOR_PROTECTION_SP].ToFloat());
	cst_oldHealthStation.SetBool(fields[FIELD_OLD_HEALTH_STATION].ToBool());
	cst_oldPlayerSpeedSP.SetBool(fields[FIELD_OLD_PLAYER_SPEED_SP].ToBool());
	cst_oldAmmoD3.SetBool(fields[FIELD_OLD_AMMO_D3].ToBool());
	cst_oldAmmoRoE.SetBool(fields[FIELD_OLD_AMMO_ROE].ToBool());
	cst_oldAmmoLM.SetBool(fields[FIELD_OLD_AMMO_LM].ToBool());

	cvarSystem->SetModifiedFlags(CVAR_ARCHIVE);
	originalFields = fields; // update the backup fields
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_GeneralOptions3::idMenuDataSource_Settings::IsDataChanged
========================
*/
bool idMenuScreen_Shell_Cst_GoD3_GeneralOptions3::idMenuDataSource_Settings::IsDataChanged() const {
	bool changed = false;
	for (int i = 0; i < fields.Num(); ++i) {
		if (i == FIELD_ARMOR_PROTECTION_SP) {
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
idMenuScreen_Shell_Cst_GoD3_GeneralOptions3::idMenuDataSource_Settings::GetField
========================
*/
idSWFScriptVar idMenuScreen_Shell_Cst_GoD3_GeneralOptions3::idMenuDataSource_Settings::GetField(const int fieldIndex) const {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return false;
	}
	if (fieldIndex == FIELD_ARMOR_PROTECTION_SP) {
		float armorProtectionSP = fields[fieldIndex].ToFloat();
		if (armorProtectionSP < 0.0f) {
			return CST_STR_00300409;
		}
		float armorProtectionPercent = armorProtectionSP * 100.0f;
		armorProtectionPercent = idMath::Rint(armorProtectionPercent * 100.0f) / 100.0f; // round to 2 digits after the decimal point
		float fracPercent = idMath::Frac(armorProtectionPercent);
		if (fracPercent == 0.0f) {
			return va("%d %%", (int)armorProtectionPercent);
		}
		return va("%.2f %%", armorProtectionPercent);
	} else {
		return fields[fieldIndex];
	}
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_GeneralOptions3::idMenuDataSource_Settings::AdjustField
========================
*/
void idMenuScreen_Shell_Cst_GoD3_GeneralOptions3::idMenuDataSource_Settings::AdjustField(const int fieldIndex, const int adjustAmount) {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return;
	}
	if (fieldIndex == FIELD_ARMOR_PROTECTION_SP) {
		if (adjustAmount == 0) {
			return;
		}
		float armorProtectionSP = fields[fieldIndex].ToFloat();
		int values[] = { -10, 2, 3, 4, 5, 6, 7, 8 };
		int numValues = sizeof(values) / sizeof(int);
		int currentValue = (int)idMath::Floor(armorProtectionSP * 10.0f);
		if (currentValue < 0) {
			currentValue = -10;
		} else {
			if (currentValue < 2) {
				currentValue = (adjustAmount < 0) ? 2 : -10;
			} else if (currentValue > 8) {
				currentValue = (adjustAmount < 0) ? -10 : 8;
			}
		}
		int adjustedValue = CstMenuUtils::AdjustOption(currentValue, values, numValues, adjustAmount);
		armorProtectionSP = adjustedValue * 0.1f;
		fields[fieldIndex].SetFloat(armorProtectionSP);
	} else {
		fields[fieldIndex].SetBool(!fields[fieldIndex].ToBool());
	}
}
//#modified-fva; END
