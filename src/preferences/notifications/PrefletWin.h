/*
 * Copyright 2010, Haiku, Inc. All Rights Reserved.
 * Copyright 2009, Pier Luigi Fiorini.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PREFLET_WIN_H
#define _PREFLET_WIN_H


#include <Message.h>
#include <Window.h>

#include "SettingsHost.h"

class BButton;

class PrefletView;

class PrefletWin : public BWindow, public SettingsHost {
public:
							PrefletWin();

	virtual	bool			QuitRequested();
	virtual	void			MessageReceived(BMessage* msg);

	virtual	void			SettingChanged();
			void			ReloadSettings();

private:
			status_t		_Revert();
			bool			_RevertPossible();
			
			PrefletView*	fMainView;
			BButton*		fRevert;
};

#endif // _PREFLET_WIN_H
