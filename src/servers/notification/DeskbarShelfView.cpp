/*
 * Copyright 2017, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Brian Hill, supernova@tycho.email
 */


#include <Application.h>
#include <Catalog.h>
#include <IconUtils.h>
#include <MenuItem.h>
#include <NodeInfo.h>
#include <Resources.h>
#include <Roster.h>

#include "DeskbarShelfView.h"

const char* kShelfviewName = "notifications_shelfview";
const char* kKeyMessenger = "messenger";
const char* kKeyMuteAll = "mute_all";

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "DeskbarShelfView"


DeskbarShelfView::DeskbarShelfView()
	:
	BView(BRect(0, 0, 15, 15), kShelfviewName, B_FOLLOW_NONE, B_WILL_DRAW),
	fRegistrationAcknowledged(false),
	fIconState(ICONSTATE_DEFAULT),
	fIcon(NULL),
	fNewIcon(NULL),
	fMuteIcon(NULL),
	fMenu(NULL)
{
	// Standard icon
	app_info info;
	be_app->GetAppInfo(&info);
	fIcon = new BBitmap(Bounds(), B_RGBA32);
	status_t result = B_ERROR;
	if (fIcon->InitCheck() == B_OK)
		result = BNodeInfo::GetTrackerIcon(&info.ref, fIcon, B_MINI_ICON);
	if (result != B_OK) {
		printf("Error creating icon bitmap\n");
		delete fIcon;
		fIcon = NULL;
	}

	result = B_ERROR;
	BResources resources(&info.ref);
	if (resources.InitCheck() == B_OK) {
		size_t size;
		// New notification icon
		const void* data = resources.LoadResource(B_VECTOR_ICON_TYPE, 1001,
			&size);
		if (data != NULL) {
			fNewIcon = new BBitmap(Bounds(), B_RGBA32);
			if (fNewIcon->InitCheck() == B_OK)
				result = BIconUtils::GetVectorIcon((const uint8 *)data,
					size, fNewIcon);
		}
		// Mute icon
		const void* mutedata = resources.LoadResource(B_VECTOR_ICON_TYPE,
			1002, &size);
		if (mutedata != NULL) {
			fMuteIcon = new BBitmap(Bounds(), B_RGBA32);
			if (fMuteIcon->InitCheck() == B_OK)
				result = BIconUtils::GetVectorIcon((const uint8 *)mutedata,
					size, fMuteIcon);
		}
	}

/*	if (result != B_OK) {
		printf("Error creating new notification bitmap\n");
		delete fNewIcon;
		fNewIcon = NULL;
	}*/

}


DeskbarShelfView::DeskbarShelfView(BMessage* message)
	:
	BView(message),
	fRegistrationAcknowledged(false),
	fIconState(ICONSTATE_DEFAULT),
	fIcon(NULL),
	fNewIcon(NULL),
	fMuteIcon(NULL),
	fMenu(NULL)
{
	BMessage iconArchive;
	status_t result = message->FindMessage("fIconArchive", &iconArchive);
	if (result == B_OK)
		fIcon = new BBitmap(&iconArchive);
	BMessage newIconArchive;
	result = message->FindMessage("fNewIconArchive", &newIconArchive);
	if (result == B_OK)
		fNewIcon = new BBitmap(&newIconArchive);
	BMessage muteIconArchive;
	result = message->FindMessage("fMuteIconArchive", &muteIconArchive);
	if (result == B_OK)
		fMuteIcon = new BBitmap(&muteIconArchive);
}


DeskbarShelfView::~DeskbarShelfView()
{
	delete fIcon;
	delete fNewIcon;
	delete fMuteIcon;
	delete fMenu;
}


void
DeskbarShelfView::AttachedToWindow()
{
	BView::AttachedToWindow();
	if (Parent())
		SetViewColor(Parent()->ViewColor());
	else
		SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetLowColor(ViewColor());
	Invalidate();

	_BuildMenu();

	// Register with the notification server
	BMessenger messenger(this);
	BMessage registration(kDeskbarRegistration);
	registration.AddMessenger(kKeyMessenger, messenger);
	fServerMessenger.SetTo("application/x-vnd.Haiku-notification_server");
	if(fServerMessenger.IsValid())
		fServerMessenger.SendMessage(&registration, this);
}

/*
void
DeskbarShelfView::DetachedFromWindow()
{

}*/


DeskbarShelfView*
DeskbarShelfView::Instantiate(BMessage* data)
{
	if (!validate_instantiation(data, "DeskbarShelfView"))
		return NULL;
	return new DeskbarShelfView(data);
}


status_t
DeskbarShelfView::Archive(BMessage* data, bool deep) const
{
	BView::Archive(data, deep);
	data->AddString("add_on", "application/x-vnd.Haiku-notification_server");
	if (fIcon != NULL) {
		BMessage archive;
		fIcon->Archive(&archive);
		data->AddMessage("fIconArchive", &archive);
	}
	if (fNewIcon != NULL) {
		BMessage newArchive;
		fNewIcon->Archive(&newArchive);
		data->AddMessage("fNewIconArchive", &newArchive);
	}
	if (fMuteIcon != NULL) {
		BMessage muteArchive;
		fMuteIcon->Archive(&muteArchive);
		data->AddMessage("fMuteIconArchive", &muteArchive);
	}
	return B_NO_ERROR;
}


void
DeskbarShelfView::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
		case kRegistrationAcknowledge:
			if (message->IsReply())
				fRegistrationAcknowledged = true;
			break;
		case kMuteAllClicked:
		{
			if(fServerMessenger.IsValid())
				fServerMessenger.SendMessage(kMuteAllClicked);
			// TODO else
			
			break;
		}
		case kDeskbarSync:
		{
			bool mute;
			status_t result = message->FindBool(kKeyMuteAll, &mute);
			if (result == B_OK) {
				BMenuItem* item = fMenu->ItemAt(0);
				item->SetMarked(mute);
				// icon
				if (mute)
					fIconState = ICONSTATE_MUTE;
				else
					fIconState = ICONSTATE_DEFAULT;
				Invalidate();
			}
			break;
		}
		case kShowSettings:
			be_roster->Launch("application/x-vnd.Haiku-Notifications");
			break;

		default:
			BView::MessageReceived(message);
	}
}


void
DeskbarShelfView::Draw(BRect rect)
{
	BBitmap* icon = NULL;
	switch (fIconState) {
		case ICONSTATE_DEFAULT:
			icon = fIcon;
			break;
		case ICONSTATE_NEW:
			icon = fNewIcon;
			break;
		case ICONSTATE_MUTE:
			icon = fMuteIcon;
			break;
	}
	if (icon == NULL)
		return;

	SetDrawingMode(B_OP_ALPHA);
	DrawBitmap(icon);
	SetDrawingMode(B_OP_COPY);
}


void
DeskbarShelfView::MouseDown(BPoint pos)
{
	if(!be_roster->IsRunning("application/x-vnd.Haiku-notification_server")) {
		_Quit(); //TODO show alert?
		return;
	}
	ConvertToScreen(&pos);
	if (fMenu)
		fMenu->Go(pos, true, true, BRect(pos.x - 2, pos.y - 2,
			pos.x + 2, pos.y + 2), true);
}


void
DeskbarShelfView::_Quit()
{
	BDeskbar deskbar;
	deskbar.RemoveItem(kShelfviewName);
}


void
DeskbarShelfView::_BuildMenu()
{
	delete fMenu;
	fMenu = new BPopUpMenu(B_EMPTY_STRING, false, false);
	fMenu->SetFont(be_plain_font);
	fMenu->AddItem(new BMenuItem(B_TRANSLATE_COMMENT("Mute all notifications",
		"Deskbar menu option"), new BMessage(kMuteAllClicked)));
	fMenu->AddSeparatorItem();
	fMenu->AddItem(new BMenuItem(B_TRANSLATE_COMMENT("Show History"
		B_UTF8_ELLIPSIS, "Deskbar menu option"), new BMessage('test')));
	fMenu->AddItem(new BMenuItem(B_TRANSLATE_COMMENT("Settings"B_UTF8_ELLIPSIS,
		"Deskbar menu option"), new BMessage(kShowSettings)));

	BMenuItem* item;
	BMessage* msg;
	for (int32 i = fMenu->CountItems(); i-- > 0;) {
		item = fMenu->ItemAt(i);
		if (item && (msg = item->Message()) != NULL)
			item->SetTarget(this);
	}
}
