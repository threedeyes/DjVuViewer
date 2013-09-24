#ifndef __LANG__
#define __LANG__

#include <Locker.h>
#include <Resources.h>
#include <storage/Path.h>
#include <Directory.h>
#include <File.h>
#include <Bitmap.h>
#include <List.h>
#include <Mime.h>
#include <image.h>

class BLang
{
 public:
 	BLang();
 	~BLang();
 	bool SetDirectory(const char *path);
 	BDirectory *GetDirectory(void);
 	bool SetLanguage(const char *name);
 	bool SetDefaultLanguage(const char* lang);
 	const char* GetCurrentLanguage(void);
 	const char* GetString(const char* name);
 	BBitmap* GetPNG(const char* name);
 private:
 	BResources *fRes;
 	BResources *fDefRes;
 	
 	BMessage   *fMes;
 	BMessage   *fDefMes;
 	
 	BPath	   fCurLang;
 	BPath	   fDefLang;
	BDirectory LanguageFolder;
	const char *fLanguage; 	
};


#endif
