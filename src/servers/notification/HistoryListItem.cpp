/*
 * Copyright 2017, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Brian Hill, supernova@tycho.email
 */
#include "HistoryListItem.h"

#include <DateFormat.h>
#include <DateTime.h>
#include <TimeFormat.h>
#include <notification/Notifications.h>


HistoryListItem::HistoryListItem(BMessage& notificationData)
	:
	BListItem(),
	fInitStatus(B_ERROR),
	fIsDateDivider(false)
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

	result = notificationData.FindBool(kNameWasAllowed, &fWasAllowed);
	if (result != B_OK)
		return;

	fInitStatus = B_OK;
}


HistoryListItem::HistoryListItem(int32 timestamp)
	:
	BListItem(),
	fInitStatus(B_ERROR),
	fIsDateDivider(true)
{
	// Convert timestamp to midnight local
//	fTimestamp = timestamp;
	BTime midnight(0, 0, 0);
	BDate date(timestamp);
	BDateTime midnightDate(date, midnight);
	fTimestamp = midnightDate.Time_t();
	
	BDateFormat formatter;
	BString dateString;
	status_t result = formatter.Format(fDateLabel, fTimestamp,
		B_MEDIUM_DATE_FORMAT);
	// TODO testing
/*	BString time;
	BTimeFormat timeFormatter;
	timeFormatter.Format(time, fTimestamp,
		B_LONG_TIME_FORMAT);
	fDateLabel.Append(" ").Append(time);*/
	
	if (result == B_OK)
		fInitStatus = B_OK;
}


/*
HistoryListItem::~HistoryListItem()
{

}*/


void
HistoryListItem::DrawItem(BView *owner, BRect item_rect, bool complete)
{
	owner->PushState();
	
	float offset_height = fFontAscent;
	
	if (fIsDateDivider) {
		// Background
		owner->SetHighColor(ui_color(B_LIST_BACKGROUND_COLOR));
		owner->SetLowColor(ui_color(B_LIST_BACKGROUND_COLOR));
		owner->FillRect(item_rect);
		
		// Round rectangle
		owner->SetHighColor(ui_color(B_LIST_SELECTED_BACKGROUND_COLOR));
		owner->SetLowColor(ui_color(B_LIST_SELECTED_BACKGROUND_COLOR));
		float xyRadius = (Height() - 2) / 2.0;
		float stringWidth = be_plain_font->StringWidth(fDateLabel);
		float listWidth;
		owner->GetPreferredSize(&listWidth, NULL);
		BRect roundRect(item_rect);
		roundRect.InsetBy((listWidth - stringWidth - (2 * xyRadius) ) / 2.0, 1);
		owner->FillRoundRect(roundRect, xyRadius, xyRadius);
		
		// Text
		owner->SetHighColor(ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR));
		owner->MovePenTo(roundRect.left + xyRadius + 1,
			roundRect.top + offset_height - 1);
		owner->DrawString(fDateLabel);
	}
	else {
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
		BPoint cursor(item_rect.left, item_rect.top + offset_height /*+ kTextMargin*/);
		if(IsSelected())
			owner->SetHighColor(ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR));
		else
			owner->SetHighColor(ui_color(B_LIST_ITEM_TEXT_COLOR));
		owner->MovePenTo(cursor.x, cursor.y);
		
		BString timeString;
		timeString << fTimestamp;
		timeString.Append(": ");
		owner->DrawString(timeString);
		/*BString time;
	BTimeFormat timeFormatter;
	timeFormatter.Format(time, timestamp,
		B_LONG_TIME_FORMAT);*/
		
		if (fTitle.Length() > 0) {
			owner->PushState();
			BFont ownerFont;
			owner->GetFont(&ownerFont);
			ownerFont.SetFace(B_BOLD_FACE);
			owner->SetFont(&ownerFont, B_FONT_FACE);
			BString text(fTitle);
			text.Append(": ");
			owner->DrawString(text);
			float stringWidth = owner->StringWidth(text);
			owner->PopState();
			owner->MovePenBy(stringWidth, 0);
		}
		owner->DrawString(fContent);
	}
	owner->PopState();
}


void
HistoryListItem::Update(BView *owner, const BFont *font)
{
	BListItem::Update(owner, font);
	font_height fontHeight;
	font->GetHeight(&fontHeight);
	fFontHeight = fontHeight.ascent + fontHeight.descent + fontHeight.leading;
	fFontAscent = fontHeight.ascent;
}


int
HistoryListItem::TimestampCompare(HistoryListItem* item)
{
	if (fTimestamp < item->fTimestamp)
		return -1;
	else if (fTimestamp > item->fTimestamp)
		return 1;
	return 0;
}


const char*
HistoryListItem::DateLabel()
{
	if (!fIsDateDivider)
		return NULL;
	else
		return fDateLabel.String();
}
