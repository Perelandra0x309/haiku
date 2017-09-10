/*
 * Copyright 2010-2017, Haiku, Inc. All Rights Reserved.
 * Copyright 2009, Pier Luigi Fiorini.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 *		Brian Hill, supernova@tycho.email
 */


#include "HistoryView.h"

#include <Alert.h>
#include <Box.h>
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

#include "NotificationListItem.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "HistoryView"

const float kEdgePadding = 5.0;
const float kCLVTitlePadding = 8.0;

const int32 kGroupSelected = '_GSL';
const int32 kNotificationSelected = '_NSL';
const int32 kNotificationInvoked = '_NIN';

const int32 kCLVDeleteRow = 'av02';

// Applications column indexes
const int32 kGroupIndex = 0;
const int32 kCountIndex = 1;

// Notifications column indexes
//const int32 kTitleIndex = 0;
//const int32 kDateIndex = 1;
//const int32 kContentIndex = 2;
//const int32 kTypeIndex = 3;
//const int32 kShownIndex = 4;

//TODO undo duplication
const float kCloseSize				= 6;
const float kExpandSize				= 8;
const float kPenSize				= 1;
//const float kEdgePadding			= 2;
const float kSmallPadding			= 2;


HistoryView::HistoryView()
	:
	BView("history", B_WILL_DRAW | B_FRAME_EVENTS),
	fShowingPreview(false),
	fCurrentPreview(NULL)
{
	BRect rect(0, 0, 500, 100);

	// Search application field
	fSearch = new BTextControl(B_TRANSLATE("Search:"), NULL,
		new BMessage(kSettingChanged));

	// Application menu
	fFontSizeMenu = new BPopUpMenu("Choose a group");
	fFontSizeMF = new BMenuField("Font Size Field", B_TRANSLATE_COMMENT("Group:", "Menu label"), fFontSizeMenu);

	// Applications list
/*	fGroups = new BColumnListView(B_TRANSLATE("Groups"),
		0, B_FANCY_BORDER, true);
	fGroups->SetSelectionMode(B_SINGLE_SELECTION_LIST);

	fGroupCol = new BStringColumn(B_TRANSLATE("Group"), 200,
		be_plain_font->StringWidth(B_TRANSLATE("Group")) +
		(kCLVTitlePadding * 2), rect.Width(), B_TRUNCATE_END, B_ALIGN_LEFT);
	fGroups->AddColumn(fGroupCol, kGroupIndex);

	fCountCol = new BStringColumn(B_TRANSLATE("Count"), 10,
		be_plain_font->StringWidth(B_TRANSLATE("Count")) +
		(kCLVTitlePadding * 2), rect.Width(), B_TRUNCATE_END, B_ALIGN_LEFT);
	fGroups->AddColumn(fCountCol, kCountIndex);*/

	// Notifications list
/*	fNotifications = new BColumnListView(B_TRANSLATE("Notifications"),
		0, B_FANCY_BORDER, true);
	fNotifications->SetSelectionMode(B_SINGLE_SELECTION_LIST);

	fDateCol = new BDateColumn(B_TRANSLATE("Date"), 100,
		be_plain_font->StringWidth(B_TRANSLATE("Date")) +
		(kCLVTitlePadding * 2), rect.Width(), B_ALIGN_LEFT);
	fNotifications->AddColumn(fDateCol, kDateIndex);
	
	fTitleCol = new BStringColumn(B_TRANSLATE("Title"), 100,
		be_plain_font->StringWidth(B_TRANSLATE("Title")) +
		(kCLVTitlePadding * 2), rect.Width(), B_TRUNCATE_END, B_ALIGN_LEFT);
	fNotifications->AddColumn(fTitleCol, kTitleIndex);

	fContentCol = new BStringColumn(B_TRANSLATE("Content"), 100,
		be_plain_font->StringWidth(B_TRANSLATE("Content")) +
		(kCLVTitlePadding * 2), rect.Width(), B_TRUNCATE_END, B_ALIGN_LEFT);
	fNotifications->AddColumn(fContentCol, kContentIndex);
	
	fTypeCol = new BStringColumn(B_TRANSLATE("Type"), 100,
		be_plain_font->StringWidth(B_TRANSLATE("Type")) +
		(kCLVTitlePadding * 2), rect.Width(), B_TRUNCATE_END, B_ALIGN_LEFT);
	fNotifications->AddColumn(fTypeCol, kTypeIndex);

	fShownCol = new BStringColumn(B_TRANSLATE("Shown"), 100,
		be_plain_font->StringWidth(B_TRANSLATE("Shown")) +
		(kCLVTitlePadding * 2), rect.Width(), B_TRUNCATE_END, B_ALIGN_LEFT);
	fNotifications->AddColumn(fShownCol, kShownIndex);*/
	
	// Application list view
	fListView = new BListView(rect, "Notification List View", B_SINGLE_SELECTION_LIST,
								B_FOLLOW_ALL_SIDES);
	fScrollView = new BScrollView("List Scroll View", fListView, B_FOLLOW_ALL_SIDES, 0,
								false, true);
	
	
	// Preview view
	fGroupView = new AppGroupView(BMessenger(this), "");
	fGroupView->SetPreviewModeOn(true);
	BBox *box = new BBox("preview");
	box->SetLabel(B_TRANSLATE("Notification Preview"));
	BGroupLayout *boxLayout = BLayoutBuilder::Group<>(B_HORIZONTAL)
    	.SetInsets(0, B_USE_DEFAULT_SPACING, 0, 0).Add(fGroupView);
    box->AddChild(boxLayout->View());
							
	// Add views
	BLayoutBuilder::Group<>(this, B_VERTICAL)
/*		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(fSearch)
		.End()*/
		.AddGroup(B_HORIZONTAL)
			.Add(fFontSizeMF)
			.AddGlue()
		.End()
//		.Add(fGroups)
//		.Add(fNotifications)
		.Add(fScrollView)
		.Add(box)
		.SetInsets(B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING,
			B_USE_WINDOW_SPACING, B_USE_DEFAULT_SPACING)
	.End();
	
//	fGroupView->Hide();
}


HistoryView::~HistoryView()
{
	_UpdatePreview(NULL, NULL);
}


void
HistoryView::AttachedToWindow()
{
//	fGroups->SetTarget(this);
//	fGroups->SetSelectionMessage(new BMessage(kGroupSelected));

//	fNotifications->SetTarget(this);
//	fNotifications->SetSelectionMessage(new BMessage(kNotificationSelected));
	
	fListView->SetTarget(this);
	fListView->SetSelectionMessage(new BMessage(kNotificationSelected));

#if 0
	fNotifications->AddFilter(new BMessageFilter(B_ANY_DELIVERY,
		B_ANY_SOURCE, B_KEY_DOWN, CatchDelete));
#endif

	_PopulateGroups();
}


void
HistoryView::FrameResized(float newWidth, float newHeight)
{
	BView::FrameResized(newWidth, newHeight);
	fListView->Invalidate();
}


void
HistoryView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kGroupSelected:
		{
			_UpdatePreview(NULL, NULL);
/*			BRow* row = fGroups->CurrentSelection();
			if (row == NULL)
				return;
			BStringField* groupName
				= dynamic_cast<BStringField*>(row->GetField(kGroupIndex));
			if (groupName == NULL)
				break;*/
			BMenuItem* item = fFontSizeMenu->FindMarked();
			if (item != NULL)
				_PopulateNotifications(item->Label());

/*			appusage_t::iterator it = fAppFilters.find(groupName->String());
			if (it != fAppFilters.end())
				_Populate(it->second);*/

			break;
		}
		case kNotificationSelected: {
/*			NotificationRow* row = dynamic_cast<NotificationRow*>(fNotifications->CurrentSelection());
			if (row == NULL)
				_UpdatePreview(NULL, NULL);
			else {
				BMessage data = row->GetMessage();
				BNotification* newNotification = new BNotification(&data);
				NotificationView* newPreview = new NotificationView(
					newNotification, bigtime_t(10), B_LARGE_ICON, true); //TODO change
			//	newPreview->SetText();
				newPreview->SetPreviewModeOn(true);
				_UpdatePreview(newPreview, newNotification->Group());
			}*/
			int32 selectedIndex = fListView->CurrentSelection();
			if(selectedIndex >= 0) {
				NotificationListItem *selectedItem = (NotificationListItem*)(fListView->ItemAt(selectedIndex));
				if (selectedItem->IsDateDivider())
					break;
				BMessage data = selectedItem->GetMessage();
				BNotification* newNotification = new BNotification(&data);
				NotificationView* newPreview = new NotificationView(
					newNotification, bigtime_t(10), B_LARGE_ICON, true); //TODO change
				newPreview->SetPreviewModeOn(true);
				_UpdatePreview(newPreview, newNotification->Group());
			}
			else
				_UpdatePreview(NULL, NULL);
			break;
		}
//		case kNotificationInvoked: {
//			fShowPreview = !fShowPreview;
//			_UpdatePreview();
//			break;
//		}
		default:
			BView::MessageReceived(message);
			break;
	}
}


void
HistoryView::_PopulateGroups()
{
//	fGroups->Clear();
	// TODO clear meanu

	// TOD save cache dir to variable
	BPath archivePath;
	find_directory(B_USER_CACHE_DIRECTORY, &archivePath);
	archivePath.Append("Notifications");
	BDirectory archiveDir(archivePath.Path());
	entry_ref entry;
	while (archiveDir.GetNextRef(&entry) == B_OK) {
		BFile archiveFile(&entry, B_READ_ONLY);
		BMessage archive;
		archive.Unflatten(&archiveFile);
		archiveFile.Unset();
		
		// Check if message archive is valid
		int32 count;
		if (!_ArchiveIsValid(archive, count))
			continue;
		
/*		BRow* row = new BRow();
		row->SetField(new BStringField(entry.name), kGroupIndex);
		BString countString;
		countString<<count;
		row->SetField(new BStringField(countString), kCountIndex);
		fGroups->AddRow(row);*/
		
		fFontSizeMenu->AddItem(new BMenuItem(entry.name,
			new BMessage(kGroupSelected)));
	}
	fFontSizeMenu->SetTargetForItems(this);
}


void
HistoryView::_PopulateNotifications(const char* group)
{
//	fNotifications->Clear();
	while(!fListView->IsEmpty())
	{
		BListItem *item = fListView->RemoveItem(int32(0));
		delete item;
	}
	
	// TODO test date list item
	NotificationListItem* testitem = new NotificationListItem("Yesterday");
	fListView->AddItem(testitem);
	
	// Get archive from file based on group
	BPath archivePath;
	find_directory(B_USER_CACHE_DIRECTORY, &archivePath);
	archivePath.Append("Notifications");
	archivePath.Append(group);
	BFile archiveFile(archivePath.Path(), B_READ_ONLY);
	BMessage archive;
	archive.Unflatten(&archiveFile);
	archiveFile.Unset();
	
	// Check if message archive is valid
	int32 count;
	if (!_ArchiveIsValid(archive, count))
		return;
	
	int32 index;
	NotificationListItem* selectItem = NULL;
	for (index = 0; index < count; index++) {
		BMessage notificationData;
		status_t result = archive.FindMessage(kNameNotificationData, index, &notificationData);
		if (result != B_OK)
			continue;
/*		NotificationRow* row = new NotificationRow(notificationData);
		if (row->InitStatus() == B_OK)
			fNotifications->AddRow(row);*/
		
		NotificationListItem* item = new NotificationListItem(notificationData);
		fListView->AddItem(item);
		
		if (index == 0) {
			selectItem = item;
		//	fListView->Select(fListView->IndexOf(item));
		//	fListView->ScrollToSelection();
		}
		
		// Todo testing
		if (index == 2) {
			NotificationListItem* testitem = new NotificationListItem("Today");
			fListView->AddItem(testitem);
		}
	}
	if (selectItem)
		fListView->Select(fListView->IndexOf(selectItem));
//	fListView->Invalidate();
//	Window()->UpdateIfNeeded();
}


bool
HistoryView::_ArchiveIsValid(BMessage& archive, int32& count)
{
	// Check message identifier
	if (archive.what != kNotificationsArchive)
		return false;
	// Check content
	type_code type;
	status_t result = archive.GetInfo(kNameNotificationData, &type, &count);
	if (result != B_OK || type != B_MESSAGE_TYPE || count < 1)
		return false;
	return true;	
}


void
HistoryView::_UpdatePreview(NotificationView* view, const char* group)
{
	if (fShowingPreview && fCurrentPreview != NULL) {
		BMessage msg(kRemoveView);
		msg.AddPointer("view", fCurrentPreview);
		fGroupView->MessageReceived(&msg);
		fCurrentPreview = NULL;
	}
	if (view == NULL) {
		fCurrentPreview = NULL;
		fShowingPreview = false;
	} else {
		fGroupView->AddInfo(view);
		fGroupView->SetGroup(group);
		fCurrentPreview = view;
		fShowingPreview = true;
	}
}
