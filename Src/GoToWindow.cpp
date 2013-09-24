#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <StringView.h>
#include <Application.h>
#include <Alert.h>
#include <Message.h>
#include <TextControl.h>
#include <TextView.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <Screen.h>

#include "MainApp.h"
#include "MainWindow.h"
#include "GoToWindow.h"
#include "ToolsContainer.h"
#include "PicButton.h"
#include "Label.h"
#include "Theme.h"
#include "AboutWindow.h"
#include "ConfigureWindow.h"


GoToSettingsView::GoToSettingsView(BRect rect):SettingsView(rect, "GoToSettingsView")
{
	ConfigureWindow *CfgWin=(ConfigureWindow*)Window();

	BTLabel *lab=new BTLabel(BRect(10,14,120,26),"Page number:",MES("GoToWnd:PageNum"),NULL,0,B_FOLLOW_BOTTOM|B_FOLLOW_LEFT);
	
	lab->SetViewColor(SETTINGS->GetColor("ViewDlgCol"));
	BFont font = SETTINGS->GetFont("TVNormalFont");
	lab->SetFont(&font);
	lab->SetTextColor(SETTINGS->GetColor("TextNormCol"));
	AddChild(lab);
	
	NumLine =  new BTextControl(BRect(130,10,Bounds().right-10,26),"line","","1",NULL);
	NumLine->SetDivider(0);
	NumLine->SetViewColor(SETTINGS->GetColor("ViewDlgCol"));
	AddChild(NumLine);
	
	
	BButton *cancelButton = new BButton(BRect(40,40,120,55),"Cancel",MES("Cancel"),new BMessage('QUTW'),B_FOLLOW_LEFT);
	AddChild(cancelButton);
	BButton *okButton = new BButton(BRect(140,40,220,55),"Ok",MES("Ok"),new BMessage('OkOk'),B_FOLLOW_LEFT);
	okButton->MakeDefault(true);
	AddChild(okButton);
	
}

void 
GoToSettingsView::AttachedToWindow()
{
	NumLine->MakeFocus(true);
}




GoToWindow::GoToWindow(int32 idx,BWindow *MainWnd, BTheme *theme)
:BWindow(BRect(0,0,280,96),MES("WT:GoToPage"),B_TITLED_WINDOW_LOOK,B_NORMAL_WINDOW_FEEL,0)//B_WILL_ACCEPT_FIRST_CLICK)
{	

//	SetSizeLimits(100,1024,32,1024);

	Idx=idx;
	Theme=theme;
	BView *view;
	BScreen *scr=new BScreen(B_MAIN_SCREEN_ID);
	
	MainWindow=MainWnd;
	flag1=PB_MOUSEOVER|PB_LABEL;
	
	SetFlags(B_NOT_RESIZABLE|Flags());

	float x,y;
	
	x=MainWnd->Frame().left+250;
	y=MainWnd->Frame().top+250;
	
	MoveTo(x,y);
	
	BRect rect=Bounds();
	view=new BView(Bounds(),"main_view",B_FOLLOW_ALL_SIDES,0);
	view->SetViewColor(255,255,255);
	AddChild(view);

	line=new BToolsContainer(Bounds(),"line",0,theme,B_FOLLOW_ALL_SIDES);
	view->AddChild(line);

	box=new GoToSettingsView(BRect(10,10,rect.right-10,rect.bottom-10));
	line->AddChild(box);	
				
	delete scr;
	
}

GoToWindow::~GoToWindow()
{
}


void GoToWindow::RefreshViewsColor()
{
	LockLooper();
	BView *views[]={box,
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

void
GoToWindow::Show(void)
{
	BWindow::Show();
	RefreshViewsColor();
}

void 
GoToWindow::WindowActivated(bool active)
{
}

void GoToWindow::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
		case 'ReTh':
		{
			line->ReloadTheme();
			break;
		}
		case 'RefW':
			{
				RefreshViewsColor();
				break;
			}
			
		case 'QUTW':
			{
				Quit();
				break;
			}
		case 'OkOk':
			{
			    int page=1;
			    page=atoi(box->NumLine->Text());
			    
				BMessage *mes=new BMessage('GoNN');
				mes->AddInt32("pagenum",page);
				MainWindow->PostMessage(mes);
				Quit();
				break;
			}
		default:
			BWindow::MessageReceived(message);
			break;
	}
}

bool	GoToWindow::QuitRequested()
{
	Quit();
	return(TRUE);
}

