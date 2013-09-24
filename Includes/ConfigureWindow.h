#ifndef __CONFIGWINDOW_H__
#define __CONFIGWINDOW_H__

#include <Application.h>
#include <be/interface/PopUpMenu.h>
#include <be/interface/MenuItem.h>
#include <be/interface/ListView.h>
#include <be/interface/ScrollView.h>
#include <be/interface/StringView.h>
#include <be/interface/InterfaceDefs.h>
#include <be/interface/OutlineListView.h>
#include <be/interface/ListItem.h>
#include <be/interface/Menu.h>
#include <be/interface/Font.h>
#include <Window.h>
#include <Alert.h>
#include <Button.h>
#include <unistd.h>
#include <Bitmap.h>
#include <Screen.h>
#include <MenuField.h>
#include <CheckBox.h>
#include <Box.h>
#include <ColorControl.h>

#include <be/storage/Path.h>
#include <Entry.h>
#include <Directory.h>
#include "ToolsContainer.h"
#include "Label.h"
#include "MainWindow.h"


struct SettingAlias
{
	const char *FullName;
	const char *SettingName;
};


class SettingsView : public BBox {
public:
	SettingsView(BRect, const char *);
	virtual ~SettingsView();
	virtual void Draw(BRect rect);
	virtual void AttachedToWindow();
private:
	float 	oldh,oldw;
//	virtual void Invalidate();
};

class BMyTextControl: public BTextView{
public:
		BMyTextControl(BRect frame,const char *name,uint32 resizingMode,uint32 flags);
		~BMyTextControl();
		virtual void Draw(BRect rect);
		virtual void FrameResized(float width, float height);
};




class MainSettingsView : public SettingsView {
public:
	MainSettingsView(BRect);
	void AttachedToWindow();

	BMenuField 		*fInterfaceLanguage;
	BMenuField 		*fCurrentTheme;
	BCheckBox		*fShowButtonLabel;	
	BCheckBox		*fUseToolTips;	
	BBox 			*fIfaceBox;
};

class ColorSettingsView : public SettingsView {
public:
	ColorSettingsView(BRect);
	void AttachedToWindow();

	BListView		*fColorList;
	BScrollView 	*fColorListScroll;
	BColorControl	*fColorCtrl;
	BTLabel			*fColView;
				
};



class InfoSettingsView : public SettingsView {
public:
	InfoSettingsView(BRect);
	void AttachedToWindow();
	BMyTextControl	*fInfoText;
};



class ConfigureWindow : public BWindow
{
//private:
public:
	BToolsContainer *Container;
 	BScreen *scr;
 	BTheme  *Theme;

 	ConfigureWindow(BRect frame, BTheme *theme, BWindow *MainWindow);

	virtual void MessageReceived(BMessage *msg);
	virtual bool QuitRequested();
	virtual void Show(void);
	void RefreshViewsColor(void);
	void RefreshControls(void);
	void ReloadTheme();


private:
	DjVuWindow		*MainWindow;
	BWindow	*fImportWindow;
	BListView 		*SelectList;
	BScrollView 	*SelectListScroll;
	
	MainSettingsView *fMainSettingsView;
	ColorSettingsView *fColorSettingsView;
	InfoSettingsView  *fInfoSettingsView;
};
//------------------------------------------------------------------------------
#endif
