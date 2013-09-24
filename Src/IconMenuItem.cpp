#include <View.h>
#include <Region.h>
#include "Theme.h"
#include "IconMenuItem.h"

IconMenuItem::IconMenuItem(BTheme *theme, const char *bmp, const char *text, BMessage *mess) :
	BMenuItem(text,mess)
{
	fTheme=theme;
	bitmap.SetTo(bmp);
	label.SetTo(text);
} 

IconMenuItem::IconMenuItem(BBitmap *icon, const char *text, BMessage *mess) :
	BMenuItem(text,mess)
{
	fTheme=NULL;
	bitmap.SetTo("");
	label.SetTo(text);
	Icon=icon;
} 

IconMenuItem::IconMenuItem(BTheme *theme, const char *bmp, const char *text, BMenu *submenu, BMessage *message):
	BMenuItem(submenu,message)
{
	fTheme=theme;
	bitmap.SetTo(bmp);
	label.SetTo(text);
}
 
IconMenuItem::~IconMenuItem() 
{
} 
 

void IconMenuItem::ChangeIcon(const char *bmp)
{
	bitmap.SetTo(bmp);
}

void
IconMenuItem::GetContentSize(float *width, float *height)
{
	BMenuItem::GetContentSize(width, height);
	*width += 20;
	*height += 2;
}


void IconMenuItem::DrawContent(void) 
{
	Menu()->PushState();

	BFont font;
	Menu()->GetFont(&font);

	BPoint drawPoint(ContentLocation());
	drawPoint.x += 18;
	drawPoint.y += 1;
	Menu()->MovePenTo(drawPoint);
	BMenuItem::DrawContent();
	
	BBitmap *icon;
	if(fTheme!=NULL) 
		icon=fTheme->GetPNG(bitmap.String());
	else icon=Icon;
	if(icon!=NULL)
	 {	 
	  float h=icon->Bounds().Height();
	  float w=icon->Bounds().Height();
	  if(icon->Bounds().Height()>Frame().Height())
	   {
	    float k=w/h;
	    h=Frame().Height();
	    w=h*k;
	   }
	  
	  float y1=Frame().top + ((Frame().Height()-h)/2);
	  float y2=Frame().top + ((Frame().Height()-h)/2)+h;
	  drawing_mode mode=Menu()->DrawingMode();
	  Menu()->SetDrawingMode(B_OP_ALPHA);
	  Menu()->DrawBitmap(icon,BRect(Frame().left+12,y1,Frame().left+12+w,y2) );
	  Menu()->SetDrawingMode(mode);
	 }
	Menu()->PopState();	  
} 
