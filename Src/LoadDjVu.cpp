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
#include <OS.h>

#include "LoadDjVu.h"

extern sem_id render_sem;

ddjvu_context_t *ctx=NULL;

BDjVuFile::BDjVuFile(const char* file)
{
 filename=new BString(file);
 if(ctx==NULL)
 {
 	app_info inf;
 	be_app->GetAppInfo(&inf);
 	BPath bpath = BPath(&(inf.ref)); 
  	if(!(ctx = ddjvu_context_create(bpath.Path())))return;
  	ddjvu_cache_set_size(ctx,100000);
 }
 
 if(!(doc = ddjvu_document_create_by_filename(ctx, file, TRUE)))return;
 while (! ddjvu_document_decoding_done(doc));
 
 MaxPages=-1;
 MaxPages = ddjvu_document_get_pagenum(doc);
 
 thumbs.MakeEmpty();
}

BDjVuFile::~BDjVuFile()
{
 ddjvu_document_release(doc);
 ddjvu_cache_clear(ctx);
// ddjvu_context_release(ctx);
}	


bool
BDjVuFile::SetFile(const char* file)
{	
	filename=new BString(file);
	ddjvu_context_release(ctx);
 	ctx = ddjvu_context_create(file);
 	doc = ddjvu_document_create_by_filename(ctx, file, TRUE);
 	while (! ddjvu_document_decoding_done(doc));
 	MaxPages = ddjvu_document_get_pagenum(doc);	
 	thumbs.MakeEmpty();
	return true;
}

const char*
BDjVuFile::GetCurrentFile(void)
{
	return filename->String();
}

BBitmap*
BDjVuFile::Render(ddjvu_page_t *page,int flag_size,int flag_subsample,int flag_scale,int w,int h)
{

  ddjvu_rect_t info_size;
  
  info_size.x=0;
  info_size.y=0;
  info_size.w=w;
  info_size.h=h;
  
  ddjvu_rect_t prect;
  ddjvu_rect_t rrect;
  ddjvu_format_style_t style;
  ddjvu_render_mode_t mode;
  ddjvu_format_t *fmt;
  int iw = ddjvu_page_get_width(page);
  int ih = ddjvu_page_get_height(page);
  int dpi = ddjvu_page_get_resolution(page);
  ddjvu_page_type_t type = ddjvu_page_get_type(page);
  char *image = 0;
  int rowsize;
  
  prect.x = 0;
  prect.y = 0;
  if (flag_size > 0)
    {
      prect.w = info_size.w;
      prect.h = info_size.h;
    }
  else if (flag_subsample > 0)
    {
      prect.w = (iw + flag_subsample - 1) / flag_subsample;
      prect.h = (ih + flag_subsample - 1) / flag_subsample;
    }
  else if (flag_scale > 0)
    {
      prect.w = (unsigned int) (iw * flag_scale) / dpi;
      prect.h = (unsigned int) (ih * flag_scale) / dpi;
    }
  else 
    {
      prect.w = (iw * 100) / dpi;
      prect.h = (ih * 100) / dpi;
    }

  double dw = (double)iw / prect.w;
  double dh = (double)ih / prect.h;
    if (dw > dh) 
       prect.h = (int)(ih / dw);
    else
       prect.w = (int)(iw / dh);

  rrect = prect;

  mode = DDJVU_RENDER_COLOR;

  unsigned int m_formatmask[3];
  m_formatmask[0] = 0x00ff0000;
  m_formatmask[1] = 0x0000ff00;
  m_formatmask[2] = 0x000000ff;

  style = DDJVU_FORMAT_RGBMASK32;

  fmt = ddjvu_format_create(style, 3, m_formatmask);
  ddjvu_format_set_row_order(fmt, 1);
  
  rowsize = rrect.w*4; 

  //image = (char*)malloc(rowsize * rrect.h);
  BBitmap *bmp=new BBitmap(BRect(0,0,rrect.w-1,rrect.h-1),B_RGB32);
  
  ddjvu_page_render(page, mode, &prect, &rrect, fmt, rowsize, (char*)bmp->Bits());
  
  //bmp->SetBits(image, rowsize * rrect.h,0,B_RGB32);

  ddjvu_format_release(fmt);
  //free(image);
  return bmp;
}


BBitmap*
BDjVuFile::GetThumb(int pageno)
{
    if(acquire_sem(render_sem)!=B_NO_ERROR)return NULL; 
	ddjvu_page_t *page;
	page = ddjvu_page_create_by_pageno(doc, pageno);
	int i=0;
	while(!ddjvu_page_decoding_done(page))
	{
	 i++;
	 if(i>300)
	  {    
	   release_sem(render_sem);
	   return NULL;
	  }
	 snooze(10000);
	}
	BBitmap *bmp=Render(page,1,-1,-1,100,100);
  	ddjvu_page_release(page);
    release_sem(render_sem);
	return bmp;
}


BBitmap*
BDjVuFile::GetPage(int pageno)
{
    if(acquire_sem(render_sem)!=B_NO_ERROR)return NULL; 
	ddjvu_page_t *page;
	page = ddjvu_page_create_by_pageno(doc, pageno);
	int i=0;
	while(!ddjvu_page_decoding_done(page))
	{
	 i++;
	 if(i>300)
	  {    
	   release_sem(render_sem);
	   return NULL;
	  }
	 snooze(10000);
	}
	BBitmap *bmp=Render(page,-1,-1,150,0,0);
  	ddjvu_page_release(page);
    release_sem(render_sem);
	return bmp;
}

