/*
 * Copyright 2010-2017, Haiku, Inc. All Rights Reserved.
 * Copyright 2009, Pier Luigi Fiorini.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 *		Brian Hill, supernova@tycho.email
 */

#include <Catalog.h>
#include <CardLayout.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <LayoutItem.h>
#include <Message.h>
#include <Window.h>

//#include "DisplayView.h"
#include "GeneralView.h"
#include "HistoryView.h"
#include "NotificationsView.h"
#include "PrefletView.h"
#include "SettingsHost.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrefletView"


//const int32 kPageSelected = '_LCH';


PrefletView::PrefletView(SettingsHost* host)
	:
	BTabView("pages", B_WIDTH_FROM_WIDEST)
{
	// Pages
	GeneralView* general = new GeneralView(host);
//	DisplayView* display = new DisplayView(host);
	NotificationsView* apps = new NotificationsView(host);
//	HistoryView* history = new HistoryView();

	// Page selector
	BTab* tab = new BTab();
	AddTab(general, tab);
	tab->SetLabel(B_TRANSLATE("Settings"));

/*	tab = new BTab();
	AddTab(display, tab);
	tab->SetLabel(B_TRANSLATE("Display"));*/

	tab = new BTab();
	AddTab(apps, tab);
	tab->SetLabel(B_TRANSLATE("Applications"));
	
/*	tab = new BTab();
	AddTab(history, tab);
	tab->SetLabel(B_TRANSLATE("History"));*/
}


BView*
PrefletView::CurrentPage()
{
	return PageAt(FocusTab());
}


int32
PrefletView::CountPages() const
{
	return 2;
}


BView*
PrefletView::PageAt(int32 index)
{
	return TabAt(index)->View();
}



void
PrefletView::Select(int32 index)
{
	if (index == Selection())
		return;

	BTabView::Select(index);

	SettingsPane* pane =
					dynamic_cast<SettingsPane*>(PageAt(index));
	bool showButtons = (pane != NULL) && pane->UseDefaultRevertButtons();
	BMessage showMessage(kShowButtons);
	showMessage.AddBool(kShowButtonsKey, showButtons);
	Window()->PostMessage(&showMessage);
}
