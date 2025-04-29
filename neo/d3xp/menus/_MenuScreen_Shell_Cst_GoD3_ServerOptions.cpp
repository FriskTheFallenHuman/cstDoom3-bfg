//#modified-fva; BEGIN
#include "precompiled.h"
#pragma hdrstop
#include "../Game_local.h"
#include "../_CstStrings.h"

enum {
	FIELD_HEADLAMP,
	FIELD_HANDHELD_FLASHLIGHT,
	FIELD_GRENADES_TOGGLE_CONTROL,
	FIELD_SMOKE_CONTROL,
	FIELD_DAMAGE_FEEDBACK_CONTROL,
	FIELD_OLD_PLAYER_SPEED_MP,
	FIELD_OLD_AMMO_MP,
	MAX_FIELDS
};

/*
========================
idMenuScreen_Shell_Cst_GoD3_ServerOptions::idMenuScreen_Shell_Cst_GoD3_ServerOptions
========================
*/
idMenuScreen_Shell_Cst_GoD3_ServerOptions::idMenuScreen_Shell_Cst_GoD3_ServerOptions() :
	dataSource(*this) {
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_ServerOptions::Initialize
========================
*/
void idMenuScreen_Shell_Cst_GoD3_ServerOptions::Initialize(idMenuHandler * data) {
	numVisibleOptions = 8;
	menuSpriteName = "cstMenu_GoD3_ServerOptions";
	btnBackLabel = CST_STR_00300001;
	headingLabel = CST_STR_00300287;
	shellArea = CST_SHELL_AREA_GO_D3_SERVER_OPTIONS;
	goBackShellArea = CST_SHELL_AREA_GAME_OPTIONS_DOOM3;
	useSmallHeading = true;

	options = new (TAG_SWF) idMenuWidget_DynamicList();
	idMenuWidget_ControlButton * control;

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300288);
	control->SetDataSource(&dataSource, FIELD_HEADLAMP);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300289);
	control->SetDataSource(&dataSource, FIELD_HANDHELD_FLASHLIGHT);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300290);
	control->SetDataSource(&dataSource, FIELD_GRENADES_TOGGLE_CONTROL);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300291);
	control->SetDataSource(&dataSource, FIELD_SMOKE_CONTROL);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300292);
	control->SetDataSource(&dataSource, FIELD_DAMAGE_FEEDBACK_CONTROL);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300294);
	control->SetDataSource(&dataSource, FIELD_OLD_PLAYER_SPEED_MP);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300293);
	control->SetDataSource(&dataSource, FIELD_OLD_AMMO_MP);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	helpText.Clear();
	helpText.Append(CST_STR_00300295); // 0
	helpText.Append(CST_STR_00300296); // 1
	helpText.Append(CST_STR_00300297); // 2
	helpText.Append(CST_STR_00300298); // 3
	helpText.Append(CST_STR_00300299); // 4
	helpText.Append(CST_STR_00300301); // 5
	helpText.Append(CST_STR_00300300); // 6

	idMenuScreen_Shell_CstMenuBase::Initialize(data);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_ServerOptions::ShowScreen
========================
*/
void idMenuScreen_Shell_Cst_GoD3_ServerOptions::ShowScreen(const mainMenuTransition_t transitionType) {
	dataSource.LoadData();
	idMenuScreen_Shell_CstMenuBase::ShowScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_ServerOptions::HideScreen
========================
*/
void idMenuScreen_Shell_Cst_GoD3_ServerOptions::HideScreen(const mainMenuTransition_t transitionType) {
	if (dataSource.IsDataChanged()) {
		dataSource.CommitData();
	}
	idMenuScreen_Shell_CstMenuBase::HideScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_ServerOptions::HandleAction
========================
*/
bool idMenuScreen_Shell_Cst_GoD3_ServerOptions::HandleAction(idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled) {
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
idMenuScreen_Shell_Cst_GoD3_ServerOptions::idMenuDataSource_Settings::idMenuDataSource_Settings
========================
*/
idMenuScreen_Shell_Cst_GoD3_ServerOptions::idMenuDataSource_Settings::idMenuDataSource_Settings(idMenuScreen_Shell_Cst_GoD3_ServerOptions &_owner) :
	owner(_owner) {
	fields.SetNum(MAX_FIELDS);
	originalFields.SetNum(MAX_FIELDS);
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_ServerOptions::idMenuDataSource_Settings::LoadData
========================
*/
void idMenuScreen_Shell_Cst_GoD3_ServerOptions::idMenuDataSource_Settings::LoadData() {
	idMenuHandler_Shell * shell = dynamic_cast<idMenuHandler_Shell *>(owner.menuData);
	if (shell && shell->GetInGame() && common->IsMultiplayer()) {
		// if multiplayer, load the settings of the server
		fields[FIELD_HEADLAMP].SetBool(gameLocal.cstSiAllowHeadlampMP);
		fields[FIELD_HANDHELD_FLASHLIGHT].SetBool(gameLocal.cstSiAllowHandFlashlightMP);
		fields[FIELD_GRENADES_TOGGLE_CONTROL].SetBool(gameLocal.cstSiAllowGrenadesToggleMP);
		fields[FIELD_SMOKE_CONTROL].SetBool(gameLocal.cstSiAllowSmokeControlMP);
		fields[FIELD_DAMAGE_FEEDBACK_CONTROL].SetBool(gameLocal.cstSiAllowDamageFeedbackControlMP);
		fields[FIELD_OLD_PLAYER_SPEED_MP].SetBool(gameLocal.cstSiOldPlayerSpeedMP);
		fields[FIELD_OLD_AMMO_MP].SetBool(gameLocal.cstSiOldAmmoMP);
	} else {
		// if singleplayer or not in-game, load the local cvars
		fields[FIELD_HEADLAMP].SetBool(cst_allowHeadlampMP.GetBool());
		fields[FIELD_HANDHELD_FLASHLIGHT].SetBool(cst_allowHandFlashlightMP.GetBool());
		fields[FIELD_GRENADES_TOGGLE_CONTROL].SetBool(cst_allowGrenadesToggleMP.GetBool());
		fields[FIELD_SMOKE_CONTROL].SetBool(cst_allowSmokeControlMP.GetBool());
		fields[FIELD_DAMAGE_FEEDBACK_CONTROL].SetBool(cst_allowDamageFeedbackControlMP.GetBool());
		fields[FIELD_OLD_PLAYER_SPEED_MP].SetBool(cst_oldPlayerSpeedMP.GetBool());
		fields[FIELD_OLD_AMMO_MP].SetBool(cst_oldAmmoMP.GetBool());
	}
	originalFields = fields;
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_ServerOptions::idMenuDataSource_Settings::CommitData
========================
*/
void idMenuScreen_Shell_Cst_GoD3_ServerOptions::idMenuDataSource_Settings::CommitData() {
	idMenuHandler_Shell * shell = dynamic_cast<idMenuHandler_Shell *>(owner.menuData);
	if (shell && shell->GetInGame() && common->IsMultiplayer()) {
		return;
	}
	cst_allowHeadlampMP.SetBool(fields[FIELD_HEADLAMP].ToBool());
	cst_allowHandFlashlightMP.SetBool(fields[FIELD_HANDHELD_FLASHLIGHT].ToBool());
	cst_allowGrenadesToggleMP.SetBool(fields[FIELD_GRENADES_TOGGLE_CONTROL].ToBool());
	cst_allowSmokeControlMP.SetBool(fields[FIELD_SMOKE_CONTROL].ToBool());
	cst_allowDamageFeedbackControlMP.SetBool(fields[FIELD_DAMAGE_FEEDBACK_CONTROL].ToBool());
	cst_oldPlayerSpeedMP.SetBool(fields[FIELD_OLD_PLAYER_SPEED_MP].ToBool());
	cst_oldAmmoMP.SetBool(fields[FIELD_OLD_AMMO_MP].ToBool());

	cvarSystem->SetModifiedFlags(CVAR_ARCHIVE);
	originalFields = fields; // update the backup fields
}

/*
========================
idMenuScreen_Shell_Cst_GoD3_ServerOptions::idMenuDataSource_Settings::IsDataChanged
========================
*/
bool idMenuScreen_Shell_Cst_GoD3_ServerOptions::idMenuDataSource_Settings::IsDataChanged() const {
	idMenuHandler_Shell * shell = dynamic_cast<idMenuHandler_Shell *>(owner.menuData);
	if (shell && shell->GetInGame() && common->IsMultiplayer()) {
		return false;
	}
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
idMenuScreen_Shell_Cst_GoD3_ServerOptions::idMenuDataSource_Settings::GetField
========================
*/
idSWFScriptVar idMenuScreen_Shell_Cst_GoD3_ServerOptions::idMenuDataSource_Settings::GetField(const int fieldIndex) const {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return false;
	}
	return fields[fieldIndex];
}


/*
========================
idMenuScreen_Shell_Cst_GoD3_ServerOptions::idMenuDataSource_Settings::AdjustField
========================
*/
void idMenuScreen_Shell_Cst_GoD3_ServerOptions::idMenuDataSource_Settings::AdjustField(const int fieldIndex, const int adjustAmount) {
	idMenuHandler_Shell * shell = dynamic_cast<idMenuHandler_Shell *>(owner.menuData);
	if (shell && shell->GetInGame() && common->IsMultiplayer()) {
		return;
	}
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return;
	}
	fields[fieldIndex].SetBool(!fields[fieldIndex].ToBool());
}
//#modified-fva; END
