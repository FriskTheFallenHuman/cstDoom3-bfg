//#modified-fva; BEGIN
#include "precompiled.h"
#pragma hdrstop
#include "../Game_local.h"
#include "../_CstStrings.h"

enum {
	FIELD_ENPRO_LIFT_LAMP,
	FIELD_ENPRO_TWOTONE_PIPE,
	FIELD_HELL1_DOOR_FIRE,
	FIELD_EREBUS5_ENVIROSUIT,
	FIELD_DELTAX_PHASING,
	FIELD_ENPRO2_LIFT_LAMP,
	FIELD_UNDERGROUND_HISS_PIPE,
	FIELD_1K_MAKE_MAD_SOUND,
	MAX_FIELDS
};

/*
========================
idMenuScreen_Shell_Cst_GoD3_MapOptions::Initialize
========================
*/
void idMenuScreen_Shell_Cst_GoD3_MapOptions::Initialize(idMenuHandler * data) {
	numVisibleOptions = 8;
	menuSpriteName = "cstMenu_GoD3_MapOptions";
	btnBackLabel = CST_STR_00300001;
	headingLabel = CST_STR_00300195;
	shellArea = CST_SHELL_AREA_GO_D3_MAP_OPTIONS;
	goBackShellArea = CST_SHELL_AREA_GAME_OPTIONS_DOOM3;
	useSmallHeading = true;

	options = new (TAG_SWF) idMenuWidget_DynamicList();
	idMenuWidget_ControlButton * control;

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300196);
	control->SetDataSource(&dataSource, FIELD_ENPRO_LIFT_LAMP);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300391);
	control->SetDataSource(&dataSource, FIELD_ENPRO_TWOTONE_PIPE);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300197);
	control->SetDataSource(&dataSource, FIELD_HELL1_DOOR_FIRE);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300198);
	control->SetDataSource(&dataSource, FIELD_EREBUS5_ENVIROSUIT);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300199);
	control->SetDataSource(&dataSource, FIELD_DELTAX_PHASING);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300392);
	control->SetDataSource(&dataSource, FIELD_ENPRO2_LIFT_LAMP);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300398);
	control->SetDataSource(&dataSource, FIELD_UNDERGROUND_HISS_PIPE);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300393);
	control->SetDataSource(&dataSource, FIELD_1K_MAKE_MAD_SOUND);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	helpText.Clear();
	helpText.Append(CST_STR_00300200); // 0
	helpText.Append(CST_STR_00300394); // 1
	helpText.Append(CST_STR_00300201); // 2
	helpText.Append(CST_STR_00300202); // 3
	helpText.Append(CST_STR_00300203); // 4
	helpText.Append(CST_STR_00300395); // 5
	helpText.Append(CST_STR_00300399); // 6
	helpText.Append(CST_STR_00300396); // 7

	idMenuScreen_Shell_CstMenuBase::Initialize(data);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_MapOptions::ShowScreen
========================
*/
void idMenuScreen_Shell_Cst_GoD3_MapOptions::ShowScreen(const mainMenuTransition_t transitionType) {
	dataSource.LoadData();
	idMenuScreen_Shell_CstMenuBase::ShowScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_MapOptions::HideScreen
========================
*/
void idMenuScreen_Shell_Cst_GoD3_MapOptions::HideScreen(const mainMenuTransition_t transitionType) {
	if (dataSource.IsDataChanged()) {
		dataSource.CommitData();
	}
	idMenuScreen_Shell_CstMenuBase::HideScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_MapOptions::HandleAction
========================
*/
bool idMenuScreen_Shell_Cst_GoD3_MapOptions::HandleAction(idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled) {
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
idMenuScreen_Shell_Cst_GoD3_MapOptions::idMenuDataSource_Settings::idMenuDataSource_Settings
========================
*/
idMenuScreen_Shell_Cst_GoD3_MapOptions::idMenuDataSource_Settings::idMenuDataSource_Settings() {
	fields.SetNum(MAX_FIELDS);
	originalFields.SetNum(MAX_FIELDS);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_MapOptions::idMenuDataSource_Settings::LoadData
========================
*/
void idMenuScreen_Shell_Cst_GoD3_MapOptions::idMenuDataSource_Settings::LoadData() {
	fields[FIELD_ENPRO_LIFT_LAMP].SetBool(!cst_mapEnpro_noLiftLamp.GetBool());
	fields[FIELD_ENPRO_TWOTONE_PIPE].SetBool(!cst_mapEnpro_noTwoTonePipe.GetBool());
	fields[FIELD_HELL1_DOOR_FIRE].SetBool(cst_mapHell1_doorFire.GetBool());
	fields[FIELD_EREBUS5_ENVIROSUIT].SetBool(cst_mapErebus5_enviroSuit.GetBool());
	fields[FIELD_DELTAX_PHASING].SetBool(cst_mapDeltaX_phasing.GetBool());
	fields[FIELD_ENPRO2_LIFT_LAMP].SetBool(!cst_mapLeEnpro2_noLiftLamp.GetBool());
	fields[FIELD_UNDERGROUND_HISS_PIPE].SetBool(!cst_mapLeUnderground_noHissPipe.GetBool());
	fields[FIELD_1K_MAKE_MAD_SOUND].SetBool(!cst_no1kMakeMadDrone.GetBool());
	originalFields = fields;
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_MapOptions::idMenuDataSource_Settings::CommitData
========================
*/
void idMenuScreen_Shell_Cst_GoD3_MapOptions::idMenuDataSource_Settings::CommitData() {
	cst_mapEnpro_noLiftLamp.SetBool(!fields[FIELD_ENPRO_LIFT_LAMP].ToBool());
	cst_mapEnpro_noTwoTonePipe.SetBool(!fields[FIELD_ENPRO_TWOTONE_PIPE].ToBool());
	cst_mapHell1_doorFire.SetBool(fields[FIELD_HELL1_DOOR_FIRE].ToBool());
	cst_mapErebus5_enviroSuit.SetBool(fields[FIELD_EREBUS5_ENVIROSUIT].ToBool());
	cst_mapDeltaX_phasing.SetBool(fields[FIELD_DELTAX_PHASING].ToBool());
	cst_mapLeEnpro2_noLiftLamp.SetBool(!fields[FIELD_ENPRO2_LIFT_LAMP].ToBool());
	cst_mapLeUnderground_noHissPipe.SetBool(!fields[FIELD_UNDERGROUND_HISS_PIPE].ToBool());
	cst_no1kMakeMadDrone.SetBool(!fields[FIELD_1K_MAKE_MAD_SOUND].ToBool());

	cvarSystem->SetModifiedFlags(CVAR_ARCHIVE);
	originalFields = fields; // update the backup fields
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_MapOptions::idMenuDataSource_Settings::IsDataChanged
========================
*/
bool idMenuScreen_Shell_Cst_GoD3_MapOptions::idMenuDataSource_Settings::IsDataChanged() const {
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
idMenuScreen_Shell_Cst_GoD3_MapOptions::idMenuDataSource_Settings::GetField
========================
*/
idSWFScriptVar idMenuScreen_Shell_Cst_GoD3_MapOptions::idMenuDataSource_Settings::GetField(const int fieldIndex) const {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return false;
	}
	return fields[fieldIndex];
}


/*
========================
idMenuScreen_Shell_Cst_GoD3_MapOptions::idMenuDataSource_Settings::AdjustField
========================
*/
void idMenuScreen_Shell_Cst_GoD3_MapOptions::idMenuDataSource_Settings::AdjustField(const int fieldIndex, const int adjustAmount) {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return;
	}
	fields[fieldIndex].SetBool(!fields[fieldIndex].ToBool());
}
//#modified-fva; END
