//#modified-fva; BEGIN
#include "precompiled.h"
#pragma hdrstop
#include "../Game_local.h"
#include "../_CstStrings.h"

enum {
	FIELD_PISTOL,
	FIELD_SHOTGUN,
	FIELD_DOUBLE_BARREL_SHOTGUN,
	FIELD_MACHINEGUN,
	FIELD_CHAINGUN,
	FIELD_PLASMAGUN,
	FIELD_BFG,
	FIELD_CHAINSAW,
	MAX_FIELDS
};

/*
========================
idMenuScreen_Shell_Cst_GoD3_WeaponSmokeOthersMP::Initialize
========================
*/
void idMenuScreen_Shell_Cst_GoD3_WeaponSmokeOthersMP::Initialize(idMenuHandler * data) {
	numVisibleOptions = 8;
	menuSpriteName = "cstMenu_GoD3_WeaponSmokeOthersMP";
	btnBackLabel = CST_STR_00300001;
	headingLabel = CST_STR_00300252;
	shellArea = CST_SHELL_AREA_GO_D3_WEAPON_SMOKE_OTHERS_MP;
	goBackShellArea = CST_SHELL_AREA_GAME_OPTIONS_DOOM3;
	useSmallHeading = true;

	options = new (TAG_SWF) idMenuWidget_DynamicList();
	idMenuWidget_ControlButton * control;

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300242);
	control->SetDataSource(&dataSource, FIELD_PISTOL);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300243);
	control->SetDataSource(&dataSource, FIELD_SHOTGUN);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300244);
	control->SetDataSource(&dataSource, FIELD_DOUBLE_BARREL_SHOTGUN);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300245);
	control->SetDataSource(&dataSource, FIELD_MACHINEGUN);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300246);
	control->SetDataSource(&dataSource, FIELD_CHAINGUN);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300247);
	control->SetDataSource(&dataSource, FIELD_PLASMAGUN);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300248);
	control->SetDataSource(&dataSource, FIELD_BFG);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300249);
	control->SetDataSource(&dataSource, FIELD_CHAINSAW);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	idMenuScreen_Shell_CstMenuBase::Initialize(data);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_WeaponSmokeOthersMP::ShowScreen
========================
*/
void idMenuScreen_Shell_Cst_GoD3_WeaponSmokeOthersMP::ShowScreen(const mainMenuTransition_t transitionType) {
	dataSource.LoadData();
	idMenuScreen_Shell_CstMenuBase::ShowScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_WeaponSmokeOthersMP::HideScreen
========================
*/
void idMenuScreen_Shell_Cst_GoD3_WeaponSmokeOthersMP::HideScreen(const mainMenuTransition_t transitionType) {
	if (dataSource.IsDataChanged()) {
		dataSource.CommitData();
	}
	idMenuScreen_Shell_CstMenuBase::HideScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_WeaponSmokeOthersMP::HandleAction
========================
*/
bool idMenuScreen_Shell_Cst_GoD3_WeaponSmokeOthersMP::HandleAction(idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled) {
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
idMenuScreen_Shell_Cst_GoD3_WeaponSmokeOthersMP::idMenuDataSource_Settings::idMenuDataSource_Settings
========================
*/
idMenuScreen_Shell_Cst_GoD3_WeaponSmokeOthersMP::idMenuDataSource_Settings::idMenuDataSource_Settings() {
	fields.SetNum(MAX_FIELDS);
	originalFields.SetNum(MAX_FIELDS);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_WeaponSmokeOthersMP::idMenuDataSource_Settings::LoadData
========================
*/
void idMenuScreen_Shell_Cst_GoD3_WeaponSmokeOthersMP::idMenuDataSource_Settings::LoadData() {
	idStr noSmokeMuzzleOthersMP = cst_noSmokeMuzzleOthersMP.GetString();
	fields[FIELD_PISTOL].SetBool(!CstMenuUtils::ReadPseudoBit(noSmokeMuzzleOthersMP, 2));
	fields[FIELD_SHOTGUN].SetBool(!CstMenuUtils::ReadPseudoBit(noSmokeMuzzleOthersMP, 3));
	fields[FIELD_DOUBLE_BARREL_SHOTGUN].SetBool(!CstMenuUtils::ReadPseudoBit(noSmokeMuzzleOthersMP, 4));
	fields[FIELD_MACHINEGUN].SetBool(!CstMenuUtils::ReadPseudoBit(noSmokeMuzzleOthersMP, 5));
	fields[FIELD_CHAINGUN].SetBool(!CstMenuUtils::ReadPseudoBit(noSmokeMuzzleOthersMP, 6));
	fields[FIELD_PLASMAGUN].SetBool(!CstMenuUtils::ReadPseudoBit(noSmokeMuzzleOthersMP, 8));
	fields[FIELD_BFG].SetBool(!CstMenuUtils::ReadPseudoBit(noSmokeMuzzleOthersMP, 10));
	fields[FIELD_CHAINSAW].SetBool(!CstMenuUtils::ReadPseudoBit(noSmokeMuzzleOthersMP, 11));
	originalFields = fields;
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_WeaponSmokeOthersMP::idMenuDataSource_Settings::CommitData
========================
*/
void idMenuScreen_Shell_Cst_GoD3_WeaponSmokeOthersMP::idMenuDataSource_Settings::CommitData() {
	idStr noSmokeMuzzleOthersMP;
	CstMenuUtils::WritePseudoBit(noSmokeMuzzleOthersMP, 2, !fields[FIELD_PISTOL].ToBool());
	CstMenuUtils::WritePseudoBit(noSmokeMuzzleOthersMP, 3, !fields[FIELD_SHOTGUN].ToBool());
	CstMenuUtils::WritePseudoBit(noSmokeMuzzleOthersMP, 4, !fields[FIELD_DOUBLE_BARREL_SHOTGUN].ToBool());
	CstMenuUtils::WritePseudoBit(noSmokeMuzzleOthersMP, 5, !fields[FIELD_MACHINEGUN].ToBool());
	CstMenuUtils::WritePseudoBit(noSmokeMuzzleOthersMP, 6, !fields[FIELD_CHAINGUN].ToBool());
	CstMenuUtils::WritePseudoBit(noSmokeMuzzleOthersMP, 8, !fields[FIELD_PLASMAGUN].ToBool());
	CstMenuUtils::WritePseudoBit(noSmokeMuzzleOthersMP, 10, !fields[FIELD_BFG].ToBool());
	CstMenuUtils::WritePseudoBit(noSmokeMuzzleOthersMP, 11, !fields[FIELD_CHAINSAW].ToBool());
	cst_noSmokeMuzzleOthersMP.SetString(noSmokeMuzzleOthersMP.c_str());

	cvarSystem->SetModifiedFlags(CVAR_ARCHIVE);
	originalFields = fields; // update the backup fields
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_WeaponSmokeOthersMP::idMenuDataSource_Settings::IsDataChanged
========================
*/
bool idMenuScreen_Shell_Cst_GoD3_WeaponSmokeOthersMP::idMenuDataSource_Settings::IsDataChanged() const {
	bool changed = false;
	for (int i = 0; i < fields.Num(); ++i) {
		if (fields[i].ToBool() != originalFields[i].ToBool()) {
			changed = true;
			break;
		}
	}
	return changed;
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_WeaponSmokeOthersMP::idMenuDataSource_Settings::GetField
========================
*/
idSWFScriptVar idMenuScreen_Shell_Cst_GoD3_WeaponSmokeOthersMP::idMenuDataSource_Settings::GetField(const int fieldIndex) const {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return false;
	}
	return fields[fieldIndex];
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_WeaponSmokeOthersMP::idMenuDataSource_Settings::AdjustField
========================
*/
void idMenuScreen_Shell_Cst_GoD3_WeaponSmokeOthersMP::idMenuDataSource_Settings::AdjustField(const int fieldIndex, const int adjustAmount) {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return;
	}
	fields[fieldIndex].SetBool(!fields[fieldIndex].ToBool());
}
//#modified-fva; END
