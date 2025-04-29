//#modified-fva; BEGIN
#include "precompiled.h"
#pragma hdrstop
#include "../Game_local.h"
#include "../_CstStrings.h"

/*
========================
idMenuScreen_Shell_CstHelp::Initialize
========================
*/
void idMenuScreen_Shell_CstHelp::Initialize(idMenuHandler *data) {
	idMenuScreen::Initialize(data);

	if (data != NULL) {
		menuGUI = data->GetGUI();
	}

	SetSpritePath("cstMenuHelp");

	helpScrollbar = new (TAG_SWF) idMenuWidget_ScrollBar();
	helpScrollbar->SetSpritePath(GetSpritePath(), "info", "info", "scrollbar");
	helpScrollbar->Initialize(data);

	helpInfo = new (TAG_SWF) idMenuWidget_InfoBox();
	helpInfo->SetSpritePath(GetSpritePath(), "info");
	helpInfo->Initialize(data);
	helpInfo->SetScrollbar(helpScrollbar);
	helpInfo->AddChild(helpScrollbar);
	helpInfo->RegisterEventObserver(this);
	AddChild(helpInfo);

	btnBack = new (TAG_SWF) idMenuWidget_Button();
	btnBack->Initialize(data);
	btnBack->SetLabel(CST_STR_00300001);
	btnBack->SetSpritePath(GetSpritePath(), "info", "btnBack");
	btnBack->AddEventAction(WIDGET_EVENT_PRESS).Set(WIDGET_ACTION_GO_BACK);
	AddChild(btnBack);

	AddEventAction(WIDGET_EVENT_SCROLL_DOWN).Set(new (TAG_SWF) idWidgetActionHandler(this, WIDGET_ACTION_EVENT_SCROLL_DOWN_START_REPEATER, WIDGET_EVENT_SCROLL_DOWN));
	AddEventAction(WIDGET_EVENT_SCROLL_UP).Set(new (TAG_SWF) idWidgetActionHandler(this, WIDGET_ACTION_EVENT_SCROLL_UP_START_REPEATER, WIDGET_EVENT_SCROLL_UP));
	AddEventAction(WIDGET_EVENT_SCROLL_DOWN_RELEASE).Set(new (TAG_SWF) idWidgetActionHandler(this, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_DOWN_RELEASE));
	AddEventAction(WIDGET_EVENT_SCROLL_UP_RELEASE).Set(new (TAG_SWF) idWidgetActionHandler(this, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_UP_RELEASE));

	AddEventAction(WIDGET_EVENT_SCROLL_DOWN_LSTICK).Set(new (TAG_SWF) idWidgetActionHandler(this, WIDGET_ACTION_EVENT_SCROLL_DOWN_START_REPEATER, WIDGET_EVENT_SCROLL_DOWN_LSTICK));
	AddEventAction(WIDGET_EVENT_SCROLL_UP_LSTICK).Set(new (TAG_SWF) idWidgetActionHandler(this, WIDGET_ACTION_EVENT_SCROLL_UP_START_REPEATER, WIDGET_EVENT_SCROLL_UP_LSTICK));
	AddEventAction(WIDGET_EVENT_SCROLL_DOWN_LSTICK_RELEASE).Set(new (TAG_SWF) idWidgetActionHandler(this, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_DOWN_LSTICK_RELEASE));
	AddEventAction(WIDGET_EVENT_SCROLL_UP_LSTICK_RELEASE).Set(new (TAG_SWF) idWidgetActionHandler(this, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_UP_LSTICK_RELEASE));

	class CstRefreshHelp : public idSWFScriptFunction_RefCounted {
	public:
		CstRefreshHelp(idMenuWidget_InfoBox *_widget) :
			widget(_widget) {
		}
		idSWFScriptVar Call(idSWFScriptObject *thisObject, const idSWFParmList &parms) {
			if (widget != NULL) {
				widget->Update();
			}
			return idSWFScriptVar();
		}
	private:
		idMenuWidget_InfoBox *widget;
	};

	if (GetSWFObject() != NULL) {
		GetSWFObject()->SetGlobal("cstRefreshHelp", new (TAG_SWF) CstRefreshHelp(helpInfo));
	}
}

/*
========================
idMenuScreen_Shell_CstHelp::Update
========================
*/
void idMenuScreen_Shell_CstHelp::Update() {
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
		}
	}

	idSWFScriptObject &root = GetSWFObject()->GetRootObject();
	if (BindSprite(root)) {
		idSWFTextInstance * heading = GetSprite()->GetScriptObject()->GetNestedText("info", "txtHeading");
		if (heading != NULL) {
			heading->SetText(CST_STR_00300002);
			heading->SetStrokeInfo(true, 0.75f, 1.75f);
		}
	}

	if (btnBack != NULL) {
		btnBack->BindSprite(root);
	}

	idMenuScreen::Update();
}

/*
========================
idMenuScreen_Shell_CstHelp::ShowScreen
========================
*/
void idMenuScreen_Shell_CstHelp::ShowScreen(const mainMenuTransition_t transitionType) {
	if (menuData != NULL && menuGUI != NULL && helpInfo != NULL) {
		idSWFScriptObject &root = menuGUI->GetRootObject();
		helpInfo->BindSprite(root);
		idMenuHandler_Shell *data = dynamic_cast<idMenuHandler_Shell *>(menuData);
		if (data != NULL) {
			helpInfo->SetBody(data->cstHelpText);
		}
	}

	idMenuScreen::ShowScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_CstHelp::HideScreen
========================
*/
void idMenuScreen_Shell_CstHelp::HideScreen(const mainMenuTransition_t transitionType) {
	if (helpScrollbar && helpScrollbar->dragging) {
		helpScrollbar->dragging = false;
	}
	idMenuScreen::HideScreen(transitionType);
}

/*
========================
idMenuScreen_Shell_CstHelp::HandleAction
========================
*/
bool idMenuScreen_Shell_CstHelp::HandleAction(idWidgetAction &action, const idWidgetEvent &event, idMenuWidget *widget, bool forceHandled) {
	if (menuData == NULL) {
		return true;
	}

	if (menuData->ActiveScreen() != CST_SHELL_AREA_HELP) {
		return false;
	}

	widgetAction_t actionType = action.GetType();
	switch (actionType) {
		case WIDGET_ACTION_GO_BACK: {
			idMenuHandler_Shell *data = dynamic_cast<idMenuHandler_Shell *>(menuData);
			if (data != NULL && data->cstHelpGoBackScreen != SHELL_AREA_INVALID) {
				menuData->SetNextScreen(data->cstHelpGoBackScreen, MENU_TRANSITION_SIMPLE);
				data->cstHelpGoBackScreen = SHELL_AREA_INVALID;
			}
			return true;
		}
		case WIDGET_ACTION_SCROLL_VERTICAL: {
			if (helpInfo && helpScrollbar && !helpScrollbar->dragging) {
				helpInfo->HandleAction(action, event, helpInfo);
			}
			return true;
		}
	}

	return idMenuScreen::HandleAction(action, event, widget, forceHandled);
}
//#modified-fva; END
