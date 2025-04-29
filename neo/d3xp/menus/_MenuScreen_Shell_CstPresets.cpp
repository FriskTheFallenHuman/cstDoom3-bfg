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
// part of the code below is adapted from other source files (with changes)

#include "precompiled.h"
#pragma hdrstop
#include "../Game_local.h"
#include "../_CstStrings.h"

enum {
	CMD_RESET_ALL_DEFAULTS,
	CMD_DEFAULT_CONTROLS_DOOM3,
	CMD_DEFAULT_CONTROLS_DOOM_CLASSIC,
	CMD_LAYERED_CONTROLS_DOOM3,
	CMD_LAYERED_CONTROLS_DOOM_CLASSIC
};

/*
========================
idMenuScreen_Shell_CstPresets::Initialize
========================
*/
void idMenuScreen_Shell_CstPresets::Initialize(idMenuHandler * data) {
	numVisibleOptions = 8;
	menuSpriteName = "cstMenuPresets";
	btnBackLabel = "#str_swf_settings";
	headingLabel = CST_STR_00300347;
	shellArea = CST_SHELL_AREA_PRESETS;
	goBackShellArea = SHELL_AREA_SETTINGS;
	showJoySelect = true;

	options = new (TAG_SWF) idMenuWidget_DynamicList();
	idList< idStr > option;

	option.Append(CST_STR_00300348);	// Reset All Defaults
	menuOptions.Append(option);
	option.Clear();
	option.Append(CST_STR_00300349);	// Default Controls - Doom 3
	menuOptions.Append(option);
	option.Clear();
	option.Append(CST_STR_00300350);	// Default Controls - Doom Classic
	menuOptions.Append(option);
	option.Clear();
	option.Append(CST_STR_00300351);	// Layered Controls - Doom 3
	menuOptions.Append(option);
	option.Clear();
	option.Append(CST_STR_00300352);	// Layered Controls - Doom Classic
	menuOptions.Append(option);

	for (int i = 0; i < numVisibleOptions; ++i) {
		idMenuWidget_Button * const button = new (TAG_SWF) idMenuWidget_Button();
		button->Initialize(data);
		int commandID = i;
		button->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_COMMAND, commandID);
		options->AddChild(button);
	}

	helpText.Clear();
	helpText.Append(CST_STR_00300353);	// 0
	helpText.Append(CST_STR_00300354);	// 1
	helpText.Append(CST_STR_00300355);	// 2
	helpText.Append(CST_STR_00300356);	// 3
	helpText.Append(CST_STR_00300357);	// 4

	idMenuScreen_Shell_CstMenuBase::Initialize(data);
}

/*
========================
idMenuScreen_Shell_CstPresets::ShowScreen
========================
*/
void idMenuScreen_Shell_CstPresets::ShowScreen(const mainMenuTransition_t transitionType) {
	restartRequired = false;
	idMenuScreen_Shell_CstMenuBase::ShowScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_CstPresets::HideScreen
========================
*/
void idMenuScreen_Shell_CstPresets::HideScreen(const mainMenuTransition_t transitionType) {
	if (restartRequired) {
		restartRequired = false;
		// code below is from idMenuScreen_Shell_SystemOptions::HideScreen
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

	idMenuScreen_Shell_CstMenuBase::HideScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_CstPresets::HandleAction
========================
*/
bool idMenuScreen_Shell_CstPresets::HandleAction(idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled) {
	if (menuData == NULL) {
		return true;
	}
	if (menuData->ActiveScreen() != shellArea) {
		return false;
	}

	widgetAction_t actionType = action.GetType();
	const idSWFParmList & parms = action.GetParms();

	if (actionType == WIDGET_ACTION_COMMAND && parms.Num() > 0) {
		switch (parms[0].ToInteger()) {
			case CMD_RESET_ALL_DEFAULTS: {
				ResetAllDefaults();
				break;
			}
			case CMD_DEFAULT_CONTROLS_DOOM3: {
				DefaultControlsDoom3();
				break;
			}
			case CMD_DEFAULT_CONTROLS_DOOM_CLASSIC: {
				DefaultControlsDoomClassic();
				break;
			}
			case CMD_LAYERED_CONTROLS_DOOM3: {
				LayeredControlsDoom3();
				break;
			}
			case CMD_LAYERED_CONTROLS_DOOM_CLASSIC: {
				LayeredControlsDoomClassic();
				break;
			}
		}
	}

	return idMenuScreen_Shell_CstMenuBase::HandleAction(action, event, widget, forceHandled);
}

/*
========================
idMenuScreen_Shell_CstPresets::ResetAllDefaults
========================
*/
void idMenuScreen_Shell_CstPresets::ResetAllDefaults() {
	class idSWFScriptFunction_ResetAllDefaults : public idSWFScriptFunction_RefCounted {
	public:
		idSWFScriptFunction_ResetAllDefaults(idMenuScreen_Shell_CstPresets *_menu, gameDialogMessages_t _msg, bool _reset) {
			menu = _menu;
			msg = _msg;
			reset = _reset;
		}
		idSWFScriptVar Call(idSWFScriptObject * thisObject, const idSWFParmList & parms) {
			common->Dialog().ClearDialog(msg);
			if (reset) {
				if (menu) {
					menu->restartRequired = true;
				}
				// reset cvars
				idStr sysLang = cvarSystem->GetCVarString("sys_lang"); // keep language
				cmdSystem->BufferCommandText(CMD_EXEC_NOW, "cvar_restart");
				cvarSystem->SetCVarString("sys_lang", sysLang.c_str());
				// reset controls
				cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "exec default.cfg\n");
				cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "exec joy_360_0.cfg\n");
				cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "seta cst_joyLeftyFlip 0\n");
				cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "exec joy_righty.cfg\n");
				cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "seta cst_bind_run_once 1\n");
				cmdSystem->ExecuteCommandBuffer();
				// ensure the config will be saved
				cvarSystem->SetModifiedFlags(CVAR_ARCHIVE);
			}
			return idSWFScriptVar();
		}
	private:
		idMenuScreen_Shell_CstPresets *menu;
		gameDialogMessages_t msg;
		bool reset;
	};
	idStaticList<idSWFScriptFunction *, 4> callbacks;
	idStaticList<idStrId, 4> optionText;
	callbacks.Append(new idSWFScriptFunction_ResetAllDefaults(this, CST_GDM_RESET_ALL_DEFAULTS, true));
	callbacks.Append(new idSWFScriptFunction_ResetAllDefaults(this, CST_GDM_RESET_ALL_DEFAULTS, false));
	optionText.Append(idStrId("#str_swf_accept"));
	optionText.Append(idStrId("#str_swf_cancel"));
	common->Dialog().AddDynamicDialog(CST_GDM_RESET_ALL_DEFAULTS, callbacks, optionText, true, CST_STR_00300358);
}

/*
========================
idMenuScreen_Shell_CstPresets::DefaultControlsDoom3
========================
*/
void idMenuScreen_Shell_CstPresets::DefaultControlsDoom3() {
	class idSWFScriptFunction_DefaultControlsDoom3 : public idSWFScriptFunction_RefCounted {
	public:
		idSWFScriptFunction_DefaultControlsDoom3(gameDialogMessages_t _msg, bool _restore) {
			msg = _msg;
			restore = _restore;
		}
		idSWFScriptVar Call(idSWFScriptObject * thisObject, const idSWFParmList & parms) {
			common->Dialog().ClearDialog(msg);
			if (restore) {
				// see the original idPlayerProfile::ExecConfig
				cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "exec cfg/cst_preset_default_d3.cfg\n");
				cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "exec cfg/cst_joy_360_0_d3.cfg\n");
				if (cvarSystem->GetCVarBool("cst_joyLeftyFlip")) {
					cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "exec cfg/cst_joy_lefty_d3.cfg\n");
				} else {
					cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "exec cfg/cst_joy_righty_d3.cfg\n");
				}
				cmdSystem->ExecuteCommandBuffer();
				// ensure the config will be saved
				cvarSystem->SetModifiedFlags(CVAR_ARCHIVE);
			}
			return idSWFScriptVar();
		}
	private:
		gameDialogMessages_t msg;
		bool restore;
	};
	idStaticList<idSWFScriptFunction *, 4> callbacks;
	idStaticList<idStrId, 4> optionText;
	callbacks.Append(new idSWFScriptFunction_DefaultControlsDoom3(CST_GDM_DEFAULT_CONTROLS_DOOM3, true));
	callbacks.Append(new idSWFScriptFunction_DefaultControlsDoom3(CST_GDM_DEFAULT_CONTROLS_DOOM3, false));
	optionText.Append(idStrId("#str_swf_accept"));
	optionText.Append(idStrId("#str_swf_cancel"));
	common->Dialog().AddDynamicDialog(CST_GDM_DEFAULT_CONTROLS_DOOM3, callbacks, optionText, true, CST_STR_00300359);
}

/*
========================
idMenuScreen_Shell_CstPresets::DefaultControlsDoomClassic
========================
*/
void idMenuScreen_Shell_CstPresets::DefaultControlsDoomClassic() {
	class idSWFScriptFunction_DefaultControlsDoomClassic : public idSWFScriptFunction_RefCounted {
	public:
		idSWFScriptFunction_DefaultControlsDoomClassic(gameDialogMessages_t _msg, bool _restore) {
			msg = _msg;
			restore = _restore;
		}
		idSWFScriptVar Call(idSWFScriptObject * thisObject, const idSWFParmList & parms) {
			common->Dialog().ClearDialog(msg);
			if (restore) {
				// see the original idPlayerProfile::ExecConfig
				cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "exec cfg/cst_preset_default_dc.cfg\n");
				cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "exec cfg/cst_joy_360_0_dc.cfg\n");
				if (cvarSystem->GetCVarBool("cst_joyLeftyFlip")) {
					cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "exec cfg/cst_joy_lefty_dc.cfg\n");
				} else {
					cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "exec cfg/cst_joy_righty_dc.cfg\n");
				}
				cmdSystem->ExecuteCommandBuffer();
				// ensure the config will be saved
				cvarSystem->SetModifiedFlags(CVAR_ARCHIVE);
			}
			return idSWFScriptVar();
		}
	private:
		gameDialogMessages_t msg;
		bool restore;
	};
	idStaticList<idSWFScriptFunction *, 4> callbacks;
	idStaticList<idStrId, 4> optionText;
	callbacks.Append(new idSWFScriptFunction_DefaultControlsDoomClassic(CST_GDM_DEFAULT_CONTROLS_DOOM_CLASSIC, true));
	callbacks.Append(new idSWFScriptFunction_DefaultControlsDoomClassic(CST_GDM_DEFAULT_CONTROLS_DOOM_CLASSIC, false));
	optionText.Append(idStrId("#str_swf_accept"));
	optionText.Append(idStrId("#str_swf_cancel"));
	common->Dialog().AddDynamicDialog(CST_GDM_DEFAULT_CONTROLS_DOOM_CLASSIC, callbacks, optionText, true, CST_STR_00300360);
}

/*
========================
idMenuScreen_Shell_CstPresets::LayeredControlsDoom3
========================
*/
void idMenuScreen_Shell_CstPresets::LayeredControlsDoom3() {
	class idSWFScriptFunction_LayeredControlsDoom3 : public idSWFScriptFunction_RefCounted {
	public:
		idSWFScriptFunction_LayeredControlsDoom3(gameDialogMessages_t _msg, bool _restore) {
			msg = _msg;
			restore = _restore;
		}
		idSWFScriptVar Call(idSWFScriptObject * thisObject, const idSWFParmList & parms) {
			common->Dialog().ClearDialog(msg);
			if (restore) {
				// see the original idPlayerProfile::ExecConfig
				cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "exec cfg/cst_preset_layered_d3.cfg\n");
				cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "exec cfg/cst_joy_360_0_d3.cfg\n");
				if (cvarSystem->GetCVarBool("cst_joyLeftyFlip")) {
					cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "exec cfg/cst_joy_lefty_d3.cfg\n");
				} else {
					cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "exec cfg/cst_joy_righty_d3.cfg\n");
				}
				cmdSystem->ExecuteCommandBuffer();
				// ensure the config will be saved
				cvarSystem->SetModifiedFlags(CVAR_ARCHIVE);
			}
			return idSWFScriptVar();
		}
	private:
		gameDialogMessages_t msg;
		bool restore;
	};
	idStaticList<idSWFScriptFunction *, 4> callbacks;
	idStaticList<idStrId, 4> optionText;
	callbacks.Append(new idSWFScriptFunction_LayeredControlsDoom3(CST_GDM_LAYERED_CONTROLS_DOOM3, true));
	callbacks.Append(new idSWFScriptFunction_LayeredControlsDoom3(CST_GDM_LAYERED_CONTROLS_DOOM3, false));
	optionText.Append(idStrId("#str_swf_accept"));
	optionText.Append(idStrId("#str_swf_cancel"));
	common->Dialog().AddDynamicDialog(CST_GDM_LAYERED_CONTROLS_DOOM3, callbacks, optionText, true, CST_STR_00300361);
}

/*
========================
idMenuScreen_Shell_CstPresets::LayeredControlsDoomClassic
========================
*/
void idMenuScreen_Shell_CstPresets::LayeredControlsDoomClassic() {
	class idSWFScriptFunction_LayeredControlsDoomClassic : public idSWFScriptFunction_RefCounted {
	public:
		idSWFScriptFunction_LayeredControlsDoomClassic(gameDialogMessages_t _msg, bool _restore) {
			msg = _msg;
			restore = _restore;
		}
		idSWFScriptVar Call(idSWFScriptObject * thisObject, const idSWFParmList & parms) {
			common->Dialog().ClearDialog(msg);
			if (restore) {
				// see the original idPlayerProfile::ExecConfig
				cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "exec cfg/cst_preset_layered_dc.cfg\n");
				cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "exec cfg/cst_joy_360_0_dc.cfg\n");
				if (cvarSystem->GetCVarBool("cst_joyLeftyFlip")) {
					cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "exec cfg/cst_joy_lefty_dc.cfg\n");
				} else {
					cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "exec cfg/cst_joy_righty_dc.cfg\n");
				}
				cmdSystem->ExecuteCommandBuffer();
				// ensure the config will be saved
				cvarSystem->SetModifiedFlags(CVAR_ARCHIVE);
			}
			return idSWFScriptVar();
		}
	private:
		gameDialogMessages_t msg;
		bool restore;
	};
	idStaticList<idSWFScriptFunction *, 4> callbacks;
	idStaticList<idStrId, 4> optionText;
	callbacks.Append(new idSWFScriptFunction_LayeredControlsDoomClassic(CST_GDM_LAYERED_CONTROLS_DOOM_CLASSIC, true));
	callbacks.Append(new idSWFScriptFunction_LayeredControlsDoomClassic(CST_GDM_LAYERED_CONTROLS_DOOM_CLASSIC, false));
	optionText.Append(idStrId("#str_swf_accept"));
	optionText.Append(idStrId("#str_swf_cancel"));
	common->Dialog().AddDynamicDialog(CST_GDM_LAYERED_CONTROLS_DOOM_CLASSIC, callbacks, optionText, true, CST_STR_00300362);
}
//#modified-fva; END
