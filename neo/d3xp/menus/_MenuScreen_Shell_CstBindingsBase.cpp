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
// code below is from MenuScreen_Shell_Bindings.cpp (with changes)

#include "precompiled.h"
#pragma hdrstop
#include "../Game_local.h"
#include "../_CstStrings.h"

const int idMenuScreen_Shell_CstBindingsBase::NUM_BIND_LISTINGS = 12;
const int idMenuScreen_Shell_CstBindingsBase::NUM_LAYERS_MENU = 10;

/*
========================
idMenuScreen_Shell_CstBindingsBase::idMenuScreen_Shell_CstBindingsBase
========================
*/
idMenuScreen_Shell_CstBindingsBase::idMenuScreen_Shell_CstBindingsBase() :
	scrollbar(NULL),
	currentLayer(0),
	refreshIndices(false),
	resetView(true),
	options(NULL),
	blinder(NULL),
	txtBlinder(NULL),
	btnBack(NULL),
	bindingsChanged(false),
	keyboardBinds(NULL),
	numBinds(0),
	bindingsDomain(0),
	menuSpriteName(NULL),
	shellArea(SHELL_AREA_INVALID),
	headingLabel(NULL),
	goBackShellArea(SHELL_AREA_INVALID),
	goBackLabel(NULL) {
}

/*
========================
idMenuScreen_Shell_CstBindingsBase::Initialize
========================
*/
void idMenuScreen_Shell_CstBindingsBase::Initialize(idMenuHandler * data) {
	idMenuScreen::Initialize( data );

	if ( data != NULL ) {
		menuGUI = data->GetGUI();
	}

	SetSpritePath(menuSpriteName);

	// add a dummy button for focusIndex 0
	idMenuWidget_Button *cstDummyButton = new (TAG_SWF) idMenuWidget_Button();
	AddChild(cstDummyButton);

	btnBack = new (TAG_SWF) idMenuWidget_Button();
	btnBack->Initialize(data);
	idStr controls(idLocalization::GetString(goBackLabel));
	controls.CstToUpperUTF8_Partial();
	btnBack->SetLabel(controls);
	btnBack->SetSpritePath(GetSpritePath(), "info", "btnBack");
	btnBack->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_GO_BACK);
	AddChild(btnBack);

	options = new idMenuWidget_DynamicList();
	//options->SetIgnoreColor(true);
	options->CstSetForceAllowTooltip(true);
	options->SetNumVisibleOptions(NUM_BIND_LISTINGS);
	options->SetSpritePath(GetSpritePath(), "info", "options");
	options->SetWrappingAllowed(true);

	scrollbar = new (TAG_SWF) idMenuWidget_ScrollBar();
	scrollbar->SetSpritePath(GetSpritePath(), "info", "scrollbar");
	scrollbar->RegisterEventObserver(this);
	scrollbar->Initialize(data);

	while (options->GetChildren().Num() < NUM_BIND_LISTINGS) {
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
	for (int i = 0; i < NUM_BIND_LISTINGS; ++i) {
		idMenuWidget_Button *button = new (TAG_SWF) idMenuWidget_Button();
		button->Initialize(data);
		button->SetLabel(CST_STR_00300000);
		int actionID = NUM_BIND_LISTINGS + i;
		button->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_PRESS_FOCUSED, actionID);
		button->SetSpritePath(GetSpritePath(), "info", "help", va("button%d", i));
		helpButtons.Append(button);
		AddChild(button);
	}

	layerButtons.Clear();
	for (int i = 0; i < NUM_LAYERS_MENU; ++i) {
		idMenuWidget_Button *button = new (TAG_SWF) idMenuWidget_Button();
		button->Initialize(data);
		idStr label;
		switch (i) {
			case 0: label = CST_STR_00300005; break;
			case 1: label = CST_STR_BINDINGS_LAYER_1_LABEL; break;
			case 2: label = CST_STR_BINDINGS_LAYER_2_LABEL; break;
			case 3: label = CST_STR_BINDINGS_LAYER_3_LABEL; break;
			case 4: label = CST_STR_BINDINGS_LAYER_4_LABEL; break;
			case 5: label = CST_STR_BINDINGS_LAYER_5_LABEL; break;
			case 6: label = CST_STR_BINDINGS_LAYER_6_LABEL; break;
			case 7: label = CST_STR_BINDINGS_LAYER_7_LABEL; break;
			case 8: label = CST_STR_BINDINGS_LAYER_8_LABEL; break;
			case 9: label = CST_STR_BINDINGS_LAYER_9_LABEL; break;
			default: label = "-"; break; // should never happen
		}
		button->SetLabel(label);
		int actionID = NUM_BIND_LISTINGS + helpButtons.Num() + i;
		button->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_PRESS_FOCUSED, actionID);
		button->SetSpritePath(GetSpritePath(), "info", "layers", va("button%d", i));
		layerButtons.Append(button);
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

	AddEventAction(WIDGET_EVENT_SCROLL_LEFT_LSTICK).Set(new (TAG_SWF) idWidgetActionHandler(this, WIDGET_ACTION_EVENT_SCROLL_LEFT_START_REPEATER, WIDGET_EVENT_SCROLL_LEFT_LSTICK));
	AddEventAction(WIDGET_EVENT_SCROLL_RIGHT_LSTICK).Set(new (TAG_SWF) idWidgetActionHandler(this, WIDGET_ACTION_EVENT_SCROLL_RIGHT_START_REPEATER, WIDGET_EVENT_SCROLL_RIGHT_LSTICK));
	AddEventAction(WIDGET_EVENT_SCROLL_LEFT_LSTICK_RELEASE).Set(new (TAG_SWF) idWidgetActionHandler(this, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_LEFT_LSTICK_RELEASE));
	AddEventAction(WIDGET_EVENT_SCROLL_RIGHT_LSTICK_RELEASE).Set(new (TAG_SWF) idWidgetActionHandler(this, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_RIGHT_LSTICK_RELEASE));

	AddEventAction(WIDGET_EVENT_SCROLL_LEFT).Set(new (TAG_SWF) idWidgetActionHandler(this, WIDGET_ACTION_EVENT_SCROLL_LEFT_START_REPEATER, WIDGET_EVENT_SCROLL_LEFT));
	AddEventAction(WIDGET_EVENT_SCROLL_RIGHT).Set(new (TAG_SWF) idWidgetActionHandler(this, WIDGET_ACTION_EVENT_SCROLL_RIGHT_START_REPEATER, WIDGET_EVENT_SCROLL_RIGHT));
	AddEventAction(WIDGET_EVENT_SCROLL_LEFT_RELEASE).Set(new (TAG_SWF) idWidgetActionHandler(this, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_LEFT_RELEASE));
	AddEventAction(WIDGET_EVENT_SCROLL_RIGHT_RELEASE).Set(new (TAG_SWF) idWidgetActionHandler(this, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_RIGHT_RELEASE));
}

/*
========================
idMenuScreen_Shell_CstBindingsBase::Update
========================
*/
void idMenuScreen_Shell_CstBindingsBase::Update() {
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
			for (int i = 0; i < numBinds; ++i) {
				if (keyboardBinds[i].helpText != NULL && keyboardBinds[i].helpText[0] != '\0') {
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

			buttonInfo = cmdBar->GetButton(idMenuWidget_CommandBar::BUTTON_JOY1);
			buttonInfo->action.Set(WIDGET_ACTION_PRESS_FOCUSED);
		}
	}

	idSWFScriptObject & root = GetSWFObject()->GetRootObject();
	if (BindSprite(root)) {
		idSWFTextInstance * heading = GetSprite()->GetScriptObject()->GetNestedText("info", "txtHeading");
		if (heading != NULL) {
			heading->SetText(headingLabel);
			heading->SetStrokeInfo(true, 0.75f, 1.75f);
		}
		idSWFTextInstance *layersTitle = GetSprite()->GetScriptObject()->GetNestedText("info", "layers", "title");
		if (layersTitle != NULL) {
			layersTitle->SetText(CST_STR_00300004);
			layersTitle->SetStrokeInfo(true, 0.75f, 2.0f);
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

	for (int i = 0; i < layerButtons.Num(); ++i) {
		if (layerButtons[i] != NULL) {
			layerButtons[i]->BindSprite(root);
			if (currentLayer == i) {
				layerButtons[i]->SetState(WIDGET_STATE_SELECTING);
			} else {
				layerButtons[i]->SetState(WIDGET_STATE_NORMAL);
			}
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
				options->SetViewIndex(1);
				options->SetFocusIndex(1);
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
idMenuScreen_Shell_CstBindingsBase::ShowScreen
========================
*/
void idMenuScreen_Shell_CstBindingsBase::ShowScreen(const mainMenuTransition_t transitionType) {
	UpdateBindingDisplay();
	refreshIndices = true;

	if (menuData != NULL) {
		menuGUI = menuData->GetGUI();
		if (menuGUI != NULL) {
			idSWFScriptObject & root = menuGUI->GetRootObject();
			txtBlinder = root.GetNestedSprite(menuSpriteName, "info", "rebind");
			blinder = root.GetNestedSprite(menuSpriteName, "info", "blinder");
		}
	}

	ToggleWait(false);
	idMenuScreen::ShowScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_CstBindingsBase::HideScreen
========================
*/
void idMenuScreen_Shell_CstBindingsBase::HideScreen(const mainMenuTransition_t transitionType) {
	if (scrollbar && scrollbar->dragging) {
		scrollbar->dragging = false;
	}
	if (bindingsChanged) {
		cvarSystem->SetModifiedFlags(CVAR_ARCHIVE);
		bindingsChanged = false;
	}
	idMenuScreen::HideScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_CstBindingsBase::UpdateBindingDisplay
========================
*/
void idMenuScreen_Shell_CstBindingsBase::UpdateBindingDisplay() {
	if (options == NULL) {
		return;
	}

	extern idCVar in_useJoystick;
	idList< idList<idStr, TAG_IDLIB_LIST_MENU>, TAG_IDLIB_LIST_MENU> bindList;

	for (int i = 0; i < numBinds; ++i) {
		idList<idStr> option;

		bool isHeading = idStr::Icmp(keyboardBinds[i].bind, "") == 0;
		if (isHeading) {
			idStr display = S_COLOR_ORANGE;
			display += idLocalization::GetString(keyboardBinds[i].display);
			display += S_COLOR_DEFAULT;
			option.Append(display.c_str());
		} else {
			option.Append(keyboardBinds[i].display);
		}

		if (idStr::Cmp(keyboardBinds[i].bind, "cst_keyPdaScoreClose") == 0) {
			const char *keyName = cst_keyPdaScoreClose.GetString();
			if (keyName[0] != '\0') {
				keyNum_t keyNum = idKeyInput::StringToKeyNum(keyName);
				idStr localizedkeyName = idKeyInput::LocalizedKeyName(keyNum);
				localizedkeyName.CstToUpperUTF8_Partial();
				option.Append(localizedkeyName);
			} else {
				option.Append("");
			}
			bindList.Append(option);
			continue;
		}

		if (!isHeading) {
			keyBindings_t bind = idKeyInput::KeyBindingsFromBinding(keyboardBinds[i].bind, currentLayer, bindingsDomain, false, true);
			idStr bindings;

			if (!bind.gamepad.IsEmpty() && in_useJoystick.GetBool()) {
				idStrList joyBinds;
				int start = 0;
				while (start < bind.gamepad.Length()) {
					int end = bind.gamepad.Find(", ", true, start);
					if (end < 0) {
						end = bind.gamepad.Length();
					}
					joyBinds.Alloc().CopyRange(bind.gamepad, start, end);
					start = end + 2;
				}
				const char * buttonsWithImages[] = {
					"JOY1", "JOY2", "JOY3", "JOY4", "JOY5", "JOY6",
					"JOY_TRIGGER1", "JOY_TRIGGER2", 0
				};
				for (int i = 0; i < joyBinds.Num(); i++) {
					if (joyBinds[i].Icmpn("JOY_STICK", 9) == 0) {
						continue; // Can't rebind the sticks, so don't even show them
					}
					bool hasImage = false;
					for (const char ** b = buttonsWithImages; *b != 0; b++) {
						if (joyBinds[i].Icmp(*b) == 0) {
							hasImage = true;
							break;
						}
					}
					if (!bindings.IsEmpty()) {
						bindings.Append(", ");
					}
					if (hasImage) {
						bindings.Append('<');
						bindings.Append(joyBinds[i]);
						bindings.Append('>');
					} else {
						bindings.Append(joyBinds[i]);
					}
				}
				bindings.Replace("JOY_DPAD", "DPAD");
			}

			if (!bind.keyboard.IsEmpty()) {
				if (!bindings.IsEmpty()) {
					bindings.Append(", ");
				}
				bindings.Append(bind.keyboard);
			}

			if (!bind.mouse.IsEmpty()) {
				if (!bindings.IsEmpty()) {
					bindings.Append(", ");
				}
				bindings.Append(bind.mouse);
			}

			bindings.CstToUpperUTF8_Partial();
			option.Append(bindings);

		} else {
			option.Append("");
		}

		bindList.Append(option);
	}

	options->SetListData(bindList);
}

/*
========================
idMenuScreen_Shell_CstBindingsBase::ToggleWait
========================
*/
void idMenuScreen_Shell_CstBindingsBase::ToggleWait(bool wait) {
	if (wait) {
		if (blinder != NULL) {
			blinder->SetVisible(true);
			if (options != NULL) {
				blinder->StopFrame(options->GetFocusIndex() + 1);
			}
		}
		if (txtBlinder != NULL) {
			txtBlinder->SetVisible(true);
		}
	} else {
		if (blinder != NULL) {
			blinder->SetVisible(false);
		}
		if (txtBlinder != NULL) {
			txtBlinder->SetVisible(false);
		}
	}
}

/*
========================
idMenuScreen_Shell_CstBindingsBase::HandleAction
========================
*/
bool idMenuScreen_Shell_CstBindingsBase::HandleAction(idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled) {
	if (menuData == NULL) {
		return true;
	}
	if (menuData->ActiveScreen() != shellArea) {
		return false;
	}

	widgetAction_t actionType = action.GetType();
	const idSWFParmList & parms = action.GetParms();

	switch (actionType) {
		case WIDGET_ACTION_GO_BACK: {
			menuData->SetNextScreen(goBackShellArea, MENU_TRANSITION_SIMPLE);
			return true;
		}
		case WIDGET_ACTION_JOY3_ON_PRESS: {
			if (options != NULL) {
				int buttonIndex = options->GetViewIndex() - options->GetViewOffset();
				HandleHelpButtonPressed(buttonIndex);
			}
			return true;
		}
		case WIDGET_ACTION_PRESS_FOCUSED: {
			if (parms.Num() > 0) {
				const int actionID = parms[0].ToInteger();
				const int firstHelpButtonID = NUM_BIND_LISTINGS;
				const int firstLayerButtonID = firstHelpButtonID + helpButtons.Num();
				const int pastLastID = firstLayerButtonID + layerButtons.Num();
				if (actionID >= firstHelpButtonID && actionID < pastLastID) {
					if (actionID < firstLayerButtonID) {
						// help button pressed
						int buttonIndex = actionID - firstHelpButtonID;
						HandleHelpButtonPressed(buttonIndex);
					} else {
						// layer button pressed
						int buttonIndex = actionID - firstLayerButtonID;
						HandleLayerButtonPressed(buttonIndex);
					}
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

			if (listIndex < 0 || listIndex >= numBinds) {
				return true;
			}

			// moved from below
			if (idStr::Icmp(keyboardBinds[listIndex].bind, "") == 0) {
				return true;
			}

			if (options->GetViewIndex() != listIndex) {
				//if (idStr::Icmp(keyboardBinds[listIndex].bind, "") == 0) {
				//	return true;
				//}
				options->SetViewIndex(listIndex);
				options->SetFocusIndex(listIndex - options->GetViewOffset());
			} else {
				idMenuHandler_Shell * data = dynamic_cast<idMenuHandler_Shell *>(menuData);
				if (data != NULL) {
					ToggleWait(true);
					Update();
					data->SetWaitForBinding(keyboardBinds[listIndex].bind, shellArea);
				}
			}
			return true;
		}
		case WIDGET_ACTION_SCROLL_VERTICAL_VARIABLE: {
			if (parms.Num() == 0 || options == NULL || (scrollbar && scrollbar->dragging)) {
				return true;
			}
			int dir = parms[0].ToInteger();
			int scroll = 0;
			int curIndex = options->GetViewIndex();
			if (dir != 0) {
				if (curIndex + dir >= numBinds) {
					scroll = dir * 2;
				} else if (curIndex + dir < 1) {
					scroll = dir * 2;
				} else {
					if (idStr::Icmp(keyboardBinds[curIndex + dir].bind, "") == 0) {
						scroll = dir * 2;
					} else {
						scroll = dir;
					}
				}
			}
			options->Scroll(scroll, true);
			UpdateHelpButtons();
			return true;
		}
		case WIDGET_ACTION_SCROLL_HORIZONTAL: {
			if (parms.Num() == 0) {
				return true;
			}
			int direction = parms[0].ToInteger();
			int buttonIndex = currentLayer;
			if (direction >= 0) {
				++buttonIndex;
				if (buttonIndex >= NUM_LAYERS_MENU) {
					buttonIndex = 0;
				}
			} else {
				--buttonIndex;
				if (buttonIndex < 0) {
					buttonIndex = NUM_LAYERS_MENU - 1;
				}
			}
			// the button wasn't actually clicked, so play the sound here
			if (menuData != NULL) {
				menuData->PlaySound(GUI_SOUND_ADVANCE);
			}
			HandleLayerButtonPressed(buttonIndex);
			return true;
		}
	}

	return idMenuScreen::HandleAction(action, event, widget, forceHandled);
}

/*
========================
idMenuScreen_Shell_CstBindingsBase::ObserveEvent
========================
*/
void idMenuScreen_Shell_CstBindingsBase::ObserveEvent(const idMenuWidget &widget, const idWidgetEvent &event) {
	const idMenuWidget_ScrollBar *eventScrollbar = dynamic_cast<const idMenuWidget_ScrollBar*>(&widget);
	if (scrollbar != NULL && scrollbar == eventScrollbar && event.type == WIDGET_EVENT_DRAG_START) {
		UpdateHelpButtons();
		return;
	}

	idMenuScreen::ObserveEvent(widget, event);
}

/*
========================
idMenuScreen_Shell_CstBindingsBase::UpdateHelpButtons
========================
*/
void idMenuScreen_Shell_CstBindingsBase::UpdateHelpButtons() {
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
		if (optionIndex < numBinds && optionIndex < options->GetTotalNumberOfOptions() && keyboardBinds[optionIndex].helpText != NULL && keyboardBinds[optionIndex].helpText[0] != '\0') {
			helpButton->GetSprite()->SetVisible(true);
		} else {
			helpButton->GetSprite()->SetVisible(false);
		}
	}
}

/*
========================
idMenuScreen_Shell_CstBindingsBase::HandleHelpButtonPressed
========================
*/
void idMenuScreen_Shell_CstBindingsBase::HandleHelpButtonPressed(int buttonIndex) {
	if (options == NULL || buttonIndex < 0 || buttonIndex >= helpButtons.Num()) {
		return;
	}
	int optionIndex = options->GetViewOffset() + buttonIndex;
	if (keyboardBinds[optionIndex].helpText != NULL) {
		idMenuHandler_Shell *data = dynamic_cast<idMenuHandler_Shell *>(menuData);
		if (data != NULL) {
			data->cstHelpText = keyboardBinds[optionIndex].helpText;
			data->cstHelpGoBackScreen = shellArea;
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
idMenuScreen_Shell_CstBindingsBase::HandleLayerButtonPressed
========================
*/
void idMenuScreen_Shell_CstBindingsBase::HandleLayerButtonPressed(int buttonIndex) {
	if (buttonIndex < 0 || buttonIndex >= layerButtons.Num()) {
		return;
	}
	const int &selectedLayer = buttonIndex;
	if (selectedLayer != currentLayer) {
		currentLayer = selectedLayer;
		UpdateBindingDisplay();
		Update();
	}
}
//#modified-fva; END
