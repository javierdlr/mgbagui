#ifndef INCLUDE_H
#define INCLUDE_H


#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/listbrowser.h>
//#include <proto/clicktab.h>
#include <proto/chooser.h>
#include <proto/layout.h>
#include <proto/graphics.h>
#include <proto/icon.h>
//#include <proto/locale.h>
#include <proto/amigainput.h>

#include <workbench/icon.h>
#include <workbench/startup.h>
#include <libraries/keymap.h> // RAWKEY_#? codes
#include <classes/window.h>
#include <classes/requester.h>
//#include <gadgets/clicktab.h>
#include <gadgets/layout.h>
#include <gadgets/button.h>
#include <gadgets/checkbox.h>
#include <gadgets/chooser.h>
//#include <gadgets/string.h>
#include <gadgets/space.h>
#include <gadgets/integer.h>
#include <gadgets/getfile.h>
#include <gadgets/slider.h>
#include <images/label.h>
#include <images/bitmap.h>

#include "mgbagui_rev.h"
#include "mgbagui_strings.h"


#define OBJ(x) Objects[x]
#define GAD(x) (struct Gadget *)Objects[x]

#define ROMS      "PROGDIR:ROMS"
#define PREVIEWS  "PROGDIR:PREVIEWS"
#define SCRSHOTS  "PROGDIR:mGBAConfig/screenshots"
#define SAVES     "PROGDIR:mGBAConfig/ram"

#define CONFIG_INI "PROGDIR:mGBAConfig/config.ini"
#define GPAD_IMG "PROGDIR:mGBAConfig/keymap.png"
#define GPAD_BG  "PROGDIR:mGBAConfig/keymap_bg.png"
#define GPAD_STR "PROGDIR:mGBAConfig/keymap_str.png"

// gui_build.c: 'make_chooser_list2(<mode>,..)'
#define ADD_LIST 0
#define NEW_LIST 1


enum {
 COL_ROM = 0,
 COL_FMT, // file extension
 LAST_COL
};

enum {
 WID_MAIN = 0,
 WID_GPAD,
 LAST_WIN
};

enum {
 OID_MAIN = 0,
 OID_BANNER_IMG,
 OID_ROMDRAWER,
 //OID_MGBAEXE,
 OID_LISTBROWSER,
 OID_PREVIEW_BTN,
 OID_PREVIEW_IMG, // MUST BE after OID_PREVIEW_BTN [updateButtonImage()]
//OID_GAME_OPTIONS,
//OID_OPTIONS_GROUP,
 OID_SAVESTATES,
 OID_GFXSCALE,
 OID_FULLSCR,
 OID_FRAMESKIP,
OID_GAMEPAD_BTN,
 OID_FORCESIZE_GROUP,
 OID_FORCESIZE,
 OID_FS_WIDTH,
 OID_FS_HEIGHT,
 OID_TOTALROMS,
// Gamepad window and buttons
OID_GPAD,
OID_GPAD_NO_BTN = OID_GPAD,
OID_GPAD_L,
OID_GPAD_R,
OID_GPAD_B,
OID_GPAD_A,
OID_GPAD_SEL,
OID_GPAD_STA,
OID_GPAD_TEXT,
OID_GPAD_IMG,
OID_GPAD_STR,
OID_GPAD_SAVE,
OID_GPAD_CANCEL,
 // Buttons
 OID_ABOUT,
 OID_SAVE,
 OID_QUIT,
 LAST_OID
};

// Listbrowser's pens
/*enum {
 ROW_O = 0,
 ROW_E,
 TXT_R,
 LAST_PEN
};
#define RGB8to32(RGB) ( (uint32)(RGB) * 0x01010101UL )*/


struct myToolTypes {
	STRPTR romsdrawer;  // ROMS_DRAWER=<path>
	int32 last_rom_run; // LAST_ROM_LAUNCHED=<value>
	BOOL show_hints;    // SHOW_HINTS
	//STRPTR newttp, ttpBuf1, ttpBuf2; // only needed if using SaveToolType()
	BOOL guifade;       // NO_GUI_FADE
	//BOOL autosnapshot;  // AUTO_SNAPSHOT
	int32 forcesize_w, forcesize_h; // FORCESIZE_W and FORCESIZE_H
	int32 viewmode;     // VIEWMODE
};

#define DISABLE_TT "*"      // "special token" used in SaveToolTypes() to
#define DISABLE_TT_CHAR '*' // disable tooltype: tooltype[=value] -> (tooltype[=value])

#define FORCESIZE_W_MIN 480
#define FORCESIZE_H_MIN 320

#define CFG_FILE_SIZE 4096 // max. size of config.ini in memory (array)

struct MGBAGUI {
	struct Screen *screen; // PUSBCREEN=<screen_name>
	struct DiskObject *iconify;
	struct List *romlist;
	struct Window *win[LAST_WIN];
	struct myToolTypes myTT;
	struct WBStartup *wbs;
	struct List *savestates_list;//, *game_opts_list;
	BOOL forcesize;
STRPTR cfg_file;
int32 keyV[6]; // gamepad buttons: L R B A Select Start
struct MsgPort *ai_port; // amigainput message port
};


#endif
