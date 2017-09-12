/*
 * Copyright 2010-2017, Haiku, Inc. All Rights Reserved.
 * Copyright 2009, Pier Luigi Fiorini.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PREFLET_VIEW_H
#define _PREFLET_VIEW_H

#include <Messenger.h>
#include <TabView.h>
#include "GeneralView.h"

class BIconRule;

class SettingsHost;

const int32 kShowButtons = '_SHR';
#define kShowButtonsKey "showButtons"

class PrefletView : public BTabView {
public:
						PrefletView(SettingsHost* host);
						~PrefletView();

			BView*		CurrentPage();
			int32		CountPages() const;
			BView*		PageAt(int32 index);
	virtual	void		Select(int32 index);
			void		StartWatchingRoster();

private:
			GeneralView*	fGeneralView;
			BMessenger		fMessenger;
			bool			fWatchingRoster;
};

#endif // PREFLETVIEW_H
