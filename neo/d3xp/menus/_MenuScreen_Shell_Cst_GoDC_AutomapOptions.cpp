//#modified-fva; BEGIN
#include "precompiled.h"
#pragma hdrstop
#include "../Game_local.h"
#include "../_CstStrings.h"

enum {
	FIELD_FOLLOW_ON_START,
	FIELD_ZOOM_STEP,
	FIELD_ZOOM_SPEED,
	FIELD_PAN_SPEED,
	MAX_FIELDS
};

/*
========================
idMenuScreen_Shell_Cst_GoDC_AutomapOptions::Initialize
========================
*/
void idMenuScreen_Shell_Cst_GoDC_AutomapOptions::Initialize(idMenuHandler * data) {
	numVisibleOptions = 8;
	menuSpriteName = "cstMenu_GoDC_AutomapOptions";
	btnBackLabel = CST_STR_00300001;
	headingLabel = CST_STR_00300306;
	shellArea = CST_SHELL_AREA_GO_DC_AUTOMAP_OPTIONS;
	goBackShellArea = CST_SHELL_AREA_GAME_OPTIONS_DOOM_CLASSIC;
	useSmallHeading = true;

	options = new (TAG_SWF) idMenuWidget_DynamicList();
	idMenuWidget_ControlButton * control;

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300307);
	control->SetDataSource(&dataSource, FIELD_FOLLOW_ON_START);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_BAR);
	control->SetLabel(CST_STR_00300308);
	control->SetDataSource(&dataSource, FIELD_ZOOM_STEP);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_BAR);
	control->SetLabel(CST_STR_00300309);
	control->SetDataSource(&dataSource, FIELD_ZOOM_SPEED);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_BAR);
	control->SetLabel(CST_STR_00300310);
	control->SetDataSource(&dataSource, FIELD_PAN_SPEED);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	helpText.Clear();
	helpText.Append(CST_STR_00300311); // 0
	helpText.Append(CST_STR_00300312); // 1
	helpText.Append(CST_STR_00300313); // 2
	helpText.Append(CST_STR_00300314); // 3

	idMenuScreen_Shell_CstMenuBase::Initialize(data);
}

/*
========================
idMenuScreen_Shell_Cst_GoDC_AutomapOptions::ShowScreen
========================
*/
void idMenuScreen_Shell_Cst_GoDC_AutomapOptions::ShowScreen(const mainMenuTransition_t transitionType) {
	dataSource.LoadData();
	idMenuScreen_Shell_CstMenuBase::ShowScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_Cst_GoDC_AutomapOptions::HideScreen
========================
*/
void idMenuScreen_Shell_Cst_GoDC_AutomapOptions::HideScreen(const mainMenuTransition_t transitionType) {
	if (dataSource.IsDataChanged()) {
		dataSource.CommitData();
	}
	idMenuScreen_Shell_CstMenuBase::HideScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_Cst_GoDC_AutomapOptions::HandleAction
========================
*/
bool idMenuScreen_Shell_Cst_GoDC_AutomapOptions::HandleAction(idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled) {
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
idMenuScreen_Shell_Cst_GoDC_AutomapOptions::idMenuDataSource_Settings::idMenuDataSource_Settings
========================
*/
idMenuScreen_Shell_Cst_GoDC_AutomapOptions::idMenuDataSource_Settings::idMenuDataSource_Settings() {
	fields.SetNum(MAX_FIELDS);
	originalFields.SetNum(MAX_FIELDS);
}

/*
========================
idMenuScreen_Shell_Cst_GoDC_AutomapOptions::idMenuDataSource_Settings::LoadData
========================
*/
void idMenuScreen_Shell_Cst_GoDC_AutomapOptions::idMenuDataSource_Settings::LoadData() {
	fields[FIELD_FOLLOW_ON_START].SetBool(cvarSystem->GetCVarBool("cst_amFollowOnStart"));
	fields[FIELD_ZOOM_STEP].SetFloat(cvarSystem->GetCVarFloat("cst_amZoomStep"));
	fields[FIELD_ZOOM_SPEED].SetFloat(cvarSystem->GetCVarFloat("cst_amZoomSpeed"));
	fields[FIELD_PAN_SPEED].SetFloat(cvarSystem->GetCVarFloat("cst_amPanSpeed"));

	originalFields = fields;
}

/*
========================
idMenuScreen_Shell_Cst_GoDC_AutomapOptions::idMenuDataSource_Settings::CommitData
========================
*/
void idMenuScreen_Shell_Cst_GoDC_AutomapOptions::idMenuDataSource_Settings::CommitData() {
	cvarSystem->SetCVarBool("cst_amFollowOnStart", fields[FIELD_FOLLOW_ON_START].ToBool());

	if (fields[FIELD_ZOOM_STEP].ToFloat() != originalFields[FIELD_ZOOM_STEP].ToFloat()) {
		cvarSystem->SetCVarFloat("cst_amZoomStep", fields[FIELD_ZOOM_STEP].ToFloat());
	}
	if (fields[FIELD_ZOOM_SPEED].ToFloat() != originalFields[FIELD_ZOOM_SPEED].ToFloat()) {
		cvarSystem->SetCVarFloat("cst_amZoomSpeed", fields[FIELD_ZOOM_SPEED].ToFloat());
	}
	if (fields[FIELD_PAN_SPEED].ToFloat() != originalFields[FIELD_PAN_SPEED].ToFloat()) {
		cvarSystem->SetCVarFloat("cst_amPanSpeed", fields[FIELD_PAN_SPEED].ToFloat());
	}

	cvarSystem->SetModifiedFlags(CVAR_ARCHIVE);
	originalFields = fields; // update the backup fields
}

/*
========================
idMenuScreen_Shell_Cst_GoDC_AutomapOptions::idMenuDataSource_Settings::IsDataChanged
========================
*/
bool idMenuScreen_Shell_Cst_GoDC_AutomapOptions::idMenuDataSource_Settings::IsDataChanged() const {
	bool changed = false;
	for (int i = 0; i < fields.Num(); ++i) {
		if (i == FIELD_FOLLOW_ON_START) {
			if (fields[i].ToBool() != originalFields[i].ToBool()) {
				changed = true;
				break;
			}
		} else {
			if (fields[i].ToFloat() != originalFields[i].ToFloat()) {
				changed = true;
				break;
			}
		}
	}
	return changed;
}

/*
========================
idMenuScreen_Shell_Cst_GoDC_AutomapOptions::idMenuDataSource_Settings::GetField
========================
*/
idSWFScriptVar idMenuScreen_Shell_Cst_GoDC_AutomapOptions::idMenuDataSource_Settings::GetField(const int fieldIndex) const {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return false;
	}
	switch (fieldIndex) {
		case FIELD_FOLLOW_ON_START:
			return fields[fieldIndex];
		case FIELD_ZOOM_STEP:
			return idMath::ClampFloat(0.0f, 100.0f, CstMenuUtils::LinearAdjust(fields[fieldIndex].ToFloat(), 1.1f, 1.5f, 0.0f, 100.0f));
		case FIELD_ZOOM_SPEED:
			return idMath::ClampFloat(0.0f, 100.0f, CstMenuUtils::LinearAdjust(fields[fieldIndex].ToFloat(), 1.02f, 1.1f, 0.0f, 100.0f));
		case FIELD_PAN_SPEED:
			return idMath::ClampFloat(0.0f, 100.0f, CstMenuUtils::LinearAdjust(fields[fieldIndex].ToFloat(), 2.0f, 4.0f, 0.0f, 100.0f));
	}
	return false;
}

/*
========================
idMenuScreen_Shell_Cst_GoDC_AutomapOptions::idMenuDataSource_Settings::AdjustField
========================
*/
void idMenuScreen_Shell_Cst_GoDC_AutomapOptions::idMenuDataSource_Settings::AdjustField(const int fieldIndex, const int adjustAmount) {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return;
	}
	switch (fieldIndex) {
		case FIELD_FOLLOW_ON_START:
			fields[fieldIndex].SetBool(!fields[fieldIndex].ToBool());
			break;
		case FIELD_ZOOM_STEP:
			fields[fieldIndex] = idMath::ClampFloat(1.1f, 1.5f, idMath::Rint((fields[fieldIndex].ToFloat() + adjustAmount * 0.05f) * 100.0f) / 100.0f);
			break;
		case FIELD_ZOOM_SPEED:
			fields[fieldIndex] = idMath::ClampFloat(1.02f, 1.1f, idMath::Rint((fields[fieldIndex].ToFloat() + adjustAmount * 0.01f) * 100.0f) / 100.0f);
			break;
		case FIELD_PAN_SPEED:
			fields[fieldIndex] = idMath::ClampFloat(2.0f, 4.0f, idMath::Rint((fields[fieldIndex].ToFloat() + adjustAmount * 0.25f) * 100.0f) / 100.0f);
			break;
	}
}
//#modified-fva; END
