#include <Application.h>
#include <InterfaceDefs.h>
#include <Window.h>
#include <Screen.h>
#include <TranslationUtils.h>

#include "MainApp.h"
#include "AboutWindow.h"
#include "Theme.h"
#include "Render.h"

/*
uint8 c_crosshairs_cursor_data[68] =
{
	16,1,7,7,
	0,0,	//0000000000000000
	1,0,	//0000000100000000
	1,0,	//0000000100000000
	1,0,	//0000000100000000
	1,0,	//0000000100000000
	1,0,	//0000000100000000
	1,0,	//0000000100000000
	127,252,//0111111101111100
	1,0,	//0000000100000000
	1,0,	//0000000100000000
	1,0,	//0000000100000000
	1,0,	//0000000100000000
	1,0,	//0000000100000000
	1,0,	//0000000100000000
	0,0,	//0000000000000000
	0,0,	//0000000000000000

	3,128,	//0000001110000000
	3,128,	//0000001110000000
	3,128,	//0000001110000000
	3,128,	//0000001110000000
	3,128,	//0000001110000000
	3,128,	//0000001110000000
	255,254,//1111111111111110
	255,254,//1111111111111110
	255,254,//1111111111111110
	3,128,	//0000001110000000
	3,128,	//0000001110000000
	3,128,	//0000001110000000
	3,128,	//0000001110000000
	3,128,	//0000001110000000
	3,128,	//0000001110000000
	0,0		//0000000000000000
};
*/

int32 lister(void *data)
{
  IterView *obj=(IterView*)data;
  for(;;) {
		obj->MyDraw();
  		obj->yshift++;
  		obj->xshift--;
  		obj->yshift2++;
  		obj->xshift2++;
  		snooze(50000);
  }
}

IterView::IterView(BRect R) : BView(R, "iterview", B_FOLLOW_ALL, B_WILL_DRAW|B_PULSE_NEEDED|B_FRAME_EVENTS)
{
	//SetViewColor(B_TRANSPARENT_32_BIT);
		
	SetViewColor(255,255,255);
	bufferView=new BView(R,"bufferview",B_FOLLOW_ALL_SIDES,0);
	buffer=new BBitmap(R,B_RGB32,true);
	
	buffer->AddChild(bufferView);
	
	size_t size=0;	
	const void *data=be_app->AppResources()->LoadResource(B_MESSAGE_TYPE,"Sky",&size);
	BMemoryIO stream(data, size);stream.Seek(0, SEEK_SET);
	SkyBitmap=BTranslationUtils::GetBitmap(&stream);

	size=0;
	const void *data2=be_app->AppResources()->LoadResource(B_MESSAGE_TYPE,"AboutPicture",&size);
	BMemoryIO stream2(data2, size);stream2.Seek(0, SEEK_SET);
	MaskBitmap=BTranslationUtils::GetBitmap(&stream2);

	
	SkyBuff=(int32*)SkyBitmap->Bits();
	ScrBuff=(int32*)buffer->Bits();
	
	H=3800;
 	D=250;
 	k=50;	
 	xshift=360;
 	yshift=0;

	H2=7000;
 	k2=60;	
 	xshift2=520;
 	yshift2=100;
 	
 	Hor=175;
 	
 	my_thread = spawn_thread(lister,"skythread",1,(void*)this);
	resume_thread(my_thread);
	 	
}

IterView::~IterView()
{
	
}


void
IterView::MyDraw(void)
{
  bool s=LockLooper();
  if(s==true)
	{
 	 bufferView->LockLooper();
 	 
 	 int SkyW=(int)(SkyBitmap->Bounds().Width())+1;
 	 int SkyH=(int)(SkyBitmap->Bounds().Height())+1;
  	 int SkrWHalf=(int)((buffer->Bounds().Width())+1)/2;
 	 int SkrW=(int)(buffer->Bounds().Width());
 	 	 
 	 ::AsmRender(ScrBuff,SkyBuff,D,H,k,xshift,yshift,H2,k2,xshift2,yshift2,Hor,SkyW,SkyH,SkrW,SkrWHalf);
	 
 	 bufferView->SetDrawingMode(B_OP_ALPHA);
	 bufferView->DrawBitmap(MaskBitmap,BPoint(0,0));
	 
 	 SetDrawingMode(B_OP_COPY);
	 DrawBitmap(buffer,bufferView->Bounds(),Bounds());

	 bufferView->UnlockLooper();
	 UnlockLooper();
	}	

}

void IterView::MouseMoved(BPoint p, uint32 transit,const BMessage *message)
{
//  if(p.x>250 && p.x<345 && p.y>220 && p.y<255)be_app->SetCursor(c_crosshairs_cursor_data);
//  else be_app->SetCursor(B_HAND_CURSOR);
}

void IterView::MouseDown(BPoint p)
{
//	be_app->SetCursor(B_HAND_CURSOR);
//	if(p.x>250 && p.x<345 && p.y>220 && p.y<255)
	Window()->QuitRequested();
}



AboutWindow::AboutWindow(BRect frame,BBitmap *bitmap):BWindow(frame,MES("WT:About"),B_MODAL_WINDOW_LOOK,B_NORMAL_WINDOW_FEEL,B_NOT_RESIZABLE|B_NOT_MINIMIZABLE|B_NOT_ZOOMABLE)
{
	scr=new BScreen(B_MAIN_SCREEN_ID);
	BPoint center=BPoint((scr->Frame().Width()-Bounds().Width())/2,(scr->Frame().Height()-Bounds().Height())/2);
	MoveTo(center);
	view = new IterView(Bounds());
	AddChild(view);
}

void AboutWindow::MessageReceived(BMessage *msg)
{
	switch(msg->what) {
	default:
		BWindow::MessageReceived(msg);
		break;
	}
}

bool AboutWindow::QuitRequested()
{
	kill_thread(find_thread("skythread"));
	Quit();
	return true;
}
