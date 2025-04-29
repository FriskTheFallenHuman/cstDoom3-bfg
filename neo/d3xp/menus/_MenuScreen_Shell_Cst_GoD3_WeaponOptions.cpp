//#modified-fva; BEGIN
#include "precompiled.h"
#pragma hdrstop
#include "../Game_local.h"
#include "../_CstStrings.h"

enum {
	FIELD_DOUBLE_BARREL_SHOTGUN_IN_D3,
	FIELD_GRABBER_IN_D3,
	FIELD_CHAINSAW_IN_ROE,
	FIELD_CHAINSAW_IN_LM,
	FIELD_SHOTGUN_SPREAD_REDUCTION,
	FIELD_GRENADES_TOGGLE,
	FIELD_GRABBER_UNLIMITED_TIME,
	FIELD_GRABBER_STABLE_THROW,
	FIELD_GRABBER_DROP_ON_HOLD_LAYER,
	FIELD_GRABBER_DROP_ON_SWITCH_LAYER,
	FIELD_DBS_ALWAYS_AUTO_RELOAD,
	FIELD_INSTANTANEOUS_RELOAD_SP,
	MAX_FIELDS
};

/*
========================
idMenuScreen_Shell_Cst_GoD3_WeaponOptions::Initialize
========================
*/
void idMenuScreen_Shell_Cst_GoD3_WeaponOptions::Initialize(idMenuHandler * data) {
	numVisibleOptions = 14;
	menuSpriteName = "cstMenu_GoD3_WeaponOptions";
	btnBackLabel = CST_STR_00300001;
	headingLabel = CST_STR_00300205;
	shellArea = CST_SHELL_AREA_GO_D3_WEAPON_OPTIONS;
	goBackShellArea = CST_SHELL_AREA_GAME_OPTIONS_DOOM3;
	useSmallHeading = true;

	options = new (TAG_SWF) idMenuWidget_DynamicList();
	idMenuWidget_ControlButton * control;

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300206);
	control->SetDataSource(&dataSource, FIELD_DOUBLE_BARREL_SHOTGUN_IN_D3);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300207);
	control->SetDataSource(&dataSource, FIELD_GRABBER_IN_D3);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300208);
	control->SetDataSource(&dataSource, FIELD_CHAINSAW_IN_ROE);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300209);
	control->SetDataSource(&dataSource, FIELD_CHAINSAW_IN_LM);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300210);
	control->SetDataSource(&dataSource, FIELD_SHOTGUN_SPREAD_REDUCTION);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300211);
	control->SetDataSource(&dataSource, FIELD_GRENADES_TOGGLE);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300212);
	control->SetDataSource(&dataSource, FIELD_GRABBER_UNLIMITED_TIME);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TEXT);
	control->SetLabel(CST_STR_00300213);
	control->SetDataSource(&dataSource, FIELD_GRABBER_STABLE_THROW);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TEXT);
	control->SetLabel(CST_STR_00300214);
	control->SetDataSource(&dataSource, FIELD_GRABBER_DROP_ON_HOLD_LAYER);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TEXT);
	control->SetLabel(CST_STR_00300215);
	control->SetDataSource(&dataSource, FIELD_GRABBER_DROP_ON_SWITCH_LAYER);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300410);
	control->SetDataSource(&dataSource, FIELD_DBS_ALWAYS_AUTO_RELOAD);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300411);
	control->SetDataSource(&dataSource, FIELD_INSTANTANEOUS_RELOAD_SP);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	helpText.Clear();
	helpText.Append(CST_STR_00300228); // 0
	helpText.Append(CST_STR_00300229); // 1
	helpText.Append(CST_STR_00300230); // 2
	helpText.Append(CST_STR_00300231); // 3
	helpText.Append(CST_STR_00300232); // 4
	helpText.Append(CST_STR_00300233); // 5
	helpText.Append(CST_STR_00300234); // 6
	helpText.Append(CST_STR_00300235); // 7
	helpText.Append(CST_STR_00300236); // 8
	helpText.Append(CST_STR_00300237); // 9
	helpText.Append(CST_STR_00300412); // 10
	helpText.Append(CST_STR_00300413); // 11

	idMenuScreen_Shell_CstMenuBase::Initialize(data);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_WeaponOptions::ShowScreen
========================
*/
void idMenuScreen_Shell_Cst_GoD3_WeaponOptions::ShowScreen(const mainMenuTransition_t transitionType) {
	dataSource.LoadData();
	idMenuScreen_Shell_CstMenuBase::ShowScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_WeaponOptions::HideScreen
========================
*/
void idMenuScreen_Shell_Cst_GoD3_WeaponOptions::HideScreen(const mainMenuTransition_t transitionType) {
	if (dataSource.IsDataChanged()) {
		dataSource.CommitData();
	}
	idMenuScreen_Shell_CstMenuBase::HideScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_WeaponOptions::HandleAction
========================
*/
bool idMenuScreen_Shell_Cst_GoD3_WeaponOptions::HandleAction(idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled) {
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
idMenuScreen_Shell_Cst_GoD3_WeaponOptions::idMenuDataSource_Settings::idMenuDataSource_Settings
========================
*/
idMenuScreen_Shell_Cst_GoD3_WeaponOptions::idMenuDataSource_Settings::idMenuDataSource_Settings() {
	fields.SetNum(MAX_FIELDS);
	originalFields.SetNum(MAX_FIELDS);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_WeaponOptions::idMenuDataSource_Settings::LoadData
========================
*/
void idMenuScreen_Shell_Cst_GoD3_WeaponOptions::idMenuDataSource_Settings::LoadData() {
	fields[FIELD_DOUBLE_BARREL_SHOTGUN_IN_D3].SetBool(cst_awDoubleBarrelShotgunD3.GetBool());
	fields[FIELD_GRABBER_IN_D3].SetBool(cst_awGrabberD3.GetBool());
	fields[FIELD_CHAINSAW_IN_ROE].SetBool(cst_awChainsawRoE.GetBool());
	fields[FIELD_CHAINSAW_IN_LM].SetBool(cst_awChainsawLM.GetBool());
	fields[FIELD_SHOTGUN_SPREAD_REDUCTION].SetBool(cst_shotgunHalfSpreadSP.GetBool());
	fields[FIELD_GRENADES_TOGGLE].SetBool(cvarSystem->GetCVarBool("cst_grenadesToggle"));
	fields[FIELD_GRABBER_UNLIMITED_TIME].SetBool(cst_grabberUnlimitedTimeSP.GetBool());
	fields[FIELD_GRABBER_STABLE_THROW].SetInteger(cst_grabberStableThrowSP.GetInteger());
	fields[FIELD_GRABBER_DROP_ON_HOLD_LAYER].SetInteger(cst_grabberDropOnHoldLayer.GetInteger());
	fields[FIELD_GRABBER_DROP_ON_SWITCH_LAYER].SetInteger(cst_grabberDropOnSwitchLayer.GetInteger());
	fields[FIELD_DBS_ALWAYS_AUTO_RELOAD].SetBool(cvarSystem->GetCVarBool("cst_autoReloadAlwaysDbs"));
	fields[FIELD_INSTANTANEOUS_RELOAD_SP].SetBool(cst_instaReloadSP.GetBool());
	originalFields = fields;
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_WeaponOptions::idMenuDataSource_Settings::CommitData
========================
*/
void idMenuScreen_Shell_Cst_GoD3_WeaponOptions::idMenuDataSource_Settings::CommitData() {
	cst_awDoubleBarrelShotgunD3.SetBool(fields[FIELD_DOUBLE_BARREL_SHOTGUN_IN_D3].ToBool());
	cst_awGrabberD3.SetBool(fields[FIELD_GRABBER_IN_D3].ToBool());
	cst_awChainsawRoE.SetBool(fields[FIELD_CHAINSAW_IN_ROE].ToBool());
	cst_awChainsawLM.SetBool(fields[FIELD_CHAINSAW_IN_LM].ToBool());
	cst_shotgunHalfSpreadSP.SetBool(fields[FIELD_SHOTGUN_SPREAD_REDUCTION].ToBool());
	cvarSystem->SetCVarBool("cst_grenadesToggle", fields[FIELD_GRENADES_TOGGLE].ToBool());
	cst_grabberUnlimitedTimeSP.SetBool(fields[FIELD_GRABBER_UNLIMITED_TIME].ToBool());
	cst_grabberStableThrowSP.SetInteger(fields[FIELD_GRABBER_STABLE_THROW].ToInteger());
	cst_grabberDropOnHoldLayer.SetInteger(fields[FIELD_GRABBER_DROP_ON_HOLD_LAYER].ToInteger());
	cst_grabberDropOnSwitchLayer.SetInteger(fields[FIELD_GRABBER_DROP_ON_SWITCH_LAYER].ToInteger());
	cvarSystem->SetCVarBool("cst_autoReloadAlwaysDbs", fields[FIELD_DBS_ALWAYS_AUTO_RELOAD].ToBool());
	cst_instaReloadSP.SetBool(fields[FIELD_INSTANTANEOUS_RELOAD_SP].ToBool());

	cvarSystem->SetModifiedFlags(CVAR_ARCHIVE);
	originalFields = fields; // update the backup fields
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_WeaponOptions::idMenuDataSource_Settings::IsDataChanged
========================
*/
bool idMenuScreen_Shell_Cst_GoD3_WeaponOptions::idMenuDataSource_Settings::IsDataChanged() const {
	bool changed = false;
	for (int i = 0; i < fields.Num(); ++i) {
		if (i == FIELD_GRABBER_STABLE_THROW || i == FIELD_GRABBER_DROP_ON_HOLD_LAYER || i == FIELD_GRABBER_DROP_ON_SWITCH_LAYER) {
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
idMenuScreen_Shell_Cst_GoD3_WeaponOptions::idMenuDataSource_Settings::GetField
========================
*/
idSWFScriptVar idMenuScreen_Shell_Cst_GoD3_WeaponOptions::idMenuDataSource_Settings::GetField(const int fieldIndex) const {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return false;
	}
	if (fieldIndex == FIELD_GRABBER_STABLE_THROW) {
		switch (fields[fieldIndex].ToInteger()) {
			case 0: return CST_STR_00300164;	// Disabled
			case 1: return CST_STR_00300216;	// Player
			case 2: return CST_STR_00300217;	// World
			default: return CST_STR_00300168;	// Unknown
		}
	} else if (fieldIndex == FIELD_GRABBER_DROP_ON_HOLD_LAYER || fieldIndex == FIELD_GRABBER_DROP_ON_SWITCH_LAYER) {
		switch (fields[fieldIndex].ToInteger()) {
			case 0: return CST_STR_00300164;	// Disabled
			case 1: return CST_STR_00300218;	// Layer 1
			case 2: return CST_STR_00300219;	// Layer 2
			case 3: return CST_STR_00300220;	// Layer 3
			case 4: return CST_STR_00300221;	// Layer 4
			case 5: return CST_STR_00300222;	// Layer 5
			case 6: return CST_STR_00300223;	// Layer 6
			case 7: return CST_STR_00300224;	// Layer 7
			case 8: return CST_STR_00300225;	// Layer 8
			case 9: return CST_STR_00300226;	// Layer 9
			case -1: return CST_STR_00300227;	// Any Layer
			default: return CST_STR_00300168;	// Unknown
		}
	} else {
		return fields[fieldIndex];
	}
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_WeaponOptions::idMenuDataSource_Settings::AdjustField
========================
*/
void idMenuScreen_Shell_Cst_GoD3_WeaponOptions::idMenuDataSource_Settings::AdjustField(const int fieldIndex, const int adjustAmount) {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return;
	}
	if (fieldIndex == FIELD_GRABBER_STABLE_THROW) {
		const int numValues = 3;
		int values[numValues] = { 0, 1, 2 };
		fields[fieldIndex].SetInteger(CstMenuUtils::AdjustOption(fields[fieldIndex].ToInteger(), values, numValues, adjustAmount));
	} else if (fieldIndex == FIELD_GRABBER_DROP_ON_HOLD_LAYER || fieldIndex == FIELD_GRABBER_DROP_ON_SWITCH_LAYER) {
		const int numValues = 11;
		int values[numValues] = { -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		fields[fieldIndex].SetInteger(CstMenuUtils::AdjustOption(fields[fieldIndex].ToInteger(), values, numValues, adjustAmount));
	} else {
		fields[fieldIndex].SetBool(!fields[fieldIndex].ToBool());
	}
}
//#modified-fva; END
