#ifndef __ABOUTWINDOW_H__
#define __ABOUTWINDOW_H__

#include <Application.h>
#include <be/interface/PopUpMenu.h>
#include <be/interface/MenuItem.h>
#include <be/interface/ListView.h>
#include <be/interface/ScrollView.h>
#include <be/interface/StringView.h>
#include <be/interface/InterfaceDefs.h>
#include <be/interface/OutlineListView.h>
#include <be/interface/ListItem.h>
#include <be/interface/Menu.h>
#include <be/interface/Font.h>
#include <Window.h>
#include <Alert.h>
#include <Button.h>
#include <unistd.h>
#include <Bitmap.h>
#include <Screen.h>

#include <be/storage/Path.h>
#include <Entry.h>
#include <Directory.h>


#define wW 352
#define wH 264

class IterView : public BView
{
 public:
					IterView(BRect R);
					~IterView();
     	 					    
	void			MyDraw(void);
 	virtual void 	MouseDown(BPoint p);
 	virtual void 	MouseMoved(BPoint point, uint32 transit, const BMessage *message);
 			
	int				x,y;	
 	int 			H,H2;
 	int 			D;
 	int 			k,k2;	
 	int 			Hor;

 	int 			xshift,yshift;
 	int 			xshift2,yshift2; 	
	
	BBitmap 		*buffer;
	BBitmap 		*SkyBitmap;	
	BBitmap 		*MaskBitmap;	
    BView 	 		*bufferView;
    
    int32			*SkyBuff;
    int32 			*ScrBuff;
    
    thread_id my_thread;
};


class AboutWindow : public BWindow
{
public:
	AboutWindow(BRect frame,BBitmap *bitmap);
	virtual void MessageReceived(BMessage *msg);
	virtual bool QuitRequested();

	BView *w;
 	BScreen *scr;
	IterView *view;
};
//------------------------------------------------------------------------------
#endif
