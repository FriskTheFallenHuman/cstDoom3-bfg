/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

//#modified-fva; BEGIN
// code below is adapted from MenuScreen_Shell_SystemOptions.cpp (with changes)

#include "precompiled.h"
#pragma hdrstop
#include "../Game_local.h"
#include "../_CstStrings.h"

extern idCVar cst_vidMode;
extern idCVar cst_vidMonitor;
extern idCVar cst_vidWidth;
extern idCVar cst_vidHeight;
extern idCVar cst_vidDisplayRefresh;

enum {
	FIELD_FULLSCREEN,
	FIELD_FRAMERATE_INT,
	FIELD_FRAMERATE_FRAC,
	FIELD_HIGH_RESOLUTION_CLOCK,
	FIELD_VSYNC,
	FIELD_ANTIALIASING,
	FIELD_MOTIONBLUR,
	FIELD_LODBIAS,
	FIELD_BRIGHTNESS,
	FIELD_VOLUME,
	MAX_FIELDS
};

/*
========================
idMenuScreen_Shell_CstSystemOptions1::Initialize
========================
*/
void idMenuScreen_Shell_CstSystemOptions1::Initialize(idMenuHandler * data) {
	numVisibleOptions = 14;
	menuSpriteName = "cstMenuSystemOptions1";
	btnBackLabel = "#str_swf_settings";
	headingLabel = CST_STR_00300340;
	shellArea = SHELL_AREA_SYSTEM_OPTIONS;
	goBackShellArea = SHELL_AREA_SETTINGS;

	options = new (TAG_SWF) idMenuWidget_DynamicList();
	idMenuWidget_ControlButton * control;

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TEXT);
	control->SetLabel("#str_02154");
	control->SetDataSource(&dataSource, FIELD_FULLSCREEN);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TEXT);
	control->SetLabel(CST_STR_00300400);
	control->SetDataSource(&dataSource, FIELD_FRAMERATE_INT);
	control->SetupEvents(2, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TEXT);
	control->SetLabel(CST_STR_00300401);
	control->SetDataSource(&dataSource, FIELD_FRAMERATE_FRAC);
	control->SetupEvents(2, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TEXT);
	control->SetLabel(CST_STR_00300418);
	control->SetDataSource(&dataSource, FIELD_HIGH_RESOLUTION_CLOCK);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TEXT);
	control->SetLabel("#str_04126");
	control->SetDataSource(&dataSource, FIELD_VSYNC);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TEXT);
	control->SetLabel("#str_04128");
	control->SetDataSource(&dataSource, FIELD_ANTIALIASING);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_TEXT);
	control->SetLabel("#str_swf_motionblur");
	control->SetDataSource(&dataSource, FIELD_MOTIONBLUR);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_BAR);
	control->SetLabel("#str_swf_lodbias");
	control->SetDataSource(&dataSource, FIELD_LODBIAS);
	control->SetupEvents(DEFAULT_REPEAT_TIME, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_BAR);
	control->SetLabel(CST_STR_00300341);
	control->SetDataSource(&dataSource, FIELD_BRIGHTNESS);
	control->SetupEvents(2, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	control = new (TAG_SWF) idMenuWidget_ControlButton();
	control->SetOptionType(OPTION_SLIDER_BAR);
	control->SetLabel("#str_02163");
	control->SetDataSource(&dataSource, FIELD_VOLUME);
	control->SetupEvents(2, options->GetChildren().Num());
	control->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, options->GetChildren().Num());
	options->AddChild(control);

	helpText.Clear();
	helpText.Append(CST_STR_00300402);	// 0
	helpText.Append(CST_STR_00300403);	// 1
	helpText.Append(CST_STR_00300404);	// 2
	helpText.Append(CST_STR_00300421);	// 3
	helpText.Append(NULL);				// 4
	helpText.Append(NULL);				// 5
	helpText.Append(NULL);				// 6
	helpText.Append(CST_STR_00300342);	// 7
	helpText.Append(CST_STR_00300343);	// 8

	idMenuScreen_Shell_CstMenuBase::Initialize(data);
}

/*
========================
idMenuScreen_Shell_CstSystemOptions1::ShowScreen
========================
*/
void idMenuScreen_Shell_CstSystemOptions1::ShowScreen(const mainMenuTransition_t transitionType) {
	dataSource.LoadData();
	if (options != NULL) {
		idMenuWidget_ControlButton * controlFullscreen = dynamic_cast<idMenuWidget_ControlButton *>(&options->GetChildByIndex(FIELD_FULLSCREEN));
		if (controlFullscreen) {
			if (cst_vidFullscreen.GetInteger() == 1 && cst_vidMode.GetInteger() == 0) {
				idStr str;
				str.Append(va("%s %d: ", idLocalization::GetString("#str_swf_monitor"), cst_vidMonitor.GetInteger()));
				str.Append(va("%d x %d", cst_vidWidth.GetInteger(), cst_vidHeight.GetInteger()));
				if (cst_vidDisplayRefresh.GetInteger() > 0) {
					str.Append(va(" @ %dhz", cst_vidDisplayRefresh.GetInteger()));
				}
				controlFullscreen->SetDescription(str.c_str());
			} else {
				controlFullscreen->SetDescription("");
			}
		}
	}

	idMenuScreen_Shell_CstMenuBase::ShowScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_CstSystemOptions1::HideScreen
========================
*/
void idMenuScreen_Shell_CstSystemOptions1::HideScreen(const mainMenuTransition_t transitionType) {
	bool skipRestart = false;
	if (menuData && menuData->NextScreen() == SHELL_AREA_RESOLUTION) {
		dataSource.SetWentToFullscreenMenu(true);
		skipRestart = true;
	}

	if (!skipRestart && dataSource.IsRestartRequired()) {
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
idMenuScreen_Shell_CstSystemOptions1::HandleAction
========================
*/
bool idMenuScreen_Shell_CstSystemOptions1::HandleAction(idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled) {
	if (menuData == NULL) {
		return true;
	}
	if (menuData->ActiveScreen() != shellArea) {
		return false;
	}

	widgetAction_t actionType = action.GetType();
	const idSWFParmList & parms = action.GetParms();

	if (actionType == WIDGET_ACTION_ADJUST_FIELD) {
		if (widget) {
			if (widget->GetDataSourceFieldIndex() == FIELD_FULLSCREEN) {
				menuData->SetNextScreen(SHELL_AREA_RESOLUTION, MENU_TRANSITION_SIMPLE);
				return true;
			} else if (widget->GetDataSourceFieldIndex() == FIELD_FRAMERATE_INT || widget->GetDataSourceFieldIndex() == FIELD_FRAMERATE_FRAC) {
				if (widget->GetDataSource() == NULL || widget->GetParent() == NULL || parms.Num() < 1) {
					return true;
				}
				widget->GetDataSource()->AdjustField(widget->GetDataSourceFieldIndex(), parms[0].ToInteger());
				widget->GetParent()->GetChildByIndex(FIELD_FRAMERATE_INT).Update();
				widget->GetParent()->GetChildByIndex(FIELD_FRAMERATE_FRAC).Update();
				return true;
			}
		}
	} else if (actionType == WIDGET_ACTION_COMMAND) {
		if (options == NULL) {
			return true;
		}
		int selectionIndex = options->GetFocusIndex();
		if (parms.Num() > 0) {
			selectionIndex = parms[0].ToInteger();
		}
		if (selectionIndex == FIELD_FULLSCREEN) {
			menuData->SetNextScreen(SHELL_AREA_RESOLUTION, MENU_TRANSITION_SIMPLE);
		} else {
			dataSource.AdjustField(selectionIndex, 1);
			options->Update();
		}
	}

	return idMenuScreen_Shell_CstMenuBase::HandleAction(action, event, widget, forceHandled);
}

/////////////////////////////////
// SETTINGS
/////////////////////////////////

extern idCVar r_multiSamples;
extern idCVar r_motionBlur;
extern idCVar r_swapInterval;
extern idCVar s_volume_dB;
extern idCVar r_lightScale;

/*
========================
idMenuScreen_Shell_CstSystemOptions1::idMenuDataSource_Settings::idMenuDataSource_Settings
========================
*/
idMenuScreen_Shell_CstSystemOptions1::idMenuDataSource_Settings::idMenuDataSource_Settings() :
	wentToFullscreenMenu(false) {
}

/*
========================
idMenuScreen_Shell_CstSystemOptions1::idMenuDataSource_Settings::LoadData
========================
*/
void idMenuScreen_Shell_CstSystemOptions1::idMenuDataSource_Settings::LoadData() {
	if (!wentToFullscreenMenu) {
		originalFramerate = com_engineHz.GetFloat();
		originalAntialias = r_multiSamples.GetInteger();
	} else {
		// keep the old values of originalFramerate and originalAntialias if coming back from the Fullscreen menu
		wentToFullscreenMenu = false;
	}
	originalHighResolutionClock = cst_hiResClock.GetInteger();
	originalMotionBlur = r_motionBlur.GetInteger();
	originalVsync = r_swapInterval.GetInteger();
	originalBrightness = r_lightScale.GetFloat();
	originalVolume = s_volume_dB.GetFloat();
}

/*
========================
idMenuScreen_Shell_CstSystemOptions1::idMenuDataSource_Settings::CommitData
========================
*/
void idMenuScreen_Shell_CstSystemOptions1::idMenuDataSource_Settings::CommitData() {
	cvarSystem->SetModifiedFlags(CVAR_ARCHIVE);
}

/*
========================
idMenuScreen_Shell_CstSystemOptions1::idMenuDataSource_Settings::IsDataChanged
========================
*/
bool idMenuScreen_Shell_CstSystemOptions1::idMenuDataSource_Settings::IsDataChanged() const {
	if (originalFramerate != com_engineHz.GetFloat() ||
		originalHighResolutionClock != cst_hiResClock.GetInteger() ||
		originalAntialias != r_multiSamples.GetInteger() ||
		originalMotionBlur != r_motionBlur.GetInteger() ||
		originalVsync != r_swapInterval.GetInteger() ||
		originalBrightness != r_lightScale.GetFloat() ||
		originalVolume != s_volume_dB.GetFloat()) {
		return true;
	}
	return false;
}

/*
========================
idMenuScreen_Shell_CstSystemOptions1::idMenuDataSource_Settings::GetField
========================
*/
idSWFScriptVar idMenuScreen_Shell_CstSystemOptions1::idMenuDataSource_Settings::GetField(const int fieldIndex) const {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return false;
	}
	switch (fieldIndex) {
		case FIELD_FULLSCREEN: {
			const int fullscreen = cst_vidFullscreen.GetInteger();
			const int vidmode = cst_vidMode.GetInteger();
			if (fullscreen == 0) {
				return "#str_swf_disabled";
			}
			if (fullscreen == -1 || vidmode == -1) {
				return "???";
			}
			if (cst_vidDisplayRefresh.GetInteger() > 0) {
				return va("%dx%d %dhz", cst_vidWidth.GetInteger(), cst_vidHeight.GetInteger(), cst_vidDisplayRefresh.GetInteger());
			} else {
				return va("%dx%d", cst_vidWidth.GetInteger(), cst_vidHeight.GetInteger());
			}
		}
		case FIELD_FRAMERATE_INT: {
			float engineHz = com_engineHz.GetFloat();
			engineHz = idMath::Rint(engineHz * 100.0f) / 100.0f;
			return va("%.2f FPS", engineHz);
		}
		case FIELD_FRAMERATE_FRAC:
			return "              "; // leave blank here and let FIELD_FRAMERATE_INT show the complete framerate value
		case FIELD_HIGH_RESOLUTION_CLOCK:
			if (cst_hiResClock.GetInteger() == 1) {
				return CST_STR_00300419;
			} else if (cst_hiResClock.GetInteger() == 2) {
				return CST_STR_00300420;
			} else {
				return "#str_swf_disabled";
			}
		case FIELD_VSYNC:
			if (r_swapInterval.GetInteger() == 1) {
				return "#str_swf_smart";
			} else if (r_swapInterval.GetInteger() == 2) {
				return "#str_swf_enabled";
			} else {
				return "#str_swf_disabled";
			}
		case FIELD_ANTIALIASING:
			if (r_multiSamples.GetInteger() == 0) {
				return "#str_swf_disabled";
			}
			return va("%dx", r_multiSamples.GetInteger());
		case FIELD_MOTIONBLUR:
			if (r_motionBlur.GetInteger() == 0) {
				return "#str_swf_disabled";
			}
			return va("%dx", idMath::IPow(2, r_motionBlur.GetInteger()));
		case FIELD_LODBIAS:
			return CstMenuUtils::LinearAdjust(r_lodBias.GetFloat(), -1.0f, 1.0f, 0.0f, 100.0f);
		case FIELD_BRIGHTNESS:
			return CstMenuUtils::LinearAdjust(r_lightScale.GetFloat(), 2.0f, 4.0f, 0.0f, 100.0f);
		case FIELD_VOLUME: {
			return 100.0f * Square(1.0f - (s_volume_dB.GetFloat() / DB_SILENCE));
		}
	}
	return false;
}

/*
========================
idMenuScreen_Shell_CstSystemOptions1::idMenuDataSource_Settings::AdjustField
========================
*/
void idMenuScreen_Shell_CstSystemOptions1::idMenuDataSource_Settings::AdjustField(const int fieldIndex, const int adjustAmount) {
	if (fieldIndex < 0 || fieldIndex >= MAX_FIELDS) {
		return;
	}
	switch (fieldIndex) {
		case FIELD_FRAMERATE_INT: {
			float engineHz = com_engineHz.GetFloat();
			engineHz += adjustAmount;
			engineHz = idMath::Rint(engineHz * 100.0f) / 100.0f;
			if (idMath::Frac(engineHz) == 0.0f) {
				com_engineHz.SetFloat(engineHz);
			} else {
				com_engineHz.SetString(va("%.2f", engineHz));
			}
			break;
		}
		case FIELD_FRAMERATE_FRAC: {
			float engineHz = com_engineHz.GetFloat();
			engineHz = idMath::Rint(engineHz * 100.0f) / 100.0f;
			float hzInt = idMath::Floor(engineHz);
			float hzFrac = idMath::Frac(engineHz) * 100.0f;
			hzFrac = idMath::Rint(hzFrac + adjustAmount);
			hzFrac = idMath::ClampFloat(0.0f, 99.0f, hzFrac);
			hzFrac /= 100.0f;
			engineHz = hzInt + hzFrac;
			if (idMath::Frac(engineHz) == 0.0f) {
				com_engineHz.SetFloat(engineHz);
			} else {
				com_engineHz.SetString(va("%.2f", engineHz));
			}
			break;
		}
		case FIELD_HIGH_RESOLUTION_CLOCK: {
			static const int numValues = 3;
			static const int values[numValues] = { 0, 1, 2 };
			cst_hiResClock.SetInteger(CstMenuUtils::AdjustOption(cst_hiResClock.GetInteger(), values, numValues, adjustAmount));
			break;
		}
		case FIELD_VSYNC: {
			static const int numValues = 3;
			static const int values[numValues] = { 0, 1, 2 };
			r_swapInterval.SetInteger(CstMenuUtils::AdjustOption(r_swapInterval.GetInteger(), values, numValues, adjustAmount));
			break;
		}
		case FIELD_ANTIALIASING: {
			static const int numValues = 5;
			static const int values[numValues] = { 0, 2, 4, 8, 16 };
			r_multiSamples.SetInteger(CstMenuUtils::AdjustOption(r_multiSamples.GetInteger(), values, numValues, adjustAmount));
			break;
		}
		case FIELD_MOTIONBLUR: {
			static const int numValues = 5;
			static const int values[numValues] = { 0, 2, 3, 4, 5 };
			r_motionBlur.SetInteger(CstMenuUtils::AdjustOption(r_motionBlur.GetInteger(), values, numValues, adjustAmount));
			break;
		}
		case FIELD_LODBIAS: {
			const float percent = CstMenuUtils::LinearAdjust(r_lodBias.GetFloat(), -1.0f, 1.0f, 0.0f, 100.0f);
			const float adjusted = percent + (float)adjustAmount * 5.0f;
			const float clamped = idMath::ClampFloat(0.0f, 100.0f, adjusted);
			r_lodBias.SetFloat(CstMenuUtils::LinearAdjust(clamped, 0.0f, 100.0f, -1.0f, 1.0f));
			break;
		}
		case FIELD_BRIGHTNESS: {
			const float percent = CstMenuUtils::LinearAdjust(r_lightScale.GetFloat(), 2.0f, 4.0f, 0.0f, 100.0f);
			const float adjusted = percent + (float)adjustAmount;
			const float clamped = idMath::ClampFloat(0.0f, 100.0f, adjusted);
			r_lightScale.SetFloat(CstMenuUtils::LinearAdjust(clamped, 0.0f, 100.0f, 2.0f, 4.0f));
			break;
		}
		case FIELD_VOLUME: {
			const float percent = 100.0f * Square(1.0f - (s_volume_dB.GetFloat() / DB_SILENCE));
			const float adjusted = percent + (float)adjustAmount;
			const float clamped = idMath::ClampFloat(0.0f, 100.0f, adjusted);
			s_volume_dB.SetFloat(DB_SILENCE - (idMath::Sqrt(clamped / 100.0f) * DB_SILENCE));
			break;
		}
	}
	cvarSystem->ClearModifiedFlags(CVAR_ARCHIVE);
}

/*
========================
idMenuScreen_Shell_CstSystemOptions1::idMenuDataSource_Settings::IsRestartRequired
========================
*/
bool idMenuScreen_Shell_CstSystemOptions1::idMenuDataSource_Settings::IsRestartRequired() const {
	if (originalAntialias != r_multiSamples.GetInteger() ||
		originalFramerate != com_engineHz.GetFloat()) {
		return true;
	}
	return false;
}
//#modified-fva; END
