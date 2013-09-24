#ifndef _SETTINGS_H
#define _SETTINGS_H

#include <string.h>

#include <Application.h>
#include <Deskbar.h>
#include <Roster.h>
#include <Font.h>
#include <Message.h>
#include <SupportDefs.h>
#include <TypeConstants.h>

struct font_data
{
	font_family family;
	font_style style;
	uint32	size;
	uint32	shear;
};

class PSettings : public BMessage {
	public:
		PSettings(void);
		~PSettings();
		bool Save(const char *file);
		bool Load(const char *file);
		void SetColor(const char *name,rgb_color *color);
		void SetColor(const char *name,unsigned char r,unsigned char g,unsigned char b);
		void SetFont(const char *name,BFont *font);
		void SetFont(const char *name,const char *family,const char *style,uint32 size,uint32 shear);
		void SetInt32(const char *name,int32 value);
		void SetFloat(const char *name,float value);
		void SetBool(const char *name,bool value);
		void SetString(const char *name,const char *text);
		void	SetRect(const char *name,BRect rect);
		
		rgb_color   GetColor(const char *name);
		BFont		GetFont(const char *name);
		int32		GetInt32(const char *name);
		int32		GetFloat(const char *name);
		int32		GetInt32(const char *name,int32 def);
		const char*	GetString(const char *name);
		bool		GetBool(const char *name);
		BRect		GetRect(const char *name);

};


#endif
