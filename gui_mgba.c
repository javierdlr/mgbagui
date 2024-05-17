#define CATCOMP_NUMBERS
//#define CATCOMP_BLOCK
//#define CATCOMP_CODE
extern struct LocaleInfo li;


#include "includes.h"
#include "debug.h"


extern BOOL OpenLibs(void);
extern void CloseLibs(void);
extern void CreateGUIwindow(struct MGBAGUI *);
//extern uint32 DoMessage(char *, char, STRPTR);
extern struct Screen *FrontMostScr(void);
//extern void GetGUIGadgets(void);
//extern int32 GetRoms(STRPTR romsdir, struct List *);
extern STRPTR DupStr(CONST_STRPTR str, int32 length);
extern VOID FreeString(STRPTR *string);
extern void free_chooserlist_nodes(struct List *);


const char *version = VERSTAG;


extern struct IconIFace *IIcon;
extern struct DOSIFace *IDOS;
extern struct IntuitionIFace *IIntuition;
//extern struct GraphicsIFace *IGraphics;
extern struct UtilityIFace *IUtility;

extern struct ListBrowserIFace *IListBrowser;

//extern Object *Objects[LAST_NUM];


/************/
/* GUI MAIN */
/************/
int gui_main(struct WBStartup *wbs)
{
	int32 error = -1;
	//char text_buf[256];
	struct DiskObject *micon = NULL;
	STRPTR ttp;
DBUG("*** START "VERS" ***\n",NULL);

	if(OpenLibs() == TRUE) {
		struct MGBAGUI *GUI = IExec->AllocVecTags(sizeof(struct MGBAGUI), AVT_ClearWithValue,NULL, TAG_END);

		GUI->myTT.romsdrawer = DupStr( ROMS, sizeof(ROMS) );
		//GUI->myTT.newttp  = IExec->AllocVecTags(MAX_DOS_PATH, AVT_ClearWithValue,NULL, TAG_END);
		//GUI->myTT.ttpBuf1 = IExec->AllocVecTags(MAX_DOS_PATH, AVT_ClearWithValue,NULL, TAG_END);
		//GUI->myTT.ttpBuf2 = IExec->AllocVecTags(MAX_DOS_PATH, AVT_ClearWithValue,NULL, TAG_END);
		GUI->myTT.show_hints = FALSE;
		GUI->myTT.guifade    = TRUE;
GUI->myTT.autosnapshot = FALSE;
		GUI->forcesize = FALSE;
		GUI->myTT.forcesize_w = 480;
		GUI->myTT.forcesize_h = 432;
		GUI->myTT.viewmode   = 1;

DBUG("WBStartup = 0x%08lx\n",wbs);
		if(wbs) // launched from WB/icon
		{
			GUI->wbs = wbs;
			// Reset icon X/Y positions so it iconifies properly on Workbench
			GUI->iconify = IIcon->GetIconTags(wbs->sm_ArgList->wa_Name, ICONGETA_FailIfUnavailable,FALSE, TAG_END);
			GUI->iconify->do_CurrentX = NO_ICON_POSITION;
			GUI->iconify->do_CurrentY = NO_ICON_POSITION;

			// Read tooltypes
			micon = IIcon->GetDiskObjectNew(wbs->sm_ArgList->wa_Name);
//DBUG("micon 0x%08lx\n",micon);
			if(micon)
			{
				ttp = IIcon->FindToolType(micon->do_ToolTypes, "PUBSCREEN");
				if(ttp)
				{
DBUG("SCREEN tooltype is '%s'\n",ttp);
					GUI->screen = IIntuition->LockPubScreen(ttp);
				}

				ttp = IIcon->FindToolType(micon->do_ToolTypes, "ROMS_DRAWER");
				if(ttp)
				{
					FreeString(&GUI->myTT.romsdrawer);
					GUI->myTT.romsdrawer = DupStr( ttp, IUtility->Strlen(ttp) );
				}
DBUG("ROMS_DRAWER tooltype is '%s'\n",GUI->myTT.romsdrawer);

				ttp = IIcon->FindToolType(micon->do_ToolTypes, "LAST_ROM_LAUNCHED");
				if(ttp)
				{
					IDOS->StrToLong(ttp, &GUI->myTT.last_rom_run);
DBUG("LAST_ROM_LAUNCHED tooltype is %ld\n",GUI->myTT.last_rom_run);
					--GUI->myTT.last_rom_run; // listbrowser starts from 0
				}

				/*ttp = IIcon->FindToolType(micon->do_ToolTypes, "DGEN_SDL");
				if(ttp)
				{
					//IDOS->StrToLong(ttp, &DGenG->myTT.dgensdl_exec);
					DGenG->myTT.dgensdl_exec = ttp[0] - 0x30; // lazy way to convert 1 char -> int32
					if(DGenG->myTT.dgensdl_exec != 2) { DGenG->myTT.dgensdl_exec = 1; }
					//if(DGenG->myTT.dgensdl_exec!=1  &&  DGenG->myTT.dgensdl_exec != 2) { DGenG->myTT.dgensdl_exec = 0; }
DBUG("DGEN_SDL tooltype is %ld\n",DGenG->myTT.dgensdl_exec);
				}*/

				/*ttp = IIcon->FindToolType(micon->do_ToolTypes, "FORCE_LOWRES");
				if(ttp)
				{
					//IDOS->StrToLong(ttp, &DGenG->myTT.force_lowres);
					DGenG->myTT.force_lowres = ttp[0] - 0x30; // lazy way to convert 1 char -> int32
					if(DGenG->myTT.force_lowres<0  &&  DGenG->myTT.force_lowres>2) { DGenG->myTT.force_lowres = 0; }
DBUG("FORCE_LOWRES tooltype is %ld\n",DGenG->myTT.force_lowres);
				}*/

				ttp = IIcon->FindToolType(micon->do_ToolTypes, "SHOW_HINTS");
				if(ttp) { GUI->myTT.show_hints = TRUE; }

				ttp = IIcon->FindToolType(micon->do_ToolTypes, "NO_GUI_FADE");
				if(ttp)
				{
					GUI->myTT.guifade = FALSE;
DBUG("NO_GUI_FADE tooltype enabled\n");
				}

ttp = IIcon->FindToolType(micon->do_ToolTypes, "AUTO_SNAPSHOT");
if(ttp) { GUI->myTT.autosnapshot = TRUE; }
DBUG("AUTO_SNAPSHOT tooltype is %ld\n",GUI->myTT.autosnapshot);

				ttp = IIcon->FindToolType(micon->do_ToolTypes, "FORCESIZE_W");
				if(ttp) {
					IDOS->StrToLong(ttp, &GUI->myTT.forcesize_w);
					if(GUI->myTT.forcesize_w < FORCESIZE_W_MIN) { GUI->myTT.forcesize_w = FORCESIZE_W_MIN; }
DBUG("FORCESIZE_W tooltype is %ld\n",GUI->myTT.forcesize_w);
					GUI->forcesize = TRUE;
				}

				ttp = IIcon->FindToolType(micon->do_ToolTypes, "FORCESIZE_H");
				if(ttp) {
					IDOS->StrToLong(ttp, &GUI->myTT.forcesize_h);
					if(GUI->myTT.forcesize_h < FORCESIZE_H_MIN) { GUI->myTT.forcesize_h = FORCESIZE_H_MIN; }
DBUG("FORCESIZE_H tooltype is %ld\n",GUI->myTT.forcesize_h);
					GUI->forcesize = TRUE;
				}

				ttp = IIcon->FindToolType(micon->do_ToolTypes, "VIEWMODE");
				if(ttp) {
					IDOS->StrToLong(ttp, &GUI->myTT.viewmode);
					if(GUI->myTT.viewmode<1  ||  GUI->myTT.viewmode>9) { GUI->myTT.viewmode = 1; }
DBUG("VIEWMODE tooltype is %ld\n",GUI->myTT.viewmode);
				}

				IIcon->FreeDiskObject(micon);
			}
		}

		if(GUI->screen == NULL) { GUI->screen = IIntuition->LockPubScreen(NULL); }
DBUG("pubscreen=0x%lx '%s'\n",GUI->screen,GUI->screen->Title);

		GUI->romlist = IExec->AllocSysObject(ASOT_LIST, NULL);
		GUI->savestates_list = IExec->AllocSysObject(ASOT_LIST, NULL);
		//DGenG->game_opts_list = IExec->AllocSysObject(ASOT_LIST, NULL);

		CreateGUIwindow(GUI);

		free_chooserlist_nodes(GUI->savestates_list);
		IExec->FreeSysObject(ASOT_LIST, GUI->savestates_list);
		GUI->savestates_list = NULL;

		/*free_chooserlist_nodes(DGenG->game_opts_list);
		IExec->FreeSysObject(ASOT_LIST, DGenG->game_opts_list);
		DGenG->game_opts_list = NULL;*/

		IListBrowser->FreeListBrowserList(GUI->romlist);
		IExec->FreeSysObject(ASOT_LIST, GUI->romlist);
		GUI->romlist = NULL;

		IIntuition->UnlockPubScreen(NULL, GUI->screen);

		FreeString(&GUI->myTT.romsdrawer);

		//IExec->FreeVec(DGenG->myTT.newttp);
		//IExec->FreeVec(DGenG->myTT.ttpBuf1);
		//IExec->FreeVec(DGenG->myTT.ttpBuf2);

		IExec->FreeVec(GUI);
	}

	CloseLibs();
DBUG("*** END "VERS" ***\n",NULL);

	return error;
}
