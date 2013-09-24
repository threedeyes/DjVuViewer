#include <stdio.h>
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

#include "Language.h"
#include "MainApp.h"

char const EmptyString[]={""};

BLang::BLang()
{
	fMes=new BMessage();
	fDefMes=new BMessage();
//	fRes=be_app->AppResources();
}

BLang::~BLang()
{
}	

bool
BLang::SetDirectory(const char *path)
{
 if(path[0]=='.' && strlen(path)>2)
 {
	BPath par=my_app->Path(path+2);
    LanguageFolder.SetTo(par.Path());
  }
  else
   LanguageFolder.SetTo(path);
  return true;
}

BDirectory *
BLang::GetDirectory(void)
{  
  return &LanguageFolder;
}


bool
BLang::SetLanguage(const char* lang)
{	
	FILE *in;
	char temp[1024];
	char name[1024];
	char text[1024];
	
	fCurLang.SetTo(&LanguageFolder,NULL);
	fCurLang.Append(lang);
	 	
//	if(fRes!=be_app->AppResources())delete fRes;
	fMes->MakeEmpty();

 	if(in=fopen(fCurLang.Path(),"rt"))
	{
		for(;;)
		 {
		 	char *beg=temp,*end=temp;
		 	if(fgets(temp,1023,in)==NULL)break; 
		 	for(;;beg++)if(*beg==' ' || *beg=='\t' || *beg=='\n')break;
			*beg=0;
			beg=strstr(beg+1,"\"");	if(beg==NULL)continue;		
			end=strstr(beg+1,"\""); if(end==NULL)continue;
			beg++;
			*end=0;
			fMes->AddString(temp,beg);
		 } 			
		fclose(in);	
	}
	return true;
}

bool
BLang::SetDefaultLanguage(const char* lang)
{	
	FILE *in;
	char temp[1024];
	char name[1024];
	char text[1024];
	
	fDefLang.SetTo(&LanguageFolder,NULL);
	fDefLang.Append(lang);
	 	
//	if(fRes!=be_app->AppResources())delete fRes;
	fDefMes->MakeEmpty();

 	if(in=fopen(fDefLang.Path(),"rt"))
	{
		for(;;)
		 {
		 	char *beg=temp,*end=temp;
		 	if(fgets(temp,1023,in)==NULL)break; 
		 	for(;;beg++)if(*beg==' ' || *beg=='\t' || *beg=='\n')break;
			*beg=0;
			beg=strstr(beg+1,"\""); if(beg==NULL)continue;
			end=strstr(beg+1,"\""); if(end==NULL)continue;
			beg++;
			*end=0;
			fDefMes->AddString(temp,beg);
		 } 			
		fclose(in);	
	}
	return true;
}


const char*
BLang::GetCurrentLanguage(void)
{
	return fCurLang.Leaf();
}


const char*
BLang::GetString(const char* name)
{
	size_t size=0;
	const char *Text;
	fMes->FindString(name,&Text);
	if(Text!=NULL)return Text;
    fDefMes->FindString(name,&Text);
	if(Text!=NULL)return Text;
    return EmptyString;
}
