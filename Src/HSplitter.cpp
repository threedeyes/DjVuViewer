#include <Application.h>
#include <Window.h>
#include <Message.h>
#include <Bitmap.h>
#include <View.h>
#include <Rect.h>
#include <SupportDefs.h>
/*
uint8 c_h_resize_cursor_data[68] =
{
	16,1,7,7,
	0,0,	//0000000000000000
	1,0,	//0000000100000000
	3,128,	//0000001110000000
	7,192,	//0000011111000000
	0,0,	//0000000000000000
	0,0,	//0000000000000000
	0,0,	//0000000000000000
	127,252,//0111111111111100
	0,0,	//0000000000000000
	0,0,	//0000000000000000
	0,0,	//0000000000000000
	7,192,	//0000011111000000
	3,128,	//0000001110000000
	1,0,	//0000000100000000
	0,0,	//0000000000000000
	0,0,	//0000000000000000

	1,0,	//0000000100000000
	3,128,	//0000001110000000
	7,192,	//0000011111000000
	15,224,	//0000111111100000
	15,224,	//0000111111100000
	0,0,	//0000000000000000
	255,254,//1111111111111110
	255,254,//1111111111111110
	255,254,//1111111111111110
	0,0,	//0000000000000000
	15,224,	//0000111111100000
	15,224,	//0000111111100000
	7,192,	//0000011111000000
	3,128,	//0000001110000000
	1,0,	//0000000100000000
	0,0		//0000000000000000
};*/

#include "Theme.h"
#include "HSplitter.h"
#include "MainWindow.h"

BHSplitter::BHSplitter(BRect rect, const char *Name, BWindow *win,uint32 _style,uint32 resize)
:BView(rect, Name, resize, B_WILL_DRAW) //B_WILL_DRAW
{
	wnd=win;
	fLastClickTime=0;
}

BHSplitter::~BHSplitter()
{
}

void BHSplitter::AttachedToWindow()
{
	SetViewColor(192,192,192);
}

void BHSplitter::MouseDown(BPoint where)
{
	drag=1;
	SetMouseEventMask(B_POINTER_EVENTS,B_NO_POINTER_HISTORY);
	//be_app->SetCursor(c_h_resize_cursor_data);
	Invalidate();
}

void BHSplitter::MouseUp(BPoint where)
{
	drag=0;
	bigtime_t now=system_time();

	bigtime_t doubleClickSpeed;
	get_click_speed(&doubleClickSpeed);
	
	if(now-fLastClickTime<doubleClickSpeed)
	 Window()->PostMessage(new BMessage('QuVi'));
	fLastClickTime=now;
	//be_app->SetCursor(B_HAND_CURSOR);

}

void BHSplitter::MouseMoved(BPoint where, uint32 code, const BMessage* message)
{
	BPoint line=ConvertToParent(where);
	BPoint p=Parent()->ConvertToParent(line);
	//if(code==B_ENTERED_VIEW)be_app->SetCursor(c_h_resize_cursor_data);
	//if((code==B_EXITED_VIEW || code==B_OUTSIDE_VIEW) && drag!=1)be_app->SetCursor(B_HAND_CURSOR);
	if(drag==1)
	{
		float split=(Bounds().Height());
		float dy=Window()->Bounds().Height()-(p.y+split/2)-2;

		if(dy!=((NaviWindow*)wnd)->GetSplitPos())
		{
			((NaviWindow*)wnd)->SetSplitPos(dy);
			Invalidate();
		}
	}
	BView::MouseMoved(where,code,message);
}

