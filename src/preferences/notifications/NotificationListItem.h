/*
 * Copyright 2017, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _NOTIFICATION_LIST_ITEM_H
#define _NOTIFICATION_LIST_ITEM_H

#include <InterfaceKit.h>
#include <Message.h>
#include <Notification.h>
#include <String.h>


class NotificationListItem : public BListItem
{
public:
							NotificationListItem(BMessage& notificationData);
//							~NotificationListItem();
	virtual void			DrawItem(BView *owner, BRect item_rect, bool complete = false);
	virtual void			Update(BView *owner, const BFont *font);
			status_t		InitStatus() { return fInitStatus; }
			BMessage		GetMessage() { return fNotificationMessage; }

private:
	status_t				fInitStatus;
	BMessage				fNotificationMessage;
	notification_type		fType;
	BString					fTitle;
	BString					fContent;
	int32					fTimestamp;
	bool					fWasShown;
	float					fFontHeight, fFontAscent;

};

#endif
