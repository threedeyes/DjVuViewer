#include <Application.h>
#include <Window.h>
#include <Message.h>
#include <Bitmap.h>
#include <View.h>
#include <Rect.h>
#include <SupportDefs.h>
#include "Theme.h"
#include "PicButton.h"
//#include "Tip.h"

//extern BTip* ToolTip;

BPicButton::BPicButton(BRect rect,const char *BmpName,const char *Label,const char *ToolTip,
			BTheme *theme,
			BMessage *mess,
			uint32 _state,uint32 _style,uint32 resize)
:BView(rect, BmpName, resize, B_WILL_DRAW|B_FRAME_EVENTS)
{
	fTheme=theme;	
	
	fPushMessage=mess;
	fBmpName=BmpName;
	fLabel=Label;
	SetToolTip(ToolTip);
	
	style=_style;
	state=_state;
	over_flag=false;
	mouse_down_flag=false;

	bgName=BString("BackGround");

    fBackGround = fTheme->GetPNG(bgName.String());
    if(fBackGround==NULL)fBackGround = fTheme->GetPNG("BackGround"); 
    if(style&PB_NO_BACKGROUND)fBackGround=NULL;
	fMouseOver = fTheme->GetPNG("MouseOver");	
	fButton = fTheme->GetPNG(BmpName);

	//OVbmp=new BBitmap(BRect(0,0,rect.Width()+1,rect.Height()+1),B_RGBA32_BIG,true);
	//OVview=new BView(BRect(0,0,rect.Width()+1,rect.Height()+1),"",0,0);

	BFont font = be_plain_font;
	font.SetSize(10);
	SetFont(&font,B_FONT_SIZE);	

	text_width=StringWidth(fLabel);
	text_height=10;
	
	text_dx=(Bounds().Width()-text_width)/2.0;
	if(style&PB_NO_3STATES)
		pic_dx=(Bounds().Width()-(fButton->Bounds().Width()))/2.0;
	else
		pic_dx=(Bounds().Width()-(fButton->Bounds().Width()/3.0))/2.0;
	
	text_shift_x=0,text_shift_y=10;
}

BPicButton::~BPicButton()
{
//	delete OVbmp;
}

void
BPicButton::FrameMoved(BPoint point)
{
  //Invalidate(Bounds());
}

const char *
BPicButton::GetLabel(void)
{
	return fLabel;
}

void BPicButton::SetBackground(const char *bgname)
{
 bgName=BString(bgname);
 if(fTheme!=NULL)
 { 
  fBackGround = fTheme->GetPNG(bgName.String());
  if(fBackGround==NULL)fBackGround = fTheme->GetPNG("BackGround"); 
  if(style&PB_NO_BACKGROUND)fBackGround=NULL;
 }
}


void BPicButton::ReloadTheme(void)
{
  	fBackGround = fTheme->GetPNG(bgName.String());
  	if(fBackGround==NULL)fBackGround = fTheme->GetPNG("BackGround"); 
    if(style&PB_NO_BACKGROUND)fBackGround=NULL;
    fMouseOver = fTheme->GetPNG("MouseOver");	
    fButton = fTheme->GetPNG(fBmpName);	
	Invalidate(Bounds());
}

void BPicButton::Draw(BRect rect)
{	

   SetDrawingMode(B_OP_COPY);

   fBackGround = fTheme->GetPNG(bgName.String());
   if(fBackGround==NULL)fBackGround = fTheme->GetPNG("BackGround"); 
   
   if(style&PB_NO_BACKGROUND)fBackGround=NULL;
   fMouseOver = fTheme->GetPNG("MouseOver");	
   fButton = fTheme->GetPNG(fBmpName);

   BRect brect=map_rect;
   brect.OffsetTo(BPoint(pic_dx,2));	
   
   if(fBackGround!=NULL)
	{
	BRect r=fBackGround->Bounds();	
	background_dx=(int)Frame().left%(int)(fBackGround->Bounds().Width()+1);
	background_dy=(int)Frame().top%(int)(fBackGround->Bounds().Height()+1);
	
	for(float y=0;y<Bounds().Height()+r.Height();y+=r.Height()+1)
		for(float x=0;x<Bounds().Width()+r.Width();x+=r.Width()+1)
		{
			//r.OffsetTo(x-background_dx,y-background_dy);
			DrawBitmapAsync(fBackGround,BPoint(x-background_dx,y-background_dy));
		}
	}
    else 
    {
     SetHighColor(Parent()->ViewColor());
     FillRect(Bounds());
    }
	
	SetDrawingMode(B_OP_ALPHA);
	if(style&PB_MOUSEOVER && fMouseOver!=NULL && over_flag && state!=PBS_DISABLE)DrawBitmapAsync(fMouseOver,Bounds());
	if(fButton!=NULL)DrawBitmapAsync(fButton,map_rect,brect);
	
	Sync();
			
	if(style&PB_LABEL)
	{
		if(state==PBS_DISABLE)SetHighColor(128,128,128,128);
		else SetHighColor(0,0,0,192);
	 	DrawString(fLabel,BPoint(text_dx+1+text_shift_x,(map_rect.bottom)+text_shift_y));
	}
	
	
    //OVview->UnlockLooper();
    
	//SetViewBitmap(OVbmp,Bounds(),Bounds(),B_FOLLOW_NONE,B_TILE_BITMAP);  	
	//Invalidate();
   //}
}

void BPicButton::AttachedToWindow()
{
	SetViewColor(Parent()->ViewColor());
//	OVbmp->AddChild(OVview);
	SetState(state);
	Invalidate(Bounds());	
}

void BPicButton::MouseDown(BPoint point)
{
	uint32 buttons = Window()->CurrentMessage()->FindInt32("buttons");
	if( buttons & B_SECONDARY_MOUSE_BUTTON)
	{
		Parent()->MouseDown(ConvertToParent(point));
		return;
	}
	
	if(style&PB_DOWN_MESS) {
		Window()->PostMessage(fPushMessage);
	}
	
	if(state!=PBS_DISABLE)SetState(1),mouse_down_flag=true;
	SetMouseEventMask(B_POINTER_EVENTS,B_NO_POINTER_HISTORY);
}

void BPicButton::MouseUp(BPoint point)
{
//	ToolTip->Stop();
	if(state!=PBS_DISABLE && mouse_down_flag==true)
	{
		mouse_down_flag=false;
		SetState(0);
		if(!(style&PB_DOWN_MESS)) {
			Window()->PostMessage(fPushMessage);
		}
	}
  Parent()->MouseUp(ConvertToParent(point));
}

void BPicButton::MouseMoved(BPoint point, uint32 transit,const BMessage *message)
{
	switch(transit)
	 {
	 	case B_INSIDE_VIEW:
	 	case B_ENTERED_VIEW:
	 		{
	 			BPoint p=ConvertToScreen(BPoint(point.x,Bounds().bottom+1));
	 			if(over_flag!=true && fMouseOver!=NULL){over_flag=true;Invalidate(Bounds());}	
//	 			if(fToolTip!=NULL)ToolTip->Go(p,(char*)fToolTip);
	 			break;
	 		}
	 	case B_EXITED_VIEW:
	 		over_flag=false;
	 		if(fMouseOver!=NULL)Invalidate(Bounds());	
//	 		ToolTip->Stop(); 
	 		break;
	 	case B_OUTSIDE_VIEW:
	 		over_flag=false;
	 		if(fMouseOver!=NULL)Invalidate(Bounds());
//	 		ToolTip->Stop(); 
	 		break; 
	 }
}

void BPicButton::SetEnabled(bool val)
{
	SetState(val?PBS_NORMAL:PBS_DISABLE);
}

void BPicButton::SetState(uint32 _state)
{
	state=_state;
	if(style&PB_NO_3STATES)
	{
		map_rect=fButton->Bounds();		
	}
	else
	{
		float e_width=((fButton->Bounds().Width()+1)/3.0);
		map_rect.left=_state*e_width;
		map_rect.right=map_rect.left+(e_width-1);
		map_rect.top=fButton->Bounds().top;
		map_rect.bottom=fButton->Bounds().bottom;
	}
	
	Invalidate(Bounds());	
}

void BPicButton::SetBmpName(const char *BmpName)
{
	fBmpName=BmpName;
	Invalidate(Bounds());	
}

const char*
BPicButton::GetBmpName(void)
{
 return fBmpName;
}

void BPicButton::SetToolTip(const char *str)
{
	fToolTip=str;
	BView::SetToolTip(fToolTip);
}
