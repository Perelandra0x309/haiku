/*
 * Copyright 2010-2017, Haiku, Inc. All Rights Reserved.
 * Copyright 2009, Pier Luigi Fiorini.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 *		Brian Hill, supernova@tycho.email
 */


#include "HistoryWindow.h"

#include <Alert.h>
#include <Box.h>
#include <Catalog.h>
#include <CheckBox.h>
#include <DateFormat.h>
#include <Directory.h>
#include <FindDirectory.h>
#include <LayoutBuilder.h>
#include <Notification.h>
#include <Path.h>
#include <StringList.h>
#include <TextControl.h>
#include <Window.h>

#include <notification/Notifications.h>

#include "HistoryListItem.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "HistoryView"
/*
const float kEdgePadding = 5.0;
const float kCLVTitlePadding = 8.0;
*/
const int32 kDateSelected = '_GSL';
const int32 kNotificationSelected = '_NSL';
/*const int32 kNotificationInvoked = '_NIN';

const int32 kCLVDeleteRow = 'av02';*/


static int
CompareListItems(const void* item1, const void* item2)
{
	HistoryListItem* first = *(HistoryListItem**)item1;
	HistoryListItem* second = *(HistoryListItem**)item2;
	return first->TimestampCompare(second);
}


HistoryWindow::HistoryWindow()
	:
	BWindow(BRect(0, 0, 500, 600), B_TRANSLATE_MARK("Notification History"), 
		B_TITLED_WINDOW,
		B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE, 
		B_ALL_WORKSPACES)
{
	fMainView = new HistoryView();
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(fMainView)
	.End();
	CenterOnScreen();
	Show();
		// TODO remove when finished testing history
}


bool
HistoryWindow::QuitRequested()
{
	if (!IsHidden())
		Hide();
	return false;
}


HistoryView::HistoryView()
	:
	BView("history", B_WILL_DRAW | B_FRAME_EVENTS),
	fShowingPreview(false),
	fCurrentPreview(NULL)
{
	find_directory(B_USER_CACHE_DIRECTORY, &fCachePath);
	fCachePath.Append("Notifications");
	
	// Search application field
//	fSearch = new BTextControl(B_TRANSLATE("Search:"), NULL,
//		new BMessage(kSettingChanged));

	// Application menu
	fDateSelectionMenu = new BPopUpMenu("notifications");
	fDateSelectionMF = new BMenuField("Font Size Field",
		B_TRANSLATE_COMMENT("View notifications:", "Menu label"), fDateSelectionMenu);
	fDateSelectionMenu->AddItem(new BMenuItem("All",
			new BMessage(kDateSelected)));
	

	// Application list view
	fListView = new BListView(BRect(0, 0, 500, 100), "Notification List View",
		B_SINGLE_SELECTION_LIST, B_FOLLOW_ALL_SIDES);
	fScrollView = new BScrollView("List Scroll View", fListView,
		B_FOLLOW_ALL_SIDES, 0, false, true);

	// Preview view
	fGroupView = new AppGroupView(BMessenger(this), "");
	fGroupView->SetPreviewModeOn(true);
	BBox *box = new BBox("preview");
	box->SetLabel(B_TRANSLATE("Notification View"));
	BGroupLayout *boxLayout = BLayoutBuilder::Group<>(B_HORIZONTAL)
    	.SetInsets(0, B_USE_DEFAULT_SPACING, 0, 0).Add(fGroupView);
    box->AddChild(boxLayout->View());

	// Add views
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.AddGroup(B_HORIZONTAL)
			.Add(fDateSelectionMF)
			.AddGlue()
		.End()
		.Add(fScrollView)
		.Add(box)
		.SetInsets(B_USE_WINDOW_SPACING)
	.End();
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

//	_PopulateGroups();

	fDateSelectionMenu->SetTargetForItems(this);
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
		case kDateSelected:
		{
			_UpdatePreview(NULL, NULL);
		//	BMenuItem* item = fDateSelectionMenu->FindMarked();
		//	if (item != NULL)
				_PopulateNotifications();

			break;
		}
		case kNotificationSelected: {
			int32 selectedIndex = fListView->CurrentSelection();
			if(selectedIndex >= 0) {
				HistoryListItem *selectedItem =
					(HistoryListItem*)(fListView->ItemAt(selectedIndex));
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
		default:
			BView::MessageReceived(message);
			break;
	}
}

/*
void
HistoryView::_PopulateGroups()
{
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
		
		fDateSelectionMenu->AddItem(new BMenuItem(entry.name,
			new BMessage(kGroupSelected)));
	}
	fDateSelectionMenu->SetTargetForItems(this);
}
*/

void
HistoryView::_PopulateNotifications()
{
	while (!fListView->IsEmpty()) {
		BListItem *item = fListView->RemoveItem(int32(0));
		delete item;
	}

	BDirectory cacheDir(fCachePath.Path());
	cacheDir.Rewind();
	BEntry entry;
	BStringList dateItemsList;
	while (cacheDir.GetNextEntry(&entry) != B_ENTRY_NOT_FOUND) {
		if (entry.InitCheck() != B_OK || !entry.IsFile())
			continue;

		BFile archiveFile(&entry, B_READ_ONLY);
		BMessage archive;
		archive.Unflatten(&archiveFile);
		archiveFile.Unset();

		// Check if message archive is valid
		int32 count;
		if (!_ArchiveIsValid(archive, count))
			return;

		// Get cache file date
		int32 timestamp = 0;
		bool foundTimestamp = false;
		status_t result = archive.FindInt32(kNameTimestamp, &timestamp);
		if (result == B_OK)
			foundTimestamp = true;

		int32 index;
		for (index = 0; index < count; index++) {
			// Get notification
			BMessage notificationData;
			status_t result = archive.FindMessage(kNameNotificationData, index,
				&notificationData);
			if (result != B_OK)
				continue;
			HistoryListItem* item = new HistoryListItem(notificationData);
			fListView->AddItem(item);
			
			// Get timestamp from notification
			if (/*index == 0 && */!foundTimestamp) {
				if (notificationData.FindInt32(kNameTimestamp, &timestamp) == B_OK) {
					//foundTimestamp = true;
					BString dateLabel;
					BDateFormat formatter;
					formatter.Format(dateLabel, timestamp, B_MEDIUM_DATE_FORMAT);
					if (!dateItemsList.HasString(dateLabel)) {
						HistoryListItem* dateItem = new HistoryListItem(timestamp);
						fListView->AddItem(dateItem);
						dateItemsList.Add(dateLabel);
					}
				}
			}
		}
		// Create date list item
		if (foundTimestamp) {
			HistoryListItem* dateItem = new HistoryListItem(timestamp);
			if (dateItem->InitStatus() == B_OK) {
				// Prevent duplicates
				const char* itemLabel = dateItem->DateLabel();
				if (itemLabel != NULL) {
					BString labelString(itemLabel);
					if (!dateItemsList.HasString(itemLabel)) {
						fListView->AddItem(dateItem);
						dateItemsList.Add(itemLabel);
					}
				}
			} else
				delete dateItem;
		}
	}
	fListView->SortItems(CompareListItems);
	
//	if (selectItem)
//		fListView->Select(fListView->IndexOf(selectItem));
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
