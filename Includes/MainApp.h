#ifndef DjVuApp_H
#define DjVuApp_H

#include <Application.h>
#include <Deskbar.h>
#include <Roster.h>
#include <Font.h>
#include <Alert.h>
#include "MainWindow.h"
#include "Label.h"
#include "Theme.h"
#include "Settings.h"
#include "Language.h"


const uint32 MSG_FILE_OPEN			= 'mFOP';
const uint32 MSG_CLOSE				= 'mCLS';
const uint32 MSG_WINDOW_QUIT		= 'mWQT';
const uint32 MSG_UPDATE_STATUS		= 'mUPS';
const uint32 MSG_INVALIDATE         = 'mIVD';
const uint32 MSG_SELECTION			= 'mSEL';
const uint32 MSG_SELECTION_BITMAP	= 'mSBT';
const uint32 MSG_MODIFIED			= 'mMOD';


#define my_app	((DjVuApp*)be_app)
#define SETTINGSFILE	"/boot/home/config/settings/DjVuViewer"
#define APPSIGMATURE	"application/x-vnd.DjVuViewer"
#define HOMEPAGE		"http://www.haikuware.ru"

#define SETTINGS (((DjVuApp*)be_app)->Settings())
#define THEME (((DjVuApp*)be_app)->Theme())
#define MES(str) ((((DjVuApp*)be_app)->Language())->GetString(str))

class DjVuApp : public BApplication {
	public:
		DjVuApp(int argc, char **argv);
		~DjVuApp();
		virtual void MessageReceived(BMessage *message);
		PSettings *Settings(void);
		BLang *Language(void);
		BTheme *Theme(void);
		BPath	  Path(const char *dir);
		void	ArgvReceived(int32 argc, char **argv);
		void	RefsReceived(BMessage *pmsg);
		void	OpenFile(const entry_ref *pref);
		void 	InstallMimeType(const char*mimetxt);
		virtual void ReadyToRun();
		void Quit();
		virtual void Pulse();
	private:
		void StartPulse();
		PSettings	ProgramSettings;
		BLang	CurrentLanguage;
		BTheme		appTheme;
		BMessenger  fTrackerMessenger;
		BFilePanel  *fOpenPanel;
		bool fPulseStarted;
};


#endif
