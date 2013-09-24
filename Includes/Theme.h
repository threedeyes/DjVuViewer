#ifndef __THEME__
#define __THEME__


#include <Locker.h>
#include <Resources.h>
#include <storage/Path.h>
#include <Directory.h>
#include <File.h>
#include <Bitmap.h>
#include <List.h>
#include <Mime.h>
#include <image.h>

class BmpListItem
{
public:
	BmpListItem(char *name,BBitmap *bitmap);
	~BmpListItem();
	char name[64];
	BBitmap *bitmap;
};	

class BTheme
{
 public:
 	BTheme();
 	~BTheme();
 	bool SetDirectory(const char *path);
 	BDirectory *GetDirectory(void);
 	bool SetTheme(const char *name);
 	const char* GetCurrentTheme(void);
 	BBitmap* GetBmp(const char* name);
 	BBitmap* GetPNG(const char* name);
// 	const char* GetString(const char* name);
// 	int32 GetInt32(const char* name);
// 	float GetFloat(const char* name);
// 	rgb_color GetColor(const char* name);
 private:
 	BList 	   *names;
 	BList 	   *bitmaps;
 	BResources *fRes;
 	BPath	   fCurTheme;
	BDirectory ThemesFolder;
	const char *fTheme; 	
	BList		*fBmpList;
};

#endif
