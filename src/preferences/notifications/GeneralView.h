/*
 * Copyright 2010, Haiku, Inc. All Rights Reserved.
 * Copyright 2009, Pier Luigi Fiorini.
 * Distributed under the terms of the MIT License.
 */
#ifndef _GENERAL_VIEW_H
#define _GENERAL_VIEW_H


#include <CheckBox.h>
#include <Menu.h>
#include <MenuField.h>
#include <Mime.h>
#include <TextControl.h>

#include "SettingsPane.h"

class GeneralView : public SettingsPane {
public:
							GeneralView(SettingsHost* host);

	virtual	void			AttachedToWindow();
	virtual	void			MessageReceived(BMessage* msg);

			// SettingsPane hooks
			status_t		Load(BMessage&);
			status_t		Save(BMessage&);
			status_t		Revert();
			bool			RevertPossible();

private:
		BCheckBox*			fNotificationBox;
//		BCheckBox*			fAutoStart;
		BTextControl*		fTimeout;
//		BCheckBox*			fHideAll;
		BTextControl*		fWindowWidth;
		BMenu*				fIconSize;
		BMenuField*			fIconSizeField;
		
		int32				fOriginalTimeout;
		float				fOriginalWidth;
		icon_size			fOriginalIconSize;

		bool				_CanFindServer(entry_ref* ref);
		bool				_IsServerRunning();
};

#endif // _GENERAL_VIEW_H
