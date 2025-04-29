//#modified-fva; BEGIN
#include "precompiled.h"
#pragma hdrstop
#include "../Game_local.h"
#include "../_CstStrings.h"

enum {
	FIELD_DOUBLE_VISION,
	FIELD_TUNNEL_VISION,
	FIELD_VIEW_ANGLE_KICK,
	FIELD_SCREEN_BLOB,
	FIELD_ARMOR_PULSE,
	FIELD_PAIN_SOUND,
	FIELD_AIR_GASP_SOUND,
	MAX_FIELDS
};

/*
========================
idMenuScreen_Shell_Cst_GoD3_DamageFeedback::Initialize
========================
*/
void idMenuScreen_Shell_Cst_GoD3_DamageFeedback::Initialize(idMenuHandler * data) {
	numVisibleOptions = 8;
	menuSpriteName = "cstMenu_GoD3_DamageFeedback";
	btnBackLabel = CST_STR_00300001;
	headingLabel = CST_STR_00300270;
	shellArea = CST_SHELL_AREA_GO_D3_DAMAGE_FEEDBACK;
	goBackShellArea = CST_SHELL_AREA_GAME_OPTIONS_DOOM3;
	useSmallHeading = true;

	options = new (TAG_SWF) idMenuWidget_DynamicList();
	idMenuWidget_ControlButton * control;

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300271);
	control->SetDataSource(&dataSource, FIELD_DOUBLE_VISION);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300272);
	control->SetDataSource(&dataSource, FIELD_TUNNEL_VISION);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300273);
	control->SetDataSource(&dataSource, FIELD_VIEW_ANGLE_KICK);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300274);
	control->SetDataSource(&dataSource, FIELD_SCREEN_BLOB);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300275);
	control->SetDataSource(&dataSource, FIELD_ARMOR_PULSE);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300276);
	control->SetDataSource(&dataSource, FIELD_PAIN_SOUND);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300277);
	control->SetDataSource(&dataSource, FIELD_AIR_GASP_SOUND);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	helpText.Clear();
	helpText.Append(CST_STR_00300278); // 0
	helpText.Append(CST_STR_00300279); // 1
	helpText.Append(CST_STR_00300280); // 2
	helpText.Append(CST_STR_00300281); // 3
	helpText.Append(CST_STR_00300282); // 4
	helpText.Append(CST_STR_00300283); // 5
	helpText.Append(CST_STR_00300284); // 6

	idMenuScreen_Shell_CstMenuBase::Initialize(data);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_DamageFeedback::ShowScreen
========================
*/
void idMenuScreen_Shell_Cst_GoD3_DamageFeedback::ShowScreen(const mainMenuTransition_t transitionType) {
	dataSource.LoadData();
	idMenuScreen_Shell_CstMenuBase::ShowScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_DamageFeedback::HideScreen
========================
*/
void idMenuScreen_Shell_Cst_GoD3_DamageFeedback::HideScreen(const mainMenuTransition_t transitionType) {
	if (dataSource.IsDataChanged()) {
		dataSource.CommitData();
	}
	idMenuScreen_Shell_CstMenuBase::HideScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_DamageFeedback::HandleAction
========================
*/
bool idMenuScreen_Shell_Cst_GoD3_DamageFeedback::HandleAction(idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled) {
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
idMenuScreen_Shell_Cst_GoD3_DamageFeedback::idMenuDataSource_Settings::idMenuDataSource_Settings
========================
*/
idMenuScreen_Shell_Cst_GoD3_DamageFeedback::idMenuDataSource_Settings::idMenuDataSource_Settings() {
	fields.SetNum(MAX_FIELDS);
	originalFields.SetNum(MAX_FIELDS);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_DamageFeedback::idMenuDataSource_Settings::LoadData
========================
*/
void idMenuScreen_Shell_Cst_GoD3_DamageFeedback::idMenuDataSource_Settings::LoadData() {
	fields[FIELD_DOUBLE_VISION].SetBool(!cst_dfNoDoubleVision.GetBool());
	fields[FIELD_TUNNEL_VISION].SetBool(!cst_dfNoTunnelVision.GetBool());
	fields[FIELD_VIEW_ANGLE_KICK].SetBool(!cst_dfNoViewAngleKick.GetBool());
	fields[FIELD_SCREEN_BLOB].SetBool(!cst_dfNoScreenBlob.GetBool());
	fields[FIELD_ARMOR_PULSE].SetBool(!cst_dfNoArmorPulse.GetBool());
	fields[FIELD_PAIN_SOUND].SetBool(!cst_dfNoSoundPain.GetBool());
	fields[FIELD_AIR_GASP_SOUND].SetBool(!cst_dfNoSoundAirGasp.GetBool());
	originalFields = fields;
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_DamageFeedback::idMenuDataSource_Settings::CommitData
========================
*/
void idMenuScreen_Shell_Cst_GoD3_DamageFeedback::idMenuDataSource_Settings::CommitData() {
	cst_dfNoDoubleVision.SetBool(!fields[FIELD_DOUBLE_VISION].ToBool());
	cst_dfNoTunnelVision.SetBool(!fields[FIELD_TUNNEL_VISION].ToBool());
	cst_dfNoViewAngleKick.SetBool(!fields[FIELD_VIEW_ANGLE_KICK].ToBool());
	cst_dfNoScreenBlob.SetBool(!fields[FIELD_SCREEN_BLOB].ToBool());
	cst_dfNoArmorPulse.SetBool(!fields[FIELD_ARMOR_PULSE].ToBool());
	cst_dfNoSoundPain.SetBool(!fields[FIELD_PAIN_SOUND].ToBool());
	cst_dfNoSoundAirGasp.SetBool(!fields[FIELD_AIR_GASP_SOUND].ToBool());

	cvarSystem->SetModifiedFlags(CVAR_ARCHIVE);
	originalFields = fields; // update the backup fields
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_DamageFeedback::idMenuDataSource_Settings::IsDataChanged
========================
*/
bool idMenuScreen_Shell_Cst_GoD3_DamageFeedback::idMenuDataSource_Settings::IsDataChanged() const {
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
idMenuScreen_Shell_Cst_GoD3_DamageFeedback::idMenuDataSource_Settings::GetField
========================
*/
idSWFScriptVar idMenuScreen_Shell_Cst_GoD3_DamageFeedback::idMenuDataSource_Settings::GetField(const int fieldIndex) const {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return false;
	}
	return fields[fieldIndex];
}


/*
========================
idMenuScreen_Shell_Cst_GoD3_DamageFeedback::idMenuDataSource_Settings::AdjustField
========================
*/
void idMenuScreen_Shell_Cst_GoD3_DamageFeedback::idMenuDataSource_Settings::AdjustField(const int fieldIndex, const int adjustAmount) {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return;
	}
	fields[fieldIndex].SetBool(!fields[fieldIndex].ToBool());
}
//#modified-fva; END
