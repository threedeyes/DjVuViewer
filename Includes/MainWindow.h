#ifndef __DjVuWindow_H__
#define __DjVuWindow_H__

#include <Window.h>
#include <ListView.h>
#include <Menu.h>
#include <TextControl.h>
#include <PopUpMenu.h>
#include <FilePanel.h>

#include "LoadDjVu.h"

#include "PrintOptionsWindow.h"

#include "ToolsContainer.h"
#include "Label.h"


const uint32 MSG_OUTPUT_TYPE		= 'BTMN';
const uint32 MSG_SAVE_PANEL			= 'mFSP';

inline int isdigit(char c)
{
 if(c<='9' && c>='0')return 0;
 return -1;
}


class DjVuWindow : public BWindow
{
public:
	DjVuWindow(const char *title);
	~DjVuWindow();
	void 		LoadMenus(BMenuBar *pbar);
	virtual void MessageReceived(BMessage *message);
	virtual void	Quit(void);
	BMenuItem *AddItemMenu(BMenu *pmenu, char *caption, long unsigned int msg,char shortcut, uint32 modifier, char target, bool benabled);
	void 		ReMakeMainToolBar(void);
	int			LoadFile(const char *filename);
	int 		UnloadFile();
	BDjVuFile 	*DJFile;
	BListView 	*SelectList;
	BMenu 		*pmenuSaveAs;
	BView		*fImageView;
	
	int			NeedRenderPage;
	
	int			NeedPrintPages;
	int			PrintFirstPage;
	int			PrintLastPage;
	
	bool		AutoFitWidth;
    char 		align_type;
    bool		FullScreenFlag;
    BRect		RestoreRect;
    BRect		ImageViewFrame;
    BTLabel 	*PageInfo;

	bool PageSetup();
	void Print();

	BMessage *fPrintSettings;
   
private:	


	BWindow	 		*fConfigureWindow;
	BToolsContainer *tools;
	BToolsContainer *line;
	BToolsContainer *logo;
	BPopUpMenu		*RightClickPopUp;
	BMenu			*ThemePopUp;
	BMenuBar		*MainMenu;
	BMenuItem 		*ScaleItem;
	BMenuItem 		*FullScreenItem;
	BMenuItem 		*ShowButtonLabelItem;
	BView *box;
	BScrollView 	*SelectListScroll;	
    thread_id 		thumb_thread;
	BFilePanel 		*fSavePanel;
	void 			SaveAs(BMessage *pmsg);
	void 			SaveToFile(BMessage *pmsg);   
};

#endif
