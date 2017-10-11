/*
 * Copyright 2010-2017, Haiku, Inc. All Rights Reserved.
 * Copyright 2009, Pier Luigi Fiorini.
 * Distributed under the terms of the MIT License.
 */
#ifndef _HISTORY_WINDOW_H
#define _HISTORY_WINDOW_H


#include <Bitmap.h>
#include <List.h>
#include <ListView.h>
#include <MenuField.h>
#include <Path.h>
#include <PopUpMenu.h>
#include <ScrollView.h>
#include <View.h>
#include <Window.h>

#include <notification/AppUsage.h>

#include "AppGroupView.h"
#include "NotificationView.h"

//typedef std::map<BString, AppUsage *> appusage_t;


class HistoryView : public BView {
public:
								HistoryView();
								~HistoryView();
	virtual	void				AttachedToWindow();
	virtual	void				FrameResized(float newWidth, float newHeight);
	virtual	void				MessageReceived(BMessage* message);
	virtual void				Draw(BRect updateRect);

private:
			void				_LoadCacheData();
			void				_PopulateNotifications();
			bool				_ArchiveIsValid(BMessage& archive, int32& count);
			void				_UpdatePreview(NotificationView* view,
									const char* group);

			BPath				fCachePath;
			bool				fShowingPreview;
//			appusage_t			fAppFilters;
//			BCheckBox*			fBlockAll;
//			BTextControl*		fSearch;
			BPopUpMenu*			fSetSelectionMenu;
			BMenuField*			fSetSelectionMF;
			int32				fSetSelection;
			AppGroupView*		fGroupView;
			BListView*			fListView;
			BScrollView*		fScrollView;
			BGroupLayout*		fPreviewLayout;
			NotificationView* 	fCurrentPreview;
			BList				fAllList;
			BList				fMutedList;
			float				fIconSize;
			BBitmap*			fNewIcon;
			BBitmap*			fMuteIcon;
};

class HistoryWindow : public BWindow {
public:
								HistoryWindow();
	virtual bool				QuitRequested();
private:
			HistoryView*		fMainView;
};

#endif // _HISTORY_WINDOW_H
