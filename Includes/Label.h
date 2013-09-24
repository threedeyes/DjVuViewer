#ifndef __T_LABEL__
#define __T_LABEL__

#include <View.h>
#include <String.h>
#include "Theme.h"

#define BL_RECT_FLAG 1

class BTLabel: public BView {
public:
	BTLabel(BRect, const char *Name, const char *Label,
				BBitmap *background,uint32 style,uint32 resize);
	~BTLabel();
	void SetTextColor(rgb_color);
	void SetText(const char *label);
	const char *GetText(void);

protected:
	virtual void Draw(BRect rect);	
	virtual void AttachedToWindow();
	virtual	void FrameResized(float width, float height);
	virtual void MouseDown(BPoint p);
	virtual void MouseUp(BPoint p);	
	
	float 		text_dx,text_dy;
	rgb_color 	text_color;
	
	uint32		style;
	uint32		lastButtons;

	BBitmap 	*fBackGround;
	BBitmap 	*OVbmp;
	BView   	*OVview;
	BString 	fLabel;
	const char  *fName;
	BPopUpMenu	*RightClickPopUp;		
};

#endif
