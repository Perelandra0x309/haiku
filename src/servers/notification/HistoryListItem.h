/*
 * Copyright 2017, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _HISTORY_LIST_ITEM_H
#define _HISTORY_LIST_ITEM_H

#include <InterfaceKit.h>
#include <Message.h>
#include <Notification.h>
#include <String.h>


class HistoryListItem : public BListItem
{
public:
							HistoryListItem(BMessage& notificationData);
							HistoryListItem(char* label);
//							~HistoryListItem();
	virtual void			DrawItem(BView *owner, BRect item_rect,
								bool complete = false);
	virtual void			Update(BView *owner, const BFont *font);
			int				TimestampCompare(HistoryListItem* item);
			status_t		InitStatus() { return fInitStatus; }
			BMessage		GetMessage() { return fNotificationMessage; }
			bool			IsDateDivider() { return fIsDateDivider; }

private:
	status_t				fInitStatus;
	BMessage				fNotificationMessage;
	notification_type		fType;
	BString					fTitle;
	BString					fContent;
	int32					fTimestamp;
	bool					fWasAllowed;
	float					fFontHeight, fFontAscent;
	bool					fIsDateDivider;
	BString					fDateLabel;
};

#endif // _HISTORY_LIST_ITEM_H
