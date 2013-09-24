#ifndef __TOOLSCONTAINER__
#define __TOOLSCONTAINER__

#include <View.h>
#include <PopUpMenu.h>
#include <be/support/ClassInfo.h>
#include <String.h>
#include "Theme.h"
#include "PicButton.h"


#define TCNT_FRAME		1
#define TCNT_BACKGROUND	2
#define TCNT_CLIP		4
#define	TCNT_DRAGZONE	8


class BToolsContainer: public BView {
public:
	BToolsContainer(BRect, const char *Name, int32 mode,BTheme *theme,uint32 resizeMask = B_FOLLOW_LEFT_RIGHT);
	~BToolsContainer();
	BTheme* Theme();
	BBitmap* ViewBitmap();
	BPicButton* AddButton(const char* BmpName,const char *Label,const char*tooltip,BMessage *mess,int32 index,uint32 flags);
	BPicButton* AddSeparator(int32 index);
	void SetContextMenu(BPopUpMenu *popup);
	void SetBackground(const char *bgname);
	void ReloadTheme(void);
	
protected:
	BRect   old;
	int32	flags;
	BString bgName;
	BBitmap *fBackGround;
	BBitmap *fMouseOver;
	BBitmap *fClipButton;
	BBitmap *fDBufferBmp;
	
	BPopUpMenu		*RightClickPopUp;

	BBitmap *OVbmp;
	BView   *OVview;	
	int     NeedUpdate;
	
	uint32 buttons;
	
	BView	*fDBuffer;
	BTheme 	*fTheme;	
	BPopUpMenu 		*fHidMenu;
	bool	hidpres;
	bool	over_flag,old_over_flag;
	virtual void AttachedToWindow();
//	virtual void MouseUp(BPoint aPoint);
	virtual void MouseDown(BPoint aPoint);
	void MyDraw(BRect rect);
	virtual void Draw(BRect rect);
	virtual void FrameResized(float width, float height);
	virtual void MouseMoved(BPoint point, uint32 transit,const BMessage *message);
};

#endif

