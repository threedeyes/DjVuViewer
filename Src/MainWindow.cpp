#include <Application.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <StringView.h>
#include <String.h>
#include <Alert.h>
#include <Message.h>
#include <ListView.h>
#include <ScrollView.h>
#include <TextControl.h>
#include <ScrollBar.h>
#include <PopUpMenu.h>
#include <Menu.h>
#include <MenuBar.h>
#include <be/support/UTF8.h>
#include <be/app/Roster.h>
#include <Screen.h>
#include <TranslationDefs.h>
#include <TranslatorFormats.h>
#include <TranslationUtils.h>
#include <FilePanel.h>
#include <PrintJob.h>

#include "MainApp.h"
#include "ThumbListItem.h"
#include "MainWindow.h"
#include "ConfigureWindow.h"
#include "GoToWindow.h"
#include "ToolsContainer.h"
#include "AboutWindow.h"
#include "PicButton.h"
#include "Label.h"
#include "Theme.h"

#include "ShowImageView.h"

#include "LoadDjVu.h"

#define kThemeChanged 		   'kTch'
#define MSG_PAGE_SETUP         'mPSU'
#define MSG_PRINT              'mPRT'

#define TRANSLATOR_FLD "be:translator"
#define TYPE_FLD "be:type"

int32 Thumber(void *data);

DjVuWindow::DjVuWindow(const char *title)
:BWindow(BRect(100,40,400,450),title,B_DOCUMENT_WINDOW,0)
{
	BView *view;
 	fConfigureWindow = NULL;
 	fSavePanel=NULL;
	fPrintSettings = NULL; 	
 	thumb_thread=-1;
 	

	MainMenu = new BMenuBar(BRect(0, 0, Bounds().right, 22), "menu_bar");
	LoadMenus(MainMenu);
	AddChild(MainMenu);

	BScreen *scr=new BScreen(B_MAIN_SCREEN_ID);
	
	if(SETTINGS->GetInt32("StorePosition")==1)
	{
		BRect rect=SETTINGS->GetRect("WinPosition");
		MoveTo(rect.left,rect.top);
		ResizeTo(rect.Width(),rect.Height());
	}
	else
	{
		BRect rect=BRect(50,100,scr->Frame().right-250,scr->Frame().bottom-100);	
		MoveTo(rect.left,rect.top);
		ResizeTo(rect.Width(),rect.Height());
		SETTINGS->SetRect("WinPosition",rect);
	}

	//-----------------------------------------------------------------------------
		
	ThemePopUp = new BMenu(MES("ConfigWnd:IfaceTheme"));
	
	BDirectory *dir=THEME->GetDirectory();
	BEntry ent;
	char filename[B_FILE_NAME_LENGTH];
	ThemePopUp->AddItem(new BMenuItem("Default", new BMessage(kThemeChanged)));		
	dir->Rewind();
	
	while (dir->GetNextEntry(&ent) == B_NO_ERROR)
	{
		ent.GetName(filename);
		ThemePopUp->AddItem(new BMenuItem(filename, new BMessage(kThemeChanged)));	
	}
	
	BMenuItem *item;
	item=ThemePopUp->FindItem(SETTINGS->GetString("Theme"));
	if (item)item->SetMarked(true);
	else {
			item=ThemePopUp->FindItem("Default");
			if (item)item->SetMarked(true);
		 }
	
	ThemePopUp->SetTargetForItems(this);
	ThemePopUp->SetRadioMode(true);
	
	RightClickPopUp = new BPopUpMenu("Context",false,false);
	RightClickPopUp->AddItem(ThemePopUp);
	RightClickPopUp->AddSeparatorItem();
	ShowButtonLabelItem=new BMenuItem(MES("ConfigWnd:ShButtLabel"), new BMessage('C-Lb'));
	RightClickPopUp->AddItem(ShowButtonLabelItem);
	ShowButtonLabelItem->SetMarked(SETTINGS->GetInt32("ShowButtonLabel")==1);
	RightClickPopUp->SetTargetForItems(this);


	//-----------------------------------------------------------------------------
		
	
	BRect rect=Bounds();
	rect.top=MainMenu->Bounds().bottom+1;
	rect.bottom=rect.top+32;
	rect.right-=40;
	view=new BView(Bounds(),"main_view",B_FOLLOW_ALL_SIDES,0);
	view->SetViewColor(255,255,255);
	tools=new BToolsContainer(rect,"buttons",TCNT_CLIP|TCNT_BACKGROUND,THEME,B_FOLLOW_LEFT_RIGHT);
	AddChild(view);
	tools->SetContextMenu(RightClickPopUp);

	rect.left=rect.right;
	rect.right+=40;
	logo=new BToolsContainer(rect,"logo",TCNT_BACKGROUND,THEME,B_FOLLOW_RIGHT);
	logo->AddButton("Logo","Logo",MES("BL:About:Tip"),new BMessage('Logo'),-1,PB_MOUSEOVER);
	logo->SetContextMenu(RightClickPopUp);

	rect.left=Bounds().left;
	rect.top=rect.bottom+1;
	rect.bottom=Bounds().bottom;
	line=new BToolsContainer(rect,"line",0,THEME,B_FOLLOW_ALL_SIDES);
	line->SetViewColor(220,220,220);
	line->SetContextMenu(RightClickPopUp);

	view->AddChild(line);
	view->AddChild(tools);
	view->AddChild(logo);

	SelectList = new BListView(BRect(5,5,124,line->Bounds().Height()-B_H_SCROLL_BAR_HEIGHT-1),"Select",B_SINGLE_SELECTION_LIST,B_FOLLOW_ALL_SIDES);
	SelectListScroll=new BScrollView("scroll_select",SelectList,B_FOLLOW_LEFT|B_FOLLOW_TOP_BOTTOM,0,false,true);
	line->AddChild(SelectListScroll);
	SelectList->SetSelectionMessage(new BMessage('PGCH'));
	SelectList->SetViewColor(SETTINGS->GetColor("ThumbBackCol"));

	PageInfo=new BTLabel(BRect(0,line->Bounds().Height()-B_H_SCROLL_BAR_HEIGHT+1,128+B_V_SCROLL_BAR_WIDTH,line->Bounds().Height()+1),
				"pageinfo","-/-",NULL,0,B_FOLLOW_BOTTOM|B_FOLLOW_LEFT); //THEME->GetPNG("BackGround")
	
	PageInfo->SetViewColor(220,220,220);
	BFont font = SETTINGS->GetFont("TVNormalFont");
	PageInfo->SetFont(&font);
	PageInfo->SetTextColor(SETTINGS->GetColor("TextNormCol"));
	line->AddChild(PageInfo);


	ImageViewFrame=BRect(SelectList->Bounds().Width()+12+B_V_SCROLL_BAR_WIDTH,5,line->Bounds().right-B_V_SCROLL_BAR_WIDTH,line->Bounds().bottom-B_H_SCROLL_BAR_HEIGHT);

	fImageView = new ShowImageView(ImageViewFrame, "image_view", B_FOLLOW_ALL, 
		B_WILL_DRAW | B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE | B_PULSE_NEEDED);	
	BScrollView *pscrollView = new BScrollView("image_scroller", fImageView,
		B_FOLLOW_ALL, 0, true, true, B_FANCY_BORDER);
	line->AddChild(pscrollView);
	
	SetSizeLimits(192,2048,280,2048);
	delete scr;
	
	SetPulseRate(100000);
	align_type='-';
	NeedRenderPage=0;
	NeedPrintPages=0;
	PrintFirstPage=0;
	PrintLastPage=0;	
	FullScreenFlag=false;
	
	ReMakeMainToolBar();
}

DjVuWindow::~DjVuWindow()
{

}


int32 Thumber(void *data)
{
	DjVuWindow *wnd=(DjVuWindow*)data;
	BInvoker *invoker = new BInvoker(new BMessage('CnPr'),wnd);
	
	ShowImageView* image_view=(ShowImageView*)(wnd->fImageView);
	snooze(100000);
	for(;;)
	{
	 
	 wnd->SelectList->LockLooper();
	 int beg=wnd->SelectList->IndexOf( wnd->SelectList->ConvertFromParent(BPoint(10,10)) );
	 int h=wnd->SelectList->ItemFrame(beg).Height();
	 int end=1+beg+wnd->SelectList->Bounds().Height()/h;
	 int current=wnd->SelectList->CurrentSelection();
	 wnd->SelectList->UnlockLooper();
	 
	 if(beg>=wnd->DJFile->MaxPages)beg=wnd->DJFile->MaxPages-1;
	 if(end>=wnd->DJFile->MaxPages)end=wnd->DJFile->MaxPages-1;
	 if(beg<0)beg=0;
	 if(end<0)end=0;
	 
	if(wnd->NeedPrintPages==1)
	 {
	  	BString name;
		
		name.SetTo(wnd->DJFile->GetCurrentFile());
		
		BPrintJob printJob(name.String());
		printJob.SetSettings(new BMessage(*(wnd->fPrintSettings)));
		
		if (printJob.ConfigJob() == B_OK)
		{
			int32  firstPage;
			int32  lastPage;
			BRect  printableRect = printJob.PrintableRect();
			float width, imageWidth, imageHeight, w1, w2;
			BBitmap* bitmap;

			// first/lastPage is unused for now
			firstPage = printJob.FirstPage();
			lastPage = printJob.LastPage();
			if (firstPage < 1)firstPage = 1;
			if (lastPage < firstPage)lastPage = firstPage;

			printJob.BeginJob();
			image_view->LockLooper();
			bool	bilinear=image_view->GetScaleBilinear();
			image_view->SetScaleBilinear(false);
			image_view->UnlockLooper();		
				
			BAlert *CancelMess = new BAlert("Print", MES("Message:003"), MES("Cancel"), NULL, NULL, B_WIDTH_AS_USUAL,B_INFO_ALERT);
			CancelMess->Go(invoker);
				
			for(int pg=firstPage;pg<=lastPage;pg++)
			{
				if(pg>wnd->DJFile->MaxPages)break;		
				if(wnd->NeedPrintPages==0)break;
				bitmap=wnd->DJFile->GetPage(pg-1);
			 	image_view->LockLooper();
				image_view->SetBitmap(bitmap);
				image_view->UnlockLooper();								
				snooze(15000);
				
				imageWidth = bitmap->Bounds().Width() + 1.0;
				imageHeight = bitmap->Bounds().Height() + 1.0;
		
				w1 = printableRect.Width()+1;
				w2 = imageWidth * (printableRect.Height() + 1) / imageHeight;
				if (w2 < w1)width = w2;	else width = w1;
				
			 	image_view->LockLooper();
				image_view->SetScale(width / imageWidth);
				printJob.DrawView(image_view, bitmap->Bounds(), BPoint(printableRect.left, printableRect.top));
				image_view->SetScale(1.0);
				image_view->UnlockLooper();								
				printJob.SpoolPage();
	    	}

    		image_view->LockLooper();		
			image_view->SetScaleBilinear(bilinear);
			image_view->UnlockLooper();		
    	
	    	if(wnd->NeedPrintPages!=0)
	    	{
	  			printJob.CommitJob();
	  			CancelMess->Quit();
	  		}
		}
		snooze(25000);	
		wnd->NeedPrintPages=0;
		wnd->NeedRenderPage=current;
	 }

	for(int i=beg;i<=end;i++)
		{	
		ThumbListItem *item=(ThumbListItem*)(wnd->SelectList->ItemAt(i));
		if(wnd->NeedRenderPage!=-1)
		    {
		    	image_view->LockLooper();
		     	image_view->SetBitmap(wnd->DJFile->GetPage(wnd->NeedRenderPage));
				if(wnd->align_type=='n' || wnd->align_type=='-')image_view->FlushToLeftTop();
				if(wnd->align_type=='p')image_view->FlushToLeftBottom();
				wnd->align_type='-';
				wnd->NeedRenderPage=-1;
				if(wnd->AutoFitWidth==true){wnd->PostMessage(new BMessage('ZFit'));wnd->AutoFitWidth=false;}
				image_view->UnlockLooper();				
				snooze(25000);
		    }
		if(item->isThumbed()==0)
			{
			BBitmap* bmp=wnd->DJFile->GetThumb(i);	
			wnd->SelectList->LockLooper();
			item->ChangeIcon(item,bmp);
			item->SetThumb(1);
			wnd->SelectList->InvalidateItem(i);
			wnd->SelectList->UnlockLooper();
			if(bmp!=NULL)delete bmp;
			snooze(15000);
			}
		}
	 snooze(100000);
	}
}

void
DjVuWindow::LoadMenus(BMenuBar *pbar)
{
	BMenu *file_menu = new BMenu(MES("MM:File"));
	file_menu->AddItem(new BMenuItem(MES("MM:OpenFile"),new BMessage('Open')));

	pmenuSaveAs = new BMenu(MES("MM:SavePageAs"), B_ITEMS_IN_COLUMN);
	BTranslationUtils::AddTranslationItems(pmenuSaveAs, B_TRANSLATOR_BITMAP);	
	file_menu->AddItem(pmenuSaveAs);
	pmenuSaveAs->SetTargetForItems(this);
	file_menu->AddSeparatorItem();
	file_menu->AddItem(new BMenuItem(MES("MM:PageSetup"),new BMessage(MSG_PAGE_SETUP),'P',B_SHIFT_KEY));
	file_menu->AddItem(new BMenuItem(MES("MM:Print"),new BMessage(MSG_PRINT),'P',0));
	file_menu->AddSeparatorItem();

	file_menu->AddItem(new BMenuItem(MES("MM:Exit"),new BMessage('Quit'),'Q'));
	pbar->AddItem(file_menu);
	
	BMenu *view_menu = new BMenu(MES("MM:View"));
	view_menu->AddItem(new BMenuItem(MES("MM:ZoomIn"),new BMessage('ZIn_'),'+',0));
	view_menu->AddItem(new BMenuItem(MES("MM:ZoomOut"),new BMessage('ZOut'),'-',0));
	view_menu->AddItem(new BMenuItem(MES("MM:SetActualSize"),new BMessage('Z100'),'/',0));
	view_menu->AddItem(new BMenuItem(MES("MM:FitWidth"),new BMessage('ZFit'),'*',0));
	view_menu->AddSeparatorItem();	
	
	ScaleItem=new BMenuItem(MES("MM:ScaleBilinear"),new BMessage('Bili'));
	view_menu->AddItem(ScaleItem);	

	FullScreenItem = new BMenuItem(MES("MM:FullScreen"),new BMessage('Full'),'F',0);
	view_menu->AddItem(FullScreenItem);
	view_menu->AddSeparatorItem();	
	view_menu->AddItem(new BMenuItem(MES("MM:Settings"),new BMessage('Stng'),'S',0));

	pbar->AddItem(view_menu);

	BMenu *nav_menu = new BMenu(MES("MM:Navigate"));
	nav_menu->AddItem(new BMenuItem(MES("MM:FirstPage"),new BMessage('GBeg'),B_LEFT_ARROW));
	nav_menu->AddItem(new BMenuItem(MES("MM:PreviousPage"),new BMessage('GPre'),B_UP_ARROW));
	nav_menu->AddItem(new BMenuItem(MES("MM:NextPage"),new BMessage('GNex'),B_DOWN_ARROW));
	nav_menu->AddItem(new BMenuItem(MES("MM:LastPage"),new BMessage('GEnd'),B_RIGHT_ARROW));
	nav_menu->AddSeparatorItem();	
	nav_menu->AddItem(new BMenuItem(MES("MM:Goto"),new BMessage('GoTo'),',',0));
	pbar->AddItem(nav_menu);

	BMenu *help_menu = new BMenu(MES("MM:Help"));
	help_menu->AddItem(new BMenuItem(MES("MM:About"),new BMessage('Logo')));
	pbar->AddItem(help_menu);

	AddShortcut(B_UP_ARROW, B_COMMAND_KEY, new BMessage('GPre'), this);
	AddShortcut(B_DOWN_ARROW, B_COMMAND_KEY, new BMessage('GNex'), this);
	AddShortcut(B_LEFT_ARROW, B_COMMAND_KEY, new BMessage('GBeg'), this);
	AddShortcut(B_RIGHT_ARROW, B_COMMAND_KEY, new BMessage('GEnd'), this);
}


BMenuItem *
DjVuWindow::AddItemMenu(BMenu *pmenu, char *caption, long unsigned int msg, 
	char shortcut, uint32 modifier, char target, bool benabled)
{
	BMenuItem* pitem;
	pitem = new BMenuItem(caption, new BMessage(msg), shortcut);
	
	if (target == 'A')
	   pitem->SetTarget(be_app);
	   
	pitem->SetEnabled(benabled);	   
	pmenu->AddItem(pitem);
	
	return pitem;
}

void DjVuWindow::MessageReceived(BMessage *message)
{
	switch(message->what) {
	
		case MSG_OUTPUT_TYPE:
			if (!fSavePanel)
				SaveAs(message);
			break;
			
		case MSG_SAVE_PANEL:
			SaveToFile(message);
			break;

		case B_CANCEL:
			delete fSavePanel;
			fSavePanel = NULL;
			break;
			
		case MSG_PAGE_SETUP:
			PageSetup();
			break;

		case MSG_PRINT:
			Print();
			break;	
			
		case 'CnPr':
			{
			 NeedPrintPages=0;
			 break;
			}		
						
		case 'Stng':
			{
 			 if(fConfigureWindow==NULL)
 			 		{
			 			fConfigureWindow=new ConfigureWindow(BRect(100,100,620,460),THEME,this);
			 			fConfigureWindow->Show();
			 			fConfigureWindow->Activate(true);
			 		}
			 	else
			 	{
			 		if(fConfigureWindow->IsHidden())fConfigureWindow->Show();
			 		else fConfigureWindow->Activate(true);
			 	}
			 break;
			}						
						
		case 'ZIn_':
			{
				((ShowImageView*)fImageView)->SetShrinkToBounds(false);
				((ShowImageView*)fImageView)->ZoomIn();
				break;
			}
		case 'ZOut':
			{
				((ShowImageView*)fImageView)->SetShrinkToBounds(false);
				((ShowImageView*)fImageView)->ZoomOut();
				break;
			}
		case 'ZFit':
			{
				float view_w=fImageView->Bounds().Width()-B_V_SCROLL_BAR_WIDTH;
				float pic_w=view_w;
				if(((ShowImageView*)fImageView)->GetBitmap()!=NULL)
				 	pic_w=((ShowImageView*)fImageView)->GetBitmap()->Bounds().Width()+32;
				float zoom=view_w/pic_w;
				((ShowImageView*)fImageView)->SetShrinkToBounds(false);
				((ShowImageView*)fImageView)->SetZoom(zoom);	
				break;
			}
		case 'Full':
			{
				FullScreenFlag=!FullScreenFlag;
				if(FullScreenFlag==true)
				  {
				   RestoreRect=Frame(); 
				   BPoint im_left_top=line->ConvertToScreen(BPoint(ImageViewFrame.left,ImageViewFrame.top));				   
				   BPoint im_right_bottom=fImageView->ConvertToScreen(BPoint(fImageView->Bounds().right,fImageView->Bounds().bottom));
				   				   
				   BScreen *Screen=new BScreen(B_MAIN_SCREEN_ID);
				   
				   BRect FullRect=BRect(Frame().left-im_left_top.x,Frame().top-im_left_top.y,
				   						Frame().right+(Screen->Frame().Width()-im_right_bottom.x), //B_V_SCROLL_BAR_WIDTH
				   						Frame().bottom+(Screen->Frame().Height()-im_right_bottom.y)); //B_H_SCROLL_BAR_HEIGHT
				   
				   MoveTo(FullRect.left,FullRect.top);
				   ResizeTo(FullRect.Width(),FullRect.Height());			
				   delete Screen;	   
				  }
				else
				  {
				   MoveTo(RestoreRect.left,RestoreRect.top);
				   ResizeTo(RestoreRect.Width(),RestoreRect.Height());
				  }			
				fImageView->MakeFocus(true);
				FullScreenItem->SetMarked(FullScreenFlag);	
				break;
			}
		case 'Z100':
			{
			 
				((ShowImageView*)fImageView)->SetShrinkToBounds(false);
				((ShowImageView*)fImageView)->SetZoom(1.0);
				break;
			}
		case 'Bili':
			{
			 	((ShowImageView*)fImageView)->SetScaleBilinear(!((ShowImageView*)fImageView)->GetScaleBilinear());
		 		ScaleItem->SetMarked(((ShowImageView*)fImageView)->GetScaleBilinear());
			 	break;
			}
		case 'PGCH':
			{
				if(DJFile!=NULL)
				{
					char temp[32];
					int32 idx=SelectList->CurrentSelection();
					ThumbListItem *item=(ThumbListItem*)(SelectList->ItemAt(idx));
					sprintf(temp,"%d/%d",idx+1,SelectList->CountItems());					
					PageInfo->SetText(temp);
				//	((ShowImageView*)fImageView)->SetBitmap(DJFile->GetPage(idx));
					NeedRenderPage=idx;
				}
				break;
			}
		case 'NxIm':
			{
				align_type='n';
				SelectList->Select(SelectList->CurrentSelection()+1);
				SelectList->ScrollToSelection();
				break;
			}
		case 'PrIm':
			{
				align_type='p';
				SelectList->Select(SelectList->CurrentSelection()-1);
				SelectList->ScrollToSelection();
				break;
			}
		case 'Logo':
			{
			int mWinIdx=-1;

			for(int i=0;i<be_app->CountWindows();i++)
	 		if(strcmp(be_app->WindowAt(i)->Title(),MES("WT:About"))==0){mWinIdx=i;break;}

			if(mWinIdx!=-1)
	  		 {
	   			be_app->WindowAt(mWinIdx)->Activate(true);
	   			break;
	  		 }
		     
		     BRect frame(150,150,150+wW-3,150+wH-3); 			 
	         AboutWindow *aw = new AboutWindow(frame,NULL);
             if(aw!=NULL)aw->Show();
			 break;
			}
		case 'GoTo':
			{
	         GoToWindow *aw = new GoToWindow(0,this,THEME);
             if(aw!=NULL)aw->Show();
			 break;			 
			}
		case 'GoNN':
			{
				int page=1;
				page=message->FindInt32("pagenum");
				SelectList->Select(page-1);
				SelectList->ScrollToSelection();
				break;
			}
		case 'Refr':
			{
				SelectList->Invalidate();
				SelectList->SetViewColor(SETTINGS->GetColor("ThumbBackCol"));
				for(int i=0;i<be_app->CountWindows();i++)
					be_app->WindowAt(i)->PostMessage(new BMessage('RefW'));	
				break;
			}			
		case kThemeChanged:
			{
				BMenuItem *item;
				message->FindPointer("source", (void**)&item);
				SETTINGS->SetString("Theme",item->Label());
				THEME->SetTheme(item->Label());
				item->SetMarked(true);
			
				for(int i=0;i<be_app->CountWindows();i++)be_app->WindowAt(i)->PostMessage(new BMessage('ReTh'));
				break;
			}
		case 'ReTh':
			{	
				BMenuItem *item=ThemePopUp->FindItem(SETTINGS->GetString("Theme"));
				if(item!=NULL)item->SetMarked(true);
				ShowButtonLabelItem->SetMarked(SETTINGS->GetInt32("ShowButtonLabel")==1);
				tools->ReloadTheme();
				logo->ReloadTheme();
				line->ReloadTheme();
				ReMakeMainToolBar();
				break;
			}	
		case 'GBeg':
			{
				SelectList->Select(0);
				SelectList->ScrollToSelection();
				break;
			}
		case 'GEnd':
			{
				SelectList->Select(SelectList->CountItems()-1);
				SelectList->ScrollToSelection();
				break;
			}
		case 'GPre':
			{
				SelectList->Select(SelectList->CurrentSelection()-1);
				SelectList->ScrollToSelection();
				break;
			}
		case 'GNex':
			{
				SelectList->Select(SelectList->CurrentSelection()+1);
				SelectList->ScrollToSelection();
				break;
			}

		case 'C-Lb':
			{
				BMenuItem *item;
				message->FindPointer("source", (void**)&item);
				
				int32 f=SETTINGS->GetInt32("ShowButtonLabel")^1;
				SETTINGS->SetInt32("ShowButtonLabel",f);

				for(int i=0;i<be_app->CountWindows();i++)be_app->WindowAt(i)->PostMessage(new BMessage('ReTh'));

				item->SetMarked(f==1);
				break;
			}
		case 'Open':
			{
				be_app->PostMessage(MSG_FILE_OPEN);
			 	break;
			}
		case 'Quit':
			{
				Lock();
				Quit();
				break;
			}
		
		default:
		
			BWindow::MessageReceived(message);
			break;
	}
}

void
DjVuWindow::SaveAs(BMessage *pmsg)
{
	// Read the translator and output type the user chose
	translator_id outTranslator;
	uint32 outType;
	if (pmsg->FindInt32(TRANSLATOR_FLD,
		reinterpret_cast<int32 *>(&outTranslator)) != B_OK)
		return;	
	if (pmsg->FindInt32(TYPE_FLD,
		reinterpret_cast<int32 *>(&outType)) != B_OK)
		return;
		
	// Add the chosen translator and output type to the
	// message that the save panel will send back
	BMessage *ppanelMsg = new BMessage(MSG_SAVE_PANEL);
	ppanelMsg->AddInt32(TRANSLATOR_FLD, outTranslator);
	ppanelMsg->AddInt32(TYPE_FLD, outType);

	// Create save panel and show it
	fSavePanel = new BFilePanel(B_SAVE_PANEL, new BMessenger(this), NULL, 0,
		false, ppanelMsg);
	if (!fSavePanel)
		return;
	fSavePanel->Window()->SetWorkspaces(B_CURRENT_WORKSPACE);
	fSavePanel->Show();
}

void
DjVuWindow::SaveToFile(BMessage *pmsg)
{
	// Read in where the file should be saved	
	entry_ref dirref;
	if (pmsg->FindRef("directory", &dirref) != B_OK)
		return;
	const char *filename;
	if (pmsg->FindString("name", &filename) != B_OK)
		return;
		
	// Read in the translator and type to be used
	// to save the output image
	translator_id outTranslator;
	uint32 outType;
	if (pmsg->FindInt32(TRANSLATOR_FLD,
		reinterpret_cast<int32 *>(&outTranslator)) != B_OK)
		return;	
	if (pmsg->FindInt32(TYPE_FLD,
		reinterpret_cast<int32 *>(&outType)) != B_OK)
		return;
	
	// Find the translator_format information needed to
	// write a MIME attribute for the image file
	BTranslatorRoster *roster = BTranslatorRoster::Default();
	const translation_format *pouts = NULL;
	int32 outsCount = 0;
	if (roster->GetOutputFormats(outTranslator, &pouts, &outsCount) != B_OK)
		return;
	if (outsCount < 1)
		return;
	int32 i;
	for (i = 0; i < outsCount; i++) {
		if (pouts[i].group == B_TRANSLATOR_BITMAP && pouts[i].type == outType)
			break;
	}
	if (i == outsCount)
		return;
	
	// Write out the image file
	BDirectory dir(&dirref);
	((ShowImageView*)fImageView)->SaveToFile(&dir, filename, NULL, &pouts[i]);
}


int	
DjVuWindow::LoadFile(const char *filename)
{
	DJFile=new BDjVuFile(filename);

	BString newname("DjVu Viewer: ");
	newname+=BString(filename);
	SetTitle(newname.String());
	
	SelectList->LockLooper();

	int n=SelectList->CountItems();
	for(int32 i=0;i<n;i++)delete(SelectList->RemoveItem((int32)0));
	
	for(int i=0;i<DJFile->MaxPages;i++)
	{
		char num[64];sprintf(num,"%d",i+1);
		SelectList->AddItem(new ThumbListItem(NULL,num,SelectList->Bounds().Width(),0, false));
	}
	
	SelectList->Select(0);
	AutoFitWidth=true;
	
	SelectList->UnlockLooper();
		
	BString th_name=BString("thumber - ");
	th_name.Append(filename);
	
	ScaleItem->SetMarked(((ShowImageView*)fImageView)->GetScaleBilinear());

	
	thumb_thread=spawn_thread(Thumber,th_name.String(),B_LOW_PRIORITY,this);
	resume_thread(thumb_thread);
}

int 
DjVuWindow::UnloadFile()
{
	thread_info ti;
	if(get_thread_info(thumb_thread,&ti)==B_OK)suspend_thread(thumb_thread);
	if(get_thread_info(thumb_thread,&ti)==B_OK)kill_thread(thumb_thread);
	status_t exit;
	wait_for_thread(thumb_thread,&exit);

	delete DJFile;
	DJFile=NULL;
	
	((ShowImageView*)fImageView)->DeleteBitmap();
	SelectList->LockLooper();
	int n=SelectList->CountItems();
	for(int32 i=0;i<n;i++)delete(SelectList->RemoveItem((int32)0));
	SelectList->UnlockLooper();
}

void 
DjVuWindow::ReMakeMainToolBar(void)
{
	float top_height = THEME->GetPNG("BackGround")!=NULL?THEME->GetPNG("BackGround")->Bounds().Height():41;	
	float logo_width = (THEME->GetPNG("Logo")->Bounds().Width()/3)+8;

	uint32 flag1=PB_MOUSEOVER;
	if(SETTINGS->GetInt32("ShowButtonLabel")==1) {
		flag1|=PB_LABEL;		
	}
	int n=tools->CountChildren();
	for(int i=0;i<n;i++)
	{
		BView *view=tools->ChildAt(0);
		tools->RemoveChild(view);
		delete view;
	}
	n=logo->CountChildren();
	for(int i=0;i<n;i++)
	{
		BView *view=logo->ChildAt(0);
		logo->RemoveChild(view);
		delete view;
	}	
	
	
	tools->ResizeTo(Bounds().Width()-logo_width,top_height);
	logo->MoveTo(tools->Bounds().right+1,logo->Frame().top);
	logo->ResizeTo(logo_width,top_height);	
	
	line->MoveTo(-1,tools->Frame().bottom);
	line->ResizeTo(Bounds().Width()+1,Bounds().Height() - tools->Frame().bottom);	
	
	tools->AddSeparator(-1);
	tools->AddButton("Open",MES("BL:Open"),MES("BL:Open:Tip"),new BMessage('Open'),-1,flag1);
	tools->AddSeparator(-1);
	tools->AddButton("Print",MES("BL:Print"),MES("BL:Print:Tip"),new BMessage(MSG_PRINT),-1,flag1);
	tools->AddSeparator(-1);
	tools->AddButton("ZoomIn",MES("BL:ZoomIn"),MES("BL:ZoomIn:Tip"),new BMessage('ZIn_'),-1,flag1);
	tools->AddButton("ZoomOut",MES("BL:ZoomOut"),MES("BL:ZoomOut:Tip"),new BMessage('ZOut'),-1,flag1);
	tools->AddButton("Zoom100",MES("BL:SetActualSize"),MES("BL:SetActualSize:Tip"),new BMessage('Z100'),-1,flag1);
	tools->AddButton("ZoomFit",MES("BL:FitWidth"),MES("BL:FitWidth:Tip"),new BMessage('ZFit'),-1,flag1);
	tools->AddSeparator(-1);
	tools->AddButton("GoToFirst",MES("BL:FirstPage"),MES("BL:FirstPage:Tip"),new BMessage('GBeg'),-1,flag1);
	tools->AddButton("GoToPrev",MES("BL:PreviousPage"),MES("BL:PreviousPage:Tip"),new BMessage('GPre'),-1,flag1);
	tools->AddButton("GoToNext",MES("BL:NextPage"),MES("BL:NextPage:Tip"),new BMessage('GNex'),-1,flag1);
	tools->AddButton("GoToLast",MES("BL:LastPage"),MES("BL:LastPage:Tip"),new BMessage('GEnd'),-1,flag1);
	tools->AddSeparator(-1);
	tools->AddButton("Settings",MES("BL:Settings"),MES("BL:Settings:Tip"),new BMessage('Stng'),-1,flag1);
	tools->AddSeparator(-1);
	tools->AddButton("FullScreen",MES("BL:FullScreen"),MES("BL:FullScreen:Tip"),new BMessage('Full'),-1,flag1);
	tools->AddSeparator(-1);
	
	logo->AddButton("Logo","Logo",MES("Button:About:Tip"),new BMessage('Logo'),-1,0);

}


bool
DjVuWindow::PageSetup()
{
	status_t st;
	BString name;
	name.SetTo(DJFile->GetCurrentFile());
	BPrintJob printJob(name.String());
	if (fPrintSettings != NULL) {
		printJob.SetSettings(new BMessage(*fPrintSettings));
	}
	st = printJob.ConfigPage();
	if (st == B_OK) {
		delete fPrintSettings;
		fPrintSettings = printJob.Settings();
	}
	return st == B_OK;
}


void
DjVuWindow::Print()
{
	if (fPrintSettings == NULL)PageSetup();
	 if(fPrintSettings!=NULL)NeedPrintPages=1;
}


void	DjVuWindow::Quit(void)
{
	UnloadFile();
		
	for(int i=0;i<be_app->CountWindows();i++)
			be_app->WindowAt(i)->PostMessage(new BMessage('QUTW'));

	SETTINGS->SetRect("WinPosition",Frame());	
	if(fConfigureWindow!=NULL)fConfigureWindow->Quit();
	SETTINGS->Save(SETTINGSFILE);

//	be_app->PostMessage(B_QUIT_REQUESTED);
	
	BWindow::Quit();
}

