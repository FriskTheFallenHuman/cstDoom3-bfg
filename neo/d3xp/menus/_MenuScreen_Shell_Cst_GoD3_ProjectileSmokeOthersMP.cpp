//#modified-fva; BEGIN
#include "precompiled.h"
#pragma hdrstop
#include "../Game_local.h"
#include "../_CstStrings.h"

enum {
	FIELD_GRENADES,
	FIELD_PLASMAGUN,
	FIELD_ROCKETS,
	MAX_FIELDS
};

/*
========================
idMenuScreen_Shell_Cst_GoD3_ProjectileSmokeOthersMP::Initialize
========================
*/
void idMenuScreen_Shell_Cst_GoD3_ProjectileSmokeOthersMP::Initialize(idMenuHandler * data) {
	numVisibleOptions = 8;
	menuSpriteName = "cstMenu_GoD3_ProjectileSmokeOthersMP";
	btnBackLabel = CST_STR_00300001;
	headingLabel = CST_STR_00300267;
	shellArea = CST_SHELL_AREA_GO_D3_PROJECTILE_SMOKE_OTHERS_MP;
	goBackShellArea = CST_SHELL_AREA_GAME_OPTIONS_DOOM3;
	useSmallHeading = true;

	options = new (TAG_SWF) idMenuWidget_DynamicList();
	idMenuWidget_ControlButton * control;

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300258);
	control->SetDataSource(&dataSource, FIELD_GRENADES);
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
	control->SetLabel(CST_STR_00300259);
	control->SetDataSource(&dataSource, FIELD_ROCKETS);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	idMenuScreen_Shell_CstMenuBase::Initialize(data);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_ProjectileSmokeOthersMP::ShowScreen
========================
*/
void idMenuScreen_Shell_Cst_GoD3_ProjectileSmokeOthersMP::ShowScreen(const mainMenuTransition_t transitionType) {
	dataSource.LoadData();
	idMenuScreen_Shell_CstMenuBase::ShowScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_ProjectileSmokeOthersMP::HideScreen
========================
*/
void idMenuScreen_Shell_Cst_GoD3_ProjectileSmokeOthersMP::HideScreen(const mainMenuTransition_t transitionType) {
	if (dataSource.IsDataChanged()) {
		dataSource.CommitData();
	}
	idMenuScreen_Shell_CstMenuBase::HideScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_ProjectileSmokeOthersMP::HandleAction
========================
*/
bool idMenuScreen_Shell_Cst_GoD3_ProjectileSmokeOthersMP::HandleAction(idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled) {
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
idMenuScreen_Shell_Cst_GoD3_ProjectileSmokeOthersMP::idMenuDataSource_Settings::idMenuDataSource_Settings
========================
*/
idMenuScreen_Shell_Cst_GoD3_ProjectileSmokeOthersMP::idMenuDataSource_Settings::idMenuDataSource_Settings() {
	fields.SetNum(MAX_FIELDS);
	originalFields.SetNum(MAX_FIELDS);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_ProjectileSmokeOthersMP::idMenuDataSource_Settings::LoadData
========================
*/
void idMenuScreen_Shell_Cst_GoD3_ProjectileSmokeOthersMP::idMenuDataSource_Settings::LoadData() {
	idStr noSmokeFlyOthersMP = cst_noSmokeFlyOthersMP.GetString();
	fields[FIELD_GRENADES].SetBool(!CstMenuUtils::ReadPseudoBit(noSmokeFlyOthersMP, 7));
	fields[FIELD_PLASMAGUN].SetBool(!CstMenuUtils::ReadPseudoBit(noSmokeFlyOthersMP, 8));
	fields[FIELD_ROCKETS].SetBool(!CstMenuUtils::ReadPseudoBit(noSmokeFlyOthersMP, 9));
	originalFields = fields;
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_ProjectileSmokeOthersMP::idMenuDataSource_Settings::CommitData
========================
*/
void idMenuScreen_Shell_Cst_GoD3_ProjectileSmokeOthersMP::idMenuDataSource_Settings::CommitData() {
	idStr noSmokeFlyOthersMP;
	CstMenuUtils::WritePseudoBit(noSmokeFlyOthersMP, 7, !fields[FIELD_GRENADES].ToBool());
	CstMenuUtils::WritePseudoBit(noSmokeFlyOthersMP, 8, !fields[FIELD_PLASMAGUN].ToBool());
	CstMenuUtils::WritePseudoBit(noSmokeFlyOthersMP, 9, !fields[FIELD_ROCKETS].ToBool());
	cst_noSmokeFlyOthersMP.SetString(noSmokeFlyOthersMP.c_str());

	cvarSystem->SetModifiedFlags(CVAR_ARCHIVE);
	originalFields = fields; // update the backup fields
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_ProjectileSmokeOthersMP::idMenuDataSource_Settings::IsDataChanged
========================
*/
bool idMenuScreen_Shell_Cst_GoD3_ProjectileSmokeOthersMP::idMenuDataSource_Settings::IsDataChanged() const {
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
idMenuScreen_Shell_Cst_GoD3_ProjectileSmokeOthersMP::idMenuDataSource_Settings::GetField
========================
*/
idSWFScriptVar idMenuScreen_Shell_Cst_GoD3_ProjectileSmokeOthersMP::idMenuDataSource_Settings::GetField(const int fieldIndex) const {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return false;
	}
	return fields[fieldIndex];
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_ProjectileSmokeOthersMP::idMenuDataSource_Settings::AdjustField
========================
*/
void idMenuScreen_Shell_Cst_GoD3_ProjectileSmokeOthersMP::idMenuDataSource_Settings::AdjustField(const int fieldIndex, const int adjustAmount) {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return;
	}
	fields[fieldIndex].SetBool(!fields[fieldIndex].ToBool());
}
//#modified-fva; END
