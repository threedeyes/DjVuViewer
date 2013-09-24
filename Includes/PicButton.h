#ifndef __PICBUTTON__
#define __PICBUTTON__

#include <View.h>
#include <String.h>
#include "Theme.h"

//States
#define PBS_NORMAL 0
#define PBS_PUSHED 1
#define PBS_DISABLE 2

//Styles
#define	PB_LABEL 1
#define PB_MOUSEOVER 2
#define PB_NO_BACKGROUND 4
#define PB_NO_3STATES 8
#define PB_DOWN_MESS	16

class BPicButton: public BView {
public:
	BPicButton(BRect, const char *BmpName,const char *Label,const char *ToolTip, BTheme *fTheme,BMessage *mess,uint32 state,uint32 style,uint32 resize);
	~BPicButton();
	void ReloadTheme(void);
	void SetState(uint32 state);
	void SetEnabled(bool val);
	void SetToolTip(const char *str);
	void SetBmpName(const char *BmpName);
	const char* GetBmpName(void);
	void SetBackground(const char *bgname);
	const char *GetLabel(void);
	BMessage *fPushMessage;
	
protected:
	uint32	style;
	uint32	state;
	BBitmap *fButton;
	BBitmap *fMouseOver;
	BBitmap *fBackGround;

//	BBitmap *OVbmp;
//	BView   *OVview;
	
	BTheme  *fTheme;

	BRect 	map_rect;
	
	BString bgName;

	const char* fBmpName;
	const char* fLabel;
	const char* fToolTip;
	bool	over_flag;
	bool	mouse_down_flag;
	float   text_width;
	float	text_height;
	float	text_dx;
	float 	pic_dx;
	float   background_dx;
	float 	background_dy;
	float 	text_shift_x;
	float	text_shift_y;
	
	virtual void AttachedToWindow();
	virtual void MouseDown(BPoint aPoint);
	virtual void MouseUp(BPoint aPoint);
	virtual void MouseMoved(BPoint point, uint32 transit,const BMessage *message);
	virtual void FrameMoved(BPoint point);

    virtual void Draw(BRect rect);	
};

#endif
