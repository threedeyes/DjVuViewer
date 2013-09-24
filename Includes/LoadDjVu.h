#ifndef __LOADDJVU__
#define __LOADDJVU__


#include <Locker.h>
#include <Resources.h>
#include <storage/Path.h>
#include <Directory.h>
#include <File.h>
#include <Bitmap.h>
#include <List.h>
#include <Mime.h>
#include <image.h>
#include <String.h>

#include <libdjvu/ddjvuapi.h>

class BDjVuFile
{
 public:
 				BDjVuFile(const char *name);
 				~BDjVuFile();
 	bool 		SetFile(const char *name);
 	const char* GetCurrentFile(void);
 	BBitmap* 	GetPage(int p);
 	BBitmap* 	GetThumb(int p);
 	int 		MaxPages;
 	BBitmap* 	Render(ddjvu_page_t *page,int flag_size,int flag_subsample,int flag_scale,int w,int h);
	BString 	*filename;
	BMessage	thumbs;
 private:
 	ddjvu_document_t *doc;
};

#endif
