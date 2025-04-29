//#modified-fva; BEGIN
#include "precompiled.h"
#pragma hdrstop
#include "../Game_local.h"
#include "../_CstStrings.h"

enum {
	FIELD_GRABBER,
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
idMenuScreen_Shell_Cst_GoD3_WeaponSmokeSelf::Initialize
========================
*/
void idMenuScreen_Shell_Cst_GoD3_WeaponSmokeSelf::Initialize(idMenuHandler * data) {
	numVisibleOptions = 14;
	menuSpriteName = "cstMenu_GoD3_WeaponSmokeSelf";
	btnBackLabel = CST_STR_00300001;
	headingLabel = CST_STR_00300240;
	shellArea = CST_SHELL_AREA_GO_D3_WEAPON_SMOKE_SELF;
	goBackShellArea = CST_SHELL_AREA_GAME_OPTIONS_DOOM3;
	useSmallHeading = true;

	options = new (TAG_SWF) idMenuWidget_DynamicList();
	idMenuWidget_ControlButton * control;

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300241);
	control->SetDataSource(&dataSource, FIELD_GRABBER);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

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

	helpText.Clear();
	helpText.Append(CST_STR_00300264); // 0

	idMenuScreen_Shell_CstMenuBase::Initialize(data);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_WeaponSmokeSelf::ShowScreen
========================
*/
void idMenuScreen_Shell_Cst_GoD3_WeaponSmokeSelf::ShowScreen(const mainMenuTransition_t transitionType) {
	dataSource.LoadData();
	idMenuScreen_Shell_CstMenuBase::ShowScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_WeaponSmokeSelf::HideScreen
========================
*/
void idMenuScreen_Shell_Cst_GoD3_WeaponSmokeSelf::HideScreen(const mainMenuTransition_t transitionType) {
	if (dataSource.IsDataChanged()) {
		dataSource.CommitData();
	}
	idMenuScreen_Shell_CstMenuBase::HideScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_WeaponSmokeSelf::HandleAction
========================
*/
bool idMenuScreen_Shell_Cst_GoD3_WeaponSmokeSelf::HandleAction(idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled) {
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
idMenuScreen_Shell_Cst_GoD3_WeaponSmokeSelf::idMenuDataSource_Settings::idMenuDataSource_Settings
========================
*/
idMenuScreen_Shell_Cst_GoD3_WeaponSmokeSelf::idMenuDataSource_Settings::idMenuDataSource_Settings() {
	fields.SetNum(MAX_FIELDS);
	originalFields.SetNum(MAX_FIELDS);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_WeaponSmokeSelf::idMenuDataSource_Settings::LoadData
========================
*/
void idMenuScreen_Shell_Cst_GoD3_WeaponSmokeSelf::idMenuDataSource_Settings::LoadData() {
	idStr noSmokeMuzzle = cst_noSmokeMuzzle.GetString();
	fields[FIELD_GRABBER].SetBool(!CstMenuUtils::ReadPseudoBit(noSmokeMuzzle, 1));
	fields[FIELD_PISTOL].SetBool(!CstMenuUtils::ReadPseudoBit(noSmokeMuzzle, 2));
	fields[FIELD_SHOTGUN].SetBool(!CstMenuUtils::ReadPseudoBit(noSmokeMuzzle, 3));
	fields[FIELD_DOUBLE_BARREL_SHOTGUN].SetBool(!CstMenuUtils::ReadPseudoBit(noSmokeMuzzle, 4));
	fields[FIELD_MACHINEGUN].SetBool(!CstMenuUtils::ReadPseudoBit(noSmokeMuzzle, 5));
	fields[FIELD_CHAINGUN].SetBool(!CstMenuUtils::ReadPseudoBit(noSmokeMuzzle, 6));
	fields[FIELD_PLASMAGUN].SetBool(!CstMenuUtils::ReadPseudoBit(noSmokeMuzzle, 8));
	fields[FIELD_BFG].SetBool(!CstMenuUtils::ReadPseudoBit(noSmokeMuzzle, 10));
	fields[FIELD_CHAINSAW].SetBool(!CstMenuUtils::ReadPseudoBit(noSmokeMuzzle, 11));
	originalFields = fields;
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_WeaponSmokeSelf::idMenuDataSource_Settings::CommitData
========================
*/
void idMenuScreen_Shell_Cst_GoD3_WeaponSmokeSelf::idMenuDataSource_Settings::CommitData() {
	idStr noSmokeMuzzle;
	CstMenuUtils::WritePseudoBit(noSmokeMuzzle, 1, !fields[FIELD_GRABBER].ToBool());
	CstMenuUtils::WritePseudoBit(noSmokeMuzzle, 2, !fields[FIELD_PISTOL].ToBool());
	CstMenuUtils::WritePseudoBit(noSmokeMuzzle, 3, !fields[FIELD_SHOTGUN].ToBool());
	CstMenuUtils::WritePseudoBit(noSmokeMuzzle, 4, !fields[FIELD_DOUBLE_BARREL_SHOTGUN].ToBool());
	CstMenuUtils::WritePseudoBit(noSmokeMuzzle, 5, !fields[FIELD_MACHINEGUN].ToBool());
	CstMenuUtils::WritePseudoBit(noSmokeMuzzle, 6, !fields[FIELD_CHAINGUN].ToBool());
	CstMenuUtils::WritePseudoBit(noSmokeMuzzle, 8, !fields[FIELD_PLASMAGUN].ToBool());
	CstMenuUtils::WritePseudoBit(noSmokeMuzzle, 10, !fields[FIELD_BFG].ToBool());
	CstMenuUtils::WritePseudoBit(noSmokeMuzzle, 11, !fields[FIELD_CHAINSAW].ToBool());
	cst_noSmokeMuzzle.SetString(noSmokeMuzzle.c_str());

	cvarSystem->SetModifiedFlags(CVAR_ARCHIVE);
	originalFields = fields; // update the backup fields
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_WeaponSmokeSelf::idMenuDataSource_Settings::IsDataChanged
========================
*/
bool idMenuScreen_Shell_Cst_GoD3_WeaponSmokeSelf::idMenuDataSource_Settings::IsDataChanged() const {
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
idMenuScreen_Shell_Cst_GoD3_WeaponSmokeSelf::idMenuDataSource_Settings::GetField
========================
*/
idSWFScriptVar idMenuScreen_Shell_Cst_GoD3_WeaponSmokeSelf::idMenuDataSource_Settings::GetField(const int fieldIndex) const {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return false;
	}
	return fields[fieldIndex];
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_WeaponSmokeSelf::idMenuDataSource_Settings::AdjustField
========================
*/
void idMenuScreen_Shell_Cst_GoD3_WeaponSmokeSelf::idMenuDataSource_Settings::AdjustField(const int fieldIndex, const int adjustAmount) {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return;
	}
	fields[fieldIndex].SetBool(!fields[fieldIndex].ToBool());
}
//#modified-fva; END
