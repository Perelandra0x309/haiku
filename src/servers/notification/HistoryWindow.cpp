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
#include <Application.h>
#include <Box.h>
#include <Catalog.h>
#include <CheckBox.h>
#include <DateFormat.h>
#include <Directory.h>
#include <FindDirectory.h>
#include <IconUtils.h>
#include <LayoutBuilder.h>
#include <NodeInfo.h>
#include <Notification.h>
#include <Path.h>
#include <Resources.h>
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
const int32 kMenuChanged = '_MCH';
const int32 kNotificationSelected = '_NSL';
/*const int32 kNotificationInvoked = '_NIN';

const int32 kCLVDeleteRow = 'av02';*/
enum {
	MENU_SELECTION_NONE = 0,
	MENU_SELECTION_ALL,
	MENU_SELECTION_MUTED
};

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
	fSetSelection(MENU_SELECTION_NONE),
	fCurrentPreview(NULL),
	fIconSize(0),
	fNewIcon(NULL),
	fMuteIcon(NULL)
{
	find_directory(B_USER_CACHE_DIRECTORY, &fCachePath);
	fCachePath.Append("Notifications");
	
	// Icons
	app_info info;
	be_app->GetAppInfo(&info);
	BResources resources(&info.ref);
	if (resources.InitCheck() == B_OK) {
		status_t result;
		font_height fontHeight;
		BFont font;
		GetFont(&font);
		font.GetHeight(&fontHeight);
		fIconSize = fontHeight.ascent + fontHeight.descent
			+ fontHeight.leading - 2;
		BRect iconRect(0, 0, fIconSize - 1, fIconSize - 1);
		size_t size;
		// New notification icon
		const void* newdata = resources.LoadResource(B_VECTOR_ICON_TYPE, 1001,
			&size);
		if (newdata != NULL) {
			result = B_ERROR;
			fNewIcon = new BBitmap(iconRect, B_RGBA32);
			if (fNewIcon->InitCheck() == B_OK)
				result = BIconUtils::GetVectorIcon((const uint8 *)newdata,
					size, fNewIcon);
			if (result != B_OK) {
				delete fNewIcon;
				fNewIcon = NULL;
			}
		}
		// Mute icon
		const void* mutedata = resources.LoadResource(B_VECTOR_ICON_TYPE,
			1002, &size);
		if (mutedata != NULL) {
			result = B_ERROR;
			fMuteIcon = new BBitmap(iconRect, B_RGBA32);
			if (fMuteIcon->InitCheck() == B_OK)
				result = BIconUtils::GetVectorIcon((const uint8 *)mutedata,
					size, fMuteIcon);
			if (result != B_OK) {
				delete fMuteIcon;
				fMuteIcon = NULL;
			}
		}
	}
	
	// Search application field
//	fSearch = new BTextControl(B_TRANSLATE("Search:"), NULL,
//		new BMessage(kSettingChanged));

	// Application menu
	fSetSelectionMenu = new BPopUpMenu("Choose:");
	fSetSelectionMF = new BMenuField("Notifications Field",
		B_TRANSLATE_COMMENT("View notifications:", "Menu label"), fSetSelectionMenu);
	fSetSelectionMenu->AddItem(new BMenuItem("All",
			new BMessage(kMenuChanged)));
	fSetSelectionMenu->AddItem(new BMenuItem("Muted notifications",
			new BMessage(kMenuChanged)));
//	fDateSelectionMenu->AddItem(new BMenuItem("Missed notifications",
//			new BMessage(kMenuChanged)));
	

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
			.Add(fSetSelectionMF)
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
	delete fNewIcon;
	fNewIcon = NULL;
	delete fMuteIcon;
	fMuteIcon = NULL;
}


void
HistoryView::AttachedToWindow()
{
	fListView->SetTarget(this);
	fListView->SetSelectionMessage(new BMessage(kNotificationSelected));

#if 0
	fNotifications->AddFilter(new BMessageFilter(B_ANY_DELIVERY,
		B_ANY_SOURCE, B_KEY_DOWN, CatchDelete));
#endif

	fSetSelectionMenu->SetTargetForItems(this);
	_LoadCacheData();
	fSetSelectionMenu->ItemAt(0)->SetMarked(true);
	BMessenger(this).SendMessage(kMenuChanged);
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
		case kMenuChanged:
		{
			int32 index = fSetSelectionMenu->FindMarkedIndex();
			switch (index) {
				case 0:
					fSetSelection = MENU_SELECTION_ALL;
					break;
				case 1:
					fSetSelection = MENU_SELECTION_MUTED;
					break;
				default:
					fSetSelection = MENU_SELECTION_NONE;
					break;
			}
			_UpdatePreview(NULL, NULL);
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


void
HistoryView::Draw(BRect updateRect)
{
	BView::Draw(updateRect);
	if (fListView->IsEmpty()) {
		fListView->PushState();
		float width, height;
		BFont font;
		BString message(B_TRANSLATE_COMMENT("No notifications to show",
			"Message in empty notifications list view"));
		fListView->SetHighColor(ui_color(B_CONTROL_BACKGROUND_COLOR));
		BRect rect = fListView->Bounds();
		fListView->FillRect(rect);
		fListView->GetPreferredSize(&width, &height);
		fListView->GetFont(&font);
		float messageWidth = fListView->StringWidth(message.String());
		fListView->MovePenTo((width - messageWidth) / 2,
			(height + font.Size()) / 2);
		fListView->SetHighColor(ui_color(B_MENU_SELECTED_BACKGROUND_COLOR));
		fListView->DrawString(message.String());
		fListView->PopState();
	}
}


void
HistoryView::_LoadCacheData()
{
	// TODO verify list view is empty??
	
	while (!fCacheData.IsEmpty()) {
		BListItem *item = (BListItem*)fCacheData.RemoveItem(int32(0));
		delete item;
	}

	BDirectory cacheDir(fCachePath.Path());
	cacheDir.Rewind();
	BEntry entry;
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

		int32 index;
		for (index = 0; index < count; index++) {
			// Get notification
			BMessage notificationData;
			status_t result = archive.FindMessage(kNameNotificationData, index,
				&notificationData);
			if (result != B_OK)
				continue;
			HistoryListItem* item = new HistoryListItem(notificationData,
				fNewIcon, fMuteIcon, fIconSize);
			fCacheData.AddItem(item);
		}
	}
}


void
HistoryView::_PopulateNotifications()
{
	while (!fListView->IsEmpty())
		fListView->RemoveItem(int32(0));

	if (fSetSelection == MENU_SELECTION_NONE)
		return;

	bool showOnlyMuted = fSetSelection == MENU_SELECTION_MUTED;

	int32 index;
	int32 count = fCacheData.CountItems();
	BStringList dateItemsList;
	BList itemsList;
	for (index = 0; index < count; index++) {
		HistoryListItem* item = (HistoryListItem*)fCacheData.ItemAt(index);
		bool showItem = fSetSelection == MENU_SELECTION_ALL
			|| (showOnlyMuted && !item->GetWasAllowed());
		if (showItem) {
			itemsList.AddItem(item);
			
			// Get timestamp from notification
			int32 timestamp = item->GetTimestamp();
			if (timestamp > 0) {
				BString dateLabel;
				BDateFormat formatter;
				formatter.Format(dateLabel, timestamp, B_MEDIUM_DATE_FORMAT);
				if (!dateItemsList.HasString(dateLabel)) {
					HistoryListItem* dateItem = new HistoryListItem(timestamp);
					itemsList.AddItem(dateItem);
					dateItemsList.Add(dateLabel);
				}
			}
		}
	}
	fListView->AddList(&itemsList);
	fListView->SortItems(CompareListItems);
	BScrollBar* scrollbar = fScrollView->ScrollBar(B_VERTICAL);
	float max;
	scrollbar->GetRange(NULL, &max);
	scrollbar->SetValue(max);
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
