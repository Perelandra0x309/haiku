/*
 * Copyright 2017, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Brian Hill, supernova@tycho.email
 */
#include "HistoryListItem.h"

#include <Application.h>
#include <DateFormat.h>
#include <IconUtils.h>
#include <Resources.h>
#include <Roster.h>
#include <TimeFormat.h>
#include <notification/Notifications.h>

#include "HistoryWindow.h"

static const float kTintedLineTint = 1.04;
static const float kIconMargin = 2.0;
static const rgb_color backgroundRGB = ui_color(B_LIST_BACKGROUND_COLOR);
static const rgb_color textRGB = ui_color(B_LIST_ITEM_TEXT_COLOR);
static const rgb_color selectedBackgroundRGB =
	ui_color(B_LIST_SELECTED_BACKGROUND_COLOR);
static const rgb_color selectedTextRGB =
	ui_color(B_LIST_SELECTED_ITEM_TEXT_COLOR);


HistoryListItem::HistoryListItem(BMessage& notificationData, float iconSize)
	:
	BListItem(),
	fInitStatus(B_ERROR),
	fWasAllowed(true),
	fIconSize(iconSize),
	fTimeStringWidth(0),
//	fStatusIcon(NULL),
	fIsDateDivider(false)
{
	status_t result = notificationData.FindMessage(kNameNotificationMessage, &fNotificationMessage);
	if (result != B_OK)
		return;
	BNotification notification(&fNotificationMessage);
	fType = notification.Type();
//	fGroup = notification.Group();
	fGroup = notification.SourceName();
	BString groupString(notification.Group());
	if (!groupString.IsEmpty()) {
		if (fGroup.IsEmpty())
			fGroup = groupString;
		else if (fGroup.Compare(groupString) != 0)
			fGroup.Append(" (").Append(groupString).Append(")");
	}
	fTitle = notification.Title();
	fContent = notification.Content();
	
	result = notificationData.FindInt32(kNameTimestamp, &fTimestamp);
	if (result == B_OK) {
		BTimeFormat timeFormatter;
		timeFormatter.Format(fTimeString, fTimestamp, B_SHORT_TIME_FORMAT);
	}
	result = notificationData.FindBool(kNameWasAllowed, &fWasAllowed);
	if (result != B_OK)
		fWasAllowed = true;
//	if (!fWasAllowed)
//		fStatusIcon = new BBitmap(muteIcon);

	fInitStatus = B_OK;
}


HistoryListItem::HistoryListItem(int32 timestamp)
	:
	BListItem(),
	fInitStatus(B_ERROR),
//	fStatusIcon(NULL),
	fIsDateDivider(true)
{
	// Convert timestamp to midnight local
	BTime midnight(23, 59, 59);
	BDate date(timestamp);
	BDateTime midnightDate(date, midnight);
	fTimestamp = midnightDate.Time_t();
	
	// Create the label
	BDate referenceDate(time(NULL));
	if (date == referenceDate) {
		fDateLabel.SetTo("Today");
		fInitStatus = B_OK;
	} else {
		referenceDate.AddDays(-1);
		if (date == referenceDate) {
			fDateLabel.SetTo("Yesterday");
			fInitStatus = B_OK;
		} else {
			BDateFormat formatter;
			status_t result = formatter.Format(fDateLabel, fTimestamp,
				B_LONG_DATE_FORMAT);
//			fDateLabel.Prepend("*").Append("*");
			if (result == B_OK)
				fInitStatus = B_OK;
		}
	}
	SetEnabled(false);
}



HistoryListItem::~HistoryListItem()
{
//	delete fStatusIcon;
//	fStatusIcon = NULL;
}


void
HistoryListItem::DrawItem(BView *owner, BRect item_rect, bool complete)
{
	owner->PushState();
	float offset_height = fFontAscent;
	
	if (fIsDateDivider) {
		// Draw date header
		rgb_color headerBackground = tint_color(selectedBackgroundRGB, 0.7);
		owner->SetHighColor(headerBackground);
		owner->SetLowColor(headerBackground);
		owner->FillRect(item_rect);
		
		// Round rectangle
/*		owner->SetHighColor(ui_color(B_LIST_SELECTED_BACKGROUND_COLOR));
		owner->SetLowColor(ui_color(B_LIST_SELECTED_BACKGROUND_COLOR));
		float xyRadius = (Height() - 2) / 2.0;
		float stringWidth = be_plain_font->StringWidth(fDateLabel);
		float listWidth;
		owner->GetPreferredSize(&listWidth, NULL);
		BRect roundRect(item_rect);
		float insetX = (listWidth - stringWidth - (2 * xyRadius) ) / 2.0;
		roundRect.InsetBy(insetX, 1);
		owner->FillRoundRect(roundRect, xyRadius, xyRadius);*/
//		owner->StrokeLine(item_rect.LeftTop(), item_rect.RightTop());
//		owner->StrokeLine(item_rect.LeftBottom(), item_rect.RightBottom());
		
		// Date text
		BFont ownerFont;
		owner->GetFont(&ownerFont);
		ownerFont.SetFace(B_BOLD_FACE);
		owner->SetFont(&ownerFont, B_FONT_FACE);
		owner->SetHighColor(selectedTextRGB);
//		owner->MovePenTo(roundRect.left + xyRadius + 1,
//			roundRect.top + offset_height - 1);
		float stringWidth = owner->StringWidth(fDateLabel);
		float listWidth;
		owner->GetPreferredSize(&listWidth, NULL);
		owner->MovePenTo(item_rect.left + (listWidth - stringWidth) / 2.0,
			item_rect.top + offset_height - 1);
		owner->DrawString(fDateLabel);
	} else {
		HistoryListView* listView = dynamic_cast<HistoryListView*>(owner);
		
		// Draw notification list item
		rgb_color backgroundColor;
		if(IsSelected())
			backgroundColor = selectedBackgroundRGB;
		else {
			backgroundColor = backgroundRGB;
			BListView* listView = dynamic_cast<BListView*>(owner);
			if(listView) {
				// Alternate tinted background
				int32 index = listView->IndexOf(this);
				if ( (index % 2) == 1)
					backgroundColor = tint_color(backgroundColor,
						kTintedLineTint);
			}
		}
		owner->SetHighColor(backgroundColor);
		owner->SetLowColor(backgroundColor);
		owner->FillRect(item_rect);

		// Time
		BPoint cursor(item_rect.left, item_rect.top + offset_height);
		if (fTimeString.FindFirst(":") == 1)
			cursor.x += owner->StringWidth("1");
			// Indent times with a single digit hour
		if(IsSelected())
			owner->SetHighColor(tint_color(selectedTextRGB, 0.7));
		else
			owner->SetHighColor(tint_color(textRGB, 0.7));
		owner->MovePenTo(cursor.x, cursor.y);
		owner->DrawString(fTimeString);
		owner->MovePenTo(item_rect.left + fTimeStringWidth, cursor.y);
		
		// Status icon
//		fStatusIcon = NULL;
		BBitmap* statusIcon = NULL;
		if (!fWasAllowed && listView != NULL) {
			// Align icons
//			float stringWidth = listView->GetTimeStringWidth();
//			owner->MovePenTo(item_rect.left + stringWidth, cursor.y);
			// Get appropriate icon
//			if (!fWasAllowed)
				statusIcon = listView->GetMuteIcon();
		} //else 
//			owner->MovePenTo(item_rect.left + owner->StringWidth("12:00 AM"),
//				cursor.y);
				// Failover alignment
		if (statusIcon != NULL && statusIcon->IsValid()) {
			owner->SetDrawingMode(B_OP_ALPHA);
			owner->DrawBitmap(statusIcon, BPoint(owner->PenLocation().x
				+ kIconMargin, item_rect.top + 1));
			owner->SetDrawingMode(B_OP_COPY);
//			fStatusIcon = NULL;
		}
		owner->MovePenBy(fIconSize + 2 * kIconMargin, 0.0);

		// Group
		if(IsSelected())
			owner->SetHighColor(selectedTextRGB);
		else
			owner->SetHighColor(textRGB);
		if (fGroup.Length() > 0) {
			owner->PushState();
			BFont ownerFont;
			owner->GetFont(&ownerFont);
			ownerFont.SetFace(B_BOLD_FACE);
			owner->SetFont(&ownerFont, B_FONT_FACE);
			BString text(fGroup);
			text.Append(": ");
			owner->DrawString(text);
			float stringWidth = owner->StringWidth(text);
			owner->PopState();
			owner->MovePenBy(stringWidth, 0);
		}
		
		// Content
		if (fTitle.Length() > 0) {
			BString contentText(fTitle);
			contentText.Append(": ").Append(fContent);
			owner->DrawString(contentText);
		} else
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
//	fFontHeight = fontHeight.ascent + fontHeight.descent + fontHeight.leading;
	fFontAscent = fontHeight.ascent;
	
	HistoryListView* listView = dynamic_cast<HistoryListView*>(owner);
	fTimeStringWidth = listView->GetTimeStringWidth();
}


int
HistoryListItem::TimestampCompare(HistoryListItem* item)
{
	if (fTimestamp < item->fTimestamp)
		return 1;
	else if (fTimestamp > item->fTimestamp)
		return -1;
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
