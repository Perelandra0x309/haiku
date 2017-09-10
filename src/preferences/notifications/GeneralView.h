/*
 * Copyright 2010, Haiku, Inc. All Rights Reserved.
 * Copyright 2009, Pier Luigi Fiorini.
 * Distributed under the terms of the MIT License.
 */
#ifndef _GENERAL_VIEW_H
#define _GENERAL_VIEW_H


#include <Button.h>
#include <CheckBox.h>
#include <Menu.h>
#include <MenuField.h>
#include <Mime.h>
#include <RadioButton.h>
#include <Slider.h>
#include <StringView.h>
#include <TextControl.h>

//#include "DateTimeEdit.h"
//#include "ScreenCornerSelector.h"
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
			status_t		Defaults();
			bool			DefaultsPossible();
			bool			UseDefaultRevertButtons();

private:
		BCheckBox*			fNotificationBox;
		BSlider*			fDurationSlider;
		BSlider*			fWidthSlider;
//		BCheckBox*			fDoNotDisturb;
//		BTextControl*		fDNDFrom;
//		BTextControl*		fDNDTo;
//		TTimeEdit*			fFromTimeEdit;
//		TTimeEdit*			fToTimeEdit;
//		ScreenCornerSelector* fCornerSelector;
		
		int32				fOriginalTimeout;
		float				fOriginalWidth;
		icon_size			fOriginalIconSize;

		void				_EnableControls();
		void				_SendSampleNotification();
		void				_SetWidthLabel(int32 value);
		void				_SetTimeoutLabel(int32 value);
		bool				_CanFindServer(entry_ref* ref);
		bool				_IsServerRunning();
};

#endif // _GENERAL_VIEW_H
