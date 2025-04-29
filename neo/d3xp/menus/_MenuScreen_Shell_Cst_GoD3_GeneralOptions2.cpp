//#modified-fva; BEGIN
#include "precompiled.h"
#pragma hdrstop
#include "../Game_local.h"
#include "../_CstStrings.h"

enum {
	FIELD_SHOW_ACCESS_CODES,
	FIELD_HEADLAMP_ALERT_MONSTERS,
	FIELD_HEADLAMP_KEEP_STATE,
	FIELD_HEADLAMP_SOUND,
	FIELD_ARMOR_FLASHLIGHT_MODEL,
	FIELD_CROSSHAIR_ALWAYS_SHOW,
	FIELD_HUD_SMART_AMMO_SP,
	FIELD_HUD_SMART_AMMO_MP,
	MAX_FIELDS
};

/*
========================
idMenuScreen_Shell_Cst_GoD3_GeneralOptions2::Initialize
========================
*/
void idMenuScreen_Shell_Cst_GoD3_GeneralOptions2::Initialize(idMenuHandler * data) {
	numVisibleOptions = 8;
	menuSpriteName = "cstMenu_GoD3_GeneralOptions2";
	btnBackLabel = CST_STR_00300001;
	headingLabel = CST_STR_00300132;
	shellArea = CST_SHELL_AREA_GO_D3_GENERAL_OPTIONS_2;
	goBackShellArea = CST_SHELL_AREA_GAME_OPTIONS_DOOM3;
	useSmallHeading = true;

	options = new (TAG_SWF) idMenuWidget_DynamicList();
	idMenuWidget_ControlButton * control;

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300133);
	control->SetDataSource(&dataSource, FIELD_SHOW_ACCESS_CODES);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300134);
	control->SetDataSource(&dataSource, FIELD_HEADLAMP_ALERT_MONSTERS);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300135);
	control->SetDataSource(&dataSource, FIELD_HEADLAMP_KEEP_STATE);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300136);
	control->SetDataSource(&dataSource, FIELD_HEADLAMP_SOUND);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TEXT);
	control->SetLabel(CST_STR_00300389);
	control->SetDataSource(&dataSource, FIELD_ARMOR_FLASHLIGHT_MODEL);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300137);
	control->SetDataSource(&dataSource, FIELD_CROSSHAIR_ALWAYS_SHOW);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300138);
	control->SetDataSource(&dataSource, FIELD_HUD_SMART_AMMO_SP);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300139);
	control->SetDataSource(&dataSource, FIELD_HUD_SMART_AMMO_MP);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	helpText.Clear();
	helpText.Append(CST_STR_00300144); // 0
	helpText.Append(CST_STR_00300145); // 1
	helpText.Append(CST_STR_00300146); // 2
	helpText.Append(CST_STR_00300147); // 3
	helpText.Append(CST_STR_00300390); // 4
	helpText.Append(CST_STR_00300148); // 5
	helpText.Append(CST_STR_00300149); // 6
	helpText.Append(CST_STR_00300150); // 7

	idMenuScreen_Shell_CstMenuBase::Initialize(data);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_GeneralOptions2::ShowScreen
========================
*/
void idMenuScreen_Shell_Cst_GoD3_GeneralOptions2::ShowScreen(const mainMenuTransition_t transitionType) {
	dataSource.LoadData();
	idMenuScreen_Shell_CstMenuBase::ShowScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_GeneralOptions2::HideScreen
========================
*/
void idMenuScreen_Shell_Cst_GoD3_GeneralOptions2::HideScreen(const mainMenuTransition_t transitionType) {
	if (dataSource.IsDataChanged()) {
		dataSource.CommitData();
	}
	idMenuScreen_Shell_CstMenuBase::HideScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_GeneralOptions2::HandleAction
========================
*/
bool idMenuScreen_Shell_Cst_GoD3_GeneralOptions2::HandleAction(idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled) {
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
idMenuScreen_Shell_Cst_GoD3_GeneralOptions2::idMenuDataSource_Settings::idMenuDataSource_Settings
========================
*/
idMenuScreen_Shell_Cst_GoD3_GeneralOptions2::idMenuDataSource_Settings::idMenuDataSource_Settings() {
	fields.SetNum(MAX_FIELDS);
	originalFields.SetNum(MAX_FIELDS);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_GeneralOptions2::idMenuDataSource_Settings::LoadData
========================
*/
void idMenuScreen_Shell_Cst_GoD3_GeneralOptions2::idMenuDataSource_Settings::LoadData() {
	fields[FIELD_SHOW_ACCESS_CODES].SetBool(cst_showAccessCodes.GetBool());
	fields[FIELD_HEADLAMP_ALERT_MONSTERS].SetBool(cst_headlampAlertMonsters.GetBool());
	fields[FIELD_HEADLAMP_KEEP_STATE].SetBool(cst_headlampKeepState.GetBool());
	fields[FIELD_HEADLAMP_SOUND].SetBool(cst_headlampSound.GetBool());
	fields[FIELD_ARMOR_FLASHLIGHT_MODEL].SetInteger(cst_armorFlashlightHideModel.GetInteger());
	fields[FIELD_CROSSHAIR_ALWAYS_SHOW].SetBool(cst_crosshairAlwaysShow.GetBool());
	fields[FIELD_HUD_SMART_AMMO_SP].SetBool(cst_hudSmartAmmoSP.GetBool());
	fields[FIELD_HUD_SMART_AMMO_MP].SetBool(cst_hudSmartAmmoMP.GetBool());
	originalFields = fields;
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_GeneralOptions2::idMenuDataSource_Settings::CommitData
========================
*/
void idMenuScreen_Shell_Cst_GoD3_GeneralOptions2::idMenuDataSource_Settings::CommitData() {
	cst_showAccessCodes.SetBool(fields[FIELD_SHOW_ACCESS_CODES].ToBool());
	cst_headlampAlertMonsters.SetBool(fields[FIELD_HEADLAMP_ALERT_MONSTERS].ToBool());
	cst_headlampKeepState.SetBool(fields[FIELD_HEADLAMP_KEEP_STATE].ToBool());
	cst_headlampSound.SetBool(fields[FIELD_HEADLAMP_SOUND].ToBool());
	cst_armorFlashlightHideModel.SetInteger(fields[FIELD_ARMOR_FLASHLIGHT_MODEL].ToInteger());
	cst_crosshairAlwaysShow.SetBool(fields[FIELD_CROSSHAIR_ALWAYS_SHOW].ToBool());
	cst_hudSmartAmmoSP.SetBool(fields[FIELD_HUD_SMART_AMMO_SP].ToBool());
	cst_hudSmartAmmoMP.SetBool(fields[FIELD_HUD_SMART_AMMO_MP].ToBool());

	cvarSystem->SetModifiedFlags(CVAR_ARCHIVE);
	originalFields = fields; // update the backup fields
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_GeneralOptions2::idMenuDataSource_Settings::IsDataChanged
========================
*/
bool idMenuScreen_Shell_Cst_GoD3_GeneralOptions2::idMenuDataSource_Settings::IsDataChanged() const {
	bool changed = false;
	for (int i = 0; i < fields.Num(); ++i) {
		if (i == FIELD_ARMOR_FLASHLIGHT_MODEL) {
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
idMenuScreen_Shell_Cst_GoD3_GeneralOptions2::idMenuDataSource_Settings::GetField
========================
*/
idSWFScriptVar idMenuScreen_Shell_Cst_GoD3_GeneralOptions2::idMenuDataSource_Settings::GetField(const int fieldIndex) const {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return false;
	}
	if (fieldIndex == FIELD_ARMOR_FLASHLIGHT_MODEL) {
		switch (fields[fieldIndex].ToInteger()) {
			case 0: return CST_STR_00300386;	// Visible
			case 1: return CST_STR_00300387;	// No Shadow
			case 2: return CST_STR_00300388;	// Hidden
			default: return CST_STR_00300168;	// Unknown
		}
	} else {
		return fields[fieldIndex];
	}
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_GeneralOptions2::idMenuDataSource_Settings::AdjustField
========================
*/
void idMenuScreen_Shell_Cst_GoD3_GeneralOptions2::idMenuDataSource_Settings::AdjustField(const int fieldIndex, const int adjustAmount) {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return;
	}
	if (fieldIndex == FIELD_ARMOR_FLASHLIGHT_MODEL) {
		const int numValues = 3;
		int values[numValues] = { 0, 1, 2 };
		fields[fieldIndex].SetInteger(CstMenuUtils::AdjustOption(fields[fieldIndex].ToInteger(), values, numValues, adjustAmount));
	} else {
		fields[fieldIndex].SetBool(!fields[fieldIndex].ToBool());
	}
}
//#modified-fva; END
