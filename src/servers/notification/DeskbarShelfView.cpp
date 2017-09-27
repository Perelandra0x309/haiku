/*
 * Copyright 2017, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Brian Hill, supernova@warpmail.net
 */


#include <Application.h>
#include <IconUtils.h>
#include <NodeInfo.h>
#include <Resources.h>
#include <Roster.h>

#include "DeskbarShelfView.h"

const char* kShelfviewName = "notifications_shelfview";
const char* kKeyMessenger = "messenger";


DeskbarShelfView::DeskbarShelfView()
	:
	BView(BRect(0, 0, 15, 15), kShelfviewName, B_FOLLOW_NONE, B_WILL_DRAW),
	fDrawNewIcon(false),
	fIcon(NULL),
	fNewIcon(NULL)
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

	// New notification icon
	result = B_ERROR;
	BResources resources(&info.ref);
	if (resources.InitCheck() == B_OK) {
		size_t size;
		const void* data = resources.LoadResource(B_VECTOR_ICON_TYPE, 1001, &size);
		if (data != NULL) {
			fNewIcon = new BBitmap(Bounds(), B_RGBA32);
			if (fNewIcon->InitCheck() == B_OK)
				result = BIconUtils::GetVectorIcon((const uint8 *)data,
					size, fNewIcon);
		}
	}
	if (result != B_OK) {
		printf("Error creating new notification bitmap\n");
		delete fNewIcon;
		fNewIcon = NULL;
	}

}


DeskbarShelfView::DeskbarShelfView(BMessage* message)
	:
	BView(message),
	fDrawNewIcon(false),
	fIcon(NULL),
	fNewIcon(NULL)
{
	BMessage iconArchive;
	status_t result = message->FindMessage("fIconArchive", &iconArchive);
	if (result == B_OK)
		fIcon = new BBitmap(&iconArchive);
	BMessage newIconArchive;
	result = message->FindMessage("fNewIconArchive", &newIconArchive);
	if (result == B_OK)
		fNewIcon = new BBitmap(&newIconArchive);
}


DeskbarShelfView::~DeskbarShelfView()
{
	delete fIcon;
	delete fNewIcon;
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

	// Register with the notification server
	BMessenger messenger(this);
	BMessage registration(kDeskbarRegistration);
	registration.AddMessenger(kKeyMessenger, messenger);
	status_t rc = B_ERROR;
	BMessenger appMessenger("application/x-vnd.Haiku-notification_server", -1, &rc);
	if(appMessenger.IsValid())
		appMessenger.SendMessage(&registration);
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
	return B_NO_ERROR;
}


void
DeskbarShelfView::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
		case kShowNewIcon:
			if (fDrawNewIcon)
				break;
			fDrawNewIcon = true;
			Invalidate();
			break;

		case kShowStandardIcon:
			if (!fDrawNewIcon)
				break;
			fDrawNewIcon = false;
			Invalidate();
			break;

		default:
			BView::MessageReceived(message);
	}
}


void
DeskbarShelfView::Draw(BRect rect)
{
	BBitmap* icon = fDrawNewIcon ? fNewIcon : fIcon;
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
	status_t rc = B_ERROR;
	BMessenger appMessenger("application/x-vnd.Haiku-notification_server", -1, &rc);
	if(!appMessenger.IsValid())
		return;
	appMessenger.SendMessage(kDeskbarReplicantClicked);

	// Draw the standard icon
	if (fDrawNewIcon) {
		fDrawNewIcon = false;
		Invalidate();
	}
}


void
DeskbarShelfView::_Quit()
{
	BDeskbar deskbar;
	deskbar.RemoveItem(kShelfviewName);
}
