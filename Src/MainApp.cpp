#include <Application.h>
#include <Deskbar.h>
#include <Roster.h>
#include <Font.h>
#include <Alert.h>
#include <FilePanel.h>
#include <Mime.h>

#include "MainWindow.h"
#include "Label.h"
#include "Theme.h"
#include "MainApp.h"

#define WINDOWS_TO_IGNORE 1

sem_id render_sem;	

DjVuApp::DjVuApp(int argc, char **argv) : BApplication("application/x-vnd.DjVuViewer")
{
	fPulseStarted = false;
	fOpenPanel = new BFilePanel(B_OPEN_PANEL);
	//fOpenPanel->Window()->SetTitle("Open file...");
	if(SETTINGS->Load(SETTINGSFILE)==false)
		{
		SETTINGS->SetColor("CursBarCol",0,74,245);
		SETTINGS->SetColor("CursFrameCol",34,34,34);
		SETTINGS->SetColor("CursTextCol",255,255,255);
		SETTINGS->SetColor("EvenLinesCol",215,215,215);
		SETTINGS->SetColor("EvenTextCol",0,0,0);
		SETTINGS->SetColor("OddLinesCol",236,236,236);
		SETTINGS->SetColor("OddTextCol",0,0,0);
		SETTINGS->SetColor("ThumbBackCol",220,220,220);
		SETTINGS->SetColor("ThumbTextCol",0,0,0);
		

		SETTINGS->SetColor("ViewFrameCol",128,128,128);
		SETTINGS->SetColor("ViewDlgCol",216,216,216);
		SETTINGS->SetColor("TextNormCol",64,64,64);
	
		SETTINGS->SetFont("WLListFont","","",13,90);
		SETTINGS->SetFont("FVFont","","",12,90);
		SETTINGS->SetFont("TVNormalFont","","",12,90);

		SETTINGS->SetInt32("StorePosition",0);
		SETTINGS->SetInt32("ShowButtonLabel",1);
		
		SETTINGS->SetString("Theme","Default");
		SETTINGS->SetString("Language","English");

		SETTINGS->SetBool("Dither",false);
		SETTINGS->SetBool("ShrinkToBounds",false);
		SETTINGS->SetBool("ZoomToBounds",false);
		SETTINGS->SetBool("ScaleBilinear",true);
	
		SETTINGS->Save(SETTINGSFILE);
		}

	CurrentLanguage.SetDirectory("./Languages");
	CurrentLanguage.SetLanguage(SETTINGS->GetString("Language"));
	CurrentLanguage.SetDefaultLanguage("English");
	
	appTheme.SetDirectory("./Themes");
	appTheme.SetTheme(SETTINGS->GetString("Theme"));
	
	update_font_families(false);

//	ToolTip=new BTip();
	//if(ToolTip!=NULL)ToolTip->Enable(SETTINGS->GetInt32("UseToolTips",true));
	
	if((render_sem=create_sem(1,"render_sem"))<B_NO_ERROR)Quit();

}

DjVuApp::~DjVuApp()
{
 delete_sem(render_sem);
}

void DjVuApp::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case MSG_FILE_OPEN:
			fOpenPanel->Show();
			break;

	case B_CANCEL:
			// File open panel was closed,
			// start checking count of open windows
			StartPulse();
			break;
	default:
		BApplication::MessageReceived(message);
		break;
	}
}

void
DjVuApp::StartPulse()
{
	if (!fPulseStarted) {
		// Tell the app to begin checking
		// for the number of open windows
		fPulseStarted = true;
		SetPulseRate(250000);
			// Set pulse to every 1/4 second
	}
}

void
DjVuApp::Pulse()
{
	// Bug: The BFilePanel is automatically closed if the volume that
	// is displayed is unmounted.
	if (!IsLaunching() && CountWindows() <= WINDOWS_TO_IGNORE)
		// If the application is not launching and
		// all windows are closed except for the file open panel,
		// quit the application
		PostMessage(B_QUIT_REQUESTED);
}

PSettings *
DjVuApp::Settings(void)
{
	return &ProgramSettings;
}
BTheme *
DjVuApp::Theme(void)
{
	return &appTheme;
}

BLang *
DjVuApp::Language(void)
{
	return &CurrentLanguage;
}

BPath
DjVuApp::Path(const char *dir)
{
	app_info inf;
   	be_app->GetAppInfo(&inf);
   	BPath bpath = BPath(&(inf.ref));
   	BPath par; 
   	bpath.GetParent(&par);
   	par.Append(dir);
	return(par);
}

void
DjVuApp::RefsReceived(BMessage *pmsg)
{
	if (pmsg->HasMessenger("TrackerViewToken")) {
		pmsg->FindMessenger("TrackerViewToken", &fTrackerMessenger);
	}

	uint32 type;
	int32 count;
	status_t ret = pmsg->GetInfo("refs", &type, &count);
	if (ret != B_OK || type != B_REF_TYPE)
		return;

	entry_ref ref;
	for (int32 i = 0; i < count; i++) {
   		if (pmsg->FindRef("refs", i, &ref) == B_OK)
   			OpenFile(&ref);
   	}
}


void
DjVuApp::ArgvReceived(int32 argc, char **argv)
{
	BMessage *pmsg = NULL;
	for (int32 i = 1; i < argc; i++) {
		entry_ref ref;
		status_t err = get_ref_for_path(argv[i], &ref);
		if (err == B_OK) {
			if (!pmsg) {
				pmsg = new BMessage;
				pmsg->what = B_REFS_RECEIVED;
			}
			pmsg->AddRef("refs", &ref);
		}
	}
	if (pmsg) {
		RefsReceived(pmsg);
		delete pmsg;
	}
}

void
DjVuApp::InstallMimeType(const char *mimetxt)
{
	// install mime type of documents
	BMimeType mime(mimetxt);
	status_t ret = mime.InitCheck();
	if (ret != B_OK) {
		fprintf(stderr, "Could not init native document mime type (%s): %s.\n",
			mimetxt, strerror(ret));
		return;
	}

	if (mime.IsInstalled() && !(modifiers() & B_SHIFT_KEY)) {
		// mime is already installed, and the user is not
		// pressing the shift key to force a re-install
		return;
	}

	ret = mime.Install();
	if (ret != B_OK && ret != B_FILE_EXISTS) {
		fprintf(stderr, "Could not install native document mime type (%s): %s.\n",
			mimetxt, strerror(ret));
		return;
	}
	// set preferred app
	ret = mime.SetPreferredApp(APPSIGMATURE);
	if (ret != B_OK) {
		fprintf(stderr, "Could not set native document preferred app: %s\n",
			strerror(ret));
	}

	// set descriptions
	ret = mime.SetShortDescription("DjVu Document");
	if (ret != B_OK) {
		fprintf(stderr, "Could not set short description of mime type: %s\n",
			strerror(ret));
	}
	ret = mime.SetLongDescription("DjVu Document");
	if (ret != B_OK) {
		fprintf(stderr, "Could not set long description of mime type: %s\n",
			strerror(ret));
	}

	// set extensions
	BMessage message('extn');
	message.AddString("extensions", "djvu");
	message.AddString("extensions", "djv");
	ret = mime.SetFileExtensions(&message);
	if (ret != B_OK) {
		fprintf(stderr, "Could not set extensions of mime type: %s\n",
			strerror(ret));
	}

	// set sniffer rule
	char snifferRule[32];
	ret = mime.SetSnifferRule("0.50 (\"AT&TFORM\")");
	if (ret != B_OK) {
		BString parseError;
		BMimeType::CheckSnifferRule(snifferRule, &parseError);
		fprintf(stderr, "Could not set sniffer rule of mime type: %s\n",
			parseError.String());
	}

	// set mime icon
	BResources* resources = AppResources();
		// does not need to be freed (belongs to BApplication base)
	if (resources != NULL) {
		size_t size;
		const void* iconData = resources->LoadResource('VICN', "BEOS:image/x-djvu",
			&size);
		if (iconData != NULL && size > 0) {
			if (mime.SetIcon(reinterpret_cast<const uint8*>(iconData), size)
				!= B_OK) {
				fprintf(stderr, "Could not set vector icon of mime type.\n");
			}
		} else {
			fprintf(stderr, "Could not find icon in app resources "
				"(data: %p, size: %ld).\n", iconData, size);
		}
	} else
		fprintf(stderr, "Could not find app resources.\n");
}


void
DjVuApp::ReadyToRun()
{
	InstallMimeType("image/x-djvu");
	
	if (CountWindows() == WINDOWS_TO_IGNORE)
		fOpenPanel->Show();
	else
		StartPulse();
}

void
DjVuApp::OpenFile(const entry_ref *pref)
{
	DjVuWindow *window=new DjVuWindow("DjVu Viewer:");
	BPath path;
	BEntry entry=BEntry(pref);
	entry.GetPath(&path);
	window->LoadFile(path.Path());
	window->Show();
}

void
DjVuApp::Quit()
{
	BApplication::Quit();
}


int main(int argc, char **argv)
{
	DjVuApp application(argc,argv);	
	
	application.Run();
	return 0;
}


