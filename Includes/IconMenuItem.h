#ifndef __ICONMENUITEM_H__
#define __ICONMENUITEM_H__

#include <String.h>
#include <Bitmap.h> 
#include <Rect.h>
#include <Font.h>
#include <Menu.h>
#include <MenuItem.h>

#include "MainApp.h"

class IconMenuItem : public BMenuItem
{ 
	public:
		IconMenuItem(BTheme *th, const char *bmp, const char *text, BMessage *mess); 
		IconMenuItem(BTheme *theme,const char *bmp, const char *text, BMenu *submenu, BMessage *message=NULL);
		IconMenuItem(BBitmap *icon, const char *text, BMessage *mess=NULL);
		virtual ~IconMenuItem(); 
		virtual void DrawContent(void); 
		virtual void GetContentSize(float *width, float *height);
		void ChangeIcon(const char *bmp);
	private:
		BString label;
		BString bitmap;
		BTheme	*fTheme;
		BBitmap	*Icon;
};

#endif