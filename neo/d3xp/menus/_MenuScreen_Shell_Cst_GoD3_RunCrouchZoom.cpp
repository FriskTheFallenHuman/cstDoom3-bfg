//#modified-fva; BEGIN
#include "precompiled.h"
#pragma hdrstop
#include "../Game_local.h"
#include "../_CstStrings.h"

enum {
	FIELD_NO_STAMINA_DROP,
	FIELD_ALWAYS_RUN,
	FIELD_TOGGLE_RUN,
	FIELD_TOGGLE_CROUCH,
	FIELD_TOGGLE_ZOOM,
	FIELD_HUD_RUN,
	FIELD_HUD_CROUCH,
	FIELD_HUD_ZOOM,
	MAX_FIELDS
};

/*
========================
idMenuScreen_Shell_Cst_GoD3_RunCrouchZoom::Initialize
========================
*/
void idMenuScreen_Shell_Cst_GoD3_RunCrouchZoom::Initialize(idMenuHandler * data) {
	numVisibleOptions = 8;
	menuSpriteName = "cstMenu_GoD3_RunCrouchZoom";
	btnBackLabel = CST_STR_00300001;
	headingLabel = CST_STR_00300177;
	shellArea = CST_SHELL_AREA_GO_D3_RUN_CROUCH_ZOOM;
	goBackShellArea = CST_SHELL_AREA_GAME_OPTIONS_DOOM3;
	useSmallHeading = true;

	options = new (TAG_SWF) idMenuWidget_DynamicList();
	idMenuWidget_ControlButton * control;

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300178);
	control->SetDataSource(&dataSource, FIELD_NO_STAMINA_DROP);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

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
	control->SetLabel(CST_STR_00300181);
	control->SetDataSource(&dataSource, FIELD_TOGGLE_CROUCH);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300182);
	control->SetDataSource(&dataSource, FIELD_TOGGLE_ZOOM);
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

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300184);
	control->SetDataSource(&dataSource, FIELD_HUD_CROUCH);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300185);
	control->SetDataSource(&dataSource, FIELD_HUD_ZOOM);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	helpText.Clear();
	helpText.Append(CST_STR_00300186); // 0
	helpText.Append(CST_STR_00300187); // 1
	helpText.Append(CST_STR_00300188); // 2
	helpText.Append(CST_STR_00300189); // 3
	helpText.Append(CST_STR_00300190); // 4
	helpText.Append(CST_STR_00300191); // 5
	helpText.Append(CST_STR_00300192); // 6
	helpText.Append(CST_STR_00300193); // 7

	idMenuScreen_Shell_CstMenuBase::Initialize(data);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_RunCrouchZoom::ShowScreen
========================
*/
void idMenuScreen_Shell_Cst_GoD3_RunCrouchZoom::ShowScreen(const mainMenuTransition_t transitionType) {
	dataSource.LoadData();
	idMenuScreen_Shell_CstMenuBase::ShowScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_RunCrouchZoom::HideScreen
========================
*/
void idMenuScreen_Shell_Cst_GoD3_RunCrouchZoom::HideScreen(const mainMenuTransition_t transitionType) {
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
bool idMenuScreen_Shell_Cst_GoD3_RunCrouchZoom::HandleAction(idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled) {
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
idMenuScreen_Shell_Cst_GoD3_RunCrouchZoom::idMenuDataSource_Settings::idMenuDataSource_Settings
========================
*/
idMenuScreen_Shell_Cst_GoD3_RunCrouchZoom::idMenuDataSource_Settings::idMenuDataSource_Settings() {
	fields.SetNum(MAX_FIELDS);
	originalFields.SetNum(MAX_FIELDS);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_RunCrouchZoom::idMenuDataSource_Settings::LoadData
========================
*/
void idMenuScreen_Shell_Cst_GoD3_RunCrouchZoom::idMenuDataSource_Settings::LoadData() {
	fields[FIELD_NO_STAMINA_DROP].SetBool(!cst_noStaminaDrop.GetBool());
	fields[FIELD_ALWAYS_RUN].SetBool(cvarSystem->GetCVarBool("in_alwaysRun"));
	fields[FIELD_TOGGLE_RUN].SetBool(cvarSystem->GetCVarBool("in_toggleRun"));
	fields[FIELD_TOGGLE_CROUCH].SetBool(cvarSystem->GetCVarBool("in_toggleCrouch"));
	fields[FIELD_TOGGLE_ZOOM].SetBool(cvarSystem->GetCVarInteger("in_toggleZoom"));
	fields[FIELD_HUD_RUN].SetBool(cst_hudRun.GetBool());
	fields[FIELD_HUD_CROUCH].SetBool(cst_hudCrouch.GetBool());
	fields[FIELD_HUD_ZOOM].SetBool(cst_hudZoom.GetBool());
	originalFields = fields;
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_RunCrouchZoom::idMenuDataSource_Settings::CommitData
========================
*/
void idMenuScreen_Shell_Cst_GoD3_RunCrouchZoom::idMenuDataSource_Settings::CommitData() {
	cst_noStaminaDrop.SetBool(!fields[FIELD_NO_STAMINA_DROP].ToBool());
	cvarSystem->SetCVarBool("in_alwaysRun", fields[FIELD_ALWAYS_RUN].ToBool());
	cvarSystem->SetCVarBool("in_toggleRun", fields[FIELD_TOGGLE_RUN].ToBool());
	cvarSystem->SetCVarBool("in_toggleCrouch", fields[FIELD_TOGGLE_CROUCH].ToBool());
	cvarSystem->SetCVarBool("in_toggleZoom", fields[FIELD_TOGGLE_ZOOM].ToBool());
	cst_hudRun.SetBool(fields[FIELD_HUD_RUN].ToBool());
	cst_hudCrouch.SetBool(fields[FIELD_HUD_CROUCH].ToBool());
	cst_hudZoom.SetBool(fields[FIELD_HUD_ZOOM].ToBool());

	cvarSystem->SetModifiedFlags(CVAR_ARCHIVE);
	originalFields = fields; // update the backup fields
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_RunCrouchZoom::idMenuDataSource_Settings::IsDataChanged
========================
*/
bool idMenuScreen_Shell_Cst_GoD3_RunCrouchZoom::idMenuDataSource_Settings::IsDataChanged() const {
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
idMenuScreen_Shell_Cst_GoD3_RunCrouchZoom::idMenuDataSource_Settings::GetField
========================
*/
idSWFScriptVar idMenuScreen_Shell_Cst_GoD3_RunCrouchZoom::idMenuDataSource_Settings::GetField(const int fieldIndex) const {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return false;
	}
	return fields[fieldIndex];
}


/*
========================
idMenuScreen_Shell_Cst_GoD3_RunCrouchZoom::idMenuDataSource_Settings::AdjustField
========================
*/
void idMenuScreen_Shell_Cst_GoD3_RunCrouchZoom::idMenuDataSource_Settings::AdjustField(const int fieldIndex, const int adjustAmount) {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return;
	}
	fields[fieldIndex].SetBool(!fields[fieldIndex].ToBool());
}
//#modified-fva; END
