//#modified-fva; BEGIN
#include "precompiled.h"
#pragma hdrstop
#include "../Game_local.h"
#include "../_CstStrings.h"

static const int NUM_VISIBLE_OPTIONS = 15;

/*
========================
idMenuScreen_Shell_CstAchievements::Initialize
========================
*/
void idMenuScreen_Shell_CstAchievements::Initialize(idMenuHandler * data) {
	idMenuScreen::Initialize(data);

	if (data != NULL) {
		menuGUI = data->GetGUI();
	}

	SetSpritePath("cstMenuAchievements");

	// add a dummy button for focusIndex 0
	idMenuWidget_Button *cstDummyButton = new (TAG_SWF) idMenuWidget_Button();
	AddChild(cstDummyButton);

	btnReset = new (TAG_SWF) idMenuWidget_Button();
	btnReset->Initialize(data);
	btnReset->SetLabel(CST_STR_00300368); // Reset Achievements
	btnReset->SetSpritePath(GetSpritePath(), "info", "btnReset");
	btnReset->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_JOY4_ON_PRESS);
	AddChild(btnReset);

	btnBack = new (TAG_SWF) idMenuWidget_Button();
	btnBack->Initialize(data);
	idMenuHandler_Shell * handler = dynamic_cast<idMenuHandler_Shell *>(data);
	btnBack->SetLabel((handler != NULL && handler->GetInGame()) ? "#str_swf_pause_menu" : "#str_02305");
	btnBack->SetSpritePath(GetSpritePath(), "info", "btnBack");
	btnBack->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_GO_BACK);
	AddChild(btnBack);

	options = new idMenuWidget_DynamicList();
	//options->SetIgnoreColor(true);
	options->CstSetForceAllowTooltip(true);
	options->SetNumVisibleOptions(NUM_VISIBLE_OPTIONS);
	options->SetSpritePath(GetSpritePath(), "info", "options");
	options->SetWrappingAllowed(true);

	scrollbar = new (TAG_SWF) idMenuWidget_ScrollBar();
	scrollbar->SetSpritePath(GetSpritePath(), "info", "scrollbar");
	scrollbar->RegisterEventObserver(this);
	scrollbar->Initialize(data);

	while (options->GetChildren().Num() < NUM_VISIBLE_OPTIONS) {
		idMenuWidget_Button * const buttonWidget = new (TAG_SWF) idMenuWidget_Button();
		buttonWidget->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_PRESS_FOCUSED, options->GetChildren().Num());
		buttonWidget->RegisterEventObserver(scrollbar);
		buttonWidget->Initialize(data);
		options->AddChild(buttonWidget);
	}
	options->Initialize(data);
	options->AddChild(scrollbar);
	AddChild(options);

	helpButtons.Clear();
	for (int i = 0; i < NUM_VISIBLE_OPTIONS; ++i) {
		idMenuWidget_Button *button = new (TAG_SWF) idMenuWidget_Button();
		button->Initialize(data);
		button->SetLabel(CST_STR_00300000);
		int actionID = NUM_VISIBLE_OPTIONS + i;
		button->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_PRESS_FOCUSED, actionID);
		button->SetSpritePath(GetSpritePath(), "info", "help", va("button%d", i));
		helpButtons.Append(button);
		AddChild(button);
	}

	AddEventAction(WIDGET_EVENT_SCROLL_DOWN_LSTICK).Set(new (TAG_SWF) idWidgetActionHandler(this, WIDGET_ACTION_EVENT_SCROLL_DOWN_START_REPEATER_VARIABLE, WIDGET_EVENT_SCROLL_DOWN_LSTICK));
	AddEventAction(WIDGET_EVENT_SCROLL_UP_LSTICK).Set(new (TAG_SWF) idWidgetActionHandler(this, WIDGET_ACTION_EVENT_SCROLL_UP_START_REPEATER_VARIABLE, WIDGET_EVENT_SCROLL_UP_LSTICK));
	AddEventAction(WIDGET_EVENT_SCROLL_DOWN_LSTICK_RELEASE).Set(new (TAG_SWF) idWidgetActionHandler(this, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_DOWN_LSTICK_RELEASE));
	AddEventAction(WIDGET_EVENT_SCROLL_UP_LSTICK_RELEASE).Set(new (TAG_SWF) idWidgetActionHandler(this, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_UP_LSTICK_RELEASE));

	AddEventAction(WIDGET_EVENT_SCROLL_DOWN).Set(new (TAG_SWF) idWidgetActionHandler(this, WIDGET_ACTION_EVENT_SCROLL_DOWN_START_REPEATER_VARIABLE, WIDGET_EVENT_SCROLL_DOWN));
	AddEventAction(WIDGET_EVENT_SCROLL_UP).Set(new (TAG_SWF) idWidgetActionHandler(this, WIDGET_ACTION_EVENT_SCROLL_UP_START_REPEATER_VARIABLE, WIDGET_EVENT_SCROLL_UP));
	AddEventAction(WIDGET_EVENT_SCROLL_DOWN_RELEASE).Set(new (TAG_SWF) idWidgetActionHandler(this, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_DOWN_RELEASE));
	AddEventAction(WIDGET_EVENT_SCROLL_UP_RELEASE).Set(new (TAG_SWF) idWidgetActionHandler(this, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_UP_RELEASE));
}

/*
========================
idMenuScreen_Shell_CstAchievements::Update
========================
*/
void idMenuScreen_Shell_CstAchievements::Update() {
	if (menuData != NULL) {
		idMenuWidget_CommandBar * cmdBar = menuData->GetCmdBar();
		if (cmdBar != NULL) {
			cmdBar->ClearAllButtons();
			idMenuWidget_CommandBar::buttonInfo_t * buttonInfo;
			buttonInfo = cmdBar->GetButton(idMenuWidget_CommandBar::BUTTON_JOY2);
			if (menuData->GetPlatform() != 2) {
				buttonInfo->label = "#str_00395";
			}
			buttonInfo->action.Set(WIDGET_ACTION_GO_BACK);

			bool hasHelpText = false;
			for (int i = 0; i < helpText.Num(); ++i) {
				if (!helpText[i].IsEmpty()) {
					hasHelpText = true;
					break;
				}
			}
			if (hasHelpText) {
				buttonInfo = cmdBar->GetButton(idMenuWidget_CommandBar::BUTTON_JOY3);
				if (menuData->GetPlatform() != 2) {
					buttonInfo->label = CST_STR_00300002;
				}
				buttonInfo->action.Set(WIDGET_ACTION_JOY3_ON_PRESS);
			}

			idMenuHandler_Shell * shell = dynamic_cast<idMenuHandler_Shell *>(menuData);
			if (shell && !shell->GetInGame()) {
				buttonInfo = cmdBar->GetButton(idMenuWidget_CommandBar::BUTTON_JOY4);
				if (menuData->GetPlatform() != 2) {
					buttonInfo->label = CST_STR_00300369;
				}
				buttonInfo->action.Set(WIDGET_ACTION_JOY4_ON_PRESS);
			}

			buttonInfo = cmdBar->GetButton(idMenuWidget_CommandBar::BUTTON_JOY1);
			buttonInfo->action.Set(WIDGET_ACTION_PRESS_FOCUSED);
		}
	}

	idSWFScriptObject & root = GetSWFObject()->GetRootObject();
	if (BindSprite(root)) {
		idSWFTextInstance * heading = GetSprite()->GetScriptObject()->GetNestedText("info", "txtHeading");
		if (heading != NULL) {
			heading->SetText(CST_STR_00300363);
			heading->SetStrokeInfo(true, 0.75f, 1.75f);
		}
		idSWFSpriteInstance * gradient = GetSprite()->GetScriptObject()->GetNestedSprite("info", "gradient");
		if (gradient != NULL && heading != NULL) {
			gradient->SetXPos(heading->GetTextLength());
		}
	}

	for (int i = 0; i < helpButtons.Num(); ++i) {
		if (helpButtons[i] != NULL) {
			helpButtons[i]->BindSprite(root);
		}
	}
	UpdateHelpButtons();

	if (btnReset != NULL) {
		btnReset->BindSprite(root);
		bool show = false;
		idMenuHandler_Shell * shell = dynamic_cast<idMenuHandler_Shell *>(menuData);
		if (menuData && menuData->GetPlatform() == 2 && shell && !shell->GetInGame()) {
			show = true;
		}
		if (!show && btnReset->GetSprite()) {
			btnReset->GetSprite()->SetVisible(false);
		}
	}
	if (btnBack != NULL) {
		btnBack->BindSprite(root);
	}

	idMenuScreen::Update();

	if (refreshIndices) {
		refreshIndices = false;
		if (resetView) {
			resetView = false;
			if (options != NULL) {
				options->SetViewOffset(0);
				options->SetViewIndex(0);
				options->SetFocusIndex(0);
			}
		} else {
			if (options != NULL) {
				options->SetViewOffset(options->GetViewOffset());
				options->SetViewIndex(options->GetViewIndex());
				options->SetFocusIndex(options->GetFocusIndex());
			}
		}
	}
}

/*
========================
idMenuScreen_Shell_CstAchievements::ShowScreen
========================
*/
void idMenuScreen_Shell_CstAchievements::ShowScreen(const mainMenuTransition_t transitionType) {
	UpdateAchievementsDisplay();
	refreshIndices = true;

	idMenuScreen::ShowScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_CstAchievements::HideScreen
========================
*/
void idMenuScreen_Shell_CstAchievements::HideScreen(const mainMenuTransition_t transitionType) {
	if (scrollbar && scrollbar->dragging) {
		scrollbar->dragging = false;
	}
	idMenuScreen::HideScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_CstAchievements::UpdateAchievementsDisplay
========================
*/
void idMenuScreen_Shell_CstAchievements::UpdateAchievementsDisplay() {
	if (options == NULL) {
		return;
	}

	idList<cstAchievementDetails_t> detailsList;
	idAchievementManager::CstGetAchievementsDetails(detailsList, true);

	if (detailsList.Num() == 0) {
		return;
	}

	idList<idList<idStr, TAG_IDLIB_LIST_MENU>, TAG_IDLIB_LIST_MENU> menuOptions;
	helpText.Clear();

	for (int i = 0; i < detailsList.Num(); ++i) {
		const cstAchievementDetails_t & details = detailsList[i];

		idList<idStr> option;

		// state
		idStr stateStr;
		if (details.state == 0) {
			stateStr = S_COLOR_ORANGE;
			stateStr += idLocalization::GetString(CST_STR_00300364); // locked
			option.Append(stateStr);
		} else if (details.state == 1) {
			stateStr = S_COLOR_GREEN;
			stateStr += idLocalization::GetString(CST_STR_00300365); // unlocked
			option.Append(stateStr);
		} else {
			stateStr = S_COLOR_RED;
			stateStr += idLocalization::GetString(CST_STR_00300168); // unknown
			option.Append(stateStr);
		}

		// progress
		idStr progressStr;
		if (details.count < 0) {
			progressStr.Format("%s/%d", idLocalization::GetString(CST_STR_00300000), details.required);
		} else {
			progressStr.Format("%d/%d", details.count, details.required);
		}
		option.Append(progressStr);

		// name
		if (details.name.IsEmpty()) {
			option.Append(CST_STR_00300168); // unknown
		} else {
			option.Append(details.name);
		}

		// help
		if (!details.name.IsEmpty() && !details.description.IsEmpty()) {
			idStr helpStr = idLocalization::GetString(details.name);
			helpStr += "\n\n";
			helpStr += idLocalization::GetString(details.description);
			if (details.gameDependent) {
				helpStr += "\n\n";
				helpStr += idLocalization::GetString(CST_STR_00300366);
			}
			helpText.Append(helpStr);
		} else {
			helpText.Append("");
		}

		menuOptions.Append(option);
	}

	options->SetListData(menuOptions);
}

/*
========================
idMenuScreen_Shell_CstAchievements::HandleAction
========================
*/
bool idMenuScreen_Shell_CstAchievements::HandleAction(idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled) {
	if (menuData == NULL) {
		return true;
	}
	if (menuData->ActiveScreen() != CST_SHELL_AREA_ACHIEVEMENTS) {
		return false;
	}

	widgetAction_t actionType = action.GetType();
	const idSWFParmList & parms = action.GetParms();

	switch (actionType) {
		case WIDGET_ACTION_GO_BACK: {
			menuData->SetNextScreen(SHELL_AREA_ROOT, MENU_TRANSITION_SIMPLE);
			return true;
		}
		case WIDGET_ACTION_JOY3_ON_PRESS: {
			if (options != NULL) {
				int buttonIndex = options->GetViewIndex() - options->GetViewOffset();
				HandleHelpButtonPressed(buttonIndex);
			}
			return true;
		}
		case WIDGET_ACTION_JOY4_ON_PRESS: {
			HandleResetAchievements();
			return true;
		}
		case WIDGET_ACTION_PRESS_FOCUSED: {
			if (parms.Num() > 0) {
				const int actionID = parms[0].ToInteger();
				const int firstHelpButtonID = NUM_VISIBLE_OPTIONS;
				const int pastLastID = firstHelpButtonID + helpButtons.Num();
				if (actionID >= firstHelpButtonID && actionID < pastLastID) {
					// help button pressed
					int buttonIndex = actionID - firstHelpButtonID;
					HandleHelpButtonPressed(buttonIndex);
					return true;
				}
			}

			if (options == NULL) {
				return true;
			}

			int listIndex = 0;
			if (parms.Num() > 0) {
				listIndex = options->GetViewOffset() + parms[0].ToInteger();
			} else {
				listIndex = options->GetViewIndex();
			}

			if (options->GetViewIndex() != listIndex) {
				options->SetViewIndex(listIndex);
				options->SetFocusIndex(listIndex - options->GetViewOffset());
			}
			return true;
		}
		case WIDGET_ACTION_SCROLL_VERTICAL_VARIABLE: {
			if (parms.Num() == 0 || options == NULL || (scrollbar && scrollbar->dragging)) {
				return true;
			}
			int scroll = parms[0].ToInteger();
			options->Scroll(scroll, true);
			UpdateHelpButtons();
			return true;
		}
	}

	return idMenuScreen::HandleAction(action, event, widget, forceHandled);
}

/*
========================
idMenuScreen_Shell_CstAchievements::ObserveEvent
========================
*/
void idMenuScreen_Shell_CstAchievements::ObserveEvent(const idMenuWidget &widget, const idWidgetEvent &event) {
	const idMenuWidget_ScrollBar *eventScrollbar = dynamic_cast<const idMenuWidget_ScrollBar*>(&widget);
	if (scrollbar != NULL && scrollbar == eventScrollbar && event.type == WIDGET_EVENT_DRAG_START) {
		UpdateHelpButtons();
		return;
	}

	idMenuScreen::ObserveEvent(widget, event);
}

/*
========================
idMenuScreen_Shell_CstAchievements::UpdateHelpButtons
========================
*/
void idMenuScreen_Shell_CstAchievements::UpdateHelpButtons() {
	for (int i = 0; i < helpButtons.Num(); ++i) {
		idMenuWidget_Button * const helpButton = helpButtons[i];
		if (!helpButton) {
			continue;
		}
		if (options == NULL) {
			helpButton->GetSprite()->SetVisible(false);
			continue;
		}
		int optionIndex = options->GetViewOffset() + i;
		if (optionIndex < helpText.Num() && optionIndex < options->GetTotalNumberOfOptions() && !helpText[optionIndex].IsEmpty()) {
			helpButton->GetSprite()->SetVisible(true);
		} else {
			helpButton->GetSprite()->SetVisible(false);
		}
	}
}

/*
========================
idMenuScreen_Shell_CstAchievements::HandleHelpButtonPressed
========================
*/
void idMenuScreen_Shell_CstAchievements::HandleHelpButtonPressed(int buttonIndex) {
	if (options == NULL || buttonIndex < 0 || buttonIndex >= helpButtons.Num()) {
		return;
	}
	int optionIndex = options->GetViewOffset() + buttonIndex;
	if (optionIndex < helpText.Num() && !helpText[optionIndex].IsEmpty()) {
		idMenuHandler_Shell *data = dynamic_cast<idMenuHandler_Shell *>(menuData);
		if (data != NULL) {
			data->cstHelpText = helpText[optionIndex];
			data->cstHelpGoBackScreen = CST_SHELL_AREA_ACHIEVEMENTS;
			menuData->SetNextScreen(CST_SHELL_AREA_HELP, MENU_TRANSITION_SIMPLE);
			if (options->GetViewIndex() != optionIndex) {
				options->SetViewIndex(optionIndex);
				options->SetFocusIndex(buttonIndex, true);
			}
		}
	}
}

/*
========================
idMenuScreen_Shell_CstAchievements::HandleResetAchievements
========================
*/
void idMenuScreen_Shell_CstAchievements::HandleResetAchievements() {
	// don't allow resetting achievements when in-game
	idMenuHandler_Shell * shell = dynamic_cast<idMenuHandler_Shell *>(menuData);
	if (!shell || shell->GetInGame()) {
		return;
	}

	// first confirmation dialog
	class idSWFScriptFunction_ResetAchievements : public idSWFScriptFunction_RefCounted {
	public:
		idSWFScriptFunction_ResetAchievements(idMenuScreen_Shell_CstAchievements * _menu, gameDialogMessages_t _msg, bool _reset) {
			menu = _menu;
			msg = _msg;
			reset = _reset;
		}
		idSWFScriptVar Call(idSWFScriptObject * thisObject, const idSWFParmList & parms) {
			common->Dialog().ClearDialog(msg);
			if (reset) {
				// second confirmation dialog
				class idSWFScriptFunction_ResetAchievements2 : public idSWFScriptFunction_RefCounted {
				public:
					idSWFScriptFunction_ResetAchievements2(idMenuScreen_Shell_CstAchievements * _menu, gameDialogMessages_t _msg, bool _reset) {
						menu = _menu;
						msg = _msg;
						reset = _reset;
					}
					idSWFScriptVar Call(idSWFScriptObject * thisObject, const idSWFParmList & parms) {
						common->Dialog().ClearDialog(msg);
						if (reset) {
							// reset all achievements
							cmdSystem->BufferCommandText(CMD_EXEC_NOW, "cstAchievementsReset all\n");
							if (menu) {
								menu->UpdateAchievementsDisplay();
								menu->Update();
							}
						}
						return idSWFScriptVar();
					}
				private:
					idMenuScreen_Shell_CstAchievements * menu;
					gameDialogMessages_t msg;
					bool reset;
				};
				idStaticList<idSWFScriptFunction *, 4> callbacks;
				idStaticList<idStrId, 4> optionText;
				callbacks.Append(new idSWFScriptFunction_ResetAchievements2(menu, CST_GDM_RESET_ACHIEVEMENTS_2, true));
				callbacks.Append(new idSWFScriptFunction_ResetAchievements2(menu, CST_GDM_RESET_ACHIEVEMENTS_2, false));
				optionText.Append(idStrId("#str_swf_accept"));
				optionText.Append(idStrId("#str_swf_cancel"));
				common->Dialog().AddDynamicDialog(CST_GDM_RESET_ACHIEVEMENTS_2, callbacks, optionText, true, CST_STR_00300371);
			}
			return idSWFScriptVar();
		}
	private:
		idMenuScreen_Shell_CstAchievements * menu;
		gameDialogMessages_t msg;
		bool reset;
	};
	idStaticList<idSWFScriptFunction *, 4> callbacks;
	idStaticList<idStrId, 4> optionText;
	callbacks.Append(new idSWFScriptFunction_ResetAchievements(this, CST_GDM_RESET_ACHIEVEMENTS, true));
	callbacks.Append(new idSWFScriptFunction_ResetAchievements(this, CST_GDM_RESET_ACHIEVEMENTS, false));
	optionText.Append(idStrId("#str_swf_accept"));
	optionText.Append(idStrId("#str_swf_cancel"));
	common->Dialog().AddDynamicDialog(CST_GDM_RESET_ACHIEVEMENTS, callbacks, optionText, true, CST_STR_00300370);
}
//#modified-fva; END
