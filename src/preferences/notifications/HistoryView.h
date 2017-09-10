/*
 * Copyright 2010-2017, Haiku, Inc. All Rights Reserved.
 * Copyright 2009, Pier Luigi Fiorini.
 * Distributed under the terms of the MIT License.
 */
#ifndef _HISTORY_VIEW_H
#define _HISTORY_VIEW_H


#include <ListView.h>
#include <MenuField.h>
#include <PopUpMenu.h>
#include <ScrollView.h>
#include <View.h>

#include <notification/AppUsage.h>

#include "AppGroupView.h"
#include "NotificationView.h"
#include "SettingsPane.h"

typedef std::map<BString, AppUsage *> appusage_t;

class BCheckBox;
class BTextControl;
//class BColumnListView;
//class BStringColumn;
//class BDateColumn;
/*
extern const float kEdgePadding;
extern const float kSmallPadding;
extern const float kCloseSize;
extern const float kExpandSize;
extern const float kPenSize;*/

class HistoryView : public BView {
public:
								HistoryView();
								~HistoryView();
	virtual	void				AttachedToWindow();
	virtual	void				FrameResized(float newWidth, float newHeight);
	virtual	void				MessageReceived(BMessage* message);

private:
			void				_PopulateGroups();
			void				_PopulateNotifications(const char* group);
			bool				_ArchiveIsValid(BMessage& archive, int32& count);
			void				_UpdatePreview(NotificationView* view,
									const char* group);

			bool				fShowingPreview;
			appusage_t			fAppFilters;
			BCheckBox*			fBlockAll;
			BTextControl*		fSearch;
			BPopUpMenu*			fFontSizeMenu;
			BMenuField*			fFontSizeMF;
/*			BColumnListView*	fGroups;
			BStringColumn*		fGroupCol;
			BStringColumn*		fCountCol;
			BColumnListView*	fNotifications;
			BStringColumn*		fTitleCol;
			BDateColumn*		fDateCol;
			BStringColumn*		fContentCol;
			BStringColumn*		fTypeCol;
			BStringColumn*		fShownCol;*/
			AppGroupView*		fGroupView;
			BListView*			fListView;
			BScrollView*		fScrollView;
			NotificationView* 	fCurrentPreview;
};

#endif // _HISTORY_VIEW_H
