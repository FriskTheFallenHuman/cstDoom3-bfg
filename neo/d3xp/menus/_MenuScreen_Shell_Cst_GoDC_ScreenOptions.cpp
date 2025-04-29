//#modified-fva; BEGIN
#include "precompiled.h"
#pragma hdrstop
#include "../Game_local.h"
#include "../_CstStrings.h"

enum {
	FIELD_PICKUP_FLASH,
	FIELD_DAMAGE_FLASH,
	FIELD_MUZZLE_FLASH,
	FIELD_INVULNERABILITY_EFFECT,
	FIELD_RAD_SUIT_EFFECT,
	FIELD_BERSERK_EFFECT,
	FIELD_TELEPORT_FOG,
	FIELD_BRIGHTNESS,
	MAX_FIELDS
};

/*
========================
idMenuScreen_Shell_Cst_GoDC_ScreenOptions::Initialize
========================
*/
void idMenuScreen_Shell_Cst_GoDC_ScreenOptions::Initialize(idMenuHandler * data) {
	numVisibleOptions = 8;
	menuSpriteName = "cstMenu_GoDC_ScreenOptions";
	btnBackLabel = CST_STR_00300001;
	headingLabel = CST_STR_00300321;
	shellArea = CST_SHELL_AREA_GO_DC_SCREEN_OPTIONS;
	goBackShellArea = CST_SHELL_AREA_GAME_OPTIONS_DOOM_CLASSIC;
	useSmallHeading = true;

	options = new (TAG_SWF) idMenuWidget_DynamicList();
	idMenuWidget_ControlButton * control;

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TEXT);
	control->SetLabel(CST_STR_00300322);
	control->SetDataSource(&dataSource, FIELD_PICKUP_FLASH);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TEXT);
	control->SetLabel(CST_STR_00300323);
	control->SetDataSource(&dataSource, FIELD_DAMAGE_FLASH);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300324);
	control->SetDataSource(&dataSource, FIELD_MUZZLE_FLASH);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TEXT);
	control->SetLabel(CST_STR_00300325);
	control->SetDataSource(&dataSource, FIELD_INVULNERABILITY_EFFECT);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TEXT);
	control->SetLabel(CST_STR_00300326);
	control->SetDataSource(&dataSource, FIELD_RAD_SUIT_EFFECT);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TEXT);
	control->SetLabel(CST_STR_00300327);
	control->SetDataSource(&dataSource, FIELD_BERSERK_EFFECT);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300328);
	control->SetDataSource(&dataSource, FIELD_TELEPORT_FOG);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_BAR);
	control->SetLabel(CST_STR_00300329);
	control->SetDataSource(&dataSource, FIELD_BRIGHTNESS);
	control->SetupEvents(2, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	helpText.Clear();
	helpText.Append(CST_STR_00300330); // 0
	helpText.Append(CST_STR_00300331); // 1
	helpText.Append(CST_STR_00300332); // 2
	helpText.Append(CST_STR_00300333); // 3
	helpText.Append(CST_STR_00300334); // 4
	helpText.Append(CST_STR_00300335); // 5
	helpText.Append(CST_STR_00300336); // 6
	helpText.Append(CST_STR_00300337); // 7

	idMenuScreen_Shell_CstMenuBase::Initialize(data);
}

/*
========================
idMenuScreen_Shell_Cst_GoDC_ScreenOptions::ShowScreen
========================
*/
void idMenuScreen_Shell_Cst_GoDC_ScreenOptions::ShowScreen(const mainMenuTransition_t transitionType) {
	dataSource.LoadData();
	idMenuScreen_Shell_CstMenuBase::ShowScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_Cst_GoDC_ScreenOptions::HideScreen
========================
*/
void idMenuScreen_Shell_Cst_GoDC_ScreenOptions::HideScreen(const mainMenuTransition_t transitionType) {
	if (dataSource.IsDataChanged()) {
		dataSource.CommitData();
	}
	idMenuScreen_Shell_CstMenuBase::HideScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_Cst_GoDC_ScreenOptions::HandleAction
========================
*/
bool idMenuScreen_Shell_Cst_GoDC_ScreenOptions::HandleAction(idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled) {
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
idMenuScreen_Shell_Cst_GoDC_ScreenOptions::idMenuDataSource_Settings::idMenuDataSource_Settings
========================
*/
idMenuScreen_Shell_Cst_GoDC_ScreenOptions::idMenuDataSource_Settings::idMenuDataSource_Settings() {
	fields.SetNum(MAX_FIELDS);
	originalFields.SetNum(MAX_FIELDS);
}

/*
========================
idMenuScreen_Shell_Cst_GoDC_ScreenOptions::idMenuDataSource_Settings::LoadData
========================
*/
void idMenuScreen_Shell_Cst_GoDC_ScreenOptions::idMenuDataSource_Settings::LoadData() {
	fields[FIELD_PICKUP_FLASH].SetInteger(cvarSystem->GetCVarInteger("cst_dcScreenPickup"));
	fields[FIELD_DAMAGE_FLASH].SetInteger(cvarSystem->GetCVarInteger("cst_dcScreenDamage"));
	fields[FIELD_MUZZLE_FLASH].SetBool(!cvarSystem->GetCVarBool("cst_dcNoMuzzleFlash"));
	fields[FIELD_INVULNERABILITY_EFFECT].SetInteger(cvarSystem->GetCVarInteger("cst_dcScreenInvul"));
	fields[FIELD_RAD_SUIT_EFFECT].SetInteger(cvarSystem->GetCVarInteger("cst_dcScreenSuit"));
	fields[FIELD_BERSERK_EFFECT].SetInteger(cvarSystem->GetCVarInteger("cst_dcScreenBerserk"));
	fields[FIELD_TELEPORT_FOG].SetBool(!cvarSystem->GetCVarBool("cst_dcNoTeleportFog"));
	fields[FIELD_BRIGHTNESS].SetFloat(cvarSystem->GetCVarFloat("cst_dcBrightness"));
	originalFields = fields;
}

/*
========================
idMenuScreen_Shell_Cst_GoDC_ScreenOptions::idMenuDataSource_Settings::CommitData
========================
*/
void idMenuScreen_Shell_Cst_GoDC_ScreenOptions::idMenuDataSource_Settings::CommitData() {
	cvarSystem->SetCVarInteger("cst_dcScreenPickup", fields[FIELD_PICKUP_FLASH].ToInteger());
	cvarSystem->SetCVarInteger("cst_dcScreenDamage", fields[FIELD_DAMAGE_FLASH].ToInteger());
	cvarSystem->SetCVarBool("cst_dcNoMuzzleFlash", !fields[FIELD_MUZZLE_FLASH].ToBool());
	cvarSystem->SetCVarInteger("cst_dcScreenInvul", fields[FIELD_INVULNERABILITY_EFFECT].ToInteger());
	cvarSystem->SetCVarInteger("cst_dcScreenSuit", fields[FIELD_RAD_SUIT_EFFECT].ToInteger());
	cvarSystem->SetCVarInteger("cst_dcScreenBerserk", fields[FIELD_BERSERK_EFFECT].ToInteger());
	cvarSystem->SetCVarBool("cst_dcNoTeleportFog", !fields[FIELD_TELEPORT_FOG].ToBool());

	if (fields[FIELD_BRIGHTNESS].ToFloat() != originalFields[FIELD_BRIGHTNESS].ToFloat()) {
		cvarSystem->SetCVarFloat("cst_dcBrightness", fields[FIELD_BRIGHTNESS].ToFloat());
	}

	cvarSystem->SetModifiedFlags(CVAR_ARCHIVE);
	originalFields = fields; // update the backup fields
}

/*
========================
idMenuScreen_Shell_Cst_GoDC_ScreenOptions::idMenuDataSource_Settings::IsDataChanged
========================
*/
bool idMenuScreen_Shell_Cst_GoDC_ScreenOptions::idMenuDataSource_Settings::IsDataChanged() const {
	if (fields[FIELD_PICKUP_FLASH].ToInteger() != originalFields[FIELD_PICKUP_FLASH].ToInteger() ||
		fields[FIELD_DAMAGE_FLASH].ToInteger() != originalFields[FIELD_DAMAGE_FLASH].ToInteger() ||
		fields[FIELD_MUZZLE_FLASH].ToBool() != originalFields[FIELD_MUZZLE_FLASH].ToBool() ||
		fields[FIELD_INVULNERABILITY_EFFECT].ToInteger() != originalFields[FIELD_INVULNERABILITY_EFFECT].ToInteger() ||
		fields[FIELD_RAD_SUIT_EFFECT].ToInteger() != originalFields[FIELD_RAD_SUIT_EFFECT].ToInteger() ||
		fields[FIELD_BERSERK_EFFECT].ToInteger() != originalFields[FIELD_BERSERK_EFFECT].ToInteger() ||
		fields[FIELD_TELEPORT_FOG].ToBool() != originalFields[FIELD_TELEPORT_FOG].ToBool() ||
		fields[FIELD_BRIGHTNESS].ToFloat() != originalFields[FIELD_BRIGHTNESS].ToFloat()) {
		return true;
	}
	return false;
}

/*
========================
idMenuScreen_Shell_Cst_GoDC_ScreenOptions::idMenuDataSource_Settings::GetField
========================
*/
idSWFScriptVar idMenuScreen_Shell_Cst_GoDC_ScreenOptions::idMenuDataSource_Settings::GetField(const int fieldIndex) const {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return false;
	}
	switch (fieldIndex) {
		case FIELD_PICKUP_FLASH:
		case FIELD_DAMAGE_FLASH:
		case FIELD_INVULNERABILITY_EFFECT:
		case FIELD_RAD_SUIT_EFFECT:
		case FIELD_BERSERK_EFFECT: {
			switch (fields[fieldIndex].ToInteger()) {
				case 0: return CST_STR_00300167;	// Full
				case 1: return CST_STR_00300338;	// Status Bar
				case 2: return CST_STR_00300164;	// Disabled
				default: return CST_STR_00300168;	// Unknown
			}
		}
		case FIELD_MUZZLE_FLASH:
		case FIELD_TELEPORT_FOG: {
			return fields[fieldIndex];
		}
		case FIELD_BRIGHTNESS: {
			return idMath::ClampFloat(0.0f, 100.0f, CstMenuUtils::LinearAdjust(fields[fieldIndex].ToFloat(), 0.1f, 1.0f, 0.0f, 100.0f));
		}
	}
	return false;
}

/*
========================
idMenuScreen_Shell_Cst_GoDC_ScreenOptions::idMenuDataSource_Settings::AdjustField
========================
*/
void idMenuScreen_Shell_Cst_GoDC_ScreenOptions::idMenuDataSource_Settings::AdjustField(const int fieldIndex, const int adjustAmount) {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return;
	}
	switch (fieldIndex) {
		case FIELD_PICKUP_FLASH:
		case FIELD_DAMAGE_FLASH:
		case FIELD_INVULNERABILITY_EFFECT:
		case FIELD_RAD_SUIT_EFFECT:
		case FIELD_BERSERK_EFFECT: {
			const int numValues = 3;
			int values[numValues] = { 0, 1, 2 };
			fields[fieldIndex].SetInteger(CstMenuUtils::AdjustOption(fields[fieldIndex].ToInteger(), values, numValues, adjustAmount));
			break;
		}
		case FIELD_MUZZLE_FLASH:
		case FIELD_TELEPORT_FOG: {
			fields[fieldIndex].SetBool(!fields[fieldIndex].ToBool());
			break;
		}
		case FIELD_BRIGHTNESS: {
			fields[fieldIndex] = idMath::ClampFloat(0.1f, 1.0f, idMath::Rint((fields[fieldIndex].ToFloat() + adjustAmount * 0.01f) * 100.0f) / 100.0f);
			break;
		}
	}
}
//#modified-fva; END
