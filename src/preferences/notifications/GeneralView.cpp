/*
 * Copyright 2010-2017, Haiku, Inc. All Rights Reserved.
 * Copyright 2009, Pier Luigi Fiorini.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 *		Brian Hill, supernova@tycho.email
 */

#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include <Alert.h>
#include <Box.h>
#include <Button.h>
#include <Catalog.h>
#include <Directory.h>
#include <File.h>
#include <FindDirectory.h>
#include <Font.h>
#include <LayoutBuilder.h>
#include <Node.h>
#include <Notification.h>
#include <Path.h>
#include <Query.h>
#include <Roster.h>
#include <String.h>
#include <SymLink.h>
#include <Volume.h>
#include <VolumeRoster.h>

#include <notification/Notifications.h>

#include "GeneralView.h"
#include "NotificationsConstants.h"
#include "SettingsHost.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "GeneralView"

const int32 kToggleNotifications = '_TSR';
const int32 kWidthChanged = '_WIC';
const int32 kTimeoutChanged = '_TIC';
const int32 kServerChangeTriggered = '_SCT';
const BString kSampleMessageID("NotificationsSample");


GeneralView::GeneralView(SettingsHost* host)
	:
	SettingsPane("general", host),
	fServerChangeTriggered(false)
{
	// Notification server
	fNotificationBox = new BCheckBox("server",
		B_TRANSLATE("Enable notifications"),
		new BMessage(kToggleNotifications));
	BBox* box = new BBox("box");
	box->SetLabel(fNotificationBox);

	// Window width
	int32 minWidth = int32(kMinimumWidth / kWidthStep);
	int32 maxWidth = int32(kMaximumWidth / kWidthStep);
	fWidthSlider = new BSlider("width", B_TRANSLATE("Window width:"),
		new BMessage(kWidthChanged), minWidth, maxWidth, B_HORIZONTAL);
	fWidthSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fWidthSlider->SetHashMarkCount(maxWidth - minWidth + 1);
	BString minWidthLabel;
	minWidthLabel << int32(kMinimumWidth);
	BString maxWidthLabel;
	maxWidthLabel << int32(kMaximumWidth);
	fWidthSlider->SetLimitLabels(
		B_TRANSLATE_COMMENT(minWidthLabel.String(), "Slider low text"),
		B_TRANSLATE_COMMENT(maxWidthLabel.String(), "Slider high text"));

	// Display time
	fDurationSlider = new BSlider("duration", B_TRANSLATE("Duration:"),
		new BMessage(kTimeoutChanged), kMinimumTimeout, kMaximumTimeout,
		B_HORIZONTAL);
	fDurationSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fDurationSlider->SetHashMarkCount(kMaximumTimeout - kMinimumTimeout + 1);
	BString minLabel;
	minLabel << kMinimumTimeout;
	BString maxLabel;
	maxLabel << kMaximumTimeout;
	fDurationSlider->SetLimitLabels(
		B_TRANSLATE_COMMENT(minLabel.String(), "Slider low text"),
		B_TRANSLATE_COMMENT(maxLabel.String(), "Slider high text"));

	box->AddChild(BLayoutBuilder::Group<>(B_VERTICAL)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.Add(fWidthSlider)
		.Add(fDurationSlider)
		.AddGlue()
		.View());
	
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_SPACING)
		.Add(box)
	.End();
}


void
GeneralView::AttachedToWindow()
{
	BView::AttachedToWindow();
	fNotificationBox->SetTarget(this);
	fWidthSlider->SetTarget(this);
	fDurationSlider->SetTarget(this);
}


void
GeneralView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case B_SOME_APP_QUIT:
		case B_SOME_APP_LAUNCHED:
		{
			//Find the signature and check if it was the notification server
			char* sig;
			if (msg->FindString("be:signature", (const char**)&sig) == B_OK)
			{
				BString signature(sig);
				if (signature.Compare(kNotificationServerSignature) == 0)
				{
					// If this preflet triggered the change then ignore message
					if (fServerChangeTriggered) {
						fServerChangeTriggered = false;
						break;
					}
					BString text;
					if (msg->what == B_SOME_APP_QUIT)
						text.SetTo(B_TRANSLATE("The notification server has quit."));
					else
						text.SetTo(B_TRANSLATE("The notification server started."));
					BAlert* alert = new BAlert("", text.String(),
						B_TRANSLATE("OK"));
					alert->Go(NULL);
					fNotificationBox->SetValue(_IsServerRunning() ?
						B_CONTROL_ON : B_CONTROL_OFF);
					_EnableControls();
				}
			}
			break;
		}
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

			if (fNotificationBox->Value() == B_CONTROL_OFF && _IsServerRunning()) {
				// Server team
				team_id team = be_roster->TeamFor(kNotificationServerSignature);

				// Establish a connection to server
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
				fServerChangeTriggered = true;
			} else if (fNotificationBox->Value() == B_CONTROL_ON && !_IsServerRunning()) {
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
				fServerChangeTriggered = true;
			}
			_EnableControls();
			break;
		}
		case kWidthChanged: {
			int32 value = fWidthSlider->Value() * 50;
			_SetWidthLabel(value);
			SettingsPane::MessageReceived(new BMessage(kSettingChanged));
			_EnableControls();
			_SendSampleNotification();
			break;
		}
		case kTimeoutChanged:
		{
			int32 value = fDurationSlider->Value();
			_SetTimeoutLabel(value);
			SettingsPane::MessageReceived(new BMessage(kSettingChanged));
			_EnableControls();
			_SendSampleNotification();
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

	if (settings.FindFloat(kWidthName, &fOriginalWidth) != B_OK
		|| fOriginalWidth > kMaximumWidth
		|| fOriginalWidth < kMinimumWidth)
		fOriginalWidth = kDefaultWidth;

	if (settings.FindInt32(kTimeoutName, &fOriginalTimeout) != B_OK
		|| fOriginalTimeout > kMaximumTimeout
		|| fOriginalTimeout < kMinimumTimeout)
		fOriginalTimeout = kDefaultTimeout;
// TODO need to save again if values outside of expected range
	int32 setting;
	if (settings.FindInt32(kIconSizeName, &setting) != B_OK)
		fOriginalIconSize = kDefaultIconSize;
	else
		fOriginalIconSize = (icon_size)setting;

	_EnableControls();
	
	return Revert();
}


status_t
GeneralView::Save(BMessage& settings)
{
	bool autoStart = (fNotificationBox->Value() == B_CONTROL_ON);
	settings.AddBool(kAutoStartName, autoStart);

	int32 timeout = fDurationSlider->Value();
	settings.AddInt32(kTimeoutName, timeout);

	float width = fWidthSlider->Value() * 50;
	settings.AddFloat(kWidthName, width);

	icon_size iconSize = B_LARGE_ICON;
	settings.AddInt32(kIconSizeName, (int32)iconSize);

	return B_OK;
}


status_t
GeneralView::Revert()
{
	fDurationSlider->SetValue(fOriginalTimeout);
	_SetTimeoutLabel(fOriginalTimeout);
	
	fWidthSlider->SetValue(fOriginalWidth / 50);
	_SetWidthLabel(fOriginalWidth);
	
	return B_OK;
}


bool
GeneralView::RevertPossible()
{
	int32 timeout = fDurationSlider->Value();
	if (fOriginalTimeout != timeout)
		return true;
	
	int32 width = fWidthSlider->Value() * 50;
	if (fOriginalWidth != width)
		return true;

	return false;
}


status_t
GeneralView::Defaults()
{
	fDurationSlider->SetValue(kDefaultTimeout);
	_SetTimeoutLabel(kDefaultTimeout);

	fWidthSlider->SetValue(kDefaultWidth / 50);
	_SetWidthLabel(kDefaultWidth);

	return B_OK;
}


bool
GeneralView::DefaultsPossible()
{
	int32 timeout = fDurationSlider->Value();
	if (kDefaultTimeout != timeout)
		return true;

	int32 width = fWidthSlider->Value() * 50;
	if (kDefaultWidth != width)
		return true;
	
	return false;
}


bool
GeneralView::UseDefaultRevertButtons()
{
	return true;
}


void
GeneralView::_EnableControls()
{
	bool enabled = fNotificationBox->Value() == B_CONTROL_ON;
	fWidthSlider->SetEnabled(enabled);
	fDurationSlider->SetEnabled(enabled);
}


void
GeneralView::_SendSampleNotification()
{
	BNotification notification(B_INFORMATION_NOTIFICATION);
	notification.SetMessageID(kSampleMessageID);
	notification.SetGroup(B_TRANSLATE("Notifications"));
	notification.SetTitle(B_TRANSLATE("Notifications preflet sample"));
	notification.SetContent(B_TRANSLATE("This is a test notification message"));
	notification.Send(fDurationSlider->Value() * 1000000);
}


void
GeneralView::_SetTimeoutLabel(int32 value)
{
	BString label(B_TRANSLATE("Timeout:"));
	label.Append(" ");
	label << value;
	label.Append(" ").Append(B_TRANSLATE("seconds"));
	fDurationSlider->SetLabel(label.String());
}


void
GeneralView::_SetWidthLabel(int32 value)
{
	BString label(B_TRANSLATE("Width:"));
	label.Append(" ");
	label << value;
	label.Append(" ").Append(B_TRANSLATE("pixels"));
	fWidthSlider->SetLabel(label.String());
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
