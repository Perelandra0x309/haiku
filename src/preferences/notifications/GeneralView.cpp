/*
 * Copyright 2010-2013, Haiku, Inc. All Rights Reserved.
 * Copyright 2009, Pier Luigi Fiorini.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include <Alert.h>
#include <Button.h>
#include <Catalog.h>
#include <Directory.h>
#include <File.h>
#include <FindDirectory.h>
#include <Font.h>
#include <LayoutBuilder.h>
#include <Node.h>
#include <Path.h>
#include <Query.h>
#include <Roster.h>
#include <SeparatorView.h>
#include <String.h>
#include <StringView.h>
#include <SymLink.h>
#include <Volume.h>
#include <VolumeRoster.h>

#include <notification/Notifications.h>

#include "GeneralView.h"
#include "SettingsHost.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "GeneralView"
const int32 kToggleNotifications = '_TSR';

GeneralView::GeneralView(SettingsHost* host)
	:
	SettingsPane("general", host)
{
	// Notifications
	fNotificationBox = new BCheckBox("server",
		B_TRANSLATE("Enable notifications"),
		new BMessage(kToggleNotifications));

	// Autostart
/*	fAutoStart = new BCheckBox("autostart",
		B_TRANSLATE("Enable notifications at startup"),
		new BMessage(kSettingChanged));*/

	// Display time
	fTimeout = new BTextControl(B_TRANSLATE("Hide notifications after"),
		NULL, new BMessage(kSettingChanged));
	BStringView* displayTimeLabel = new BStringView("dt_label",
		B_TRANSLATE("seconds of inactivity"));

	// Window width
	fWindowWidth = new BTextControl(B_TRANSLATE("Window width:"), NULL,
		new BMessage(kSettingChanged));

	// Icon size
	fIconSize = new BMenu("iconSize");
	fIconSize->AddItem(new BMenuItem(B_TRANSLATE("Mini icon"),
		new BMessage(kSettingChanged)));
	fIconSize->AddItem(new BMenuItem(B_TRANSLATE("Large icon"),
		new BMessage(kSettingChanged)));
	fIconSize->SetLabelFromMarked(true);
	fIconSizeField = new BMenuField(B_TRANSLATE("Icon size:"), fIconSize);

	// Default position
	// TODO: Here will come a screen representation with the four corners
	// clickable

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.AddGroup(B_VERTICAL)
			.SetInsets(B_USE_WINDOW_SPACING)
			.AddGroup(B_HORIZONTAL)
				.Add(fNotificationBox)
				.AddGlue()
			.End()
			.AddGroup(B_HORIZONTAL)
				.Add(fWindowWidth)
				.AddGlue()
			.End()
			.AddGroup(B_HORIZONTAL)
				.Add(fIconSizeField)
				.AddGlue()
			.End()
			.AddGroup(B_HORIZONTAL, 4)
				.Add(fTimeout)
				.Add(displayTimeLabel)
			.End()
		.End()
		.AddGlue()
	.End();
}


void
GeneralView::AttachedToWindow()
{
	fNotificationBox->SetTarget(this);
	fTimeout->SetTarget(this);
	fWindowWidth->SetTarget(this);
	fIconSize->SetTargetForItems(this);
}


void
GeneralView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case kToggleNotifications:
		{
			entry_ref ref;

			// Check if server is available
			if (!_CanFindServer(&ref)) {
				BAlert* alert = new BAlert(B_TRANSLATE("Notifications"),
					B_TRANSLATE("The notifications server cannot be"
					" found, this means your InfoPopper installation was"
					" not successfully completed."), B_TRANSLATE("OK"),
					NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
				alert->SetFlags(alert->Flags() | B_CLOSE_ON_ESCAPE);
				(void)alert->Go();
				return;
			}

			if (fNotificationBox->Value() == B_CONTROL_OFF) {
				// Server team
				team_id team = be_roster->TeamFor(kNotificationServerSignature);

				// Establish a connection to infopopper_server
				status_t ret = B_ERROR;
				BMessenger messenger(kNotificationServerSignature, team, &ret);
				if (ret != B_OK) {
					BAlert* alert = new BAlert(B_TRANSLATE(
						"Notifications"), B_TRANSLATE("Notifications "
						"cannot be stopped, because the server can't be"
						" reached."), B_TRANSLATE("OK"), NULL, NULL,
						B_WIDTH_AS_USUAL, B_STOP_ALERT);
					alert->SetFlags(alert->Flags() | B_CLOSE_ON_ESCAPE);
					(void)alert->Go();
					return;
				}

				// Send quit message
				if (messenger.SendMessage(B_QUIT_REQUESTED) != B_OK) {
					BAlert* alert = new BAlert(B_TRANSLATE(
						"Notifications"), B_TRANSLATE("Cannot disable"
						" notifications because the server can't be "
						"reached."), B_TRANSLATE("OK"),
						NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
					alert->SetFlags(alert->Flags() | B_CLOSE_ON_ESCAPE);
					(void)alert->Go();
					return;
				}
			} else if (!_IsServerRunning()) {
				// Start server
				status_t err = be_roster->Launch(kNotificationServerSignature);
				if (err != B_OK) {
					BAlert* alert = new BAlert(B_TRANSLATE(
						"Notifications"), B_TRANSLATE("Cannot enable"
						" notifications because the server cannot be "
						"found.\nThis means your InfoPopper installation"
						" was not successfully completed."),
						B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL,
						B_STOP_ALERT);
					alert->SetFlags(alert->Flags() | B_CLOSE_ON_ESCAPE);
					(void)alert->Go();
					return;
				}
			}
			break;
		}
		case kSettingChanged:
			SettingsPane::MessageReceived(msg);
			break;
		default:
			BView::MessageReceived(msg);
			break;
	}
}


status_t
GeneralView::Load(BMessage& settings)
{
	fNotificationBox->SetValue(_IsServerRunning() ? B_CONTROL_ON : B_CONTROL_OFF);

/*	bool autoStart;
	if (settings.FindBool(kAutoStartName, &autoStart) != B_OK)
		autoStart = kDefaultAutoStart;
	fAutoStart->SetValue(autoStart ? B_CONTROL_ON : B_CONTROL_OFF);*/

	if (settings.FindInt32(kTimeoutName, &fOriginalTimeout) != B_OK)
		fOriginalTimeout = kDefaultTimeout;

	if (settings.FindFloat(kWidthName, &fOriginalWidth) != B_OK)
		fOriginalWidth = kDefaultWidth;

	int32 setting;
	if (settings.FindInt32(kIconSizeName, &setting) != B_OK)
		fOriginalIconSize = kDefaultIconSize;
	else
		fOriginalIconSize = (icon_size)setting;

	return Revert();
}


status_t
GeneralView::Save(BMessage& settings)
{
//	bool autoStart = (fAutoStart->Value() == B_CONTROL_ON);
//	settings.AddBool(kAutoStartName, autoStart);

	int32 timeout = atol(fTimeout->Text());
	settings.AddInt32(kTimeoutName, timeout);

	float width = atof(fWindowWidth->Text());
	settings.AddFloat(kWidthName, width);

	icon_size iconSize = kDefaultIconSize;
	switch (fIconSize->IndexOf(fIconSize->FindMarked())) {
		case 0:
			iconSize = B_MINI_ICON;
			break;
		default:
			iconSize = B_LARGE_ICON;
	}
	settings.AddInt32(kIconSizeName, (int32)iconSize);

	return B_OK;
}


status_t
GeneralView::Revert()
{
	char buffer[255];
	(void)sprintf(buffer, "%" B_PRId32, fOriginalTimeout);
	fTimeout->SetText(buffer);
	
	char widthText[255];
	BMenuItem* item = NULL;
	(void)sprintf(widthText, "%.2f", fOriginalWidth);
	fWindowWidth->SetText(widthText);
	
	if (fOriginalIconSize == B_MINI_ICON)
		item = fIconSize->ItemAt(0);
	else
		item = fIconSize->ItemAt(1);
	if (item)
		item->SetMarked(true);
	
	return B_OK;
}


bool
GeneralView::RevertPossible()
{
	int32 timeout = atol(fTimeout->Text());
	if (fOriginalTimeout != timeout)
		return true;
	
	float width = atof(fWindowWidth->Text());
	if (fOriginalWidth != width)
		return true;
	
	icon_size iconSize = kDefaultIconSize;
	switch (fIconSize->IndexOf(fIconSize->FindMarked())) {
		case 0:
			iconSize = B_MINI_ICON;
			break;
		default:
			iconSize = B_LARGE_ICON;
	}
	if (fOriginalIconSize != iconSize)
		return true;
	
	return false;
}


bool
GeneralView::_CanFindServer(entry_ref* ref)
{
	// Try searching with be_roster
	if (be_roster->FindApp(kNotificationServerSignature, ref) == B_OK)
		return true;

	// Try with a query and take the first result
	BVolumeRoster vroster;
	BVolume volume;
	char volName[B_FILE_NAME_LENGTH];

	vroster.Rewind();

	while (vroster.GetNextVolume(&volume) == B_OK) {
		if ((volume.InitCheck() != B_OK) || !volume.KnowsQuery())
			continue;

		volume.GetName(volName);

		BQuery *query = new BQuery();
		query->SetPredicate("(BEOS:APP_SIG==\"" kNotificationServerSignature
			"\")");
		query->SetVolume(&volume);
		query->Fetch();

		if (query->GetNextRef(ref) == B_OK)
			return true;
	}

	return false;
}


bool
GeneralView::_IsServerRunning()
{
	return be_roster->IsRunning(kNotificationServerSignature);
}
