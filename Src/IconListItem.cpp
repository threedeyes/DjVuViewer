#include <View.h>
#include <Region.h>
#include "IconListItem.h"

IconListItem::IconListItem(BBitmap *micon,const char *text,const char *datatext, int32 div, int level,bool expanded) :
	BListItem(level,expanded)
{
	divider=div;
	font_height FontAttributes;
	be_plain_font->GetHeight(&FontAttributes);
	float FontHeight = ceil(FontAttributes.ascent) + ceil(FontAttributes.descent);

	if (micon!=NULL)
	{ 
		if(FontHeight < micon->Bounds().bottom)FontHeight=micon->Bounds().bottom;
		BRect rect=micon->Bounds();
		icon = new BBitmap(rect, B_RGBA32);		
		if ((micon->BytesPerRow() == icon->BytesPerRow()) &&
        	(micon->BitsLength() == icon->BitsLength()))
        	memcpy(icon->Bits(), micon->Bits(), micon->BitsLength());
        else delete icon;
	} 
	else icon=NULL;
	
	label.SetTo(text);
	data.SetTo(datatext);
	BFont font(be_plain_font);
	SetWidth(font.StringWidth(text));
	SetHeight(FontHeight+2);
} 
 
IconListItem::~IconListItem() 
{
	if(icon!=NULL){delete icon;icon = NULL;}
} 
 

void IconListItem::ChangeIcon(IconListItem* item,BBitmap *micon)
{
	if(item->icon!=NULL){delete item->icon;icon = NULL;} 
	if (micon!=NULL)
	{ 
		BRect rect=micon->Bounds();
		item->icon = new BBitmap(rect, B_RGBA32);		
		if ((micon->BytesPerRow() == item->icon->BytesPerRow()) &&
        	(micon->BitsLength() == item->icon->BitsLength()))
        	memcpy(item->icon->Bits(), micon->Bits(), micon->BitsLength());
        else delete icon;
	} 	
}


void IconListItem::DrawItem(BView *view, BRect frame, bool complete) 
{
   rgb_color CursBarCol=SETTINGS->GetColor("CursBarCol");
   rgb_color CursFrameCol=SETTINGS->GetColor("CursFrameCol");
   rgb_color CursTextCol=SETTINGS->GetColor("CursTextCol");
   rgb_color EvenLinesCol=SETTINGS->GetColor("EvenLinesCol");
   rgb_color EvenTextCol=SETTINGS->GetColor("EvenTextCol");
   rgb_color OddLinesCol=SETTINGS->GetColor("OddLinesCol");
   rgb_color OddTextCol=SETTINGS->GetColor("OddTextCol");
   
   rgb_color iBarColor1;
   rgb_color iFrameColor1;
   rgb_color iTextColor1;
   rgb_color iBarColor2;
   rgb_color iFrameColor2;
   rgb_color iTextColor2;
   
   int32 n=((BListView*)view)->IndexOf(this);

	BRect frame1=frame;
	BRect frame2=frame;
	
	if(data!="")
	{
		frame1.right=divider;
		frame2.left=divider+1;
    }

	BRegion AllReg=BRegion(view->Bounds());
//	view->ConstrainClippingRegion(&AllReg);
   
   	view->SetDrawingMode(B_OP_OVER);

   	if(n%2==0){ iBarColor1=EvenLinesCol,iFrameColor1=EvenLinesCol,iTextColor1=EvenTextCol;
   				iBarColor2=OddLinesCol,iFrameColor2=OddLinesCol,iTextColor2=OddTextCol;}
   	else 	 {	iBarColor1=OddLinesCol,iFrameColor1=OddLinesCol,iTextColor1=OddTextCol;
   				iBarColor2=EvenLinesCol,iFrameColor2=EvenLinesCol,iTextColor2=EvenTextCol;}
   
   	if(IsSelected()){iBarColor1=CursBarCol,iFrameColor1=CursFrameCol,iTextColor1=CursTextCol;
   					iBarColor2=CursBarCol,iFrameColor2=CursFrameCol,iTextColor2=CursTextCol;}
 
   	view->SetHighColor(iBarColor1);
	view->FillRect(frame1);
	view->SetHighColor(iFrameColor1);
	view->StrokeRect(BRect(frame1.left-1,frame1.top,frame1.right+1,frame1.bottom));

	if(data!="")
	{
  		view->SetHighColor(iBarColor2);
		view->FillRect(frame2);
		view->SetHighColor(iFrameColor2);
		view->StrokeRect(BRect(frame2.left-1,frame2.top,frame2.right+1,frame2.bottom));
	}

	if (icon!=NULL)
	{
		frame.left += icon->Bounds().Width()+4;frame.top;frame.bottom = frame.top + Height();

		float dY=(Height()-icon->Bounds().bottom)/2;

		BRect rect(frame.left-icon->Bounds().Width()-1,
				   frame.top+dY,
				   frame.left-1,
				   frame.top+dY+icon->Bounds().bottom);
							
		view->MovePenTo(0,frame.top);

		view->SetDrawingMode(B_OP_ALPHA);
		view->DrawBitmap(icon, rect);
		view->SetDrawingMode(B_OP_OVER);
	}

	frame.left+=5.0;
	frame.right=frame.left+Width()+10.0;
	
	float dy=((frame.bottom-frame.top)-be_plain_font->Size())/2.0;

	BRegion Reg=BRegion(frame1);
//	view->ConstrainClippingRegion(&Reg);
	view->MovePenTo(frame.left+4.0,frame.bottom-dy);
	view->SetHighColor(iTextColor1);
	view->DrawString(label.String());

	Reg=BRegion(frame2);
//	view->ConstrainClippingRegion(&Reg);
	view->MovePenTo(frame.left+4.0+divider,frame.bottom-dy);
	view->SetHighColor(iTextColor2);
	view->DrawString(data.String());
	
//	view->ConstrainClippingRegion(&AllReg);
} 

void IconListItem::Update(BView *view, const BFont *font)
{
}

const char *
IconListItem::GetLabel(void)
{
	return label.String();
}
