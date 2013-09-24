#ifndef __T_HSPLITTER__
#define __T_HSPLITTER__

#include <View.h>
#include "Theme.h"

class BHSplitter: public BView {
public:
	BHSplitter(BRect, const char *Name, BWindow *wnd,uint32 style,uint32 resize);
	~BHSplitter();
protected:
	int drag;
	bigtime_t fLastClickTime;
	BWindow *wnd;
	virtual void AttachedToWindow();
	virtual void MouseDown(BPoint where);
	virtual void MouseMoved(BPoint where, uint32 code, const BMessage *message);
	virtual void MouseUp(BPoint where);			

};

#endif
