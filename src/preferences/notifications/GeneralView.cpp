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
//#include <SeparatorView.h>
#include <Screen.h>
#include <String.h>
#include <SymLink.h>
#include <Volume.h>
#include <VolumeRoster.h>

#include <notification/Notifications.h>

#include "GeneralView.h"
#include "SettingsHost.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "GeneralView"
const int32 kToggleNotifications = '_TSR';
const int32 kWidthChanged = '_WIC';
const int32 kTimeoutChanged = '_TIC';
const BString kSampleMessageID("NotificationsSample");


GeneralView::GeneralView(SettingsHost* host)
	:
	SettingsPane("general", host)
{
	// Notifications
	fNotificationBox = new BCheckBox("server",
		B_TRANSLATE("Enable notifications"),
		new BMessage(kToggleNotifications));
/*	BStringView* serverStatusLabel = new BStringView("status_label",
		B_TRANSLATE("Notifications:"));
	serverStatusLabel->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_TOP));
	fStatusEnabled = new BRadioButton("status_enabled", B_TRANSLATE("Enabled"),
		new BMessage(kToggleNotifications));
	fStatusDisabled = new BRadioButton("status_disabled", B_TRANSLATE("Disabled"),
		new BMessage(kToggleNotifications));*/

	// Autostart
//	fAutoStart = new BCheckBox("autostart",
//		B_TRANSLATE("Enable notifications at startup"),
//		new BMessage(kSettingChanged));

	// Window width
/*	fWindowWidth = new BTextControl(B_TRANSLATE("Window width:"), NULL,
		new BMessage(kSettingChanged));
	fWidthLabel = new BStringView("width_label", B_TRANSLATE("Window width:"));*/
	fWidthSlider = new BSlider("width", B_TRANSLATE("Window width:"),
		new BMessage(kWidthChanged), 6, 20, B_HORIZONTAL);
	fWidthSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fWidthSlider->SetHashMarkCount(15);
	fWidthSlider->SetLimitLabels(B_TRANSLATE_COMMENT("300", "Slider low text"),
						B_TRANSLATE_COMMENT("1000", "Slider high text"));

	// Display time
/*	fTimeout = new BTextControl(B_TRANSLATE("Duration (seconds):"),
		NULL, new BMessage(kSettingChanged));
	fDisplayTimeLabel = new BStringView("dt_label",
		B_TRANSLATE("seconds of inactivity"));*/
	fDurationSlider = new BSlider("duration", B_TRANSLATE("Duration:"),
		new BMessage(kTimeoutChanged), 5, 60, B_HORIZONTAL);
	fDurationSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
//	fDurationSlider->SetHashMarkCount(5);
	fDurationSlider->SetLimitLabels(B_TRANSLATE_COMMENT("5", "Slider low text"),
						B_TRANSLATE_COMMENT("60", "Slider high text"));

	// Icon size
/*	fIconSize = new BMenu("iconSize");
	fIconSize->AddItem(new BMenuItem(B_TRANSLATE("Mini icon"),
		new BMessage(kSettingChanged)));
	fIconSize->AddItem(new BMenuItem(B_TRANSLATE("Large icon"),
		new BMessage(kSettingChanged)));
	fIconSize->SetLabelFromMarked(true);
	fIconSizeField = new BMenuField(B_TRANSLATE("Icon size:"), fIconSize);*/
/*	BStringView* sizeLabel = new BStringView("size_label",
		B_TRANSLATE("Icon size:"));
	sizeLabel->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_TOP));
	fMiniSize = new BRadioButton("mini", B_TRANSLATE("Mini icon"),
		new BMessage(kSettingChanged));
	fLargeSize = new BRadioButton("large", B_TRANSLATE("Large icon"),
		new BMessage(kSettingChanged));*/

	// Do not disturb
	fDoNotDisturb = new BCheckBox("donotdisturb", B_TRANSLATE("Do not disturb:"),
		new BMessage(kSettingChanged));
/*	fDNDFrom = new BTextControl(B_TRANSLATE("From:"), NULL,
		new BMessage(kSettingChanged));
	fDNDTo = new BTextControl(B_TRANSLATE("To:"), NULL,
		new BMessage(kSettingChanged));*/
	BStringView* fromTimeLabel = new BStringView("from_label",
		B_TRANSLATE("From"));
	fFromTimeEdit = new TTimeEdit("timeEdit", 5);
	BStringView* toTimeLabel = new BStringView("to_label",
		B_TRANSLATE("To"));
	fToTimeEdit = new TTimeEdit("timeEdit", 5);

	// Default position
	// TODO: Here will come a screen representation with the four corners
	// clickable
	font_height fontHeight;
	be_plain_font->GetHeight(&fontHeight);
	float textHeight = ceilf(fontHeight.ascent + fontHeight.descent);
	float monitorHeight = 10 + textHeight * 3;
	float aspectRatio = 4.0f / 3.0f;
	float monitorWidth = monitorHeight * aspectRatio;
	BRect monitorRect = BRect(0, 0, monitorWidth, monitorHeight);
	BStringView* cornerLabel = new BStringView("corner_label",
		B_TRANSLATE("Location:"));
	fCornerSelector = new ScreenCornerSelector(monitorRect, "FadeNow",
		NULL, B_FOLLOW_NONE);
	
	BBox* box = new BBox("box");
	box->SetLabel(fNotificationBox);

	box->AddChild(BLayoutBuilder::Group<>(B_VERTICAL)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.Add(fWidthSlider)
		.Add(fDurationSlider)
		.Add(fDoNotDisturb)
		.AddGroup(B_HORIZONTAL)
			.SetInsets(B_USE_DEFAULT_SPACING, 0, 0, 0)
			.AddGrid()
				.Add(fromTimeLabel, 0, 0)
				.Add(fFromTimeEdit, 1, 0)
				.Add(toTimeLabel, 2, 0)
				.Add(fToTimeEdit, 3, 0)
			.End()
			.AddGlue(10)
		.End()
		.AddGrid(B_USE_DEFAULT_SPACING, B_USE_WINDOW_SPACING)
/*			.Add(fWidthLabel, 0, 0)
			.Add(fWidthSlider, 1, 0, 2)
			.Add(new BStringView("duration_label", "Duration:"), 0, 1)
			.Add(fDurationSlider, 1, 1, 2)*/
	//		.AddTextControl(fTimeout, 0, 1)
	//		.Add(fDisplayTimeLabel, 2, 1)
//			.End()
//			.AddGlue(10)
//		.End()
/*			.AddGlue(0, 2, 3)
			.Add(fDoNotDisturb, 0, 3)
			.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING, 1, 3, 2)
				.AddGrid(B_USE_SMALL_SPACING)
					.Add(fromTimeLabel, 0, 0)
					.Add(fFromTimeEdit, 1, 0)
					.Add(toTimeLabel, 0, 1)
					.Add(fToTimeEdit, 1, 1)
				.End()
				.AddGlue(10)
			.End()*/
			.AddGlue(0, 4, 3)
	//		.AddGroup(B_HORIZONTAL)
			.Add(cornerLabel, 0, 5)
			.Add(fCornerSelector, 1, 5)
	//			.AddGlue(10)
			
			.AddGlue(0, 6, 3)
		.End()
//		.AddGlue()
		.View());
/*	box->AddChild(BLayoutBuilder::Group<>(B_VERTICAL)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.AddGroup(B_HORIZONTAL)
			.AddGrid()
				.Add(fWidthSlider, 0, 2, 3)
				.AddTextControl(fTimeout, 0, 3)
				.Add(fDisplayTimeLabel, 2, 3)
			.End()
			.AddGlue(10)
		.End()
		.Add(fDoNotDisturb)
		.AddGroup(B_HORIZONTAL)
			.SetInsets(B_USE_DEFAULT_SPACING, 0, 0, 0)
			.AddGrid()
				.Add(fromTimeLabel, 0, 0)
				.Add(fFromTimeEdit, 1, 0)
				.Add(toTimeLabel, 2, 0)
				.Add(fToTimeEdit, 3, 0)
			.End()
			.AddGlue(10)
		.End()
		.AddGroup(B_HORIZONTAL)
			.Add(cornerLabel)
			.Add(fCornerSelector)
			.AddGlue(10)
		.End()
		.AddGlue()
		.View());*/
	
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_SPACING)
		.Add(box)
	.End();
/*	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_SPACING)
//		.Add(fNotificationBox)
//		.Add(fAutoStart)
		.AddGroup(B_HORIZONTAL)
			.AddGrid()
				.Add(serverStatusLabel, 0, 0)
				.AddGroup(new BGroupView(B_VERTICAL, 0), 1, 0)
					.Add(fStatusEnabled)
					.Add(fStatusDisabled)
				.End()
				.AddTextControl(fWindowWidth, 0, 1)
				.Add(sizeLabel, 0, 2)
				.AddGroup(B_VERTICAL, 0, 1, 2)
					.Add(fMiniSize)
					.Add(fLargeSize)
				.End()
				.AddTextControl(fTimeout, 0, 3)
				.Add(displayTimeLabel, 2, 3)
			.End()
			.AddGlue(10)
		.End()
		.Add(fAutoStart)
		
//		.Add(new BSeparatorView(B_HORIZONTAL))
//		.Add(fStatusBox)
		.AddGlue()
	.End();*/
}


void
GeneralView::AttachedToWindow()
{
	BView::AttachedToWindow();
	fNotificationBox->SetTarget(this);
//	fTimeout->SetTarget(this);
//	fWindowWidth->SetTarget(this);
	fWidthSlider->SetTarget(this);
	fDurationSlider->SetTarget(this);
//	fMiniSize->SetTarget(this);
//	fLargeSize->SetTarget(this);
//	fStatusEnabled->SetTarget(this);
//	fStatusDisabled->SetTarget(this);
//	fIconSize->SetTargetForItems(this);
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

			if (fNotificationBox->Value() == B_CONTROL_OFF && _IsServerRunning()) {
//			if (fStatusDisabled->Value() == B_CONTROL_ON && _IsServerRunning()) {
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
			} else if (fNotificationBox->Value() == B_CONTROL_ON && !_IsServerRunning()) {
//			} else if (fStatusEnabled->Value() == B_CONTROL_ON && !_IsServerRunning()) {
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
			_EnableControls();
			break;
		}
		case kWidthChanged: {
			int32 value = fWidthSlider->Value() * 50;
			_SetWidthLabel(value);
		/*	BString label(B_TRANSLATE("Window width:"));
			label.Append(" ");
			label << value;
			label.Append("%");
			fWidthSlider->SetLabel(label.String());*/
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
/*	if (_IsServerRunning())
		fStatusEnabled->SetValue(B_CONTROL_ON);
	else
		fStatusDisabled->SetValue(B_CONTROL_ON);*/

//	bool autoStart;
//	if (settings.FindBool(kAutoStartName, &autoStart) != B_OK)
//		autoStart = kDefaultAutoStart;
//	fAutoStart->SetValue(autoStart ? B_CONTROL_ON : B_CONTROL_OFF);

	if (settings.FindInt32(kTimeoutName, &fOriginalTimeout) != B_OK)
		fOriginalTimeout = kDefaultTimeout;

	if (settings.FindFloat(kWidthName, &fOriginalWidth) != B_OK)
		fOriginalWidth = kDefaultWidth;

	int32 setting;
	if (settings.FindInt32(kIconSizeName, &setting) != B_OK)
		fOriginalIconSize = kDefaultIconSize;
	else
		fOriginalIconSize = (icon_size)setting;

	/*int32 hour;
	int32 minute;
	int32 second;
	if (message->FindInt32("hour", &hour) == B_OK
		&& message->FindInt32("minute", &minute) == B_OK
		&& message->FindInt32("second", &second) == B_OK) {
		fClock->SetTime(hour, minute, second);
		fTimeEdit->SetTime(hour, minute, second);
	}*/

	_EnableControls();
	
	return Revert();
}


status_t
GeneralView::Save(BMessage& settings)
{
//	bool autoStart = (fAutoStart->Value() == B_CONTROL_ON);
	bool autoStart = (fNotificationBox->Value() == B_CONTROL_ON);
	settings.AddBool(kAutoStartName, autoStart);

//	int32 timeout = atol(fTimeout->Text());
	int32 timeout = fDurationSlider->Value();
	settings.AddInt32(kTimeoutName, timeout);

//	float width = atof(fWindowWidth->Text());
	// TODO change server to dynamically use % value
//	BScreen screen;
//	float percent = fWidthSlider->Value() / 100.0;
//	float width = percent * screen.Frame().Width();
	float width = fWidthSlider->Value() * 50;
	settings.AddFloat(kWidthName, width);

/*	icon_size iconSize = kDefaultIconSize;
	switch (fIconSize->IndexOf(fIconSize->FindMarked())) {
		case 0:
			iconSize = B_MINI_ICON;
			break;
		default:
			iconSize = B_LARGE_ICON;
	}*/
//	icon_size iconSize = fMiniSize->Value() == B_CONTROL_ON ? B_MINI_ICON : B_LARGE_ICON;
	icon_size iconSize = B_LARGE_ICON;
	settings.AddInt32(kIconSizeName, (int32)iconSize);

	return B_OK;
}


status_t
GeneralView::Revert()
{
//	char buffer[255];
//	(void)sprintf(buffer, "%" B_PRId32, fOriginalTimeout);
//	fTimeout->SetText(buffer);
	fDurationSlider->SetValue(fOriginalTimeout);
	_SetTimeoutLabel(fOriginalTimeout);
	
//	char widthText[255];
//	(void)sprintf(widthText, "%.0f", fOriginalWidth);
//	fWindowWidth->SetText(widthText);
	fWidthSlider->SetValue(fOriginalWidth / 50);
	_SetWidthLabel(fOriginalWidth);
	
/*	if (fOriginalIconSize == B_MINI_ICON)
		fMiniSize->SetValue(B_CONTROL_ON);
	else
		fLargeSize->SetValue(B_CONTROL_ON);*/
/*		item = fIconSize->ItemAt(0);
	else
		item = fIconSize->ItemAt(1);
	if (item)
		item->SetMarked(true);*/
	
	return B_OK;
}


bool
GeneralView::RevertPossible()
{
//	int32 timeout = atol(fTimeout->Text());
	int32 timeout = fDurationSlider->Value();
	if (fOriginalTimeout != timeout)
		return true;
	
//	float width = atof(fWindowWidth->Text());
	int32 width = fWidthSlider->Value() * 50;
	if (fOriginalWidth != width)
		return true;
	
/*	icon_size iconSize = kDefaultIconSize;
	switch (fIconSize->IndexOf(fIconSize->FindMarked())) {
		case 0:
			iconSize = B_MINI_ICON;
			break;
		default:
			iconSize = B_LARGE_ICON;
	}*/
/*	icon_size iconSize = fMiniSize->Value() == B_CONTROL_ON ? B_MINI_ICON : B_LARGE_ICON;
	if (fOriginalIconSize != iconSize)
		return true;*/
	
	return false;
}


status_t
GeneralView::Defaults()
{
/*	char buffer[255];
	(void)sprintf(buffer, "%" B_PRId32, kDefaultTimeout);
	fTimeout->SetText(buffer);*/
	fDurationSlider->SetValue(kDefaultTimeout);
	_SetTimeoutLabel(kDefaultTimeout);
	
//	char widthText[255];
//	(void)sprintf(widthText, "%.0f", kDefaultWidth);
//	fWindowWidth->SetText(widthText);
	fWidthSlider->SetValue(kDefaultWidth / 50);
	_SetWidthLabel(kDefaultWidth);
	
/*	if (kDefaultIconSize == B_MINI_ICON)
		fMiniSize->SetValue(B_CONTROL_ON);
	else
		fLargeSize->SetValue(B_CONTROL_ON);*/
	
	return B_OK;
}


bool
GeneralView::DefaultsPossible()
{
//	int32 timeout = atol(fTimeout->Text());
	int32 timeout = fDurationSlider->Value();
	if (kDefaultTimeout != timeout)
		return true;

//	float width = atof(fWindowWidth->Text());
	int32 width = fWidthSlider->Value() * 50;
	if (kDefaultWidth != width)
		return true;

//	icon_size iconSize = fMiniSize->Value() == B_CONTROL_ON ? B_MINI_ICON : B_LARGE_ICON;
//	if (kDefaultIconSize != iconSize)
//		return true;
	
	return false;
}


void
GeneralView::_EnableControls()
{
	bool enabled = fNotificationBox->Value() == B_CONTROL_ON;
//	fTimeout->SetEnabled(enabled);
//	fDisplayTimeLabel->SetEnabled(enabled);
//	fWindowWidth->SetEnabled(enabled);
	fWidthSlider->SetEnabled(enabled);
	fDurationSlider->SetEnabled(enabled);
	fDoNotDisturb->SetEnabled(enabled);
	fFromTimeEdit->SetEnabled(enabled);
//	fMiniSize->SetEnabled(enabled);
//	fLargeSize->SetEnabled(enabled);
}


void
GeneralView::_SendSampleNotification()
{
	BNotification notification(B_INFORMATION_NOTIFICATION);
	notification.SetMessageID(kSampleMessageID);
	notification.SetGroup(B_TRANSLATE("Notifications"));
	notification.SetTitle(B_TRANSLATE("Notifications preflet sample"));
	notification.SetContent(B_TRANSLATE("This is a test notification message"));
	notification.Send();
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
