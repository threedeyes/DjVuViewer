#include <stdio.h>
#include <string.h>

#include <Window.h>
#include <Alert.h>
#include <Picture.h>
#include <TextControl.h>
#include <Box.h>
#include <ListView.h>
#include <Menu.h>
#include <Message.h>
#include <Bitmap.h>
#include <File.h>
#include <storage/Path.h>
#include <Entry.h>
#include <PopUpMenu.h>
#include <DataIO.h>
#include <Resources.h>
#include <Application.h>
#include <SupportDefs.h>
#include <translation/TranslationUtils.h>
#include <translation/TranslatorRoster.h>
#include <translation/BitmapStream.h>
#include <Roster.h>

#include "Theme.h"

BmpListItem::BmpListItem(char *Name,BBitmap *Bmp)
{
	strcpy(name,Name);
	bitmap=Bmp;
}

BmpListItem::~BmpListItem()
{
	
}

BTheme::BTheme()
{
	fRes=be_app->AppResources();

	type_code typeFound;
	int32 idFound;
	const char *nameFound;
	size_t lengthFound;
	int n=0;
	for(n=0;fRes->GetResourceInfo(n,&typeFound,&idFound,&nameFound,&lengthFound)==true;n++);
	fBmpList=new BList(n+1);
}

BTheme::~BTheme()
{
	BmpListItem *anItem;
	for(int32 i=0;fBmpList->ItemAt(i)!=NULL;i++) {
			anItem=(BmpListItem*)fBmpList->ItemAt(i);
			delete anItem;
	}
	delete fBmpList;
}	

bool
BTheme::SetDirectory(const char *path)
{
 if(path[0]=='.' && strlen(path)>2)
 {
   app_info inf;
   be_app->GetAppInfo(&inf);
   BPath bpath = BPath(&(inf.ref));
   BPath par; bpath.GetParent(&par);
   par.Append(path+2);
   ThemesFolder.SetTo(par.Path());
  }
  else
   ThemesFolder.SetTo(path);
  return true;
}

BDirectory *
BTheme::GetDirectory(void)
{  
  return &ThemesFolder;
}


bool
BTheme::SetTheme(const char* theme)
{	
	BFile file;
 	fCurTheme.SetTo(&ThemesFolder,NULL);
	fCurTheme.Append(theme);
 	
	if(fRes!=be_app->AppResources())delete fRes;

 	if(file.SetTo(fCurTheme.Path(), B_READ_ONLY)==B_OK)
 		{
			fRes=new BResources(&file);
 		}
 	else	
			fRes=be_app->AppResources();

	type_code typeFound;
	int32 idFound;
	const char *nameFound;
	size_t lengthFound;
	int n=0;
	for(n=0;fRes->GetResourceInfo(n,&typeFound,&idFound,&nameFound,&lengthFound)==true;n++);
	
	BmpListItem *anItem;
	for(int32 i=0;fBmpList->ItemAt(i)!=NULL;i++) {
			anItem=(BmpListItem*)fBmpList->ItemAt(i);
			delete anItem;
	}
	delete fBmpList;
	fBmpList=new BList(n+1);

	return true;
}

const char*
BTheme::GetCurrentTheme(void)
{
	return fCurTheme.Leaf();
}


BBitmap*
BTheme::GetBmp(const char* name)
{
	BmpListItem *item;
	for(int32 i=0;fBmpList->ItemAt(i)!=NULL;i++)
	{
		item=(BmpListItem*)(fBmpList->ItemAt(i));
		if(strcmp(item->name,name)==0)return item->bitmap;
	}
	
	size_t size=0;
	const void *data=fRes->LoadResource(B_MESSAGE_TYPE,name,&size);
	BMemoryIO stream(data, size);stream.Seek(0, SEEK_SET);
	BMessage archive;archive.Unflatten(&stream);
	BBitmap *bmp = new BBitmap(&archive);	
	fBmpList->AddItem(new BmpListItem((char*)name,bmp));
	return bmp;
}

BBitmap*
BTheme::GetPNG(const char* name)
{
	BmpListItem *item;
	for(int32 i=0;fBmpList->ItemAt(i)!=NULL;i++)
	{
		item=(BmpListItem*)(fBmpList->ItemAt(i));
		if(strcmp(item->name,name)==0)return item->bitmap;
	}

	size_t size=0;
	const void *data=fRes->LoadResource(B_MESSAGE_TYPE,name,&size);
	BMemoryIO stream(data, size);stream.Seek(0, SEEK_SET);
	BBitmap *bmp=BTranslationUtils::GetBitmap(&stream);
	fBmpList->AddItem(new BmpListItem((char*)name,bmp));
	return bmp;
}
