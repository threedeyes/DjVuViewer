#include <Application.h>
#include <Message.h>
#include <File.h>
#include <stdlib.h>
#include <be/support/SupportDefs.h>
#include <be/support/TypeConstants.h>
#include <Flattenable.h>

#include "Settings.h"

PSettings::PSettings() : BMessage()
{
			
}

PSettings::~PSettings()
{

}

bool 
PSettings::Save(const char *file)
{
	BFile out;
	if(out.SetTo(file,B_READ_WRITE|B_CREATE_FILE|B_ERASE_FILE)!=B_OK)return false;
	if(Flatten(&out)!=B_OK)return false;;
	out.Unset();	
	return true;
}

bool 
PSettings::Load(const char *file)
{
	BFile out;
	if(out.SetTo(file,B_READ_ONLY)!=B_OK)return false;
	if(Unflatten(&out)!=B_OK)return false;
	out.Unset();		
	return true;
}

void 
PSettings::SetColor(const char *name,rgb_color *color)
{
	rgb_color *old;
	ssize_t	  size;
	status_t s=FindData(name,B_ANY_TYPE,(const void**)&old,&size);
	if(s==B_OK)ReplaceData(name,B_RGB_COLOR_TYPE,color,sizeof(rgb_color));
	else AddData(name,B_RGB_COLOR_TYPE,color,sizeof(rgb_color));
}

void 
PSettings::SetColor(const char *name,unsigned char r,unsigned char g,unsigned char b)
{
	rgb_color  color={r,g,b};
	rgb_color *old;
	ssize_t	  size;
	status_t s=FindData(name,B_ANY_TYPE,(const void**)&old,&size);
	if(s==B_OK)ReplaceData(name,B_RGB_COLOR_TYPE,&color,sizeof(rgb_color));
	else AddData(name,B_RGB_COLOR_TYPE,&color,sizeof(rgb_color));
}

void
PSettings::SetFont(const char *name,BFont *font)
{
	font_data *old;
	font_data newfont;
	font->GetFamilyAndStyle(&newfont.family,&newfont.style);
	newfont.size=font->Size();
	newfont.shear=font->Shear();
	ssize_t	  size;
	status_t s=FindData(name,B_ANY_TYPE,(const void**)&old,&size);
	if(s==B_OK)ReplaceData(name,B_RAW_TYPE,&newfont,sizeof(font_data));
	else AddData(name,B_RAW_TYPE,&newfont,sizeof(font_data));
}

void
PSettings::SetFont(const char *name,const char *family,const char *style,uint32 size,uint32 shear)
{
	font_data *old;
	font_data newfont;
	strcpy(newfont.family,family);
	strcpy(newfont.style,style);
	newfont.size=size;
	newfont.shear=shear;
	ssize_t	  sz;
	status_t s=FindData(name,B_ANY_TYPE,(const void**)&old,&sz);
	if(s==B_OK)ReplaceData(name,B_RAW_TYPE,&newfont,sizeof(font_data));
	else AddData(name,B_RAW_TYPE,&newfont,sizeof(font_data));
}


void
PSettings::SetInt32(const char *name,int32 value)
{
	int32	old;
	status_t s=FindInt32(name,&old);
	if(s==B_OK)ReplaceInt32(name,value);
	else AddInt32(name,value);	
}

void
PSettings::SetFloat(const char *name,float value)
{
	float	old;
	status_t s=FindFloat(name,&old);
	if(s==B_OK)ReplaceFloat(name,value);
	else AddFloat(name,value);	
}

void
PSettings::SetRect(const char *name,BRect rect)
{
	BRect	old;
	status_t s=FindRect(name,&old);
	if(s==B_OK)ReplaceRect(name,rect);
	else AddRect(name,rect);	
}

void
PSettings::SetBool(const char *name,bool value)
{
	bool	old;
	status_t s=FindBool(name,&old);
	if(s==B_OK)ReplaceBool(name,value);
	else AddBool(name,value);	
}


void
PSettings::SetString(const char *name,const char *text)
{
	const char *old;
	status_t s=FindString(name,&old);
	if(s==B_OK)ReplaceString(name,text);
	else AddString(name,text);	
		
}

rgb_color 
PSettings::GetColor(const char *name)
{
	rgb_color *color;
	ssize_t	  size;
	status_t s=FindData(name,B_ANY_TYPE,(const void**)&color,&size);
	return *color;
}

BFont
PSettings::GetFont(const char *name)
{
	BFont font = be_plain_font;
	font_data *val;
	ssize_t	  size;
	status_t s=FindData(name,B_ANY_TYPE,(const void**)&val,&size);
	if(s==B_OK)
	{
		char *fstyle=NULL,*ffamily=NULL;
		if(val->style[0]!=0)fstyle=val->style;
		if(val->family[0]!=0)ffamily=val->family;
		font.SetFamilyAndStyle(ffamily,fstyle);
		font.SetSize(val->size);
		font.SetShear(val->shear);
	}
	return font;	
}

int32		
PSettings::GetInt32(const char *name)
{
	int32	val=0;
	FindInt32(name,&val);
	return val;
}

int32		
PSettings::GetFloat(const char *name)
{
	float	val=0;
	FindFloat(name,&val);
	return val;
}

int32		
PSettings::GetInt32(const char *name,int32 def)
{
	int32	val=0;
	if(FindInt32(name,&val)==B_OK)return val;
	else
	return def;
}

bool		
PSettings::GetBool(const char *name)
{
	bool	val=false;
	FindBool(name,&val);
	return val;
}

BRect
PSettings::GetRect(const char *name)
{
	BRect	rect;
	status_t s=FindRect(name,&rect);
	return rect;
}

const char*		
PSettings::GetString(const char *name)
{
	const char *old;
	status_t s=FindString(name,&old);
	return old;
}
