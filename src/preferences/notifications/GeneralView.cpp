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
#include <Font.h>
#include <LayoutBuilder.h>
#include <Node.h>
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

const uint32 kToggleNotifications = '_TSR';
const uint32 kWidthChanged = '_WIC';
const uint32 kTimeoutChanged = '_TIC';
const uint32 kShowDeskbarChanged = '_SDC';
const uint32 kServerChangeTriggered = '_SCT';
const BString kSampleMessageID("NotificationsSample");


GeneralView::GeneralView(SettingsHost* host)
	:
	SettingsPane("general", host)
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

	// Deskbar replicant view
	fShowDeskbar = new BCheckBox("deskbar",
		B_TRANSLATE("Show status in Deskbar"),
		new BMessage(kShowDeskbarChanged));

	box->AddChild(BLayoutBuilder::Group<>(B_VERTICAL)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.Add(fWidthSlider)
		.Add(fDurationSlider)
		.Add(fShowDeskbar)
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
	fShowDeskbar->SetTarget(this);
}


void
GeneralView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case kToggleNotifications:
		case kShowDeskbarChanged:
		{
			SettingsPane::SettingsChanged(false);
			_EnableControls();
			break;
		}
		case kWidthChanged: {
			int32 value = fWidthSlider->Value() * 50;
			_SetWidthLabel(value);
			SettingsPane::SettingsChanged(true);
			break;
		}
		case kTimeoutChanged:
		{
			int32 value = fDurationSlider->Value();
			_SetTimeoutLabel(value);
			SettingsPane::SettingsChanged(true);
			break;
		}
		default:
			BView::MessageReceived(msg);
			break;
	}
}


status_t
GeneralView::Load(BMessage& settings)
{
	bool autoStart = settings.GetBool(kAutoStartName, true);
	fNotificationBox->SetValue(autoStart ? B_CONTROL_ON : B_CONTROL_OFF);

	if (settings.FindFloat(kWidthName, &fOriginalWidth) != B_OK
		|| fOriginalWidth > kMaximumWidth
		|| fOriginalWidth < kMinimumWidth)
		fOriginalWidth = kDefaultWidth;

	if (settings.FindInt32(kTimeoutName, &fOriginalTimeout) != B_OK
		|| fOriginalTimeout > kMaximumTimeout
		|| fOriginalTimeout < kMinimumTimeout)
		fOriginalTimeout = kDefaultTimeout;
// TODO need to save again if values outside of expected range

	if (settings.FindBool(kShowDeskbarName, &fOriginalShowDeskbar) != B_OK)
		fOriginalShowDeskbar = kDefaultShowDeskbar;

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

	bool showDeskbar = (fShowDeskbar->Value() == B_CONTROL_ON);
	settings.AddBool(kShowDeskbarName, showDeskbar);

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

	fShowDeskbar->SetValue(fOriginalShowDeskbar ?
		B_CONTROL_ON : B_CONTROL_OFF);

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

	bool showDeskbar = (fShowDeskbar->Value() == B_CONTROL_ON);
	if (fOriginalShowDeskbar != showDeskbar)
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

	fShowDeskbar->SetValue(kDefaultShowDeskbar ?
		B_CONTROL_ON : B_CONTROL_OFF);

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

	bool showDeskbar = (fShowDeskbar->Value() == B_CONTROL_ON);
	if (kDefaultShowDeskbar != showDeskbar)
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
	fShowDeskbar->SetEnabled(enabled);
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
GeneralView::_IsServerRunning()
{
	return be_roster->IsRunning(kNotificationServerSignature);
}
