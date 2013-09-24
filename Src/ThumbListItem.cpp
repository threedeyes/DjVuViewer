#include <View.h>
#include <Region.h>
#include "ThumbListItem.h"

ThumbListItem::ThumbListItem(BBitmap *thumb,const char *text,int32 w, int level,bool expanded) :
	BListItem(level,expanded)
{
	width=w;
	f_thumb=0;
	BFont font(be_plain_font);
	font_height FontAttributes;
	be_plain_font->GetHeight(&FontAttributes);
	float FontHeight = ceil(FontAttributes.ascent) + ceil(FontAttributes.descent);
	float Width=font.StringWidth(text);
	FontHeight += 108;	
	
	if (thumb!=NULL)
	{ 
		Width = thumb->Bounds().Width()+8;
		BRect rect=thumb->Bounds();
		icon = new BBitmap(rect, B_RGBA32);		
		if ((thumb->BytesPerRow() == icon->BytesPerRow()) &&
        	(thumb->BitsLength() == icon->BitsLength()))
        	memcpy(icon->Bits(), thumb->Bits(), thumb->BitsLength());
        else delete icon;
	} 
	else icon=NULL;
	
	label.SetTo(text);
	SetWidth(Width);
	SetHeight(FontHeight+2);
} 
 
ThumbListItem::~ThumbListItem() 
{
	if(icon!=NULL){delete icon;icon = NULL;}
} 
 

void ThumbListItem::ChangeIcon(ThumbListItem* item,BBitmap *micon)
{
	BFont font(be_plain_font);
	font_height FontAttributes;
	be_plain_font->GetHeight(&FontAttributes);
	float FontHeight = ceil(FontAttributes.ascent) + ceil(FontAttributes.descent);
	float Width=font.StringWidth(label.String());
	
	if(item->icon!=NULL){delete item->icon;icon = NULL;} 
	if (micon!=NULL)
	{ 
		FontHeight += micon->Bounds().bottom+8;
		Width = micon->Bounds().Width()+8;
		
		BRect rect=micon->Bounds();
		item->icon = new BBitmap(rect, B_RGBA32);		
		if ((micon->BytesPerRow() == item->icon->BytesPerRow()) &&
        	(micon->BitsLength() == item->icon->BitsLength()))
        	memcpy(item->icon->Bits(), micon->Bits(), micon->BitsLength());
        else delete icon;
	} 	
}


void ThumbListItem::DrawItem(BView *view, BRect frame, bool complete) 
{
   rgb_color ThumbBackCol=SETTINGS->GetColor("ThumbBackCol");
   rgb_color ThumbTextCol=SETTINGS->GetColor("ThumbTextCol");
   
   
   BRect frame1=frame;
   BRect frame2=frame;
   BRect ThFrame=BRect(1,1,100,100);
   
   float shift=0;
   if(IsSelected())shift=4;	
   
   view->SetDrawingMode(B_OP_OVER);

   
   view->SetHighColor(ThumbBackCol);
   view->FillRect(frame1);

   float dX=(width - ThFrame.Width()) /2;
   BRect rect(frame.left+dX+shift,
				   frame.top+4+shift,
				   frame.left+dX+ThFrame.Width()+shift,
				   frame.top+4+ThFrame.Height()+shift);

	if (icon!=NULL)
	{
	   float dX=(width - icon->Bounds().Width()) /2;
   	   rect=BRect(frame.left+dX+shift,
				   frame.top+4+shift,
				   frame.left+dX+icon->Bounds().Width()+shift,
				   frame.top+4+icon->Bounds().Height()+shift);
		view->MovePenTo(0,frame.top);

		view->SetDrawingMode(B_OP_COPY);
		view->DrawBitmap(icon, rect);
	}
	
	view->SetHighColor(0,0,0);
	view->StrokeRect(rect);

	if(IsSelected())
		 {
		    view->SetHighColor(0,0,120);
	  		for(int i=0;i<2;i++)view->StrokeRect(BRect(rect.left+i,rect.top+i,rect.right-i,rect.bottom-i));
		 }
	else
		for(int i=0;i<4;i++)
		 {
			view->SetHighColor(i*8+64,i*8+64,i*8+64);
			view->StrokeLine(BPoint(rect.left+4,rect.bottom+i),BPoint(rect.right+3,rect.bottom+i));		  
			view->StrokeLine(BPoint(rect.right+i,rect.top+4),BPoint(rect.right+i,rect.bottom+3));		  
		 }
		view->SetDrawingMode(B_OP_OVER);
	
	float dy=be_plain_font->Size()-5-shift;
	float dx=((width-be_plain_font->StringWidth(label.String()))/2.0)+shift;

	view->MovePenTo(frame.left+dx,frame.bottom-dy);
	view->SetHighColor(ThumbTextCol);
	view->DrawString(label.String());
		
} 

void ThumbListItem::Update(BView *view, const BFont *font)
{
	
}

const char *
ThumbListItem::GetLabel(void)
{
	return label.String();
}

void
ThumbListItem::SetThumb(int t)
{
 f_thumb=t;
}

int 
ThumbListItem::isThumbed(void)
{
 return f_thumb;
}
