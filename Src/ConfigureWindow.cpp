#include <Application.h>
#include <stdlib.h>
#include <stdio.h>
#include <be/interface/PopUpMenu.h>
#include <be/interface/MenuItem.h>
#include <be/interface/ListView.h>
#include <be/interface/ScrollView.h>
#include <be/interface/StringView.h>
#include <be/interface/InterfaceDefs.h>
#include <be/interface/OutlineListView.h>
#include <be/interface/ListItem.h>
#include <be/interface/Menu.h>
#include <be/interface/Font.h>
#include <Window.h>
#include <Alert.h>
#include <Button.h>
#include <String.h>
#include <unistd.h>
#include <Bitmap.h>
#include <Screen.h>
#include <Region.h>
#include <be/storage/Path.h>
#include <Entry.h>
#include <Directory.h>
#include <ListView.h>
#include <be/app/Roster.h>

#include "MainApp.h"
#include "MainWindow.h"
#include "ConfigureWindow.h"
#include "IconListItem.h"
#include "Theme.h"
#include "PicButton.h"

//extern BTip* ToolTip;

#define kColorControlChanged 'CCCh'
#define kViewSelectorChanged 'VSCh'
#define kColorListChanged 'CLCh'
#define kMainChecker 		'MChk'
#define kRefreshControls	'RfCt'
#define kThemeChanged		'ThCh'
#define kLangChanged		'LnCh'

SettingAlias ColorAliases[]=
	{
		{"CA:CursBarCol","CursBarCol"},
		{"CA:CursFrameCol","CursFrameCol"},
		{"CA:CursTextCol","CursTextCol"},
		{"CA:EvenLinesCol","EvenLinesCol"},
		{"CA:EvenTextCol","EvenTextCol"},
		{"CA:OddLinesCol","OddLinesCol"},
		{"CA:OddTextCol","OddTextCol"},
		{"CA:ViewFrameCol","ViewFrameCol"},
		{"CA:ViewDlgCol","ViewDlgCol"},
		{"CA:TextNormCol","TextNormCol"},	
		{"CA:ThumbBackCol","ThumbBackCol"},		
		{"CA:ThumbTextCol","ThumbTextCol"},		
			
		{NULL,NULL}
	};



BMyTextControl::BMyTextControl(BRect frame,const char *name,uint32 resizingMode,uint32 flags)
				:BTextView(frame,name,BRect(4,2,frame.Width()-8,frame.Height()-4),resizingMode,flags|B_FRAME_EVENTS)
{
  	 SetColorSpace(B_RGBA32_BIG);
 	 SetViewColor(255,255,255);
}

BMyTextControl::~BMyTextControl()
{

}

void
BMyTextControl::Draw(BRect rect)
{
  SetViewColor(255,255,255);
  BTextView::Draw(rect);
  SetHighColor(SETTINGS->GetColor("ViewFrameCol"));
  StrokeRect(Bounds());
}

void 
BMyTextControl::FrameResized(float width, float height)
{
 SetTextRect(BRect(4,2,width-8,height-4));
 Invalidate(); 
}


SettingsView::SettingsView(BRect rect, const char *name)
	:	BBox(rect, name, B_FOLLOW_ALL_SIDES, B_WILL_DRAW|B_FRAME_EVENTS,B_PLAIN_BORDER)//B_NO_BORDER)
{
	SetViewColor(SETTINGS->GetColor("ViewDlgCol"));
}

SettingsView::~SettingsView()
{
}

void
SettingsView::AttachedToWindow()
{
	SetViewColor(SETTINGS->GetColor("ViewDlgCol"));
}

void
SettingsView::Draw(BRect r)
{
	BBox::Draw(r);
  	SetHighColor(SETTINGS->GetColor("ViewFrameCol"));
  	StrokeRect(Bounds());	
}


ConfigureWindow::ConfigureWindow(BRect frame, BTheme *theme, BWindow *mainwin)
:BWindow(frame,MES("WT:Settings"),B_TITLED_WINDOW_LOOK,B_NORMAL_WINDOW_FEEL,B_NOT_ZOOMABLE|B_NOT_RESIZABLE)
{
	Theme=theme;
	SetSizeLimits(Bounds().Width(),1024,Bounds().Height(),1024);
	BRect newpos=SETTINGS->GetRect("ConfigureWinPosition");
	
	MainWindow=(DjVuWindow*)mainwin;
	scr=new BScreen(B_MAIN_SCREEN_ID);
	if(newpos.Width()==0 || SETTINGS->GetInt32("StorePosition")==0)
	{
		BPoint center=BPoint((scr->Frame().Width()-Bounds().Width())/2,(scr->Frame().Height()-Bounds().Height())/2);
		MoveTo(center);
	}
	else
	{
		MoveTo(newpos.left,newpos.top);
		ResizeTo(newpos.Width(),newpos.Height());
	}
	Container=new BToolsContainer(Bounds(),"line",0,theme,B_FOLLOW_ALL_SIDES);
	Container->SetViewColor(220,220,220);
	AddChild(Container);
	
	SelectList = new BListView(BRect(8,8,140,Bounds().Height()-8),"Select",B_SINGLE_SELECTION_LIST,B_FOLLOW_ALL_SIDES);
	SelectListScroll=new BScrollView("scroll_select",SelectList,B_FOLLOW_LEFT|B_FOLLOW_TOP_BOTTOM,0,false,true);
	Container->AddChild(SelectListScroll);
	SelectList->SetSelectionMessage(new BMessage(kViewSelectorChanged));
	
	SelectList->AddItem(new IconListItem(theme->GetPNG("Settings: Main"),MES("ConfigWnd:Main"),"",0,0, false));
	SelectList->AddItem(new IconListItem(theme->GetPNG("Settings: Colors"),MES("ConfigWnd:Colors"),"",0, 0, false));
	SelectList->AddItem(new IconListItem(theme->GetPNG("Settings: Info"),MES("ConfigWnd:Info"),"",0, 0, false));
	SelectList->SetViewColor(SETTINGS->GetColor("ViewDlgCol"));
	
	fMainSettingsView=new MainSettingsView(BRect(164,8,Bounds().Width()-8,Bounds().Height()-8));
	Container->AddChild(fMainSettingsView);
	
	fColorSettingsView=new ColorSettingsView(BRect(164,8,Bounds().Width()-8,Bounds().Height()-8));
	Container->AddChild(fColorSettingsView);
	
	fInfoSettingsView=new InfoSettingsView(BRect(164,8,Bounds().Width()-8,Bounds().Height()-8));
	Container->AddChild(fInfoSettingsView);
		
	SelectList->Select(0);
}
//------------------------------------------------------------------------------
void ConfigureWindow::ReloadTheme()
{
	Container->ReloadTheme();

	IconListItem *item=(IconListItem*)SelectList->ItemAt(0);
	item->ChangeIcon(item,Theme->GetPNG("Settings: Main"));
	item=(IconListItem*)SelectList->ItemAt(1);
	item->ChangeIcon(item,Theme->GetPNG("Settings: Colors"));
	item=(IconListItem*)SelectList->ItemAt(2);
	item->ChangeIcon(item,Theme->GetPNG("Settings: Info"));
	SelectList->Invalidate();
}

//------------------------------------------------------------------------------
void ConfigureWindow::RefreshViewsColor()
{
	LockLooper();
	BView *views[]={SelectList,
					fColorSettingsView->fColorCtrl,
					fColorSettingsView->fColorList,
					fInfoSettingsView,
					fColorSettingsView,
					fMainSettingsView,
					fMainSettingsView->fIfaceBox,
					NULL
					};
	for(int i=0;views[i]!=NULL;i++)
		{
		 views[i]->SetViewColor(SETTINGS->GetColor("ViewDlgCol"));
		 for(int j=0;j<views[i]->CountChildren();j++)
		 {
		 	views[i]->ChildAt(j)->SetViewColor(SETTINGS->GetColor("ViewDlgCol"));
		 	views[i]->ChildAt(j)->Invalidate();
		 }
		 views[i]->Invalidate();
		}
	
	UnlockLooper();
}

void ConfigureWindow::RefreshControls()
{
	LockLooper();
	//---update theme menu
	
	BMenu *theme_menu=fMainSettingsView->fCurrentTheme->Menu();
	int n=theme_menu->CountItems();
	for(int i=0;i<n;i++)delete theme_menu->RemoveItem((int32)0);
	
	BDirectory *dir=Theme->GetDirectory();
	BEntry ent;
	char filename[B_FILE_NAME_LENGTH];
	theme_menu->AddItem(new BMenuItem("Default", new BMessage(kThemeChanged)));		
	dir->Rewind();
	
	while (dir->GetNextEntry(&ent) == B_NO_ERROR)
	{
		ent.GetName(filename);
		theme_menu->AddItem(new BMenuItem(filename, new BMessage(kThemeChanged)));	
	}
	
	BMenuItem *item;
	item=theme_menu->FindItem(SETTINGS->GetString("Theme"));
	if (item)item->SetMarked(true);
	else {
			item=theme_menu->FindItem("Default");
			if (item)item->SetMarked(true);
		 }
	
	//---update lang menu
	
	BMenu *lang_menu=fMainSettingsView->fInterfaceLanguage->Menu();
	n=lang_menu->CountItems();
	for(int i=0;i<n;i++)delete lang_menu->RemoveItem((int32)0);
	
	dir=((DjVuApp*)be_app)->Language()->GetDirectory();
	dir->Rewind();
	
	while (dir->GetNextEntry(&ent) == B_NO_ERROR)
	{
		ent.GetName(filename);
		lang_menu->AddItem(new BMenuItem(filename, new BMessage(kLangChanged)));	
	}
	
	item=lang_menu->FindItem(SETTINGS->GetString("Language"));
	if (item)item->SetMarked(true);
	else {
			item=theme_menu->FindItem("English");
			if (item)item->SetMarked(true);
		 }
		 
	UnlockLooper();
}


void ConfigureWindow::MessageReceived(BMessage *msg)
{
	switch(msg->what)
	{
	case 'ReTh':
		{
			BMenuItem *item = fMainSettingsView->fCurrentTheme->Menu()->FindItem(SETTINGS->GetString("Theme"));
			if(item!=NULL)item->SetMarked(true);
			fMainSettingsView->fShowButtonLabel->SetValue(SETTINGS->GetInt32("ShowButtonLabel"));
			ReloadTheme();
			break;
		}
	case kThemeChanged:
		{
			BMenuItem *item = fMainSettingsView->fCurrentTheme->Menu()->FindMarked();
			SETTINGS->SetString("Theme",item->Label());
			Theme->SetTheme(item->Label());
			
			for(int i=0;i<be_app->CountWindows();i++)be_app->WindowAt(i)->PostMessage(new BMessage('ReTh'));
			
			break;
		}
	case kLangChanged:
		{
			BMenuItem *item = fMainSettingsView->fInterfaceLanguage->Menu()->FindMarked();
			SETTINGS->SetString("Language",item->Label());
			SETTINGS->Save(SETTINGSFILE);
		  	
			int32 button_index=(new BAlert(MES("Warning"), MES("Message:001"), MES("Ok"), NULL, NULL, B_WIDTH_AS_USUAL,B_INFO_ALERT))->Go();
			break;
		}
	case kRefreshControls:
		{
			RefreshControls();
			break;	
		}
	case kMainChecker:
		{
			if(SETTINGS->GetInt32("ShowButtonLabel")!=fMainSettingsView->fShowButtonLabel->Value())
				{
					SETTINGS->SetInt32("ShowButtonLabel",fMainSettingsView->fShowButtonLabel->Value());
					MainWindow->PostMessage(new BMessage('ReTh'));
				}
//			SETTINGS->SetInt32("UseToolTips",fMainSettingsView->fUseToolTips->Value());
//			if(ToolTip!=NULL)ToolTip->Enable(SETTINGS->GetInt32("UseToolTips"));			
			break;
		}
	case kColorControlChanged:
		{
			int32 idx=fColorSettingsView->fColorList->CurrentSelection();
			if(idx>=0)
				{
					rgb_color c=fColorSettingsView->fColorCtrl->ValueAsColor();
					SETTINGS->SetColor(ColorAliases[idx].SettingName,&c);
					RefreshViewsColor();
					fColorSettingsView->fColView->SetViewColor(c);
					fColorSettingsView->fColView->Invalidate();
					MainWindow->PostMessage(new BMessage('Refr'));
				}
			break;
		}
	case kColorListChanged:
		{
			int32 idx=fColorSettingsView->fColorList->CurrentSelection();
			if(idx>=0)
				{
					rgb_color color=SETTINGS->GetColor(ColorAliases[idx].SettingName);
					fColorSettingsView->fColorCtrl->SetValue(color);
					fColorSettingsView->fColView->SetViewColor(color);
					fColorSettingsView->fColView->Invalidate();
				}
			break;
		}
	case kViewSelectorChanged:
		{
			RefreshControls();
			
			int idx=SelectList->CurrentSelection();
			if(!fMainSettingsView->IsHidden())fMainSettingsView->Hide();
			if(!fColorSettingsView->IsHidden())fColorSettingsView->Hide();
			if(!fInfoSettingsView->IsHidden())fInfoSettingsView->Hide();
			
			if(idx==0)fMainSettingsView->Show();
			if(idx==1)fColorSettingsView->Show();
			if(idx==2)fInfoSettingsView->Show();
			break;
		}
	default:
		BWindow::MessageReceived(msg);
		break;
	}
}

//------------------------------------------------------------------------------
void
ConfigureWindow::Show(void)
{
	BWindow::Show();
	RefreshControls();
	RefreshViewsColor();
}

bool ConfigureWindow::QuitRequested()
{
	SETTINGS->SetRect("ConfigureWinPosition",Frame());
	SETTINGS->Save(SETTINGSFILE);

	bool isHidden = false;

	if (Lock())
	{
		isHidden = IsHidden();
		Unlock();
	} else return true;
	
	if (isHidden)return true;

	Hide();
	return false;
}

//------------------------------------------------------------------------------

MainSettingsView::MainSettingsView(BRect rect):SettingsView(rect, "MainSettingsView")
{
	font_height FontAttributes;
	be_plain_font->GetHeight(&FontAttributes);
	float FontHeight = ceil(FontAttributes.ascent) + ceil(FontAttributes.descent);
	rect=Bounds();
	float h=FontHeight;
	
	fIfaceBox=new BBox(BRect(rect.left+8,rect.top+8,rect.right-8,rect.top+8),"fIfaceBox");
	AddChild(fIfaceBox);
	fIfaceBox->SetLabel(MES("ConfigWnd:Interface"));
	fIfaceBox->SetResizingMode(B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	
	BPopUpMenu *IntLangMenu = new BPopUpMenu("English");
	IntLangMenu->AddItem(new BMenuItem("English", new BMessage('ChLn')));

	fInterfaceLanguage = new BMenuField(BRect(10,h,580,h+FontHeight), "English", MES("ConfigWnd:IfaceLang"), IntLangMenu);
	fInterfaceLanguage->SetDivider(fInterfaceLanguage->StringWidth(MES("ConfigWnd:IfaceLang"))+8);
	fIfaceBox->AddChild(fInterfaceLanguage);
	fInterfaceLanguage->ResizeToPreferred();
	h+=fInterfaceLanguage->Bounds().Height()+8;
	
	BPopUpMenu *fThemeMenu = new BPopUpMenu("Default");
	fThemeMenu->AddItem(new BMenuItem("Default", new BMessage('ChTh')));

	fCurrentTheme = new BMenuField(BRect(10,h,580,h+FontHeight), "Theme", MES("ConfigWnd:IfaceTheme"), fThemeMenu);
	fCurrentTheme->SetDivider(fCurrentTheme->StringWidth(MES("ConfigWnd:IfaceTheme"))+8);
	fIfaceBox->AddChild(fCurrentTheme);
	fCurrentTheme->ResizeToPreferred();
	h+=fCurrentTheme->Bounds().Height()+8;
	
	fShowButtonLabel = new BCheckBox(BRect(10,h,220,h+FontHeight), "", MES("ConfigWnd:ShButtLabel"),new BMessage(kMainChecker));
	fIfaceBox->AddChild(fShowButtonLabel);
	fShowButtonLabel->SetValue(SETTINGS->GetInt32("ShowButtonLabel"));
	fShowButtonLabel->ResizeToPreferred();
	h+=fShowButtonLabel->Bounds().Height()+2;

/*	fUseToolTips = new BCheckBox(BRect(10,h,220,h+FontHeight), "", MES("ConfigWnd:UseToolTips"),new BMessage(kMainChecker));
	fIfaceBox->AddChild(fUseToolTips);
	fUseToolTips->SetValue(SETTINGS->GetInt32("UseToolTips",true));
	fUseToolTips->ResizeToPreferred();
	h+=fUseToolTips->Bounds().Height()+2;*/

	fIfaceBox->ResizeBy(0,h+FontHeight);

	h=FontHeight;
}

void
MainSettingsView::AttachedToWindow()
{
}

//------------------------------------------------------------------------------

ColorSettingsView::ColorSettingsView(BRect rect):SettingsView(rect, "ColorSettingsView")
{
	ConfigureWindow *CfgWin=(ConfigureWindow*)Window();

	fColorCtrl = new BColorControl(
			BPoint(8,Bounds().Height()/2 + 8),
			B_CELLS_32x8,0,"ColorControl",new BMessage(kColorControlChanged));
	fColorCtrl->SetValue(0);
	fColorCtrl->ResizeToPreferred();
//	fColorCtrl->ResizeTo(Bounds().Width()-16-32,fColorCtrl->Bounds().Height());
	fColorCtrl->MoveTo(8,Bounds().Height()- fColorCtrl->Bounds().Height() - 8);
	fColorCtrl->SetFlags(fColorCtrl->Flags()|B_FOLLOW_BOTTOM|B_FOLLOW_LEFT);
	
	fColorList = new BListView(BRect(8,8,Bounds().Width()-8-B_V_SCROLL_BAR_WIDTH,Bounds().Height()- fColorCtrl->Bounds().Height()-16),
				"ColorItem",B_SINGLE_SELECTION_LIST,B_FOLLOW_ALL_SIDES);
	fColorListScroll=new BScrollView("scroll_color",fColorList,B_FOLLOW_ALL_SIDES,0,false,true);
	fColorList->SetSelectionMessage(new BMessage(kColorListChanged));
	fColorList->SetViewColor(SETTINGS->GetColor("ViewDlgCol"));
	
	fColView= new BTLabel(BRect(fColorCtrl->Frame().right+8,fColorCtrl->Frame().top,Bounds().Width()-8,Bounds().Height()-8),
				"ColorView","",NULL,BL_RECT_FLAG,B_FOLLOW_BOTTOM|B_FOLLOW_LEFT);
	
	fColView->SetViewColor(0,0,0);
	AddChild(fColView);
	AddChild(fColorCtrl);
	AddChild(fColorListScroll);
	
}

void 
ColorSettingsView::AttachedToWindow()
{
	for(int i=0;ColorAliases[i].FullName!=NULL;i++)
	{
		IconListItem *item=new IconListItem(NULL,MES(ColorAliases[i].FullName),"",0, 0, false);
		fColorList->AddItem(item);
	}
	fColorList->Select(0);
	Window()->PostMessage(kColorListChanged);
}

//------------------------------------------------------------------------------

InfoSettingsView::InfoSettingsView(BRect rect):SettingsView(rect, "InfoSettingsView")
{
	
	fInfoText = new BMyTextControl(BRect(10,10,rect.Width()-10,rect.Height()-10),"info_text",B_FOLLOW_ALL_SIDES,B_WILL_DRAW);
	fInfoText->MakeEditable(false);
	fInfoText->SetStylable(true);
	fInfoText->SetWordWrap(true);
	AddChild(fInfoText);	
	BString Txt;
 	Txt.SetTo(MES("Message:002"));
 	Txt.ReplaceAll("\\n","\n");
 	Txt.ReplaceAll("\\t","\t");
	fInfoText->SetText(Txt.String());
	
}

void 
InfoSettingsView::AttachedToWindow()
{

}

