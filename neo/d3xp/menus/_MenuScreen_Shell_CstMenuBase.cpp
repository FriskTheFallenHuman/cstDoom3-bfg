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
// code below is adapted from various MenuScreen_Shell_*.cpp files (with changes)

#include "precompiled.h"
#pragma hdrstop
#include "../Game_local.h"
#include "../_CstStrings.h"

/*
========================
idMenuScreen_Shell_CstMenuBase::idMenuScreen_Shell_CstMenuBase
========================
*/
idMenuScreen_Shell_CstMenuBase::idMenuScreen_Shell_CstMenuBase() :
	btnBack(NULL),
	useSmallHeading(false),
	btnBackLabelToUpper(false),
	headingLabelToUpper(false),
	showJoySelect(false),
	options(NULL),
	numVisibleOptions(0),
	menuSpriteName(NULL),
	btnBackLabel(NULL),
	headingLabel(NULL),
	shellArea(SHELL_AREA_INVALID),
	goBackShellArea(SHELL_AREA_INVALID) {
}

/*
========================
idMenuScreen_Shell_CstMenuBase::Initialize
========================
*/
void idMenuScreen_Shell_CstMenuBase::Initialize(idMenuHandler * data) {
	idMenuScreen::Initialize(data);

	if (data != NULL) {
		menuGUI = data->GetGUI();
	}

	SetSpritePath(menuSpriteName);

	if (options) {
		if (menuOptions.Num() > 0) {
			options->SetListData(menuOptions);
		}
		options->SetNumVisibleOptions(numVisibleOptions);
		options->SetSpritePath(GetSpritePath(), "info", "options");
		options->SetWrappingAllowed(true);
		if (menuOptions.Num() == 0) {
			options->SetControlList(true);
		}
		options->Initialize(data);
		AddChild(options);
	}

	idMenuWidget_Help * const helpWidget = new (TAG_SWF) idMenuWidget_Help();
	helpWidget->SetSpritePath(GetSpritePath(), "info", "helpTooltip");
	AddChild(helpWidget);

	if (options) {
		for (int i = 0; i < options->GetChildren().Num(); ++i) {
			idMenuWidget_Button *button = dynamic_cast<idMenuWidget_Button *>(&options->GetChildByIndex(i));
			if (button) {
				button->RegisterEventObserver(helpWidget);
			}
		}
	}

	btnBack = new (TAG_SWF) idMenuWidget_Button();
	btnBack->Initialize(data);
	if (btnBackLabelToUpper) {
		idStr label(idLocalization::GetString(btnBackLabel));
		label.CstToUpperUTF8_Partial();
		btnBack->SetLabel(label);
	} else {
		btnBack->SetLabel(btnBackLabel);
	}
	btnBack->SetSpritePath(GetSpritePath(), "info", "btnBack");
	btnBack->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_GO_BACK);
	AddChild(btnBack);

	helpButtons.Clear();
	for (int i = 0; i < numVisibleOptions; ++i) {
		idMenuWidget_Button *button = new (TAG_SWF) idMenuWidget_Button();
		button->Initialize(data);
		button->SetLabel(CST_STR_00300000);
		int actionID = i;
		button->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_PRESS_FOCUSED, actionID);
		button->SetSpritePath(GetSpritePath(), "info", "help", va("button%d", i));
		helpButtons.Append(button);
		AddChild(button);
	}

	if (options) {
		options->AddEventAction(WIDGET_EVENT_SCROLL_DOWN).Set(new (TAG_SWF) idWidgetActionHandler(options, WIDGET_ACTION_EVENT_SCROLL_DOWN_START_REPEATER, WIDGET_EVENT_SCROLL_DOWN));
		options->AddEventAction(WIDGET_EVENT_SCROLL_UP).Set(new (TAG_SWF) idWidgetActionHandler(options, WIDGET_ACTION_EVENT_SCROLL_UP_START_REPEATER, WIDGET_EVENT_SCROLL_UP));
		options->AddEventAction(WIDGET_EVENT_SCROLL_DOWN_RELEASE).Set(new (TAG_SWF) idWidgetActionHandler(options, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_DOWN_RELEASE));
		options->AddEventAction(WIDGET_EVENT_SCROLL_UP_RELEASE).Set(new (TAG_SWF) idWidgetActionHandler(options, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_UP_RELEASE));
		options->AddEventAction(WIDGET_EVENT_SCROLL_DOWN_LSTICK).Set(new (TAG_SWF) idWidgetActionHandler(options, WIDGET_ACTION_EVENT_SCROLL_DOWN_START_REPEATER, WIDGET_EVENT_SCROLL_DOWN_LSTICK));
		options->AddEventAction(WIDGET_EVENT_SCROLL_UP_LSTICK).Set(new (TAG_SWF) idWidgetActionHandler(options, WIDGET_ACTION_EVENT_SCROLL_UP_START_REPEATER, WIDGET_EVENT_SCROLL_UP_LSTICK));
		options->AddEventAction(WIDGET_EVENT_SCROLL_DOWN_LSTICK_RELEASE).Set(new (TAG_SWF) idWidgetActionHandler(options, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_DOWN_LSTICK_RELEASE));
		options->AddEventAction(WIDGET_EVENT_SCROLL_UP_LSTICK_RELEASE).Set(new (TAG_SWF) idWidgetActionHandler(options, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_UP_LSTICK_RELEASE));
	}
}

/*
========================
idMenuScreen_Shell_CstMenuBase::Update
========================
*/
void idMenuScreen_Shell_CstMenuBase::Update() {
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
				if (helpText[i] && helpText[i][0] != '\0') {
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
			if (showJoySelect && menuData->GetPlatform() != 2) {
				buttonInfo->label = "#str_SWF_SELECT";
			}
			buttonInfo->action.Set(WIDGET_ACTION_PRESS_FOCUSED);
		}
	}

	idSWFScriptObject & root = GetSWFObject()->GetRootObject();
	if (BindSprite(root)) {
		const char * headingID = useSmallHeading ? "txtHeadingSmall" : "txtHeading";
		idSWFTextInstance * heading = GetSprite()->GetScriptObject()->GetNestedText("info", headingID);
		if (heading != NULL) {
			if (headingLabelToUpper) {
				idStr label(idLocalization::GetString(headingLabel));
				label.CstToUpperUTF8_Partial();
				heading->SetText(label);
			} else {
				heading->SetText(headingLabel);
			}
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

	if (btnBack != NULL) {
		btnBack->BindSprite(root);
	}

	idMenuScreen::Update();
}

/*
========================
idMenuScreen_Shell_CstMenuBase::HandleAction
========================
*/
bool idMenuScreen_Shell_CstMenuBase::HandleAction(idWidgetAction & action, const idWidgetEvent & event, idMenuWidget * widget, bool forceHandled) {
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
				const int buttonIndex = parms[0].ToInteger();
				HandleHelpButtonPressed(buttonIndex);
				return true;
			}
			break;
		}
		case WIDGET_ACTION_COMMAND: {
			if (options == NULL) {
				return true;
			}
			int selectionIndex = options->GetFocusIndex();
			if (parms.Num() > 0) {
				selectionIndex = parms[0].ToInteger();
			}
			if (selectionIndex != options->GetFocusIndex()) {
				options->SetViewIndex(options->GetViewOffset() + selectionIndex);
				options->SetFocusIndex(selectionIndex);
			}
			return true;
		}
		case WIDGET_ACTION_START_REPEATER: {
			if (options == NULL) {
				return true;
			}
			if (parms.Num() == 4) {
				int selectionIndex = parms[3].ToInteger();
				if (selectionIndex != options->GetFocusIndex()) {
					options->SetViewIndex(options->GetViewOffset() + selectionIndex);
					options->SetFocusIndex(selectionIndex);
				}
			}
			break;
		}
	}

	return idMenuScreen::HandleAction(action, event, widget, forceHandled);
}

/*
========================
idMenuScreen_Shell_CstMenuBase::UpdateHelpButtons
========================
*/
void idMenuScreen_Shell_CstMenuBase::UpdateHelpButtons() {
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
		if (optionIndex < helpText.Num() && optionIndex < options->GetTotalNumberOfOptions() && helpText[optionIndex] != NULL && helpText[optionIndex][0] != '\0') {
			helpButton->GetSprite()->SetVisible(true);
		} else {
			helpButton->GetSprite()->SetVisible(false);
		}
	}
}

/*
========================
idMenuScreen_Shell_CstMenuBase::HandleHelpButtonPressed
========================
*/
void idMenuScreen_Shell_CstMenuBase::HandleHelpButtonPressed(int buttonIndex) {
	if (options == NULL || buttonIndex < 0 || buttonIndex >= helpButtons.Num()) {
		return;
	}
	int optionIndex = options->GetViewOffset() + buttonIndex;
	if (optionIndex < helpText.Num() && helpText[optionIndex] != NULL && helpText[optionIndex][0] != '\0') {
		idMenuHandler_Shell *data = dynamic_cast<idMenuHandler_Shell *>(menuData);
		if (data != NULL) {
			data->cstHelpText = helpText[optionIndex];
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
================================================
namespace CstMenuUtils
================================================
*/

/*
========================
CstMenuUtils::ReadPseudoBit
========================
*/
bool CstMenuUtils::ReadPseudoBit(const idStr &str, int pseudoBit) {
	if (pseudoBit < 0) {
		return false;
	}

	if (str.IsEmpty()) {
		// implicit zeros when empty
		return false;
	}
	int lastCharIndex = str.Length() - 1;
	int index = lastCharIndex - pseudoBit;
	if (index < 0) {
		// implicit zeros to the left
		return false;
	}
	if (str[index] == '1') {
		return true;
	}
	return false;
}

/*
========================
CstMenuUtils::WritePseudoBit
========================
*/
void CstMenuUtils::WritePseudoBit(idStr &str, int pseudoBit, bool value) {
	if (pseudoBit < 0) {
		return;
	}

	int lastCharIndex = str.Length() - 1;
	int index = lastCharIndex - pseudoBit;

	if (index < 0) {
		if (value) {
			idStr outString = "1";
			int missingZeros = -index - 1;
			for (int i = 0; i < missingZeros; i++) {
				outString += "0";
			}
			outString += str;
			str = outString;
			return;
		} else {
			// nothing to do
			return;
		}
	}

	if (value) {
		str[index] = '1';
	} else {
		str[index] = '0';
	}
	int zerosToTheLeft = 0;
	for (int i = 0; i < str.Length(); ++i) {
		if (str[i] == '1') {
			break;
		} else {
			zerosToTheLeft++;
		}
	}
	if (zerosToTheLeft) {
		if (zerosToTheLeft == str.Length()) {
			str = "";
			return;
		}
		idStr outString(str, zerosToTheLeft, str.Length());
		str = outString;
	}
}

/*
========================
CstMenuUtils::AdjustOption - Code is from MenuScreen_Shell_SystemOptions.cpp
Given a current value in an array of possible values, returns the next n value
========================
*/
int CstMenuUtils::AdjustOption(int currentValue, const int values[], int numValues, int adjustment) {
	int index = 0;
	for (int i = 0; i < numValues; i++) {
		if (currentValue == values[i]) {
			index = i;
			break;
		}
	}
	index += adjustment;
	while (index < 0) {
		index += numValues;
	}
	index %= numValues;
	return values[index];
}

/*
========================
CstMenuUtils::LinearAdjust - Code is from MenuScreen_Shell_SystemOptions.cpp
Linearly converts a float from one scale to another
========================
*/
float CstMenuUtils::LinearAdjust(float input, float currentMin, float currentMax, float desiredMin, float desiredMax) {
	return ((input - currentMin) / (currentMax - currentMin)) * (desiredMax - desiredMin) + desiredMin;
}
//#modified-fva; END
