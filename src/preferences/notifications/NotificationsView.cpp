/*
 * Copyright 2010, Haiku, Inc. All Rights Reserved.
 * Copyright 2009, Pier Luigi Fiorini.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <Alert.h>
#include <Button.h>
#include <Catalog.h>
#include <CheckBox.h>
#include <ColumnListView.h>
#include <ColumnTypes.h>
#include <Directory.h>
#include <FindDirectory.h>
#include <LayoutBuilder.h>
#include <Notification.h>
#include <Path.h>
#include <TextControl.h>
#include <Window.h>

#include <notification/Notifications.h>
#include <notification/NotificationReceived.h>

#include "NotificationsConstants.h"
#include "NotificationsView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "NotificationView"

//const int32 kCLVDeleteRow = 'av02';

// Applications column indexes
const int32 kAppNameIndex = 0;
//const int32 kSignatureIndex = 1;
const int32 kAppEnabledIndex = 1;

// Notifications column indexes
//const int32 kDateIndex = 0;
//const int32 kTitleIndex = 1;
//const int32 kTypeIndex = 2;
//const int32 kAllowIndex = 3;


AppRow::AppRow(const char* name, const char* signature, bool allowed)
	:
	BRow(),
	fName(name),
	fSignature(signature),
	fAllowed(allowed)
{
	SetField(new BStringField(fName.String()), kAppNameIndex);
	BString text = fAllowed ? B_TRANSLATE("Allowed") : B_TRANSLATE("Muted");
	SetField(new BStringField(text.String()), kAppEnabledIndex);
}


void
AppRow::SetAllowed(bool allowed)
{
	fAllowed = allowed;
	RefreshEnabledField();
}


void
AppRow::RefreshEnabledField()
{
	BStringField* field = (BStringField*)GetField(kAppEnabledIndex);
	BString text = fAllowed ? B_TRANSLATE("Allowed") : B_TRANSLATE("Muted");
	field->SetString(text.String());
	Invalidate();
}


NotificationsView::NotificationsView(SettingsHost* host)
	:
	SettingsPane("apps", host),
	fSelectedRow(NULL)
{
	BRect rect(0, 0, 100, 100);

	// Search application field
//	fSearch = new BTextControl(B_TRANSLATE("Search:"), NULL,
//		new BMessage(kSettingChanged));

	// Applications list
	fApplications = new BColumnListView(B_TRANSLATE("Applications"),
		0, B_FANCY_BORDER, false);
	fApplications->SetSelectionMode(B_SINGLE_SELECTION_LIST);
	fApplications->SetSelectionMessage(new BMessage(kApplicationSelected));

	fAppCol = new BStringColumn(B_TRANSLATE("Application"), 200, 200, 400,
	//	be_plain_font->StringWidth(B_TRANSLATE("Application")) +
	//	(kCLVTitlePadding * 2), rect.Width(),
		B_TRUNCATE_END, B_ALIGN_LEFT);
	fApplications->AddColumn(fAppCol, kAppNameIndex);

//	fSignatureCol = new BStringColumn(B_TRANSLATE("Signature"), 200, 200, 400,
	//	be_plain_font->StringWidth(B_TRANSLATE("Application")) +
	//	(kCLVTitlePadding * 2), rect.Width(),
//		B_TRUNCATE_END, B_ALIGN_LEFT);
//	fApplications->AddColumn(fSignatureCol, kSignatureIndex);

	fAppEnabledCol = new BStringColumn(B_TRANSLATE("Status"), 10,
		be_plain_font->StringWidth(B_TRANSLATE("Status")) +
		(kCLVTitlePadding * 4), rect.Width(), B_TRUNCATE_END, B_ALIGN_LEFT);
	fApplications->AddColumn(fAppEnabledCol, kAppEnabledIndex);
	fApplications->SetSortColumn(fAppCol, true, true);
	
	fAddButton = new BButton("add_app", B_TRANSLATE("Add" B_UTF8_ELLIPSIS),
		new BMessage(kAddApplication));
	fRemoveButton = new BButton("add_app", B_TRANSLATE("Remove"),
		new BMessage(kRemoveApplication));
	fRemoveButton->SetEnabled(false);
	
	fBlockAll = new BCheckBox("block", B_TRANSLATE("Mute notifications"),
		new BMessage(kMuteChanged));

	// Notifications list
/*	fNotifications = new BColumnListView(B_TRANSLATE("Notifications"),
		0, B_FANCY_BORDER, true);
	fNotifications->SetSelectionMode(B_SINGLE_SELECTION_LIST);

	fTitleCol = new BStringColumn(B_TRANSLATE("Title"), 100,
		be_plain_font->StringWidth(B_TRANSLATE("Title")) +
		(kCLVTitlePadding * 2), rect.Width(), B_TRUNCATE_END, B_ALIGN_LEFT);
	fNotifications->AddColumn(fTitleCol, kTitleIndex);

	fDateCol = new BDateColumn(B_TRANSLATE("Last received"), 100,
		be_plain_font->StringWidth(B_TRANSLATE("Last received")) +
		(kCLVTitlePadding * 2), rect.Width(), B_ALIGN_LEFT);
	fNotifications->AddColumn(fDateCol, kDateIndex);

	fTypeCol = new BStringColumn(B_TRANSLATE("Type"), 100,
		be_plain_font->StringWidth(B_TRANSLATE("Type")) +
		(kCLVTitlePadding * 2), rect.Width(), B_TRUNCATE_END, B_ALIGN_LEFT);
	fNotifications->AddColumn(fTypeCol, kTypeIndex);

	fAllowCol = new BStringColumn(B_TRANSLATE("Allowed"), 100,
		be_plain_font->StringWidth(B_TRANSLATE("Allowed")) +
		(kCLVTitlePadding * 2), rect.Width(), B_TRUNCATE_END, B_ALIGN_LEFT);
	fNotifications->AddColumn(fAllowCol, kAllowIndex);*/

	// Add views
	BLayoutBuilder::Group<>(this, B_VERTICAL)
/*		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(fSearch)
		.End()*/
		.AddGroup(B_HORIZONTAL)
			.Add(fApplications)
			.AddGroup(B_VERTICAL)
				.Add(fAddButton)
				.Add(fRemoveButton)
				.AddGlue()
			.End()
		.End()
		.Add(fBlockAll)
//		.Add(fNotifications)
		.SetInsets(B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING,
			B_USE_WINDOW_SPACING, B_USE_DEFAULT_SPACING);

	//File Panel
	fPanelFilter = new AppRefFilter();
	fAddAppPanel = new BFilePanel(B_OPEN_PANEL, NULL, NULL, B_FILE_NODE, false, NULL, fPanelFilter);
}


NotificationsView::~NotificationsView()
{
	delete fAddAppPanel;
	delete fPanelFilter;
}


void
NotificationsView::AttachedToWindow()
{
	fApplications->SetTarget(this);
	fApplications->SetInvocationMessage(new BMessage(kApplicationSelected));
	fAddButton->SetTarget(this);
	fRemoveButton->SetTarget(this);
	fBlockAll->SetTarget(this);
	fAddAppPanel->SetTarget(this);
	_RecallItemSettings();

//	fNotifications->SetTarget(this);
//	fNotifications->SetInvocationMessage(new BMessage(kNotificationSelected));

#if 0
	fNotifications->AddFilter(new BMessageFilter(B_ANY_DELIVERY,
		B_ANY_SOURCE, B_KEY_DOWN, CatchDelete));
#endif
}


void
NotificationsView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case kApplicationSelected:
		{
			Window()->Lock();
			_ClearItemSettings();
			_UpdateSelectedItem();
			_RecallItemSettings();
			Window()->Unlock();
			break;
		}
		case kMuteChanged: {
			bool allowed = fBlockAll->Value() == B_CONTROL_OFF;
			fSelectedRow->SetAllowed(allowed);
			appusage_t::iterator it = fAppFilters.find(fSelectedRow->Signature());
			if (it != fAppFilters.end())
				it->second->SetAllowed(allowed);
			Window()->PostMessage(kApply);
			break;
		}
		case kAddApplication: {
			BMessage addmsg(kAddApplicationRef);
			fAddAppPanel->SetMessage(&addmsg);
			fAddAppPanel->Show();
			break;
		}
		case kAddApplicationRef: {
			entry_ref srcRef;
			msg->FindRef("refs", &srcRef);
			BEntry srcEntry(&srcRef, true);
			BPath path(&srcEntry);
			BNode node(&srcEntry);
			char *buf = new char[B_ATTR_NAME_LENGTH];
			ssize_t size;
			if( (size = node.ReadAttr("BEOS:APP_SIG",0,0,buf,B_ATTR_NAME_LENGTH)) > 0 )
			{
				AppUsage* appUsage = new AppUsage(path.Leaf(), buf, true);
				fAppFilters[appUsage->Signature()] = appUsage;
		//		Window()->Lock();
				AppRow* row = new AppRow(appUsage->AppName(),
					appUsage->Signature(), appUsage->Allowed());
				fApplications->AddRow(row);
				fApplications->DeselectAll();
				fApplications->AddToSelection(row);
				fApplications->ScrollTo(row);
				_UpdateSelectedItem();
				_RecallItemSettings();
				//row->Invalidate();
				//fApplications->InvalidateRow(row);
				// TODO redraw row
			//	Window()->Unlock();
				Window()->PostMessage(kApply);
			}
			else {
				BAlert* alert = new BAlert("",
					B_TRANSLATE_COMMENT("Application does not have "
						"a valid signature", "Alert message"),
					B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL,
					B_WARNING_ALERT);
				alert->Go();
			}
			delete[] buf;
			break;
		}
		case kRemoveApplication: {
			if (fSelectedRow) {
				appusage_t::iterator it = fAppFilters.find(fSelectedRow->Signature());
				if (it != fAppFilters.end()) {
					delete it->second;
					fAppFilters.erase(it);
				}
				fApplications->RemoveRow(fSelectedRow);
				delete fSelectedRow;
				fSelectedRow = NULL;
				_ClearItemSettings();
				_UpdateSelectedItem();
				_RecallItemSettings();
				Window()->PostMessage(kApply);
			}
			break;
		}
//		case kNotificationSelected:
//			break;
		default:
			BView::MessageReceived(msg);
			break;
	}
}


status_t
NotificationsView::Load(BMessage& settings)
{
	type_code type;
	int32 count = 0;

	if (settings.GetInfo("app_usage", &type, &count) != B_OK)
		return B_ERROR;

	// Clean filters
	appusage_t::iterator auIt;
	for (auIt = fAppFilters.begin(); auIt != fAppFilters.end(); auIt++)
		delete auIt->second;
	fAppFilters.clear();

	// Add new filters
	for (int32 i = 0; i < count; i++) {
		AppUsage* app = new AppUsage();
		settings.FindFlat("app_usage", i, app);
		fAppFilters[app->Signature()] = app;
	}

	// Load the applications list
	_PopulateApplications();

	return B_OK;
}


status_t
NotificationsView::Save(BMessage& storage)
{
	appusage_t::iterator fIt;
	for (fIt = fAppFilters.begin(); fIt != fAppFilters.end(); fIt++)
		storage.AddFlat("app_usage", fIt->second);

	return B_OK;
}


void
NotificationsView::_ClearItemSettings()
{
	fBlockAll->SetValue(B_CONTROL_OFF);
}


void
NotificationsView::_UpdateSelectedItem()
{
	fSelectedRow = dynamic_cast<AppRow*>(fApplications->CurrentSelection());
	
}


void
NotificationsView::_RecallItemSettings()
{
	//Enable or disable objects
	if(fSelectedRow == NULL)//no selected item
	{
		fBlockAll->SetValue(B_CONTROL_OFF);
		fBlockAll->SetEnabled(false);
		fRemoveButton->SetEnabled(false);
	}
	else
	{
		fBlockAll->SetEnabled(true);
		fRemoveButton->SetEnabled(true);
		appusage_t::iterator it = fAppFilters.find(fSelectedRow->Signature());
		if (it != fAppFilters.end()) {
			fBlockAll->SetValue(!(it->second->Allowed()));
		}
	}
}


status_t
NotificationsView::Revert()
{
	return B_OK;
}


bool
NotificationsView::RevertPossible()
{
	return false;
}


status_t
NotificationsView::Defaults()
{
	return B_OK;
}


bool
NotificationsView::DefaultsPossible()
{
	return false;
}


bool
NotificationsView::UseDefaultRevertButtons()
{
	return false;
}


void
NotificationsView::_PopulateApplications()
{
	appusage_t::iterator it;

	fApplications->Clear();

	for (it = fAppFilters.begin(); it != fAppFilters.end(); ++it) {
		AppUsage* appUsage = it->second;
		AppRow* row = new AppRow(appUsage->AppName(),
			appUsage->Signature(), appUsage->Allowed());
		fApplications->AddRow(row);
	}
}

/*
void
NotificationsView::_Populate(AppUsage* usage)
{
	// Sanity check
	if (!usage)
		return;

	int32 size = usage->Notifications();

	if (usage->Allowed() == false)
		fBlockAll->SetValue(B_CONTROL_ON);

	fNotifications->Clear();

	for (int32 i = 0; i < size; i++) {
		NotificationReceived* notification = usage->NotificationAt(i);
		time_t updated = notification->LastReceived();
		const char* allow = notification->Allowed() ? B_TRANSLATE("Yes")
			: B_TRANSLATE("No");
		const char* type = "";

		switch (notification->Type()) {
			case B_INFORMATION_NOTIFICATION:
				type = B_TRANSLATE("Information");
				break;
			case B_IMPORTANT_NOTIFICATION:
				type = B_TRANSLATE("Important");
				break;
			case B_ERROR_NOTIFICATION:
				type = B_TRANSLATE("Error");
				break;
			case B_PROGRESS_NOTIFICATION:
				type = B_TRANSLATE("Progress");
				break;
			default:
				type = B_TRANSLATE("Unknown");
		}

		BRow* row = new BRow();
		row->SetField(new BStringField(notification->Title()), kTitleIndex);
		row->SetField(new BDateField(&updated), kDateIndex);
		row->SetField(new BStringField(type), kTypeIndex);
		row->SetField(new BStringField(allow), kAllowIndex);

		fNotifications->AddRow(row);
	}
}*/
