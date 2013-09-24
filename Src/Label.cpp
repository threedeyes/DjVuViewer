#include <Application.h>
#include <Window.h>
#include <Message.h>
#include <Bitmap.h>
#include <View.h>
#include <Rect.h>
#include <SupportDefs.h>
#include <PopUpMenu.h>

#include "MainApp.h"
#include "Theme.h"
#include "Label.h"
#include "IconMenuItem.h"

BTLabel::BTLabel(BRect rect, const char *Name, const char *Label,
				BBitmap *background,uint32 _style,uint32 resize)
:BView(rect, Name, resize, B_FRAME_EVENTS|B_WILL_DRAW) //
{
	fBackGround=background;
	fName=Name;
	fLabel.SetTo(Label);
	style=_style;
	
	RightClickPopUp=NULL;
	
	BFont font = be_plain_font;
	font.SetSize(10);
	SetFont(&font);	
	
	text_color=HighColor();
    
    lastButtons=0;
}

BTLabel::~BTLabel()
{

}

void BTLabel::SetTextColor(rgb_color c)
{
	text_color=c;
}

void BTLabel::SetText(const char *label)
{
	fLabel.SetTo(label);
	Invalidate();
}

const char *
BTLabel::GetText(void)
{
	return fLabel.String();
}

void BTLabel::Draw(BRect rect)
{	
   SetDrawingMode(B_OP_COPY);
   
   if(fBackGround!=NULL)
	{
	BRect r=fBackGround->Bounds();	
	float background_dx=(int)Frame().left%(int)(fBackGround->Bounds().Width()+1);
	float background_dy=(int)Frame().top%(int)(fBackGround->Bounds().Height()+1);
	
	for(float y=0;y<Bounds().Height()+r.Height();y+=r.Height()+1)
		for(float x=0;x<Bounds().Width()+r.Width();x+=r.Width()+1)
		{
			r.OffsetTo(x-background_dx,y-background_dy);
			DrawBitmapAsync(fBackGround,r);
		}
	}
	else
	{
		SetHighColor(ViewColor());
		FillRect(Bounds());			
	}

	BFont font;
	GetFont(&font);
	SetFont(&font);

	float text_width=StringWidth(fLabel.String());
	float text_height=10;
	
	text_dx=(Bounds().Width()-text_width)/2.0;
	text_dy=((Bounds().Height()-text_height)/2.0)+1;
	
	if(style&BL_RECT_FLAG==BL_RECT_FLAG)
	{
		SetHighColor(128,128,128);
		StrokeRect(Bounds());	
	}
				
	SetDrawingMode(B_OP_ALPHA);
	SetHighColor(text_color.red,text_color.green,text_color.blue,250);
	DrawString(fLabel.String(),BPoint(text_dx+1,Bounds().bottom-text_dy));	
	Sync();

}

void
BTLabel::MouseUp(BPoint p)
{
   uint32 buttons=lastButtons;
   lastButtons=0;
   
   if( buttons & B_SECONDARY_MOUSE_BUTTON )
	 	{
	 		BMessage *mes=new BMessage('LBCP');
	 		mes->AddString("text",fLabel.String());
  			if(RightClickPopUp!=NULL)delete RightClickPopUp;
 			RightClickPopUp = new BPopUpMenu("Context",false,false);
			RightClickPopUp->AddItem(new IconMenuItem(THEME,"MI:Copy",MES("CPM:Copy"), mes));
			RightClickPopUp->SetTargetForItems(Window());
			RightClickPopUp->Go(ConvertToScreen(p),true,true,true);		
			BView::MouseUp(p); 	
			return;
	  	}
	BView::MouseUp(p); 	
}

void
BTLabel::MouseDown(BPoint p)
{
   lastButtons = Window()->CurrentMessage()->FindInt32("buttons");  
   if( lastButtons & B_SECONDARY_MOUSE_BUTTON )return;
   BView::MouseDown(p);
}

void BTLabel::AttachedToWindow()
{
	//SetViewColor(Parent()->ViewColor());
}

void
BTLabel::FrameResized(float width, float height)
{

	Invalidate();
}