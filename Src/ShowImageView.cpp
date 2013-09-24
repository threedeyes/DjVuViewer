/*****************************************************************************/
// ShowImageView
// Written by Fernando Francisco de Oliveira, Michael Wilber, Michael Pfeiffer
//
// ShowImageView.cpp
//
//
// Copyright (c) 2003 OpenBeOS Project
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the 
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included 
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
/*****************************************************************************/

#include <stdio.h>
#include <math.h>
#include <Debug.h>
#include <Message.h>
#include <ScrollBar.h>
#include <StopWatch.h>
#include <Alert.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <File.h>
#include <Bitmap.h>
#include <TranslatorRoster.h>
#include <BitmapStream.h>
#include <Rect.h>
#include <SupportDefs.h>
#include <Directory.h>
#include <Entry.h>
#include <Application.h>
#include <Roster.h>
#include <NodeInfo.h>
#include <Clipboard.h>
#include <Path.h>
#include <PopUpMenu.h>
#include <Region.h>
#include <Screen.h>
#include <TranslationUtils.h>
#include <FilePanel.h>
#include <TranslationDefs.h>

#include "MainApp.h"
#include "LoadDjVu.h"
#include "Settings.h"


#include "ShowImageView.h"
//#include "ShowImageConstants.h"
//#include "ShowImageSettings.h"

#ifndef min
#define min(a,b) ((a)>(b)?(b):(a))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

#define SHOW_IMAGE_ORIENTATION_ATTRIBUTE "ShowImage:orientation"
#define BORDER_WIDTH 16
#define BORDER_HEIGHT 16
#define PEN_SIZE 1.0f
const rgb_color kBorderColor = { 0, 0, 0, 255 };

enum ShowImageView::image_orientation 
ShowImageView::fTransformation[ImageProcessor::kNumberOfAffineTransformations][kNumberOfOrientations] =
{
	// rotate 90°
	{k90, k180, k270, k0, k270V, k0V, k90V, k0H},
	// rotate -90°
	{k270, k0, k90, k180, k90V, k0H, k270V, k0V},
	// mirror vertical
	{k0H, k270V, k0V, k90V, k180, k270, k0, k90},
	// mirror horizontal
	{k0V, k90V, k0H, k270V, k0, k90, k180, k270}
};


// use patterns to simulate marching ants for selection
void 
ShowImageView::InitPatterns()
{
	uchar p;
	uchar p1 = 0x33;
	uchar p2 = 0xCC;
	for (int i = 0; i <= 7; i ++) {
		fPatternLeft.data[i] = p1;
		fPatternRight.data[i] = p2;
		if ((i / 2) % 2 == 0) {
			p = 255;
		} else {
			p = 0;
		}
		fPatternUp.data[i] = p;
		fPatternDown.data[i] = ~p;
	}
}

void
ShowImageView::RotatePatterns()
{
	int i;
	uchar p;
	bool set;
	
	// rotate up
	p = fPatternUp.data[0];
	for (i = 0; i <= 6; i ++) {
		fPatternUp.data[i] = fPatternUp.data[i+1];	
	}
	fPatternUp.data[7] = p;
	
	// rotate down
	p = fPatternDown.data[7];
	for (i = 7; i >= 1; i --) {
		fPatternDown.data[i] = fPatternDown.data[i-1];
	}
	fPatternDown.data[0] = p;
	
	// rotate to left
	p = fPatternLeft.data[0];
	set = (p & 0x80) != 0;
	p <<= 1;
	p &= 0xfe;
	if (set) p |= 1;
	memset(fPatternLeft.data, p, 8);
	
	// rotate to right
	p = fPatternRight.data[0];
	set = (p & 1) != 0;
	p >>= 1;
	if (set) p |= 0x80;
	memset(fPatternRight.data, p, 8);
}

void
ShowImageView::AnimateSelection(bool a)
{
	fAnimateSelection = a;
}

void
ShowImageView::Pulse()
{
	// animate marching ants
	if (HasSelection() && fAnimateSelection && Window()->IsActive()) {	
		RotatePatterns();
		DrawSelectionBox();
	}
}

ShowImageView::ShowImageView(BRect rect, const char *name, uint32 resizingMode,
	uint32 flags)
	: BView(rect, name, resizingMode, flags)
{
	InitPatterns();
	fDither = false;
	fBitmap = NULL;
	fSelBitmap = NULL;
	fDocumentIndex = 1;
	fDocumentCount = 1;
	fAnimateSelection = true;
	fHasSelection = false;
	fShrinkToBounds = false;
	fZoomToBounds = false;
	fHasBorder = true;
	fHAlignment = B_ALIGN_CENTER;
	fVAlignment = B_ALIGN_TOP;
	fZoom = 1.0;
	fMovesImage = false;
	fScaleBilinear = false;
	fScaler = NULL;

	fDither = SETTINGS->GetBool("Dither");
	fShrinkToBounds = SETTINGS->GetBool("ShrinkToBounds");
	fZoomToBounds = SETTINGS->GetBool("ZoomToBounds");
	fScaleBilinear = SETTINGS->GetBool("ScaleBilinear");
	
	SetViewColor(B_TRANSPARENT_COLOR);
	SetHighColor(kBorderColor);
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetPenSize(PEN_SIZE);
	
	pmenuSaveAs = new BMenu(MES("MM:SavePageAs"), B_ITEMS_IN_COLUMN);
	BTranslationUtils::AddTranslationItems(pmenuSaveAs, B_TRANSLATOR_BITMAP);
}

ShowImageView::~ShowImageView()
{
	DeleteBitmap();
}

// returns B_ERROR if problems reading ref
// B_OK if ref is not a directory
// B_OK + 1 if ref is a directory
status_t
ent_is_dir(const entry_ref *ref)
{
	BEntry ent(ref);
	if (ent.InitCheck() != B_OK)
		return B_ERROR;
		
	struct stat st;
	if (ent.GetStat(&st) != B_OK)
		return B_ERROR;
	
	return S_ISDIR(st.st_mode) ? (B_OK + 1) : B_OK;
}

bool
ShowImageView::IsImage(const entry_ref *pref)
{
	if (!pref)
		return false;
		
	if (ent_is_dir(pref) != B_OK)
		// if ref is erroneous or a directory, return false
		return false;

	BFile file(pref, B_READ_ONLY);
	if (file.InitCheck() != B_OK)
		return false;

	BTranslatorRoster *proster = BTranslatorRoster::Default();
	if (!proster)
		return false;
		
	BMessage ioExtension;
	if (ioExtension.AddInt32("/documentIndex", fDocumentIndex) != B_OK)
		return false;

	translator_info info;
	memset(&info, 0, sizeof(translator_info));
	if (proster->Identify(&file, &ioExtension, &info, 0, NULL,
		B_TRANSLATOR_BITMAP) != B_OK)
		return false;
		
	return true;
}

void
ShowImageView::SetTrackerMessenger(const BMessenger& trackerMessenger)
{
	fTrackerMessenger = trackerMessenger;
}

// send message to parent about new image
void 
ShowImageView::Notify(const char* status)
{
	BMessage msg(MSG_UPDATE_STATUS);
	if (status != NULL) {
		msg.AddString("status", status);
	}
	msg.AddInt32("colors", fBitmap->ColorSpace());
	BMessenger msgr(Window());
	msgr.SendMessage(&msg);

	FixupScrollBars();
	Invalidate();
}

void
ShowImageView::AddToRecentDocuments()
{
	be_roster->AddToRecentDocuments(&fCurrentRef, APPSIGMATURE);
}

void
ShowImageView::DeleteScaler()
{
	if (fScaler) {
		fScaler->Stop();
		delete fScaler;
		fScaler = NULL;
	}
}

void
ShowImageView::DeleteBitmap()
{
	DeleteScaler();
	DeleteSelBitmap();
	delete fBitmap;
	fBitmap = NULL;
}

void ShowImageView::DeleteSelBitmap()
{
	delete fSelBitmap;
	fSelBitmap = NULL;
}

status_t
ShowImageView::SetBitmap(BBitmap *bmp)
{
	if(bmp==NULL)return B_ERROR;
	BBitmap *newBitmap = bmp;
	
		
	// Now that I've successfully loaded the new bitmap,
	// I can be sure it is safe to delete the old one, 
	// and clear everything
	SetHasSelection(false);
	fMakesSelection = false;
	DeleteBitmap();
	fBitmap = newBitmap;
	newBitmap = NULL;
	
	fDocumentCount = 1;
	FixupScrollBars();
	Invalidate();

	return B_OK;
}


status_t
ShowImageView::SetSelection(const entry_ref *pref, BPoint point)
{
	BTranslatorRoster *proster = BTranslatorRoster::Default();
	if (!proster)
		return B_ERROR;
	BFile file(pref, B_READ_ONLY);
	translator_info info;
	memset(&info, 0, sizeof(translator_info));
	if (proster->Identify(&file, NULL, &info, 0, NULL,
		B_TRANSLATOR_BITMAP) != B_OK)
		return B_ERROR;
	
	// Translate image data and create a new ShowImage window
	BBitmapStream outstream;
	if (proster->Translate(&file, &info, NULL, &outstream,
		B_TRANSLATOR_BITMAP) != B_OK)
		return B_ERROR;
	BBitmap *newBitmap = NULL;
	if (outstream.DetachBitmap(&newBitmap) != B_OK)
		return B_ERROR;
		
	return true;
}

void
ShowImageView::SetDither(bool dither)
{
	if (fDither != dither) {
		SETTINGS->SetBool("Dither", dither);
		fDither = dither;
		Invalidate();
	}
}

void
ShowImageView::SetShrinkToBounds(bool enable)
{
	if (fShrinkToBounds != enable) {
		SETTINGS->SetBool("ShrinkToBounds", enable);
		fShrinkToBounds = enable;
		FixupScrollBars();
		Invalidate();
	}
}

void
ShowImageView::SetZoomToBounds(bool enable)
{
	if (fZoomToBounds != enable) {
		SETTINGS->SetBool("ZoomToBounds", enable);
		fZoomToBounds = enable;
		FixupScrollBars();
		Invalidate();
	}
}

void
ShowImageView::SetBorder(bool hasBorder)
{
	if (fHasBorder != hasBorder) {
		fHasBorder = hasBorder;
		FixupScrollBars();
		Invalidate();
	}
}

void 
ShowImageView::SetAlignment(alignment horizontal, vertical_alignment vertical)
{
	bool hasChanged;
	hasChanged = fHAlignment != horizontal || fVAlignment != vertical;
	if (hasChanged) {
		fHAlignment = horizontal;
		fVAlignment = vertical;
		FixupScrollBars();
		Invalidate();
	}
}

BBitmap *
ShowImageView::GetBitmap()
{
	return fBitmap;
}

void
ShowImageView::GetName(BString *name)
{
	*name = "";
	BEntry entry(&fCurrentRef);
	if (entry.InitCheck() == B_OK) {
		char n[B_FILE_NAME_LENGTH];
		if (entry.GetName(n) == B_OK) {
			name->SetTo(n);
		}
	}		
}

void
ShowImageView::GetPath(BString *name)
{
	*name = "";
	BEntry entry(&fCurrentRef);
	if (entry.InitCheck() == B_OK) {
		BPath path;
		entry.GetPath(&path);
		if (path.InitCheck() == B_OK) {
			name->SetTo(path.Path());
		}
	}		
}

void
ShowImageView::FlushToLeftTop()
{
	BRect rect = AlignBitmap();
	BPoint p(rect.left-BORDER_WIDTH, (rect.top-BORDER_WIDTH));
	ScrollTo(p);
}

void
ShowImageView::FlushToLeftBottom()
{
	BRect rect = AlignBitmap();
	BPoint p(rect.left-BORDER_WIDTH, (rect.bottom-BORDER_WIDTH));
	ScrollTo(p);
}

void
ShowImageView::SetScaleBilinear(bool s)
{
	if (fScaleBilinear != s) {
		SETTINGS->SetBool("ScaleBilinear", s);
		fScaleBilinear = s; Invalidate();
	}
}

void
ShowImageView::AttachedToWindow()
{
	FixupScrollBars();
}

BRect
ShowImageView::AlignBitmap()
{
	if(fBitmap==NULL)return BRect(0,0,0,0);
	BRect rect(fBitmap->Bounds());
	float width, height;
	width = Bounds().Width()-2*PEN_SIZE+1;
	height = Bounds().Height()-2*PEN_SIZE+1;
	if (width == 0 || height == 0) return rect;
	fShrinkOrZoomToBounds = fShrinkToBounds && 
		(rect.Width() >= Bounds().Width() || rect.Height() >= Bounds().Height()) ||
		fZoomToBounds && rect.Width() < Bounds().Width() && rect.Height() < Bounds().Height();
	if (fShrinkOrZoomToBounds) {
		float s;
		s = width / (rect.Width()+1.0);
			
		if (s * (rect.Height()+1.0) <= height) {
			rect.right = width-1;
			rect.bottom = static_cast<int>(s * (rect.Height()+1.0))-1;
			// center vertically
			rect.OffsetBy(0, static_cast<int>((height - rect.Height()) / 2));
		} else {
			s = height / (rect.Height()+1.0);
			rect.right = static_cast<int>(s * (rect.Width()+1.0))-1;
			rect.bottom = height-1;
			// center horizontally
			rect.OffsetBy(static_cast<int>((width - rect.Width()) / 2), 0);
		}
	} else {
		float zoom;
		if (fShrinkToBounds || fZoomToBounds) {
			// ignore user zoom setting in automatic zoom modes
			zoom = 1.0;
		} else {
			zoom = fZoom;
		}
		// zoom image
		rect.right = static_cast<int>((rect.right+1.0)*zoom)-1; 
		rect.bottom = static_cast<int>((rect.bottom+1.0)*zoom)-1;
		// align
		switch (fHAlignment) {
			case B_ALIGN_CENTER:
				if (width > rect.Width()) {
					rect.OffsetBy((width - rect.Width()) / 2.0, 0);
					break;
				}
				// fall through
			default:
			case B_ALIGN_LEFT:
				if (fHasBorder) {
					rect.OffsetBy(BORDER_WIDTH, 0);
				}
				break;
		}
		switch (fVAlignment) {
			case B_ALIGN_MIDDLE:
				if (height > rect.Height()) {
					rect.OffsetBy(0, (height - rect.Height()) / 2.0);
				break;
				}
				// fall through
			default:
			case B_ALIGN_TOP:
				if (fHasBorder) {
					rect.OffsetBy(0, BORDER_WIDTH);
				}
				break;
		}
	}
	rect.OffsetBy(PEN_SIZE, PEN_SIZE);
	return rect;
}

void
ShowImageView::Setup(BRect rect)
{
	fLeft = floorf(rect.left);
	fTop = floorf(rect.top);
	fScaleX = (rect.Width()+1.0) / (fBitmap->Bounds().Width()+1.0);
	fScaleY = (rect.Height()+1.0) / (fBitmap->Bounds().Height()+1.0);
}

BPoint
ShowImageView::ImageToView(BPoint p) const
{
	p.x = floorf(fScaleX * p.x + fLeft);
	p.y = floorf(fScaleY * p.y + fTop);
	return p;
}

BPoint
ShowImageView::ViewToImage(BPoint p) const
{
	p.x = floorf((p.x - fLeft) / fScaleX);
	p.y = floorf((p.y - fTop) / fScaleY);
	return p;
}

BRect
ShowImageView::ImageToView(BRect r) const
{
	BPoint leftTop(ImageToView(BPoint(r.left, r.top)));
	BPoint rightBottom(r.right, r.bottom);
	rightBottom += BPoint(1, 1);
	rightBottom = ImageToView(rightBottom);
	rightBottom -= BPoint(1, 1);
	return BRect(leftTop.x, leftTop.y, rightBottom.x, rightBottom.y);
}

void
ShowImageView::DrawBorder(BRect border)
{
	BRect bounds(Bounds());
	// top
	FillRect(BRect(0, 0, bounds.right, border.top-1), B_SOLID_LOW);
	// left
	FillRect(BRect(0, border.top, border.left-1, border.bottom), B_SOLID_LOW);
	// right
	FillRect(BRect(border.right+1, border.top, bounds.right, border.bottom), B_SOLID_LOW);
	// bottom
	FillRect(BRect(0, border.bottom+1, bounds.right, bounds.bottom), B_SOLID_LOW);
}


Scaler*
ShowImageView::GetScaler(BRect rect)
{
	if (fScaler == NULL || !fScaler->Matches(rect, fDither)) {
		DeleteScaler();
		BMessenger msgr(this, Window());
		fScaler = new Scaler(fBitmap, rect, msgr, MSG_INVALIDATE, fDither);
		fScaler->Start();
	}
	return fScaler;
}

void
ShowImageView::DrawImage(BRect rect)
{
	if (fScaleBilinear || fDither) {
	Scaler* scaler = GetScaler(rect);
		if (scaler != NULL && !scaler->IsRunning()) {
			BBitmap* bitmap = scaler->GetBitmap();
			if (bitmap) {
				DrawBitmap(bitmap, BPoint(rect.left, rect.top));
				return;
			}
		}
	}
	DrawBitmap(fBitmap, fBitmap->Bounds(), rect);
}

void
ShowImageView::Draw(BRect updateRect)
{
	if (fBitmap) {
		if (!IsPrinting()) {
			BRect rect = AlignBitmap();
			Setup(rect);
			
			BRect border(rect);
			border.InsetBy(-PEN_SIZE, -PEN_SIZE);

			// Draw image
			DrawImage(rect);	
	
			DrawBorder(border);
	
			// Draw black rectangle around image
			StrokeRect(border);
									
			if (HasSelection()) {
				if (fSelBitmap) {
					BRect srcBits, destRect;
					GetSelMergeRects(srcBits, destRect);
					destRect = ImageToView(destRect);
					//DrawBitmap(fSelBitmap, srcBits, destRect);
				}
				DrawSelectionBox();
			}
		} else {
			DrawBitmap(fBitmap);
		}
	}
	 else
	 {
	  SetHighColor(255,255,255);
	  FillRect(Bounds());
	 }
}

void
ShowImageView::DrawSelectionBox()
{
	BRect r(fSelectionRect);
	ConstrainToImage(r);
	r = ImageToView(r);
	// draw selection box *around* selection
	r.InsetBy(-1, -1);
	PushState();
	rgb_color white = {255, 255, 255};
	SetLowColor(white);
	StrokeLine(BPoint(r.left, r.top), BPoint(r.right, r.top), fPatternLeft);
	StrokeLine(BPoint(r.right, r.top+1), BPoint(r.right, r.bottom-1), fPatternUp);
	StrokeLine(BPoint(r.left, r.bottom), BPoint(r.right, r.bottom), fPatternRight);
	StrokeLine(BPoint(r.left, r.top+1), BPoint(r.left, r.bottom-1), fPatternDown);
	PopState();
}

void
ShowImageView::FrameResized(float /* width */, float /* height */)
{
	FixupScrollBars();
}

void
ShowImageView::ConstrainToImage(BPoint &point)
{
   if(fBitmap!=NULL)point.ConstrainTo(fBitmap->Bounds());
}

void
ShowImageView::ConstrainToImage(BRect &rect)
{
	BRect bounds = fBitmap->Bounds();
	BPoint leftTop, rightBottom;
	
	leftTop = rect.LeftTop();
	leftTop.ConstrainTo(bounds);
	
	rightBottom = rect.RightBottom();
	rightBottom.ConstrainTo(bounds);

	rect.SetLeftTop(leftTop);
	rect.SetRightBottom(rightBottom);
}

BBitmap*
ShowImageView::CopyFromRect(BRect srcRect)
{	
	BRect rect(0, 0, srcRect.Width(), srcRect.Height());
	BView view(rect, NULL, B_FOLLOW_NONE, B_WILL_DRAW);
	BBitmap *bitmap = new BBitmap(rect, fBitmap->ColorSpace(), true);
	if (bitmap == NULL) return NULL;
	
	if (bitmap->Lock()) {
		bitmap->AddChild(&view);
		view.DrawBitmap(fBitmap, srcRect, rect);
		view.Sync();
		bitmap->RemoveChild(&view);
		bitmap->Unlock();
	}
	
	return bitmap;
}

BBitmap*
ShowImageView::CopySelection(uchar alpha, bool imageSize)
{
	bool hasAlpha = alpha != 255;
	
	if (!HasSelection()) return NULL;
	
	BRect rect(0, 0, fSelectionRect.Width(), fSelectionRect.Height());
	if (!imageSize) {
		// scale image to view size
		rect.right = floorf((rect.right + 1.0) * fScaleX - 1.0); 
		rect.bottom = floorf((rect.bottom + 1.0) * fScaleY - 1.0);
	}
	BView view(rect, NULL, B_FOLLOW_NONE, B_WILL_DRAW);
	BBitmap *bitmap = new BBitmap(rect, hasAlpha ? B_RGBA32 : fBitmap->ColorSpace(), true);
	if (bitmap == NULL) return NULL;
	
	if (bitmap->Lock()) {
		bitmap->AddChild(&view);
		if (fSelBitmap)
			view.DrawBitmap(fSelBitmap, fSelBitmap->Bounds(), rect);
		else
			view.DrawBitmap(fBitmap, fCopyFromRect, rect);
		if (hasAlpha) {
			view.SetDrawingMode(B_OP_SUBTRACT);
			view.SetHighColor(0, 0, 0, 255-alpha);
			view.FillRect(rect, B_SOLID_HIGH);
		}
		view.Sync();
		bitmap->RemoveChild(&view);
		bitmap->Unlock();
	}
	
	return bitmap;
}

bool
ShowImageView::AddSupportedTypes(BMessage* msg, BBitmap* bitmap)
{
	bool found = false;
	BTranslatorRoster *roster = BTranslatorRoster::Default();
	if (roster == NULL) return false;
	
	BBitmapStream stream(bitmap);
		
	translator_info *outInfo;
	int32 outNumInfo;		
	if (roster->GetTranslators(&stream, NULL, &outInfo, &outNumInfo) == B_OK) {
		for (int32 i = 0; i < outNumInfo; i++) {
			const translation_format *fmts;
			int32 num_fmts;
			roster->GetOutputFormats(outInfo[i].translator, &fmts, &num_fmts);
			for (int32 j = 0; j < num_fmts; j++) {
				if (strcmp(fmts[j].MIME, "image/x-be-bitmap") != 0) {
					// needed to send data in message
 					msg->AddString("be:types", fmts[j].MIME);
 					// needed to pass data via file
					msg->AddString("be:filetypes", fmts[j].MIME);
					msg->AddString("be:type_descriptions", fmts[j].name);
				}
				found = true;
			}
		}
	}
	stream.DetachBitmap(&bitmap);
	return found;
}

void
ShowImageView::BeginDrag(BPoint sourcePoint)
{
	BBitmap* bitmap = CopySelection(128, false);
	if (bitmap == NULL) return;

	SetMouseEventMask(B_POINTER_EVENTS);
	BPoint leftTop(fSelectionRect.left, fSelectionRect.top);

	// fill the drag message
	BMessage drag(B_SIMPLE_DATA);
	drag.AddInt32("be:actions", B_COPY_TARGET);
	drag.AddString("be:clip_name", "Bitmap Clip"); 
	// ShowImage specific fields
	drag.AddPoint("be:_source_point", sourcePoint);
	drag.AddRect("be:_frame", fSelectionRect);
	if (AddSupportedTypes(&drag, bitmap)) {
		// we also support "Passing Data via File" protocol
		drag.AddString("be:types", B_FILE_MIME_TYPE);
		// avoid flickering of dragged bitmap caused by drawing into the window
		AnimateSelection(false);
		sourcePoint -= leftTop;
		sourcePoint.x *= fScaleX;
		sourcePoint.y *= fScaleY;
		// DragMessage takes ownership of bitmap
		DragMessage(&drag, bitmap, B_OP_ALPHA, sourcePoint);
		bitmap = NULL;
	}
}

bool
ShowImageView::OutputFormatForType(BBitmap* bitmap, const char* type, translation_format* format)
{
	bool found = false;

	BTranslatorRoster *roster = BTranslatorRoster::Default();
	if (roster == NULL) return false;

	BBitmapStream stream(bitmap);
	
	translator_info *outInfo;
	int32 outNumInfo;
	if (roster->GetTranslators(&stream, NULL, &outInfo, &outNumInfo) == B_OK) {		
		for (int32 i = 0; i < outNumInfo; i++) {
			const translation_format *fmts;
			int32 num_fmts;
			roster->GetOutputFormats(outInfo[i].translator, &fmts, &num_fmts);
			for (int32 j = 0; j < num_fmts; j++) {
				if (strcmp(fmts[j].MIME, type) == 0) {
					*format = fmts[j];
					found = true; 
					break;
				}
			}
		}
	}
	stream.DetachBitmap(&bitmap);
	return found;
}

void
ShowImageView::SaveToFile(BDirectory* dir, const char* name, BBitmap* bitmap, const translation_format* format)
{
	if (!bitmap)
		// If no bitmap is supplied, write out the whole image
		bitmap = fBitmap;

	BBitmapStream stream(bitmap);
		
	bool loop = true;
	while (loop) {
		BTranslatorRoster *roster = BTranslatorRoster::Default();
		if (!roster)
			break;
		// write data
		BFile file(dir, name, B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
		if (file.InitCheck() != B_OK)
			break;
		if (roster->Translate(&stream, NULL, NULL, &file, format->type) < B_OK)
			break;
		// set mime type
		BNodeInfo info(&file);
		if (info.InitCheck() == B_OK)
			info.SetType(format->MIME);
			
		loop = false;
			// break out of loop gracefully (indicates no errors)
	}
	if (loop) {
		// If loop terminated because of a break, there was an error
		BString errText;
		errText << "Файл " << name << "' не может быть записан.";
		BAlert *palert = new BAlert(NULL, errText.String(), "Ok");
		palert->Go();
	}
	
	stream.DetachBitmap(&bitmap);
		// Don't allow the bitmap to be deleted, this is
		// especially important when using fBitmap as the bitmap
}

void
ShowImageView::SendInMessage(BMessage* msg, BBitmap* bitmap, translation_format* format)
{
	BMessage reply(B_MIME_DATA);
	BBitmapStream stream(bitmap); // destructor deletes bitmap
	BTranslatorRoster *roster = BTranslatorRoster::Default();
	BMallocIO memStream;
	if (roster->Translate(&stream, NULL, NULL, &memStream, format->type) == B_OK) {
		reply.AddData(format->MIME, B_MIME_TYPE, memStream.Buffer(), memStream.BufferLength());
		msg->SendReply(&reply);
	}
}

void
ShowImageView::HandleDrop(BMessage* msg)
{
	BMessage data(B_MIME_DATA);
	entry_ref dirRef;
	BString name, type;
	bool saveToFile;
	bool sendInMessage;
	BBitmap *bitmap;

	saveToFile = msg->FindString("be:filetypes", &type) == B_OK &&
				 msg->FindRef("directory", &dirRef) == B_OK &&
				 msg->FindString("name", &name) == B_OK;
	
	sendInMessage = (!saveToFile) && msg->FindString("be:types", &type) == B_OK;

	bitmap = CopySelection();
	if (bitmap == NULL) return;

	translation_format format;
	if (!OutputFormatForType(bitmap, type.String(), &format)) {
		delete bitmap;
		return;
	}

	if (saveToFile) {
		BDirectory dir(&dirRef);
		SaveToFile(&dir, name.String(), bitmap, &format);
		delete bitmap;
	} else if (sendInMessage) {
		SendInMessage(msg, bitmap, &format);
	} else {
		delete bitmap;
	}
}

void
ShowImageView::MoveImage()
{
	BPoint point, delta;
	uint32 buttons;
	// get CURRENT position
	GetMouse(&point, &buttons);
	point = ConvertToScreen(point);
	delta = fFirstPoint - point;
	fFirstPoint = point;
	ScrollRestrictedBy(delta.x, delta.y);
	// in case we miss MouseUp
	if ((GetMouseButtons() & B_TERTIARY_MOUSE_BUTTON) == 0)
		fMovesImage = false;
}

uint32
ShowImageView::GetMouseButtons()
{
	uint32 buttons;
	BPoint point;
	GetMouse(&point, &buttons);
	if (buttons == B_PRIMARY_MOUSE_BUTTON) {
		if ((modifiers() & B_CONTROL_KEY) != 0) {
			buttons = B_SECONDARY_MOUSE_BUTTON; // simulate second button 
		} else if ((modifiers() & B_SHIFT_KEY) != 0) {
			buttons = B_TERTIARY_MOUSE_BUTTON; // simulate third button 
		}
	}

	if(((DjVuWindow*)Window())->FullScreenFlag && buttons==B_PRIMARY_MOUSE_BUTTON)
		buttons=B_TERTIARY_MOUSE_BUTTON;
	
	return buttons;
}

void
ShowImageView::GetMergeRects(BBitmap *merge, BRect selection, BRect &srcBits, BRect &destRect)
{
	destRect = selection;
	ConstrainToImage(destRect);
	
	srcBits = selection;
	if (srcBits.left < 0)
		srcBits.left = -(srcBits.left);
	else
		srcBits.left = 0;
	if (srcBits.top < 0)
		srcBits.top = -(srcBits.top);
	else
		srcBits.top = 0;
	if (srcBits.right > fBitmap->Bounds().right)
		srcBits.right = srcBits.left + destRect.Width();
	else
		srcBits.right = merge->Bounds().right;
	if (srcBits.bottom > fBitmap->Bounds().bottom)
		srcBits.bottom = srcBits.top + destRect.Height();
	else
		srcBits.bottom = merge->Bounds().bottom;	
}

void
ShowImageView::GetSelMergeRects(BRect &srcBits, BRect &destRect)
{
	GetMergeRects(fSelBitmap, fSelectionRect, srcBits, destRect);
}

void
ShowImageView::MergeWithBitmap(BBitmap *merge, BRect selection)
{
/*	BView view(fBitmap->Bounds(), NULL, B_FOLLOW_NONE, B_WILL_DRAW);
	BBitmap *bitmap = new BBitmap(fBitmap->Bounds(), fBitmap->ColorSpace(), true);
	if (bitmap == NULL)
		return;

	if (bitmap->Lock()) {
		bitmap->AddChild(&view);
		view.DrawBitmap(fBitmap, fBitmap->Bounds());
		BRect srcBits, destRect;
		GetMergeRects(merge, selection, srcBits, destRect);
		view.DrawBitmap(merge, srcBits, destRect);
		
		view.Sync();
		bitmap->RemoveChild(&view);
		bitmap->Unlock();
		
		DeleteBitmap();
		fBitmap = bitmap;
		
		BMessenger msgr(Window());
		msgr.SendMessage(MSG_MODIFIED);
	} else
		delete bitmap; */
}

void
ShowImageView::MergeSelection()
{
/*	if (!HasSelection())
		return;
		
	if (!fSelBitmap) {
		// Even though the merge will not change
		// the background image, I still need to save
		// some undo information here
		fUndo.SetTo(fSelectionRect, NULL, CopySelection());
		return;
	}

	// Merge selection with background
	fUndo.SetTo(fSelectionRect, CopyFromRect(fSelectionRect), CopySelection());
	MergeWithBitmap(fSelBitmap, fSelectionRect);*/
}

void
ShowImageView::MouseDown(BPoint position)
{
 if (fBitmap)
 {
	BPoint point;
	uint32 buttons;
	MakeFocus(true);

	point = ViewToImage(position);
	buttons = GetMouseButtons();
	
	if (HasSelection() && fSelectionRect.Contains(point) &&
		(buttons & (B_PRIMARY_MOUSE_BUTTON | B_SECONDARY_MOUSE_BUTTON))) {
		if (!fSelBitmap) {
			fSelBitmap = CopySelection();
		}
		BPoint sourcePoint = point;
		BeginDrag(sourcePoint);
		
		while (buttons) {
			// Keep reading mouse movement until
			// the user lets up on all mouse buttons
			GetMouse(&point, &buttons);		
			snooze(25 * 1000);
				// sleep for 25 milliseconds to minimize CPU usage during loop
		}
		
		if (Bounds().Contains(point)) {
			// If selection stayed inside this view
			// (Some of the selection may be in the border area, which can be OK)
			BPoint last, diff;
			last = ViewToImage(point);
			diff = last - sourcePoint;	
			
			BRect newSelection = fSelectionRect;
			newSelection.OffsetBy(diff);
			
			if (fBitmap->Bounds().Intersects(newSelection)) {
				// Do not accept the new selection box location
				// if it does not intersect with the bitmap rectangle
				//fSelectionRect = newSelection;
				Invalidate();
			}
		}
		
		AnimateSelection(true);
		
	} else if (buttons == B_PRIMARY_MOUSE_BUTTON) {
		MergeSelection();
			// If there is an existing selection, 
			// Make it part of the background image
	
		// begin new selection
		SetHasSelection(true);
		fMakesSelection = true;
		SetMouseEventMask(B_POINTER_EVENTS);
		ConstrainToImage(point);
		fFirstPoint = point;
		fCopyFromRect.Set(point.x, point.y, point.x, point.y);
		fSelectionRect = fCopyFromRect;
		Invalidate(); 
	} else if (buttons == B_SECONDARY_MOUSE_BUTTON) {
		ShowPopUpMenu(ConvertToScreen(position));
	} else if (buttons == B_TERTIARY_MOUSE_BUTTON) {
		// move image in window
		SetMouseEventMask(B_POINTER_EVENTS);
		fMovesImage = true;
		fFirstPoint = ConvertToScreen(position);
	}
 }
}

void
ShowImageView::UpdateSelectionRect(BPoint point, bool final)
{
	BRect oldSelection = fCopyFromRect;
	point = ViewToImage(point);
	ConstrainToImage(point);
	fCopyFromRect.left = min(fFirstPoint.x, point.x);
	fCopyFromRect.right = max(fFirstPoint.x, point.x);
	fCopyFromRect.top = min(fFirstPoint.y, point.y);
	fCopyFromRect.bottom = max(fFirstPoint.y, point.y);
	fSelectionRect = fCopyFromRect;
	if (final) {
		// selection must be at least 2 pixels wide or 2 pixels tall
		if (fCopyFromRect.Width() < 1.0 && fCopyFromRect.Height() < 1.0)
			SetHasSelection(false);
	}
	if (oldSelection != fCopyFromRect || !HasSelection()) {
		BRect updateRect;
		updateRect = oldSelection | fCopyFromRect;
		updateRect = ImageToView(updateRect);
		updateRect.InsetBy(-PEN_SIZE, -PEN_SIZE);
		Invalidate(updateRect);
	}
}

void
ShowImageView::MouseMoved(BPoint point, uint32 state, const BMessage *pmsg)
{
	if (fMakesSelection) {
		UpdateSelectionRect(point, false);
	} else if (fMovesImage) {
		MoveImage();
	}
}

void
ShowImageView::MouseUp(BPoint point)
{
	if (fMakesSelection) {
		UpdateSelectionRect(point, true);
		fMakesSelection = false;
	} else if (fMovesImage) {
		MoveImage();
		if (fMovesImage)
			fMovesImage = false;
	}
	AnimateSelection(true);
}

float
ShowImageView::LimitToRange(float v, orientation o, bool absolute)
{
	BScrollBar* psb = ScrollBar(o);
	if (psb) {
		float min, max, pos;
		pos = v;
		if (!absolute) {
			pos += psb->Value();
		}
		psb->GetRange(&min, &max);
		if (pos < min) {
			pos = min;
		} else if (pos > max) {
			pos = max;
		}
		v = pos;
		if (!absolute) {
			v -= psb->Value();
		}
	}
	return v;
}

void
ShowImageView::ScrollRestricted(float x, float y, bool absolute,bool pager)
{
	if (x != 0) {
		x = LimitToRange(x, B_HORIZONTAL, absolute);
	}
	
	if (y != 0) {
		int y0=LimitToRange(y, B_VERTICAL, absolute);
		if(pager)
		{
			float min, max, pos;
			BScrollBar* psb = ScrollBar(B_VERTICAL);
			if (psb)
			{
				psb->GetRange(&min, &max);	
				pos=psb->Value();
				if(y0-y<0)
				 {
					if(pos==max){Window()->PostMessage(new BMessage('NxIm'));return;};
					if(y0<y){y=y0;ScrollBy(x, y);return;}
				 }
				 else
				 {
					if(pos==min){Window()->PostMessage(new BMessage('PrIm'));return;};
					if(y0<y){y=y0;ScrollBy(x, y);return;};
				 }
			}
		}
	}
	
	// hide the caption when using mouse wheel
	// in full screen mode

	ScrollBy(x, y);
}

// XXX method is not unused
void
ShowImageView::ScrollRestrictedTo(float x, float y)
{
	ScrollRestricted(x, y, true,false);
}

void
ShowImageView::ScrollRestrictedBy(float x, float y)
{
	ScrollRestricted(x, y, false,false);
}

void
ShowImageView::KeyDown (const char * bytes, int32 numBytes)
{
	if (numBytes == 1) {
		switch (*bytes) {
			case B_DOWN_ARROW: 
				ScrollRestrictedBy(0, 10);
				break;
			case B_UP_ARROW: 
				ScrollRestrictedBy(0, -10);
				break;
			case B_LEFT_ARROW: 
				ScrollRestrictedBy(-10, 0);
				break;
			case B_RIGHT_ARROW: 
				ScrollRestrictedBy(10, 0);
				break;
			case B_HOME:
				ScrollRestrictedTo(0,-Bounds().Height());
				break;
			case B_END:
				ScrollRestrictedTo(0,Bounds().Height());
				break;
			case B_PAGE_UP: 
				ScrollRestricted(0, -100,false,true);
				break;
			case B_PAGE_DOWN: 
				ScrollRestricted(0, 100,false,true);
				break;

	}	
	}
}

void
ShowImageView::MouseWheelChanged(BMessage *msg)
{
	// The BeOS driver does not currently support
	// X wheel scrolling, therefore, dx is zero.
	// |dy| is the number of notches scrolled up or down.
	// When the wheel is scrolled down (towards the user) dy > 0
	// When the wheel is scrolled up (away from the user) dy < 0
	const float kscrollBy = 32;
	float dy, dx;
	float x, y;
	x = 0; y = 0; 
	if (msg->FindFloat("be:wheel_delta_x", &dx) == B_OK) {
		x = dx * kscrollBy;
	}
	if (msg->FindFloat("be:wheel_delta_y", &dy) == B_OK) {
		y = dy * kscrollBy;
	}
		
	ScrollRestrictedBy(x, y);
}

void
ShowImageView::ShowPopUpMenu(BPoint screen)
{
	BPopUpMenu* menu = new BPopUpMenu("PopUpMenu",false,false);
	
	menu->AddItem(new BMenuItem(MES("MM:FirstPage"),new BMessage('GBeg'),B_LEFT_ARROW));
	menu->AddItem(new BMenuItem(MES("MM:PreviousPage"),new BMessage('GPre'),B_UP_ARROW));
	menu->AddItem(new BMenuItem(MES("MM:NextPage"),new BMessage('GNex'),B_DOWN_ARROW));
	menu->AddItem(new BMenuItem(MES("MM:LastPage"),new BMessage('GEnd'),B_RIGHT_ARROW));
	menu->AddSeparatorItem();	
	menu->AddItem(new BMenuItem(MES("MM:Goto"),new BMessage('GoTo'),',',0));
	menu->AddSeparatorItem();			
	menu->AddItem(new BMenuItem(MES("MM:ZoomIn"),new BMessage('ZIn_'),'+',0));
	menu->AddItem(new BMenuItem(MES("MM:ZoomOut"),new BMessage('ZOut'),'-',0));
	menu->AddItem(new BMenuItem(MES("MM:SetActualSize"),new BMessage('Z100'),'/',0));
	menu->AddItem(new BMenuItem(MES("MM:FitWidth"),new BMessage('ZFit'),'*',0));
	menu->AddSeparatorItem();
	
	BMenuItem *ScaleItem=new BMenuItem(MES("MM:ScaleBilinear"),new BMessage('Bili'));
	ScaleItem->SetMarked(GetScaleBilinear());
	menu->AddItem(ScaleItem);
	menu->AddSeparatorItem();	
	
	BMenuItem *FullScreenItem = new BMenuItem(MES("MM:FullScreen"),new BMessage('Full'),'F',0);			
	FullScreenItem->SetMarked( ((DjVuWindow*)Window())->FullScreenFlag );
	menu->AddItem(FullScreenItem);
	menu->AddSeparatorItem();

	menu->AddItem( pmenuSaveAs);
	pmenuSaveAs->SetTargetForItems(Window());
	
	menu->SetTargetForItems(Window());
	
	screen -= BPoint(10, 10);
	menu->Go(screen, true, false, true);
}


void
ShowImageView::MessageReceived(BMessage *pmsg)
{
	switch (pmsg->what) {
	
		case MSG_SELECTION_BITMAP:
		{
			BMessage msg;
			msg.AddPointer("be:_bitmap_ptr", CopySelection());
			pmsg->SendReply(&msg);
			break;
		}
		
		case B_COPY_TARGET:
			HandleDrop(pmsg);
			break;
		case B_MOUSE_WHEEL_CHANGED:
			MouseWheelChanged(pmsg);
			break;
		case MSG_INVALIDATE:
			Invalidate();
			break;
		default:
			BView::MessageReceived(pmsg);
			break;
	}
}

void
ShowImageView::FixupScrollBar(orientation o, float bitmapLength, float viewLength)
{
	float prop, range;
	BScrollBar *psb;	

	psb = ScrollBar(o);
	if (psb) {
		if (fHasBorder && !fShrinkOrZoomToBounds) {
			bitmapLength += BORDER_WIDTH*2;
		}
		range = bitmapLength - viewLength;
		if (range < 0.0) {
			range = 0.0;
		}
		prop = viewLength / bitmapLength;
		if (prop > 1.0) {
			prop = 1.0;
		}
		psb->SetRange(0, range);
		psb->SetProportion(prop);
		psb->SetSteps(10, 100);		
	}
}

void
ShowImageView::FixupScrollBars()
{
	BRect rctview = Bounds(), rctbitmap(0, 0, 0, 0);
	if (fBitmap) {
		BRect rect(AlignBitmap());
		rctbitmap.Set(0, 0, rect.Width(), rect.Height());
	}
	
	FixupScrollBar(B_HORIZONTAL, rctbitmap.Width(), rctview.Width());
	FixupScrollBar(B_VERTICAL, rctbitmap.Height(), rctview.Height());
}

int32
ShowImageView::CurrentPage()
{
	return fDocumentIndex;
}

int32
ShowImageView::PageCount()
{
	return fDocumentCount;
}

void
ShowImageView::AddWhiteRect(BRect &rect)
{
	// Paint white rectangle, using rect, into the background image
	BView view(fBitmap->Bounds(), NULL, B_FOLLOW_NONE, B_WILL_DRAW);
	BBitmap *bitmap = new BBitmap(fBitmap->Bounds(), fBitmap->ColorSpace(), true);
	if (bitmap == NULL)
		return;

	if (bitmap->Lock()) {
		bitmap->AddChild(&view);
		view.DrawBitmap(fBitmap, fBitmap->Bounds());
		
		view.FillRect(rect, B_SOLID_LOW);
			// draw white rect
		
		view.Sync();
		bitmap->RemoveChild(&view);
		bitmap->Unlock();
		
		DeleteBitmap();
		fBitmap = bitmap;
		
		BMessenger msgr(Window());
		msgr.SendMessage(MSG_MODIFIED);
	} else
		delete bitmap;
}

void
ShowImageView::RemoveSelection(bool bToClipboard)
{
	if (HasSelection()) {
		BRect rect = fSelectionRect;
		bool bCutBackground = (fSelBitmap) ? false : true;
		BBitmap *selection, *restore = NULL;
		selection = CopySelection();
	
		if (bToClipboard)
			CopySelectionToClipboard();
		SetHasSelection(false);
		
		if (bCutBackground) {
			// If the user hasn't dragged the selection,
			// paint a white rectangle where the selection was
			restore = CopyFromRect(rect);
			AddWhiteRect(rect);
		}
		
		Invalidate();
	}
}

void
ShowImageView::Cut()
{
	RemoveSelection(true);
}

void
ShowImageView::SelectAll()
{
	SetHasSelection(true);
	fCopyFromRect.Set(0, 0, fBitmap->Bounds().Width(), fBitmap->Bounds().Height());
	fSelectionRect = fCopyFromRect;
	Invalidate();
}

void
ShowImageView::ClearSelection()
{
	// Remove the selection,
	// DON'T copy it to the clipboard
	RemoveSelection(false);
}

void
ShowImageView::SetHasSelection(bool bHasSelection)
{
	DeleteSelBitmap();
	fHasSelection = bHasSelection;
	
	BMessage msg(MSG_SELECTION);
	msg.AddBool("has_selection", fHasSelection);
	BMessenger msgr(Window());
	msgr.SendMessage(&msg);
}

void
ShowImageView::CopySelectionToClipboard()
{
	if (HasSelection() && be_clipboard->Lock()) {
		be_clipboard->Clear();
		BMessage *clip = NULL;
		if ((clip = be_clipboard->Data()) != NULL) {
			BMessage data;
			BBitmap* bitmap = CopySelection();
			if (bitmap != NULL) {
				#if 0
				// According to BeBook and Becasso, Gobe Productive do the following.
				// Paste works in Productive, but not in Becasso and original ShowImage.
				BMessage msg(B_OK); // Becasso uses B_TRANSLATOR_BITMAP, BeBook says its unused
				bitmap->Archive(&msg);
				clip->AddMessage("image/x-be-bitmap", &msg);
				#else
				// original ShowImage performs this. Paste works with original ShowImage.
				bitmap->Archive(clip);
				// original ShowImage uses be:location for insertion point
				clip->AddPoint("be:location", BPoint(fSelectionRect.left, fSelectionRect.top));
				#endif
				delete bitmap;
				be_clipboard->Commit();
			}
		}
		be_clipboard->Unlock();
	}
}


int 
ShowImageView::CompareEntries(const void* a, const void* b)
{
	entry_ref *r1, *r2;
	r1 = *(entry_ref**)a;
	r2 = *(entry_ref**)b;
	return strcasecmp(r1->name, r2->name);
}

void
ShowImageView::FreeEntries(BList* entries)
{
	const int32 n = entries->CountItems();
	for (int32 i = 0; i < n; i ++) {
		entry_ref* ref = (entry_ref*)entries->ItemAt(i);
		delete ref;		
	}
	entries->MakeEmpty();
}


void 
ShowImageView::SetTrackerSelectionToCurrent()
{
	BMessage setsel(B_SET_PROPERTY);
	setsel.AddSpecifier("Selection");
	setsel.AddRef("data", &fCurrentRef);
	fTrackerMessenger.SendMessage(&setsel);
}


void
ShowImageView::SetZoom(float zoom)
{
	if ((fScaleBilinear || fDither) && fZoom != zoom) {
		DeleteScaler();
	}
	fZoom = zoom;
	FixupScrollBars();
	Invalidate();
}

float
ShowImageView::GetZoom(void)
{
	return fZoom;
}

void
ShowImageView::ZoomIn()
{
	if (fZoom < 16) {
		SetZoom(fZoom + 0.1);
	}
}

void
ShowImageView::ZoomOut()
{
	if (fZoom > 0.1) {
		SetZoom(fZoom - 0.1);
	}
}

void
ShowImageView::DoImageOperation(ImageProcessor::operation op, bool quiet) 
{
	BMessenger msgr;
	ImageProcessor imageProcessor(op, fBitmap, msgr, 0);
	imageProcessor.Start(false);
	BBitmap* bm = imageProcessor.DetachBitmap();
	if (bm == NULL) {
		// operation failed
		return;	
	}
	
	// update orientation state
	if (op != ImageProcessor::kInvert) {
		// Note: If one of these fails, check its definition in class ImageProcessor.
		ASSERT(ImageProcessor::kRotateClockwise < ImageProcessor::kNumberOfAffineTransformations);
		ASSERT(ImageProcessor::kRotateAntiClockwise < ImageProcessor::kNumberOfAffineTransformations);
		ASSERT(ImageProcessor::kMirrorVertical < ImageProcessor::kNumberOfAffineTransformations);
		ASSERT(ImageProcessor::kMirrorHorizontal < ImageProcessor::kNumberOfAffineTransformations);
		fImageOrientation = fTransformation[op][fImageOrientation];
	} else {
		fInverted = !fInverted;
	}
	
	if (!quiet) {
		// write orientation state
		BNode node(&fCurrentRef);
		int32 orientation = fImageOrientation;
		if (fInverted) orientation += 256;
		if (orientation != k0) {
			node.WriteAttr(SHOW_IMAGE_ORIENTATION_ATTRIBUTE, B_INT32_TYPE, 0, &orientation, sizeof(orientation));
		} else {
			node.RemoveAttr(SHOW_IMAGE_ORIENTATION_ATTRIBUTE);
		}
	}

	// set new bitmap
	DeleteBitmap();
	fBitmap = bm; 
	
	if (!quiet) {
		// remove selection
		SetHasSelection(false);
		Notify(NULL);
	}	
}

// image operation initiated by user
void
ShowImageView::UserDoImageOperation(ImageProcessor::operation op, bool quiet)
{
	DoImageOperation(op, quiet);
}

void
ShowImageView::Rotate(int degree)
{
	if (degree == 90) {
		UserDoImageOperation(ImageProcessor::kRotateClockwise);
	} else if (degree == 270) {
		UserDoImageOperation(ImageProcessor::kRotateAntiClockwise);
	}
}
