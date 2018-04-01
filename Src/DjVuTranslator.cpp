#include <Application.h>
#include <Alert.h>
#include <StringView.h>
#include <TranslatorAddOn.h>
#include <TranslationKit.h>
#include <String.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>

#include <libdjvu/ddjvuapi.h>

#include "DjVuTranslator.h"

sem_id render_sem;

// !!! Set these five accordingly
#define NATIVE_TRANSLATOR_ACRONYM "DJVU"
#define NATIVE_TRANSLATOR_FORMAT 'DJVU'
#define NATIVE_TRANSLATOR_MIME_STRING "image/x-djvu"
#define NATIVE_TRANSLATOR_DESCRIPTION "DjVu document"
#define copyright_string "© 3dEyes**"

// The translation kit's native file type
#define B_TRANSLATOR_BITMAP_MIME_STRING "image/x-be-bitmap"
#define B_TRANSLATOR_BITMAP_DESCRIPTION "Be Bitmap image"

// Translation Kit required globals
char translatorName[32];
char translatorInfo[100];
int32 translatorVersion = 0x101;

// A couple other useful variables
char native_translator_file_name[32];
char native_translator_window_title[32];

static int debug = 0;

status_t CopyInToOut(BPositionIO *in, BPositionIO *out);
status_t NativeBitmapToTranslatorBitmap(BPositionIO *in, BPositionIO *out, BMessage *ioExtension);

// Initialize the above
class InitTranslator {
	public:
		InitTranslator() {
			sprintf(translatorName, "%s Documents", NATIVE_TRANSLATOR_ACRONYM);
			sprintf(translatorInfo, "%s image translator v%d.%d.%d, %s", NATIVE_TRANSLATOR_ACRONYM,
				(int)(translatorVersion >> 8), (int)((translatorVersion >> 4) & 0xf),
				(int)(translatorVersion & 0xf), __DATE__);
			sprintf(native_translator_file_name, "%sTranslator", NATIVE_TRANSLATOR_ACRONYM);
			sprintf(native_translator_window_title, "%s Settings", NATIVE_TRANSLATOR_ACRONYM);
		}
};
	
static InitTranslator it;

// Define the formats we know how to read
translation_format inputFormats[] = {
	{ NATIVE_TRANSLATOR_FORMAT, B_TRANSLATOR_BITMAP, 0.5, 0.5,
		NATIVE_TRANSLATOR_MIME_STRING, NATIVE_TRANSLATOR_DESCRIPTION },
	{ 0, 0, 0, 0, 0, 0 }
};

// Define the formats we know how to write
translation_format outputFormats[] = {
	{ B_TRANSLATOR_BITMAP, B_TRANSLATOR_BITMAP, 0.5, 0.5,
		B_TRANSLATOR_BITMAP_MIME_STRING, B_TRANSLATOR_BITMAP_DESCRIPTION },
	{ 0, 0, 0, 0, 0, 0 }
};

// Try to add a configuration view, if it doesn't exist display a message and exit
TranslatorWindow::TranslatorWindow(BRect rect, const char *name) :
	BWindow(rect, name, B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE) {

	BRect extent(0, 0, 239, 239);
	BView *config = NULL;
	status_t err = MakeConfig(NULL, &config, &extent);
	if ((err < B_OK) || (config == NULL)) {
		char error_message[255];
		sprintf(error_message, "%s does not currently allow user configuration.", native_translator_file_name);
		BAlert *alert = new BAlert("No Config", error_message, "Quit");
		alert->Go();
		exit(1);
	}
	
	ResizeTo(extent.Width(), extent.Height());
	AddChild(config);
}

// We're the only window so quit the app too
bool TranslatorWindow::QuitRequested() {
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}

// Build the configuration view, making it font sensitive
TranslatorView::TranslatorView(BRect rect, const char *name) :
	BView(rect, name, B_FOLLOW_ALL, B_WILL_DRAW) {

	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	font_height fh;
	be_bold_font->GetHeight(&fh);
	BRect r(10, 15, 10 + be_bold_font->StringWidth(translatorName), 15 + fh.ascent + fh.descent);
	
	BStringView *title = new BStringView(r, "Title", translatorName);
	title->SetFont(be_bold_font);
	AddChild(title);
	
	char version_string[100];
	sprintf(version_string, "v%d.%d.%d %s", (int)(translatorVersion >> 8), (int)((translatorVersion >> 4) & 0xf),
		(int)(translatorVersion & 0xf), __DATE__);
	r.top = r.bottom + 20;
	be_plain_font->GetHeight(&fh);
	r.bottom = r.top + fh.ascent + fh.descent;
	r.right = r.left + be_plain_font->StringWidth(version_string);
	
	BStringView *version = new BStringView(r, "Version", version_string);
	version->SetFont(be_plain_font);
	AddChild(version);
	
	r.top = r.bottom + 10;
	r.bottom = r.top + fh.ascent + fh.descent;
	r.right = r.left + be_plain_font->StringWidth(copyright_string);
	
	BStringView *copyright = new BStringView(r, "Copyright", copyright_string);
	copyright->SetFont(be_plain_font);
	AddChild(copyright);

	r.top = r.bottom + 10;
	r.bottom = r.top + fh.ascent + fh.descent;
	r.right = r.left + Bounds().right;
	
	BStringView *libre = new BStringView(r, "BasedOn", "Based on djvulibre library");
	libre->SetFont(be_plain_font);
	AddChild(libre);
}

// Application entry point
int main() {
	char app_signature[255];
	sprintf(app_signature, "application/x-vnd.Haiku-%s", native_translator_file_name);
	BApplication app(app_signature);
	
	BRect window_rect(100, 100, 339, 339);
	TranslatorWindow *window = new TranslatorWindow(window_rect, native_translator_window_title);
	window->Show();
	
	app.Run();
	return 0;
}

// Determine whether or not we can handle this data
status_t Identify(BPositionIO *inSource, const translation_format *inFormat, BMessage *ioExtension,
	translator_info *outInfo, uint32 outType) {

	if ((outType != 0) && (outType != B_TRANSLATOR_BITMAP) && (outType != NATIVE_TRANSLATOR_FORMAT)) {
		if (debug) printf("Identify(): outType %x is unknown\n", (int)outType);
		return B_NO_TRANSLATOR;
	}
	
	// !!! You might need to make this buffer bigger to test for your native format
	char header[sizeof(TranslatorBitmap)];
	status_t err = inSource->Read(header, sizeof(TranslatorBitmap));
	if (err < B_OK) return err;
	
//	if ( strncmp( ((DjVuHeader*)header)->id,"AT&TFORM",8)==0 && strncmp(((DjVuHeader*)header)->id2,"DJVM",4)==0 ) {
	if ( strncmp( ((DjVuHeader*)header)->id,"AT&TFORM",8)==0 ) {
		outInfo->type = inputFormats[0].type;
		outInfo->translator = 0;
		outInfo->group = inputFormats[0].group;
		outInfo->quality = inputFormats[0].quality;
		outInfo->capability = inputFormats[0].capability;
		strcpy(outInfo->name, inputFormats[0].name);
		strcpy(outInfo->MIME, inputFormats[0].MIME);
		
		inSource->Seek(0,SEEK_SET);
		
		int32 pages = 1;

		ddjvu_context_t *context=NULL;
		ddjvu_document_t *document=NULL;
		ddjvu_message_t *message;
	
		if(!(context = ddjvu_context_create("DjVu Translator")))
			return B_ERROR;

		ddjvu_cache_set_size(context,1);
		document = ddjvu_document_create(context, "file://", 0);
	
		message = ddjvu_message_wait(context);

		if (message->m_any.tag != DDJVU_NEWSTREAM)
                return B_ERROR;    /* error! */
        else  {
                int streamid = message->m_newstream.streamid;
                char buffer[512];
                int i;
                do {
                        i = inSource->Read(buffer, 512);
                        ddjvu_stream_write(document,
                                           streamid,
                                           buffer,
                                           i);
                } while (i == 512);
                ddjvu_stream_close(document, streamid, 0);
        }
        ddjvu_message_pop(context);

        message = ddjvu_message_wait(context);
        switch (message->m_any.tag){
        case DDJVU_DOCINFO: {
        		pages = ddjvu_document_get_pagenum(document);
                if (pages < 1) /* != */
                        return B_NO_TRANSLATOR;    /* error (unexpected) */
                break; 
        }
        default:
                return B_NO_TRANSLATOR;    /* error (unexpected) */
                
        };
        
        
	 	ddjvu_document_release(document);
 		ddjvu_cache_clear(context);
 		ddjvu_context_release(context); 

		int32 pageIndex = 1;
 		
 		if(ioExtension != NULL) {
	 		ioExtension->RemoveName("/documentCount");
 			ioExtension->AddInt32("/documentCount", pages);
 		
        	if(ioExtension->FindInt32("/documentIndex", &pageIndex) != B_OK)
        		pageIndex = 1;
		}

		BString docName(inputFormats[0].name);
		docName << " " << pageIndex << "/" << pages;

		snprintf(outInfo->name, sizeof(outInfo->name), docName.String());
 			    		
		return B_OK;
	}
	return B_NO_TRANSLATOR;
}

// Arguably the most important method in the add-on
status_t Translate(BPositionIO *inSource, const translator_info *inInfo, BMessage *ioExtension,
	uint32 outType, BPositionIO *outDestination) {

	// If no specific type was requested, convert to the interchange format
	if (outType == 0) outType = B_TRANSLATOR_BITMAP;
	
	if (inInfo->type == NATIVE_TRANSLATOR_FORMAT && outType == B_TRANSLATOR_BITMAP) {
		return NativeBitmapToTranslatorBitmap(inSource, outDestination, ioExtension);
	}

	return B_NO_TRANSLATOR;
}

// Hook to create and return our configuration view
status_t MakeConfig(BMessage *ioExtension, BView **outView, BRect *outExtent) {
	outExtent->Set(0, 0, 239, 239);
	*outView = new TranslatorView(*outExtent, "TranslatorView");
	return B_OK;
}

// Decode the native format
status_t NativeBitmapToTranslatorBitmap(BPositionIO *in, BPositionIO *out, BMessage *ioExtension)
{
	int i=0;
	ddjvu_context_t *context=NULL;
	ddjvu_document_t *document=NULL;
	ddjvu_message_t *message;
	
	if(!(context = ddjvu_context_create("DjVu Translator")))
		return B_ERROR;

	ddjvu_cache_set_size(context,1);
	document = ddjvu_document_create(context, "file://", 0);
	
	message = ddjvu_message_wait(context);

	if (message->m_any.tag != DDJVU_NEWSTREAM)
                /* ddjvu_message_pop(context); */
                return B_ERROR;    /* error! */
        else  {
                int streamid = message->m_newstream.streamid;
                char buffer[512];
                int i;
                do {
                        i = in->Read(buffer, 512);
                        ddjvu_stream_write(document,
                                           streamid,
                                           buffer,
                                           i);
                } while (i == 512);
                ddjvu_stream_close(document, streamid, 0);
        }
        ddjvu_message_pop(context);

        message = ddjvu_message_wait(context);
        switch (message->m_any.tag){
        case DDJVU_DOCINFO: {
        		int pages = ddjvu_document_get_pagenum(document);
                printf("Pages: %d\n",pages);

                if (pages < 1) /* != */
                        return B_ERROR;    /* error (unexpected) */
                break; 
        }
        default:
                return B_ERROR;    /* error (unexpected) */
                
        };
        ddjvu_message_pop(context);
        
        int32 pageIndex = 1;
        if(ioExtension != NULL)
        	if(ioExtension->FindInt32("/documentIndex", &pageIndex) != B_OK)
        		pageIndex =1;
        
        ddjvu_page_t *page = ddjvu_page_create_by_pageno(document, pageIndex - 1);

        message = ddjvu_message_wait(context);
        ddjvu_message_pop(context);	
	
	while(!ddjvu_page_decoding_done(page)){i++;if(i>20){return B_ERROR; } snooze(100000); }
	
	
		
	ddjvu_rect_t info_size;
  
  	info_size.x=0;
  	info_size.y=0;
  	info_size.w=0;
  	info_size.h=0;
  
  	ddjvu_rect_t prect;
  	ddjvu_rect_t rrect;
  	ddjvu_format_t *fmt;
  	int iw = ddjvu_page_get_width(page);
  	int ih = ddjvu_page_get_height(page);
  	int dpi = ddjvu_page_get_resolution(page);
  	ddjvu_page_type_t type = ddjvu_page_get_type(page);
  	char *image = 0;
  	int rowsize;
  
	prect.w = (unsigned int) (iw * 100) / dpi;
    prect.h = (unsigned int) (ih * 100) / dpi;

  	double dw = (double)iw / prect.w;
  	double dh = (double)ih / prect.h;
    
    if(dw>dh)prect.h=(int)(ih/dw);
    else prect.w=(int)(iw/dh);

  	rrect = prect;

    unsigned int *m_formatmask = new unsigned int[4];
    m_formatmask[0] = 0x00ff0000;
    m_formatmask[1] = 0x0000ff00;
    m_formatmask[2] = 0x000000ff;
    m_formatmask[3] = 0xff000000;  	

	fmt = ddjvu_format_create(DDJVU_FORMAT_RGBMASK32, 4, m_formatmask);
  	ddjvu_format_set_row_order(fmt, 1);
  	
   	rowsize = rrect.w*4; 
  	image = (char*)malloc(rowsize * rrect.h);
  	if (image == NULL) return B_NO_MEMORY;

  	ddjvu_page_render(page, DDJVU_RENDER_COLOR, &prect, &rrect, fmt, rowsize, image);
  	ddjvu_format_release(fmt);	
  	ddjvu_page_release(page);
  	
  	//----------------------------------------------------------------------------------
  	BRect bounds=BRect(1,1,rrect.w,rrect.h);
	
	int width = bounds.IntegerWidth() + 1;
	int height = bounds.IntegerHeight() + 1;
	int row_bytes = (bounds.IntegerWidth() + 1) * 4;
	
	// Fill out the B_TRANSLATOR_BITMAP's header
	TranslatorBitmap header;
	header.magic = B_HOST_TO_BENDIAN_INT32(B_TRANSLATOR_BITMAP);
	header.bounds.left = B_HOST_TO_BENDIAN_FLOAT(bounds.left);
	header.bounds.top = B_HOST_TO_BENDIAN_FLOAT(bounds.top);
	header.bounds.right = B_HOST_TO_BENDIAN_FLOAT(bounds.right);
	header.bounds.bottom = B_HOST_TO_BENDIAN_FLOAT(bounds.bottom);
	header.colors = (color_space)B_HOST_TO_BENDIAN_INT32(B_RGBA32);
	header.rowBytes = B_HOST_TO_BENDIAN_INT32(row_bytes);
	header.dataSize = B_HOST_TO_BENDIAN_INT32(row_bytes * (bounds.IntegerHeight() + 1));
	
	// Write out the header
	status_t err = out->Write(&header, sizeof(TranslatorBitmap));
	if (err < B_OK) return err;
	else if (err < (int)sizeof(TranslatorBitmap)) return B_ERROR;
	
	//-----------------------------------------------------------------------------------
    
	if(image!=NULL)
	{
	 out->Write(image, header.dataSize);
	 free(image);
	}

 	ddjvu_document_release(document);
 	ddjvu_cache_clear(context);
 	ddjvu_context_release(context);

	return B_OK;
}
