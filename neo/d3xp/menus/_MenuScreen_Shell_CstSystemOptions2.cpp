//#modified-fva; BEGIN
#include "precompiled.h"
#pragma hdrstop
#include "../Game_local.h"
#include "../_CstStrings.h"

enum {
	FIELD_LANGUAGE,
	FIELD_ACHIEVEMENTS_NOTIFICATION,
	FIELD_ALLOW_SKIP_CINEMATICS,
	FIELD_SKIP_INTRO_VIDEOS,
	FIELD_FAST_LEGAL_SCREEN,
	FIELD_RESOLUTION_SCALING,
	FIELD_UNLOCK_CONSOLE,
	FIELD_SHOW_FPS,
	MAX_FIELDS
};

/*
========================
idMenuScreen_Shell_CstSystemOptions2::Initialize
========================
*/
void idMenuScreen_Shell_CstSystemOptions2::Initialize(idMenuHandler * data) {
	numVisibleOptions = 8;
	menuSpriteName = "cstMenuSystemOptions2";
	btnBackLabel = "#str_swf_settings";
	headingLabel = CST_STR_00300156;
	shellArea = CST_SHELL_AREA_SYSTEM_OPTIONS_2;
	goBackShellArea = SHELL_AREA_SETTINGS;

	options = new (TAG_SWF) idMenuWidget_DynamicList();
	idMenuWidget_ControlButton * control;

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TEXT);
	control->SetLabel(CST_STR_00300384);
	control->SetDataSource(&dataSource, FIELD_LANGUAGE);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300157);
	control->SetDataSource(&dataSource, FIELD_ACHIEVEMENTS_NOTIFICATION);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300158);
	control->SetDataSource(&dataSource, FIELD_ALLOW_SKIP_CINEMATICS);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300159);
	control->SetDataSource(&dataSource, FIELD_SKIP_INTRO_VIDEOS);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300160);
	control->SetDataSource(&dataSource, FIELD_FAST_LEGAL_SCREEN);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TEXT);
	control->SetLabel(CST_STR_00300161);
	control->SetDataSource(&dataSource, FIELD_RESOLUTION_SCALING);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300162);
	control->SetDataSource(&dataSource, FIELD_UNLOCK_CONSOLE);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TOGGLE);
	control->SetLabel(CST_STR_00300163);
	control->SetDataSource(&dataSource, FIELD_SHOW_FPS);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	helpText.Clear();
	helpText.Append(CST_STR_00300385); // 0
	helpText.Append(CST_STR_00300169); // 1
	helpText.Append(CST_STR_00300170); // 2
	helpText.Append(CST_STR_00300171); // 3
	helpText.Append(CST_STR_00300172); // 4
	helpText.Append(CST_STR_00300173); // 5
	helpText.Append(CST_STR_00300174); // 6
	helpText.Append(CST_STR_00300175); // 7

	idMenuScreen_Shell_CstMenuBase::Initialize(data);
}

/*
========================
idMenuScreen_Shell_CstSystemOptions2::ShowScreen
========================
*/
void idMenuScreen_Shell_CstSystemOptions2::ShowScreen(const mainMenuTransition_t transitionType) {
	dataSource.LoadData();
	idMenuScreen_Shell_CstMenuBase::ShowScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_CstSystemOptions2::HideScreen
========================
*/
void idMenuScreen_Shell_CstSystemOptions2::HideScreen(const mainMenuTransition_t transitionType) {
	if (dataSource.IsRestartRequired()) {
		class idSWFScriptFunction_Restart : public idSWFScriptFunction_RefCounted {
		public:
			idSWFScriptFunction_Restart(gameDialogMessages_t _msg, bool _restart) {
				msg = _msg;
				restart = _restart;
			}
			idSWFScriptVar Call(idSWFScriptObject * thisObject, const idSWFParmList & parms) {
				common->Dialog().ClearDialog(msg);
				if (restart) {
					idStr cmdLine = Sys_GetCmdLine();
					if (cmdLine.Find("com_skipIntroVideos") < 0) {
						cmdLine.Append(" +set com_skipIntroVideos 1");
					}
					Sys_ReLaunch((void*)cmdLine.c_str(), cmdLine.Length());
				}
				return idSWFScriptVar();
			}
		private:
			gameDialogMessages_t msg;
			bool restart;
		};
		idStaticList<idSWFScriptFunction *, 4> callbacks;
		idStaticList<idStrId, 4> optionText;
		callbacks.Append(new idSWFScriptFunction_Restart(GDM_GAME_RESTART_REQUIRED, false));
		callbacks.Append(new idSWFScriptFunction_Restart(GDM_GAME_RESTART_REQUIRED, true));
		optionText.Append(idStrId("#str_00100113")); // Continue
		optionText.Append(idStrId("#str_02487")); // Restart Now
		common->Dialog().AddDynamicDialog(GDM_GAME_RESTART_REQUIRED, callbacks, optionText, true, idStr());
	}

	if (dataSource.IsDataChanged()) {
		dataSource.CommitData();
	}
	idMenuScreen_Shell_CstMenuBase::HideScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_CstSystemOptions2::HandleAction
========================
*/
bool idMenuScreen_Shell_CstSystemOptions2::HandleAction(idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled) {
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
idMenuScreen_Shell_CstSystemOptions2::idMenuDataSource_Settings::idMenuDataSource_Settings
========================
*/
idMenuScreen_Shell_CstSystemOptions2::idMenuDataSource_Settings::idMenuDataSource_Settings() {
}

/*
========================
idMenuScreen_Shell_CstSystemOptions2::idMenuDataSource_Settings::LoadData
========================
*/
void idMenuScreen_Shell_CstSystemOptions2::idMenuDataSource_Settings::LoadData() {
	originalLanguage = cvarSystem->GetCVarString("sys_lang");
	originalAchievementsNotification = !cvarSystem->GetCVarBool("cst_achievementsNoPrint");
	originalAllowSkipCinematics = cvarSystem->GetCVarBool("cst_allowSkipCinematics");
	originalSkipIntroVideos = cvarSystem->GetCVarBool("cst_skipIntroVideos");
	originalFastLegalScreen = cvarSystem->GetCVarBool("cst_fastLegalScreen");
	originalResolutionScaling = cvarSystem->GetCVarInteger("rs_enable");
	originalUnlockConsole = cvarSystem->GetCVarBool("com_allowConsole");
	originalShowFPS = cvarSystem->GetCVarBool("com_showFPS");
}

/*
========================
idMenuScreen_Shell_CstSystemOptions2::idMenuDataSource_Settings::CommitData
========================
*/
void idMenuScreen_Shell_CstSystemOptions2::idMenuDataSource_Settings::CommitData() {
	cvarSystem->SetModifiedFlags(CVAR_ARCHIVE);
}

/*
========================
idMenuScreen_Shell_CstSystemOptions2::idMenuDataSource_Settings::IsDataChanged
========================
*/
bool idMenuScreen_Shell_CstSystemOptions2::idMenuDataSource_Settings::IsDataChanged() const {
	if (originalLanguage != cvarSystem->GetCVarString("sys_lang") ||
		originalAchievementsNotification != !cvarSystem->GetCVarBool("cst_achievementsNoPrint") ||
		originalAllowSkipCinematics != cvarSystem->GetCVarBool("cst_allowSkipCinematics") ||
		originalSkipIntroVideos != cvarSystem->GetCVarBool("cst_skipIntroVideos") ||
		originalFastLegalScreen != cvarSystem->GetCVarBool("cst_fastLegalScreen") ||
		originalResolutionScaling != cvarSystem->GetCVarInteger("rs_enable") ||
		originalUnlockConsole != cvarSystem->GetCVarBool("com_allowConsole") ||
		originalShowFPS != cvarSystem->GetCVarBool("com_showFPS")) {
		return true;
	}
	return false;
}

/*
========================
idMenuScreen_Shell_CstSystemOptions2::idMenuDataSource_Settings::GetField
========================
*/
idSWFScriptVar idMenuScreen_Shell_CstSystemOptions2::idMenuDataSource_Settings::GetField(const int fieldIndex) const {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return false;
	}
	switch (fieldIndex) {
		case FIELD_LANGUAGE: {
			const char * lang = cvarSystem->GetCVarString("sys_lang");
			if (idStr::Icmp(lang, ID_LANG_ENGLISH) == 0) { return CST_STR_00300378; }
			if (idStr::Icmp(lang, ID_LANG_FRENCH) == 0) { return CST_STR_00300379; }
			if (idStr::Icmp(lang, ID_LANG_ITALIAN) == 0) { return CST_STR_00300380; }
			if (idStr::Icmp(lang, ID_LANG_GERMAN) == 0) { return CST_STR_00300381; }
			if (idStr::Icmp(lang, ID_LANG_SPANISH) == 0) { return CST_STR_00300382; }
			if (idStr::Icmp(lang, ID_LANG_JAPANESE) == 0) { return CST_STR_00300383; }
			return CST_STR_00300168; // Unknown
		}
		case FIELD_ACHIEVEMENTS_NOTIFICATION:
			return !cvarSystem->GetCVarBool("cst_achievementsNoPrint");
		case FIELD_ALLOW_SKIP_CINEMATICS:
			return cvarSystem->GetCVarBool("cst_allowSkipCinematics");
		case FIELD_SKIP_INTRO_VIDEOS:
			return cvarSystem->GetCVarBool("cst_skipIntroVideos");
		case FIELD_FAST_LEGAL_SCREEN:
			return cvarSystem->GetCVarBool("cst_fastLegalScreen");
		case FIELD_RESOLUTION_SCALING: {
			switch (cvarSystem->GetCVarInteger("rs_enable")) {
				case 0: return CST_STR_00300164;	// Disabled
				case 1: return CST_STR_00300165;	// Horizontal
				case 2: return CST_STR_00300166;	// Vertical
				case 3: return CST_STR_00300167;	// Full
				default: return CST_STR_00300168;	// Unknown
			}
		}
		case FIELD_UNLOCK_CONSOLE:
			return cvarSystem->GetCVarBool("com_allowConsole");
		case FIELD_SHOW_FPS:
			return cvarSystem->GetCVarBool("com_showFPS");
	}
	return false;
}

/*
========================
idMenuScreen_Shell_CstSystemOptions2::idMenuDataSource_Settings::AdjustField
========================
*/
void idMenuScreen_Shell_CstSystemOptions2::idMenuDataSource_Settings::AdjustField(const int fieldIndex, const int adjustAmount) {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return;
	}
	switch (fieldIndex) {
		case FIELD_LANGUAGE: {
			const idStrList &installedLanguages = common->CstGetInstalledLanguages();
			if (installedLanguages.Num() == 0) {
				break; // no languages are installed; do nothing
			}
			int langIndex = installedLanguages.FindIndex(cvarSystem->GetCVarString("sys_lang"));
			if (langIndex >= 0) {
				if (adjustAmount > 0) {
					++langIndex;
					if (langIndex >= installedLanguages.Num()) {
						langIndex = 0;
					}
				} else if (adjustAmount < 0) {
					--langIndex;
					if (langIndex < 0) {
						langIndex = installedLanguages.Num() - 1;
					}
				}
				cvarSystem->SetCVarString("sys_lang", installedLanguages[langIndex].c_str());
			} else {
				cvarSystem->SetCVarString("sys_lang", installedLanguages[0].c_str());
			}
			break;
		}
		case FIELD_ACHIEVEMENTS_NOTIFICATION:
			cvarSystem->SetCVarBool("cst_achievementsNoPrint", !cvarSystem->GetCVarBool("cst_achievementsNoPrint"));
			break;
		case FIELD_ALLOW_SKIP_CINEMATICS:
			cvarSystem->SetCVarBool("cst_allowSkipCinematics", !cvarSystem->GetCVarBool("cst_allowSkipCinematics"));
			break;
		case FIELD_SKIP_INTRO_VIDEOS:
			cvarSystem->SetCVarBool("cst_skipIntroVideos", !cvarSystem->GetCVarBool("cst_skipIntroVideos"));
			break;
		case FIELD_FAST_LEGAL_SCREEN:
			cvarSystem->SetCVarBool("cst_fastLegalScreen", !cvarSystem->GetCVarBool("cst_fastLegalScreen"));
			break;
		case FIELD_RESOLUTION_SCALING: {
			const int numValues = 4;
			int values[numValues] = { 0, 1, 2, 3 };
			cvarSystem->SetCVarInteger("rs_enable", CstMenuUtils::AdjustOption(cvarSystem->GetCVarInteger("rs_enable"), values, numValues, adjustAmount));
			break;
		}
		case FIELD_UNLOCK_CONSOLE:
			cvarSystem->SetCVarBool("com_allowConsole", !cvarSystem->GetCVarBool("com_allowConsole"));
			break;
		case FIELD_SHOW_FPS:
			cvarSystem->SetCVarBool("com_showFPS", !cvarSystem->GetCVarBool("com_showFPS"));
			break;
	}
	cvarSystem->ClearModifiedFlags(CVAR_ARCHIVE);
}

/*
========================
idMenuScreen_Shell_CstSystemOptions2::idMenuDataSource_Settings::IsRestartRequired
========================
*/
bool idMenuScreen_Shell_CstSystemOptions2::idMenuDataSource_Settings::IsRestartRequired() const {
	if (originalLanguage != cvarSystem->GetCVarString("sys_lang")) {
		return true;
	}
	return false;
}
//#modified-fva; END
