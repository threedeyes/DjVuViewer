#include <stdio.h>
#include <Window.h>
#include <Alert.h>
#include <Picture.h>
#include <TextControl.h>
#include <Box.h>
#include <Rect.h>
#include <ListView.h>
#include <Menu.h>
#include <Message.h>
#include <Bitmap.h>
#include <File.h>
#include <PopUpMenu.h>
#include <DataIO.h>
#include <Resources.h>
#include <SupportDefs.h>
#include <Menu.h>
#include <PopUpMenu.h>
#include <MenuItem.h>
#include <Region.h>
#include <storage/Path.h>

#include "ToolsContainer.h"
#include "Theme.h"
#include "PicButton.h"
#include "IconMenuItem.h"

static const int32 kMaxHistory = 32;

static const rgb_color kBgColor = {220, 220, 220, 255};
static const rgb_color kShineColor = {255, 255, 255, 255};
static const rgb_color kHalfDarkColor = {200, 200, 200, 255};
static const rgb_color kDarkColor = {166, 166, 166, 255};



BToolsContainer::BToolsContainer(BRect rect,const char *Name,int32 _flags,BTheme *theme,uint32 resizeMask)
	: BView(rect, Name, resizeMask, B_WILL_DRAW|B_FRAME_EVENTS) //
{
	fTheme=theme;
	fHidMenu=NULL;
	bgName=BString("BackGround");
	if(theme!=NULL)
	 {
	 fBackGround = fTheme->GetPNG(bgName.String());
	 if(fBackGround==NULL)fBackGround = fTheme->GetPNG("BackGround");
	 fMouseOver = fTheme->GetPNG("MouseOver");	
	 fClipButton = fTheme->GetPNG("ToolsClipButton");	
	}
	else 
	{
	 fBackGround=NULL;
	 fMouseOver=NULL;
	 fClipButton=NULL;
	}
	old=rect;
	flags=_flags;
	hidpres=false;
	
	//OVbmp=NULL;//new BBitmap(BRect(0,0,rect.Width(),rect.Height()),B_RGBA32_BIG,true);
	//OVview=new BView(BRect(0,0,rect.Width(),rect.Height()),"",0,0);
	 
	RightClickPopUp=NULL;
	
	BFont font = be_plain_font;
	font.SetSize(10);
	//SetFont(&font,B_FONT_SIZE);	
	SetFont(&font,B_FONT_SIZE);
	//NeedUpdate=1;
//	OVbmp->AddChild(OVview);
	over_flag=false;
	old_over_flag=false;
	buttons=0;
		
}

BToolsContainer::~BToolsContainer()
{
	//delete OVbmp;
//	delete fBackGround;
//	delete fMouseOver;
//	delete fClipButton;
}

void BToolsContainer::AttachedToWindow()
{	
  FrameResized(Bounds().Width(),Bounds().Height());
}

void BToolsContainer::MouseMoved(BPoint point, uint32 transit,const BMessage *message)
{
 float clipw=flags&TCNT_CLIP?fClipButton->Bounds().Width()+1:1;
 float cliph=flags&TCNT_CLIP?fClipButton->Bounds().Height()+1:1;
 
 if(flags&TCNT_CLIP)	
  {
	switch(transit)
	 {
	 	case B_INSIDE_VIEW:
	 	case B_ENTERED_VIEW:
	 		{
   				over_flag = point.x>Bounds().right-(fClipButton->Bounds().Width()+2) && 
   				 		  	point.y<Bounds().top+fClipButton->Bounds().Height()+4;
	 			break;
	 		}
	 	case B_EXITED_VIEW:
	 		over_flag=false;
	 		break;
	 	case B_OUTSIDE_VIEW:
	 		over_flag=false;
	 		break; 
	 }
	 
	if(old_over_flag!=over_flag)Invalidate(BRect((Bounds().Width()-clipw)-2,-2,Bounds().Width()+1,cliph+2));
    old_over_flag=over_flag;
  }
 BView::MouseMoved( point,  transit, message);
}

//void BToolsContainer::MouseDown(BPoint point)
//{
//  buttons = Window()->CurrentMessage()->FindInt32("buttons");
//}

void BToolsContainer::MouseDown(BPoint point)
{
   int n=0;
   buttons = Window()->CurrentMessage()->FindInt32("buttons");
   uint32	btn=buttons;
   buttons=0;
   if(flags&TCNT_CLIP)	
   if(point.x>Bounds().right-(fClipButton->Bounds().Width()+2) && 
	   point.y<Bounds().top+fClipButton->Bounds().Height()+4)
	{
			if(fHidMenu!=NULL)delete fHidMenu;
			fHidMenu=new BPopUpMenu("hidden");
			BMenuItem *mi;
			
		    for(int i=0;i<CountChildren();i++)
			{
			 BView *child=ChildAt(i);
			 if(child->IsHidden())
			  {
			   const char *label;
			   if(is_kind_of(child,BPicButton)==true)
			    {
			     label=((BPicButton*)child)->GetLabel();
			     //fHidMenu->AddItem(new IconMenuItem(&THEME,((BPicButton*)child)->GetBmpName(),label,((BPicButton*)child)->fPushMessage));
			      fHidMenu->AddItem(new BMenuItem(label,new BMessage(*((BPicButton*)child)->fPushMessage)));
			    }
			   else
			    {
			     label=child->Name();
			     fHidMenu->AddItem(new BMenuItem(label,NULL));
			    }
			   n++;
			  }
			}			
			if(n!=0)
			{
			// point.x-=6;point.y-=6;
			 ConvertToScreen(&point);
			 fHidMenu->SetTargetForItems(Window());
			 mi=fHidMenu->Go(point,true,true,true);//,true,true,false);	
			 return;
			}
	}
	
   
   if( btn & B_SECONDARY_MOUSE_BUTTON && RightClickPopUp!=NULL)
	 	{
			point.x-=6;point.y-=6;
			RightClickPopUp->Go(ConvertToScreen(point),true,true,true);	
			return;
	  	}
   BView::MouseDown(point);	
}


void BToolsContainer::FrameResized(float width, float height)
{
	bool pres_flag=false;
	bool changed=false;
	float clipw=flags&TCNT_CLIP?fClipButton->Bounds().Width()+1:1;
	
	if(flags&TCNT_CLIP)	
	 for(int i=0;i<CountChildren();i++)
		{
			BView *child=ChildAt(i);
			if(child->Frame().right+clipw > width)
			{
				if(!child->IsHidden()){child->Hide();changed=true;}
				pres_flag=true;
			}
			else
			{
			 	if(child->IsHidden()){child->Show();changed=true;}
			}
		}
	hidpres=pres_flag;

	if(width>old.right)Invalidate( BRect( (old.right-clipw)-2,-1,width+1,height+1));
		else Invalidate(BRect((width-clipw)-2,-2,width+1,height+1));	
	if(height>old.bottom)Invalidate( BRect( -1, old.bottom-1 , width+1, height+1));
	
	old=Bounds();
	
	if(changed==true)Invalidate();
}

void BToolsContainer::Draw(BRect rect)
{	
 BRegion reg;
 BRegion newr=BRegion(rect);
 GetClippingRegion(&reg);
 ConstrainClippingRegion(&newr);
 
 SetDrawingMode(B_OP_COPY);
 
 
 if(fBackGround!=NULL && flags&TCNT_BACKGROUND)
  {	
	BRect all=Bounds();
	BRect r=fBackGround->Bounds();
	 for(float y=0;y<all.Height();y+=r.Height()+1)
		for(float x=0;x<all.Width();x+=r.Width()+1)
		{
			DrawBitmapAsync(fBackGround,BPoint(x,y));
		}
   }	
   else 
   {
    SetHighColor(ViewColor());
    FillRect(Bounds());
   }
	
	SetDrawingMode(B_OP_ALPHA);
	
	if(flags&TCNT_CLIP && hidpres)
	 {
	    float shift=0;
	    if(over_flag==true)shift=1;
		DrawBitmapAsync(fClipButton,BPoint((Bounds().right-(fClipButton->Bounds().Width()+2))+shift,2+shift));
	 }

	Sync();
	
	if(flags&TCNT_FRAME)
	 {
		BeginLineArray(4);
		AddLine(Bounds().LeftTop(), Bounds().RightTop(), kShineColor);
		AddLine(Bounds().LeftTop(), Bounds().LeftBottom() - BPoint(0, 1), kShineColor);
		AddLine(Bounds().LeftBottom(), Bounds().RightBottom(), kDarkColor);
		AddLine(Bounds().RightTop(), Bounds().RightBottom(), kDarkColor);
		EndLineArray();
	 }

   ConstrainClippingRegion(&reg);
}

BTheme*
BToolsContainer::Theme()
{
 return fTheme;
}

BBitmap*
BToolsContainer::ViewBitmap()
{
 return fBackGround;
}

BPicButton*
BToolsContainer::AddSeparator(int32 index)
{
  uint32 f=0;
  if(fBackGround!=NULL && flags&TCNT_BACKGROUND)f=0; else f=PB_NO_BACKGROUND;
  return AddButton("Separator","Separator",NULL,new BMessage('none'),index,f);
}

BPicButton*
BToolsContainer::AddButton(const char* BmpName,const char* Label,const char *ToolTip,BMessage *mess,int32 index,uint32 flags)
{	
	BBitmap *pic=fTheme->GetPNG(BmpName);
	//if(pic==NULL)return NULL;

	float text_width=StringWidth(Label);
	float pic_width=pic->Bounds().Width()/3.0;
	float max_width=flags&PB_LABEL?((text_width>pic_width?text_width:pic_width)+4.0):(pic_width+2.0);

	BRect rect=BRect(0,0,max_width,pic->Bounds().Height()+(flags&PB_LABEL==PB_LABEL?12:4));
	rect.OffsetTo(2,0);
	if(CountChildren()!=0)
		if(index<0)
			rect.OffsetTo(ChildAt(CountChildren()-1)->Frame().right+4,0);
		else
		{
			rect=ChildAt(index)->Frame();
			for(int i=index;i<CountChildren();i++)ChildAt(i)->MoveBy(rect.Width()+4,0);
		}
	
	float y_shift=(Bounds().Height()-rect.Height())/2.0;
	rect.OffsetBy(0,y_shift);
			
	BPicButton *button=new BPicButton(rect,BmpName,Label,ToolTip,fTheme,mess,0,flags,B_FOLLOW_LEFT|B_FOLLOW_TOP);
	button->SetBackground(bgName.String());
	if(index>=0)AddChild(button,ChildAt(index));
	else AddChild(button);
	
	FrameResized(Bounds().Width(),Bounds().Height());
	
	return button;
}

void BToolsContainer::SetBackground(const char *bgname)
{
 bgName=BString(bgname);
 if(fTheme!=NULL)
 { 
  fBackGround = fTheme->GetPNG(bgName.String());
  if(fBackGround==NULL)fBackGround = fTheme->GetPNG("BackGround"); 
 }
}


void BToolsContainer::ReloadTheme(void)
{
	if(fTheme!=NULL)
	{
	 fBackGround = fTheme->GetPNG(bgName.String());
 	 if(fBackGround==NULL)fBackGround = fTheme->GetPNG("BackGround");
	 fMouseOver = fTheme->GetPNG("MouseOver");	
	 fClipButton = fTheme->GetPNG("ToolsClipButton");
	}
	
	FrameResized(Bounds().Width(),Bounds().Height());
	for(int i=0;i<CountChildren();i++)
		{
			if(is_kind_of(ChildAt(i),BPicButton)==true)
			 {
			   ((BPicButton*)ChildAt(i))->SetBackground(bgName.String());
			   ((BPicButton*)ChildAt(i))->ReloadTheme();
			 }
		}
	Invalidate();
}

void BToolsContainer::SetContextMenu(BPopUpMenu *popup)
{
	RightClickPopUp=popup;	
}
