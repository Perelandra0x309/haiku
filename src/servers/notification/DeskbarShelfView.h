/*
 * Copyright 2017, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _DESKBAR_SHELF_VIEW_H
#define _DESKBAR_SHELF_VIEW_H


#include <stdio.h>

#include <Bitmap.h>
#include <Deskbar.h>
#include <Messenger.h>
#include <PopUpMenu.h>
#include <Rect.h>
#include <View.h>

extern const char* kShelfviewName;
extern const char* kKeyMessenger;
extern const char* kKeyMuteAll;

//const uint32 kDeskbarReplicantClicked = 'DeCl';
const uint32 kDeskbarRegistration = 'DeRe';
const uint32 kRegistrationAcknowledge = 'ReAc';
//const uint32 kShowNewIcon = 'NeIc';
//const uint32 kShowStandardIcon = 'StIc';
const uint32 kMuteAllClicked = 'MuCl';
const uint32 kShowHistory = 'ShHi';
const uint32 kShowSettings = 'ShSe';
const uint32 kDeskbarSync = 'DeSy';

enum {
	ICONSTATE_DEFAULT = 0,
	ICONSTATE_NEW,
	ICONSTATE_MUTE,
	ICONSTATE_END
};

class _EXPORT DeskbarShelfView : public BView {
public:
							DeskbarShelfView();
							DeskbarShelfView(BMessage* data);
	virtual					~DeskbarShelfView();

	virtual void			AttachedToWindow();
//	virtual void			DetachedFromWindow();
	static DeskbarShelfView*	Instantiate(BMessage* data);
	virtual	status_t		Archive(BMessage* data, bool deep = true) const;
	virtual void			MessageReceived(BMessage* message);
	virtual void			Draw(BRect rect);
	virtual void	 		MouseDown(BPoint);

private:
	void					_Quit();
	void					_BuildMenu();

	BMessenger				fServerMessenger;
	bool					fRegistrationAcknowledged;
	uint32					fIconState;
	BBitmap*				fIcon;
	BBitmap*				fNewIcon;
	BBitmap*				fMuteIcon;
	BPopUpMenu*				fMenu;
};

#endif	// _DESKBAR_SHELF_VIEW_H
