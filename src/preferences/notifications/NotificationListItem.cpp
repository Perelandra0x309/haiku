/*
 * Copyright 2017, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#include "NotificationListItem.h"

#include <notification/Notifications.h>


NotificationListItem::NotificationListItem(BMessage& notificationData)
	:
	BListItem(),
	fInitStatus(B_ERROR)
{
	status_t result = notificationData.FindMessage(kNameNotificationMessage, &fNotificationMessage);
	if (result != B_OK)
		return;
	BNotification notification(&fNotificationMessage);
	fType = notification.Type();
	fTitle = notification.Title();
	fContent = notification.Content();
	
	result = notificationData.FindInt32(kNameTimestamp, &fTimestamp);
	if (result != B_OK)
		return;

	result = notificationData.FindBool(kNameWasShown, &fWasShown);
	if (result != B_OK)
		return;

	fInitStatus = B_OK;
}

/*
NotificationListItem::~NotificationListItem()
{

}*/


void
NotificationListItem::DrawItem(BView *owner, BRect item_rect, bool complete)
{
	float offset_width = 0, offset_height = fFontAscent;
//	float listItemHeight = Height();
	rgb_color backgroundColor;

	//background
	if(IsSelected()) {
		if(owner->IsFocus())
			backgroundColor = ui_color(B_LIST_SELECTED_BACKGROUND_COLOR);
		else
			backgroundColor = ui_color(B_LIST_BACKGROUND_COLOR);
	}
	else {
		backgroundColor = ui_color(B_LIST_BACKGROUND_COLOR);
	}
	owner->SetHighColor(backgroundColor);
	owner->SetLowColor(backgroundColor);
	owner->FillRect(item_rect);

	//text
//	if(listItemHeight > fTextOnlyHeight)
//		offset_height += floor( (listItemHeight - fTextOnlyHeight)/2 );
			// center the text vertically
	BPoint cursor(item_rect.left + offset_width, item_rect.top + offset_height /*+ kTextMargin*/);
	if(IsSelected())
		owner->SetHighColor(ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR));
	else
		owner->SetHighColor(ui_color(B_LIST_ITEM_TEXT_COLOR));
	owner->MovePenTo(cursor.x, cursor.y);
	
	if (fTitle.Length() > 0) {
		owner->PushState();
		owner->SetFont(be_bold_font);
		BString text(fTitle);
		text.Append(": ");
		owner->DrawString(text);
		float stringWidth = owner->StringWidth(text);
		owner->PopState();
		owner->MovePenBy(stringWidth, 0);
	}
	owner->DrawString(fContent);
}


void
NotificationListItem::Update(BView *owner, const BFont *font)
{
	BListItem::Update(owner, font);
	font_height fontHeight;
	font->GetHeight(&fontHeight);
	fFontHeight = fontHeight.ascent + fontHeight.descent + fontHeight.leading;
	fFontAscent = fontHeight.ascent;
}
