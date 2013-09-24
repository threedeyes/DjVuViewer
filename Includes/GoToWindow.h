#ifndef __GOTOWINDOW_H__
#define __GOTOWINDOW_H__

#include <Window.h>
#include "ToolsContainer.h"
#include <TextControl.h>
#include "MainWindow.h"
#include "ConfigureWindow.h"


class GoToSettingsView : public SettingsView {
public:
	GoToSettingsView(BRect);
	void AttachedToWindow();
	BTextControl	*NumLine;

};


class GoToWindow : public BWindow
{
public:
	uint32 flag1;
	GoToWindow(int32 idx, BWindow *MainWnd, BTheme *theme);
	~GoToWindow();
	virtual void MessageReceived(BMessage *message);
	virtual bool	QuitRequested();
	virtual void WindowActivated(bool active);
	virtual void Show(void);
	void RefreshViewsColor(void);
	
	BToolsContainer *line;
	BWindow			*MainWindow;
	BTheme			*Theme;
	GoToSettingsView *box;
	int32 	Idx;
};

#endif
