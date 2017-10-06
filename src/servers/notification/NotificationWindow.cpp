/*
 * Copyright 2010-2017, Haiku, Inc. All Rights Reserved.
 * Copyright 2008-2009, Pier Luigi Fiorini. All Rights Reserved.
 * Copyright 2004-2008, Michael Davidson. All Rights Reserved.
 * Copyright 2004-2007, Mikael Eiman. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson, slaad@bong.com.au
 *		Mikael Eiman, mikael@eiman.tv
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 *		Brian Hill, supernova@tycho.email
 */
#include "NotificationWindow.h"

#include <algorithm>

#include <Alert.h>
#include <Application.h>
#include <Catalog.h>
#include <DateTime.h>
#include <Deskbar.h>
#include <Directory.h>
#include <File.h>
#include <FindDirectory.h>
#include <GroupLayout.h>
#include <NodeMonitor.h>
#include <Notifications.h>
#include <Path.h>
#include <PropertyInfo.h>

#include "AppGroupView.h"
#include "AppUsage.h"
#include "DeskbarShelfView.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "NotificationWindow"


property_info main_prop_list[] = {
	{"message", {B_GET_PROPERTY, 0}, {B_INDEX_SPECIFIER, 0}, 
		"get a message"},
	{"message", {B_COUNT_PROPERTIES, 0}, {B_DIRECT_SPECIFIER, 0},
		"count messages"},
	{"message", {B_CREATE_PROPERTY, 0}, {B_DIRECT_SPECIFIER, 0},
		"create a message"},
	{"message", {B_SET_PROPERTY, 0}, {B_INDEX_SPECIFIER, 0},
		"modify a message"},
	{0}
};


NotificationWindow::NotificationWindow()
	:
	BWindow(BRect(0, 0, -1, -1), B_TRANSLATE_MARK("Notification"), 
		B_BORDERED_WINDOW_LOOK, B_FLOATING_ALL_WINDOW_FEEL, B_AVOID_FRONT
		| B_AVOID_FOCUS | B_NOT_CLOSABLE | B_NOT_ZOOMABLE | B_NOT_MINIMIZABLE
		| B_NOT_RESIZABLE | B_NOT_MOVABLE | B_AUTO_UPDATE_SIZE_LIMITS, 
		B_ALL_WORKSPACES),
	fShouldRun(true),
	fShowDeskbarView(true),
	fMuteAllFlag(false)
{
	status_t result = find_directory(B_USER_CACHE_DIRECTORY, &fCachePath);
	fCachePath.Append("Notifications");
	BDirectory cacheDir;
	result = cacheDir.SetTo(fCachePath.Path());
	if(result == B_ENTRY_NOT_FOUND)
		cacheDir.CreateDirectory(fCachePath.Path(), NULL);
	
	SetLayout(new BGroupLayout(B_VERTICAL, 0));

	_LoadSettings(true);

	// Start the message loop
	Hide();
	Show();
}


NotificationWindow::~NotificationWindow()
{
	_ShowShelfView(false);

	appfilter_t::iterator aIt;
	for (aIt = fAppFilters.begin(); aIt != fAppFilters.end(); aIt++)
		delete aIt->second;
}


bool
NotificationWindow::QuitRequested()
{
	appview_t::iterator aIt;
	for (aIt = fAppViews.begin(); aIt != fAppViews.end(); aIt++) {
		aIt->second->RemoveSelf();
		delete aIt->second;
	}

	BMessenger(be_app).SendMessage(B_QUIT_REQUESTED);
	return BWindow::QuitRequested();
}


void
NotificationWindow::WorkspaceActivated(int32 /*workspace*/, bool active)
{
	// Ensure window is in the correct position
	if (active)
		SetPosition();
}


void
NotificationWindow::FrameResized(float width, float height)
{
	SetPosition();
}


void
NotificationWindow::ScreenChanged(BRect frame, color_space mode)
{
	SetPosition();
}


void
NotificationWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_NODE_MONITOR:
		{
			_LoadSettings();
			break;
		}
		case kNotificationMessage:
		{
			if (!fShouldRun)
				break;

			BMessage reply(B_REPLY);
			BNotification* notification = new BNotification(message);

			if (notification->InitCheck() == B_OK) {
				
				bool allow = false;
				
				if (!fMuteAllFlag) {
					BString sourceSignature(notification->SourceSignature());
					BString sourceName(notification->SourceName());
	
					appfilter_t::iterator it =
						fAppFilters.find(sourceSignature.String());
					AppUsage* appUsage = NULL;
					if (it == fAppFilters.end()) {
						if (sourceSignature.Length() > 0
							&& sourceName.Length() > 0) {
							appUsage = new AppUsage(sourceName.String(),
								sourceSignature.String(), true);
							fAppFilters[sourceSignature.String()] = appUsage;
							// TODO save back to settings file
						}
						allow = true;
					} else {
						appUsage = it->second;
						allow = appUsage->Allowed();
					}
				}

				if (allow) {
					BString groupName(notification->Group());
					appview_t::iterator aIt = fAppViews.find(groupName);
					AppGroupView* group = NULL;
					if (aIt == fAppViews.end()) {
						group = new AppGroupView(this,
							groupName == "" ? NULL : groupName.String());
						fAppViews[groupName] = group;
						GetLayout()->AddView(group);
					} else
						group = aIt->second;

					bigtime_t timeout;
					if (message->FindInt64("timeout", &timeout) != B_OK)
						timeout = fTimeout;
					
					NotificationView* view = new NotificationView(notification,
						timeout, fIconSize);
					group->AddInfo(view);

					_ShowHide();

					reply.AddInt32("error", B_OK);
				} else
					reply.AddInt32("error", B_NOT_ALLOWED);

				// Cache notification
			//	BString text("Group: ");
			//	text.Append(notification->Group()).Append("\nSig: ").Append(info.signature);
			//	text.Append("\nMessenger valid: ").Append(messenger.IsValid()?"true":"false");
			//	(new BAlert("sig", text, "OK"))->Go(NULL);
				BPath path = fCachePath;
		/*		BString group(notification->Group());
				if (group == "")
					path.Append("_no_group");
				else
					path.Append(group);*/
				BDate currentDate(time(NULL));
				BString dateString;
				dateString << currentDate.Year();
				int32 month = currentDate.Month();
				if (month < 10)
					dateString.Append("0");
				dateString << month;
				int32 day = currentDate.Day();
				if (day < 10)
					dateString.Append("0");
				dateString << day;
		//		printf("Date: %s\n", dateString.String());
				path.Append(dateString.String());
				BMessage archive(kNotificationsArchive);
				BFile file(path.Path(), B_READ_ONLY | B_CREATE_FILE);
				archive.Unflatten(&file);
				file.Unset();
				BMessage notificationData(kNotificationData);
				notificationData.AddMessage(kNameNotificationMessage, message);
				notificationData.AddBool(kNameWasAllowed, allow);
				notificationData.AddInt32(kNameTimestamp, time(NULL));
				archive.AddMessage(kNameNotificationData, &notificationData);
				file.SetTo(path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
				archive.Flatten(&file);
				file.Unset();

			} else {
				reply.what = B_MESSAGE_NOT_UNDERSTOOD;
				reply.AddInt32("error", B_ERROR);
			}

			message->SendReply(&reply);
			break;
		}
		case kRemoveGroupView:
		{
			AppGroupView* view = NULL;
			if (message->FindPointer("view", (void**)&view) != B_OK)
				return;

			// It's possible that between sending this message, and us receiving
			// it, the view has become used again, in which case we shouldn't
			// delete it.
			if (view->HasChildren())
				return;

			// this shouldn't happen
			if (fAppViews.erase(view->Group()) < 1)
				break;

			view->RemoveSelf();
			delete view;

			_ShowHide();
			break;
		}
		case kDeskbarRegistration:
		{
			status_t result = message->FindMessenger(kKeyMessenger,
				&fDeskbarViewMessenger);
			if (result != B_OK || !fDeskbarViewMessenger.IsValid()) {
				BAlert* alert = new BAlert("error",
					B_TRANSLATE("The Notification server could not "
					"successfully connect to its deskbar replicant.  The "
					"replicant will not indicate when notifications are "
					"muted."), B_TRANSLATE("OK"),
					NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
				alert->SetShortcut(0, B_ESCAPE);
				alert->Go(NULL);
			} else {
				BMessage reply(kRegistrationAcknowledge);
				message->SendReply(&reply);
				_SyncDeskbar();
			}
			break;
		}
		case kMuteAllClicked:
			fMuteAllFlag = !fMuteAllFlag;
			_SyncDeskbar();
			break;
		default:
			BWindow::MessageReceived(message);
	}
}


icon_size
NotificationWindow::IconSize()
{
	return fIconSize;
}


int32
NotificationWindow::Timeout()
{
	return fTimeout;
}


float
NotificationWindow::Width()
{
	return fWidth;
}


void
NotificationWindow::_ShowHide()
{
	if (fAppViews.empty() && !IsHidden()) {
		Hide();
		return;
	}

	if (IsHidden()) {
		SetPosition();
		Show();
	}
}


void
NotificationWindow::SetPosition()
{
	Layout(true);

	BRect bounds = DecoratorFrame();
	float width = Bounds().Width() + 1;
	float height = Bounds().Height() + 1;

	float leftOffset = Frame().left - bounds.left;
	float topOffset = Frame().top - bounds.top + 1;
	float rightOffset = bounds.right - Frame().right;
	float bottomOffset = bounds.bottom - Frame().bottom;
		// Size of the borders around the window
	
	float x = Frame().left;
	float y = Frame().top;
		// If we can't guess, don't move...

	BDeskbar deskbar;
	BRect frame = deskbar.Frame();

	switch (deskbar.Location()) {
		case B_DESKBAR_TOP:
			// Put it just under, top right corner
			y = frame.bottom + topOffset;
			x = frame.right - width + rightOffset;
			break;
		case B_DESKBAR_BOTTOM:
			// Put it just above, lower left corner
			y = frame.top - height - bottomOffset;
			x = frame.right - width + rightOffset;
			break;
		case B_DESKBAR_RIGHT_TOP:
			x = frame.left - width - rightOffset;
			y = frame.top - topOffset + 1;
			break;
		case B_DESKBAR_LEFT_TOP:
			x = frame.right + leftOffset;
			y = frame.top - topOffset + 1;
			break;
		case B_DESKBAR_RIGHT_BOTTOM:
			y = frame.bottom - height + bottomOffset;
			x = frame.left - width - rightOffset;
			break;
		case B_DESKBAR_LEFT_BOTTOM:
			y = frame.bottom - height + bottomOffset;
			x = frame.right + leftOffset;
			break;
		default:
			break;
	}

	MoveTo(x, y);
}


void
NotificationWindow::_LoadSettings(bool startMonitor)
{
	BPath path;
	BMessage settings;

	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) != B_OK)
		return;

	path.Append(kSettingsFile);

	BFile file(path.Path(), B_READ_ONLY | B_CREATE_FILE);
	settings.Unflatten(&file);

	_LoadGeneralSettings(settings);
	_LoadDisplaySettings(settings);
	_LoadAppFilters(settings);
	_ShowShelfView(true);

	if (startMonitor) {
		node_ref nref;
		BEntry entry(path.Path());
		entry.GetNodeRef(&nref);

		if (watch_node(&nref, B_WATCH_ALL, BMessenger(this)) != B_OK) {
			BAlert* alert = new BAlert(B_TRANSLATE("Warning"),
				B_TRANSLATE("Couldn't start general settings monitor.\n"
					"Live filter changes disabled."), B_TRANSLATE("OK"));
			alert->SetFlags(alert->Flags() | B_CLOSE_ON_ESCAPE);
			alert->Go(NULL);
		}
	}
}


void
NotificationWindow::_LoadAppFilters(BMessage& settings)
{
	type_code type;
	int32 count = 0;

	if (settings.GetInfo("app_usage", &type, &count) != B_OK)
		return;

	for (int32 i = 0; i < count; i++) {
		AppUsage* app = new AppUsage();
		if (settings.FindFlat("app_usage", i, app) == B_OK)
			fAppFilters[app->Signature()] = app;
		else
			delete app;
	}
}


void
NotificationWindow::_LoadGeneralSettings(BMessage& settings)
{
	if (settings.FindBool(kAutoStartName, &fShouldRun) == B_OK) {
		if (fShouldRun == false) {
			// We should not start. Quit the app!
			be_app_messenger.SendMessage(B_QUIT_REQUESTED);
		}
	} else
		fShouldRun = true;

	if (settings.FindInt32(kTimeoutName, &fTimeout) != B_OK)
		fTimeout = kDefaultTimeout;
	fTimeout *= 1000000;
		// Convert from seconds to microseconds

	if (settings.FindBool(kShowDeskbarName, &fShowDeskbarView) != B_OK)
		fShowDeskbarView = true;
}


void
NotificationWindow::_LoadDisplaySettings(BMessage& settings)
{
	int32 setting;
	float originalWidth = fWidth;

	if (settings.FindFloat(kWidthName, &fWidth) != B_OK)
		fWidth = kDefaultWidth;
	if (originalWidth != fWidth)
		GetLayout()->SetExplicitSize(BSize(fWidth, B_SIZE_UNSET));

	if (settings.FindInt32(kIconSizeName, &setting) != B_OK)
		fIconSize = kDefaultIconSize;
	else
		fIconSize = (icon_size)setting;

	// Notify the views about the change
	appview_t::iterator aIt;
	for (aIt = fAppViews.begin(); aIt != fAppViews.end(); ++aIt) {
		AppGroupView* view = aIt->second;
		view->Invalidate();
	}
}


void
NotificationWindow::_ShowShelfView(bool show)
{
	show &= fShowDeskbarView;
	BDeskbar deskbar;
	// Don't add another DeskbarShelfView to the Deskbar if one is already
	// attached
	if (show && !deskbar.HasItem(kShelfviewName)) {
		BView* shelfView = new DeskbarShelfView();
		status_t status = deskbar.AddItem(shelfView);
		if (status != B_OK)
			fprintf(stderr, "Can't add deskbar replicant: %s\n",
				strerror(status));
		delete shelfView;
	}
	// Remove DeskbarShelfView if there is one in the deskbar
	else if (!show && deskbar.HasItem(kShelfviewName))
		deskbar.RemoveItem(kShelfviewName);
}


void
NotificationWindow::_SyncDeskbar()
{
	if (fShowDeskbarView && fDeskbarViewMessenger.IsValid()) {
		BMessage message(kDeskbarSync);
		message.AddBool(kKeyMuteAll, fMuteAllFlag);
		fDeskbarViewMessenger.SendMessage(&message);
	}
}
