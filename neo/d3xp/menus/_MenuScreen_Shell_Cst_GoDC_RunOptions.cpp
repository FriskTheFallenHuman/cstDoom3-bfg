//#modified-fva; BEGIN
#include "precompiled.h"
#pragma hdrstop
#include "../Game_local.h"
#include "../_CstStrings.h"

enum {
	FIELD_ALWAYS_RUN,
	FIELD_TOGGLE_RUN,
	FIELD_HUD_RUN,
	MAX_FIELDS
};

/*
========================
idMenuScreen_Shell_Cst_GoDC_RunOptions::Initialize
========================
*/
void idMenuScreen_Shell_Cst_GoDC_RunOptions::Initialize(idMenuHandler * data) {
	numVisibleOptions = 8;
	menuSpriteName = "cstMenu_GoDC_RunOptions";
	btnBackLabel = CST_STR_00300001;
	headingLabel = CST_STR_00300316;
	shellArea = CST_SHELL_AREA_GO_DC_RUN_OPTIONS;
	goBackShellArea = CST_SHELL_AREA_GAME_OPTIONS_DOOM_CLASSIC;
	useSmallHeading = true;

	options = new (TAG_SWF) idMenuWidget_DynamicList();
	idMenuWidget_ControlButton * control;

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300179);
	control->SetDataSource(&dataSource, FIELD_ALWAYS_RUN);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300180);
	control->SetDataSource(&dataSource, FIELD_TOGGLE_RUN);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300183);
	control->SetDataSource(&dataSource, FIELD_HUD_RUN);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	helpText.Clear();
	helpText.Append(CST_STR_00300317); // 0
	helpText.Append(CST_STR_00300318); // 1
	helpText.Append(CST_STR_00300319); // 2

	idMenuScreen_Shell_CstMenuBase::Initialize(data);
}

/*
========================
idMenuScreen_Shell_Cst_GoDC_RunOptions::ShowScreen
========================
*/
void idMenuScreen_Shell_Cst_GoDC_RunOptions::ShowScreen(const mainMenuTransition_t transitionType) {
	dataSource.LoadData();
	idMenuScreen_Shell_CstMenuBase::ShowScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_Cst_GoDC_RunOptions::HideScreen
========================
*/
void idMenuScreen_Shell_Cst_GoDC_RunOptions::HideScreen(const mainMenuTransition_t transitionType) {
	if (dataSource.IsDataChanged()) {
		dataSource.CommitData();
	}
	idMenuScreen_Shell_CstMenuBase::HideScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_Cst_GoDC_RunOptions::HandleAction
========================
*/
bool idMenuScreen_Shell_Cst_GoDC_RunOptions::HandleAction(idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled) {
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
idMenuScreen_Shell_Cst_GoDC_RunOptions::idMenuDataSource_Settings::idMenuDataSource_Settings
========================
*/
idMenuScreen_Shell_Cst_GoDC_RunOptions::idMenuDataSource_Settings::idMenuDataSource_Settings() {
	fields.SetNum(MAX_FIELDS);
	originalFields.SetNum(MAX_FIELDS);
}

/*
========================
idMenuScreen_Shell_Cst_GoDC_RunOptions::idMenuDataSource_Settings::LoadData
========================
*/
void idMenuScreen_Shell_Cst_GoDC_RunOptions::idMenuDataSource_Settings::LoadData() {
	fields[FIELD_ALWAYS_RUN].SetBool(cvarSystem->GetCVarBool("cst_dcAlwaysRun"));
	fields[FIELD_TOGGLE_RUN].SetBool(cvarSystem->GetCVarBool("cst_dcToggleRun"));
	fields[FIELD_HUD_RUN].SetBool(cvarSystem->GetCVarBool("cst_dcHudRun"));
	originalFields = fields;
}

/*
========================
idMenuScreen_Shell_Cst_GoDC_RunOptions::idMenuDataSource_Settings::CommitData
========================
*/
void idMenuScreen_Shell_Cst_GoDC_RunOptions::idMenuDataSource_Settings::CommitData() {
	cvarSystem->SetCVarBool("cst_dcAlwaysRun", fields[FIELD_ALWAYS_RUN].ToBool());
	cvarSystem->SetCVarBool("cst_dcToggleRun", fields[FIELD_TOGGLE_RUN].ToBool());
	cvarSystem->SetCVarBool("cst_dcHudRun", fields[FIELD_HUD_RUN].ToBool());

	cvarSystem->SetModifiedFlags(CVAR_ARCHIVE);
	originalFields = fields; // update the backup fields
}

/*
========================
idMenuScreen_Shell_Cst_GoDC_RunOptions::idMenuDataSource_Settings::IsDataChanged
========================
*/
bool idMenuScreen_Shell_Cst_GoDC_RunOptions::idMenuDataSource_Settings::IsDataChanged() const {
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
idMenuScreen_Shell_Cst_GoDC_RunOptions::idMenuDataSource_Settings::GetField
========================
*/
idSWFScriptVar idMenuScreen_Shell_Cst_GoDC_RunOptions::idMenuDataSource_Settings::GetField(const int fieldIndex) const {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return false;
	}
	return fields[fieldIndex];
}

/*
========================
idMenuScreen_Shell_Cst_GoDC_RunOptions::idMenuDataSource_Settings::AdjustField
========================
*/
void idMenuScreen_Shell_Cst_GoDC_RunOptions::idMenuDataSource_Settings::AdjustField(const int fieldIndex, const int adjustAmount) {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return;
	}
	fields[fieldIndex].SetBool(!fields[fieldIndex].ToBool());
}
//#modified-fva; END
