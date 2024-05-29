#define CATCOMP_NUMBERS
//#define CATCOMP_BLOCK
//#define CATCOMP_CODE
extern struct LocaleInfo li;

#include "includes.h"
#include "debug.h"


void CreateGUIwindow(struct MGBAGUI *);
BOOL ProcessGUI(struct MGBAGUI *);
int32 beginCommand(STRPTR rom_file, STRPTR rom_ext);
void updateList(struct MGBAGUI *);
BOOL make_chooser_list2(BOOL, struct List *, int32, int32); // using CATCOMP_NUMBERS and index

extern uint32 DoMessage(char *message, char reqtype, STRPTR buttons);
extern int32 GetRoms(STRPTR const romsdir, struct List *);
extern void LaunchRom(struct MGBAGUI *);
extern void ShowPreview(struct MGBAGUI *);
extern STRPTR DupStr(CONST_STRPTR str, int32 length);
extern VOID FreeString(STRPTR *string);
extern int32 SaveToolType(STRPTR iconname, STRPTR ttpName, STRPTR ttpArg);
extern void free_chooserlist_nodes(struct List *);
extern void openGamepadWin(struct MGBAGUI *);


extern struct IconIFace *IIcon;
extern struct DOSIFace *IDOS;
extern struct IntuitionIFace *IIntuition;
extern struct GraphicsIFace *IGraphics;
extern struct UtilityIFace *IUtility;

// the class pointer
extern Class /**ClickTabClass,*/ *ListBrowserClass, *ButtonClass, *LabelClass, *GetFileClass,
             *CheckBoxClass, *ChooserClass, *BitMapClass, *LayoutClass, *WindowClass,
             *RequesterClass, *SpaceClass, *IntegerClass, *GetFileClass, *SliderClass;
// some interfaces needed
//extern struct ListBrowserIFace *IListBrowser;
//extern struct ClickTabIFace *IClickTab;
//extern struct LayoutIFace *ILayout;
extern struct ChooserIFace *IChooser;

extern struct WBStartup *WBenchMsg;


Object *Objects[LAST_OID];
uint32 res_prev; // avoid "reload" already selected ROM
char win_fs[5] = ""; // used in win_fs_lvlFunc()


#define CMDLINE_LENGTH 2048
int32 beginCommand(STRPTR rom_file, STRPTR rom_ext)
{
	STRPTR cmdline = IExec->AllocVecTags(CMDLINE_LENGTH, TAG_END);
	uint32 res_val;
	int32 res_value = 0;
	struct Node *res_nod;
DBUG("beginCommand()\n",NULL);
	// Check if romfile exists
	IUtility->Strlcpy(cmdline, rom_file, CMDLINE_LENGTH);
	IUtility->Strlcat(cmdline, rom_ext, CMDLINE_LENGTH);
	struct ExamineData *dat = IDOS->ExamineObjectTags(EX_StringNameInput,cmdline, TAG_END);
	if(dat == NULL) {
		IExec->FreeVec(cmdline);
		return -1;
	}
DBUG("  ExamineObjectTags(): '%s' %s%lld bytes\n",dat->Name,"",dat->FileSize);
	IDOS->FreeDosObject(DOS_EXAMINEDATA, dat);

	// Add executable to commandline string
		IUtility->Strlcpy(cmdline, "mGBA", CMDLINE_LENGTH);

	// Add savestate (if chosen) to commandline string
		IIntuition->GetAttrs(OBJ(OID_SAVESTATES), CHOOSER_Selected,&res_val, CHOOSER_SelectedNode,(uint32*)&res_nod, TAG_DONE);
		if(res_val != 0) {
			CONST_STRPTR filename = IDOS->FilePart(rom_file);
			char savestate_str[2] = "";
			// CNA_UserData holds savestatus index (as char)
			IChooser->GetChooserNodeAttrs(res_nod, CNA_UserData,(APTR)&res_val, TAG_DONE);
			// 0xC0DEDEAD == "auto" entry/item
			if(res_val != 0xC0DEDEAD) { savestate_str[0] = res_val; }
DBUG("  load savestate %lc (0x%08lx)\n",savestate_str[0],res_val);
			IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -t \""SAVES"/%s.ss%s\"",cmdline,filename,savestate_str);
		}

	// Add scale window to commandline string
	//IIntuition->GetAttr(CHOOSER_Selected, OBJ(OID_GFXSCALE), &res_val);
	IIntuition->GetAttr(SLIDER_Level, OBJ(OID_GFXSCALE), &res_val);
	if(res_val != 1) {
		IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -%lX",cmdline,res_val==9? 0xF:res_val);
	}

	// Add frameskip to commandline string
	IIntuition->GetAttr(SLIDER_Level, OBJ(OID_FRAMESKIP), &res_val);
	if(res_val != 0) {
		IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -s %ld",cmdline,res_val);
	}

	// Add force size
	IIntuition->GetAttr(GA_Selected, OBJ(OID_FORCESIZE), &res_val);
	if(res_val == TRUE) {
		uint32 w, h;
		IIntuition->GetAttr(INTEGER_Number, OBJ(OID_FS_WIDTH), &w);
		IIntuition->GetAttr(INTEGER_Number, OBJ(OID_FS_HEIGHT), &h);
DBUG("  OID_FORCESIZE %ld x %ld\n",w,h);
		IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s -C width=%ld -C height=%ld",cmdline,w,h);
	}

	// Add ROM file to commandline string
	IUtility->SNPrintf(cmdline, CMDLINE_LENGTH, "%s \"%s%s\"",cmdline,rom_file,rom_ext);
DBUG("  %s\n",cmdline);

	// Launch 'mgba'
	res_value = IDOS->SystemTags(cmdline, SYS_Input,NULL, SYS_Output,NULL, //SYS_Error,NULL,
	                             NP_Priority,0, SYS_Asynch,FALSE, TAG_END);

	IExec->FreeVec(cmdline);

	return res_value;
}

uint32 selectListEntry(struct Window *pw, uint32 res_val)
{
	IIntuition->SetAttrs(OBJ(OID_LISTBROWSER),
	                     LISTBROWSER_Selected,res_val,
	                     LISTBROWSER_MakeVisible,res_val, TAG_DONE);
	IIntuition->RefreshGadgets(GAD(OID_LISTBROWSER), pw, NULL);

	return res_val;
}

uint32 selectListEntryNode(struct Window *pw, struct Node *n)
{
	uint32 res_val;
	IIntuition->SetAttrs(OBJ(OID_LISTBROWSER),
                      LISTBROWSER_SelectedNode,n,
                      LISTBROWSER_MakeNodeVisible,n, TAG_DONE);
	IIntuition->RefreshGadgets(GAD(OID_LISTBROWSER), pw, NULL);
	IIntuition->GetAttrs(OBJ(OID_LISTBROWSER), LISTBROWSER_Selected,&res_val, TAG_DONE);

	return res_val;
}

BOOL ProcessGUI(struct MGBAGUI *gui)
{
	BOOL done = TRUE;
	uint16 code = 0;
	uint32 result = WMHI_LASTMSG, res_value = 0, res_totnode = 0, res_temp = 0,
	       siggot = 0, wsigmask = 0;
	STRPTR res_str = NULL;

	IIntuition->GetAttr(WINDOW_SigMask, OBJ(OID_MAIN), &wsigmask);
	siggot = IExec->Wait(wsigmask|SIGBREAKF_CTRL_C);

	if(siggot & SIGBREAKF_CTRL_C) { return FALSE; }

	while( (result=IIntuition->IDoMethod(OBJ(OID_MAIN), WM_HANDLEINPUT, &code)) != WMHI_LASTMSG )
	{
//DBUG("result=0x%lx\n",result);
		switch(result & WMHI_CLASSMASK)
		{
			case WMHI_CLOSEWINDOW:
				done = FALSE;
			break;
			case WMHI_ICONIFY:
DBUG("WMHI_ICONIFY (win[WID_MAIN]=0x%08lx)\n",gui->win[WID_MAIN]);
				if( IIntuition->IDoMethod(OBJ(OID_MAIN), WM_ICONIFY) ) {
					gui->win[WID_MAIN] = NULL;
				}
			break;
			case WMHI_UNICONIFY:
				if( (gui->win[WID_MAIN]=(struct Window *)IIntuition->IDoMethod(OBJ(OID_MAIN), WM_OPEN, NULL)) ) {
DBUG("WMHI_UNICONIFY (win[WID_MAIN]=0x%08lx)\n",gui->win[WID_MAIN]);
					gui->screen = gui->win[WID_MAIN]->WScreen;
					IIntuition->ScreenToFront(gui->screen);
				}
				else { done = FALSE; }
			break;
			/*case WMHI_JUMPSCREEN:
				IIntuition->GetAttrs(OBJ(OID_MAIN), WA_PubScreen,&gui->screen, TAG_DONE);
				//IIntuition->SetAttrs(OBJ(OID_MAIN), WA_PubScreen,gui->screen, TAG_DONE);
		break;*/
		case WMHI_VANILLAKEY:
		{
DBUG("[WMHI_VANILLAKEY] = 0x%lx (0x%lx)\n",code,result&WMHI_KEYMASK);
			struct Node *node1 = NULL, *next_node1 = NULL;
			STRPTR node_val, next_n_val;
			char char_node, char_keyb;

			if(code == 0x1b) // ESC
			{
				done = FALSE;
				break;
			}

			if(code == 0x0d) // ENTER/RETURN
			{
				LaunchRom(gui);
				break;
			}

			char_keyb = IUtility->ToUpper(result & WMHI_KEYMASK); // uppercase'd (key pressed)
DBUG("[WMHI_VANILLAKEY] '%lc' (0x%lx)\n",char_keyb,char_keyb);
			IIntuition->GetAttrs(OBJ(OID_LISTBROWSER), LISTBROWSER_SelectedNode,&node1, TAG_DONE);
			IListBrowser->GetListBrowserNodeAttrs(node1, LBNA_Column,COL_ROM, LBNCA_Text,&node_val, TAG_DONE);
			next_node1 = IExec->GetSucc(node1);
			IListBrowser->GetListBrowserNodeAttrs(next_node1, LBNA_Column,COL_ROM, LBNCA_Text,&next_n_val, TAG_DONE);
DBUG("  Actual Node -> Next Node\n",NULL);
DBUG("   0x%08lx -> 0x%08lx\n",node1,next_node1);
DBUG("          '%lC' -> '%lC'\n",*node_val,*next_n_val);
			// SELECT IT: NEXT node starts with KEY pressed and NEXT node = ACTUAL node
			if( char_keyb==IUtility->ToUpper(*next_n_val)  &&  IUtility->ToUpper(*next_n_val)==IUtility->ToUpper(*node_val) )
			{
				res_prev = selectListEntryNode(gui->win[WID_MAIN], next_node1);
				ShowPreview(gui);
			}
			// GO TO KEY PRESSED FIRST NODE: 1)ACTUAL node starts with KEY pressed and NEXT node != ACTUAL node
			// OR 2)pressed another KEY OR 3)reached end of listbrowser (next_node1=NULL)
			if( (char_keyb==IUtility->ToUpper(*node_val)  &&  IUtility->ToUpper(*next_n_val)!=IUtility->ToUpper(*node_val))
			   ||  char_keyb!=IUtility->ToUpper(*node_val)  ||  next_node1==NULL )
			{
				next_node1 = node1; // avoid refreshing/reloading single ROM filename entries
				for(node1=IExec->GetHead(gui->romlist); node1!=NULL; node1=IExec->GetSucc(node1) ) {
					IListBrowser->GetListBrowserNodeAttrs(node1, LBNA_Column,COL_ROM, LBNCA_Text,&res_str, TAG_DONE);
					char_node = IUtility->ToUpper(*res_str); // uppercased (romfile 1st letter)
					if(char_node==char_keyb  &&  node1!=next_node1) {
						res_prev = selectListEntryNode(gui->win[WID_MAIN], node1);
						ShowPreview(gui);
						node1 = NULL;
					}
				}
			}

		}
		break;
		case WMHI_RAWKEY:
		{
			int32 sel_entry = -1; // -1: key pressed not valid
DBUG("[WMHI_RAWKEY] 0x%lx (win[WID_MAIN]=0x%08lx)\n",code,gui->win[WID_MAIN]);
			/*if(code == RAWKEY_ESC)
			{
				done = FALSE;
				break;
			}*/

			IIntuition->GetAttrs(OBJ(OID_LISTBROWSER), LISTBROWSER_Selected,&res_value,
			                     LISTBROWSER_TotalNodes,&res_totnode, TAG_DONE);
DBUG("  sel=%ld  nodes=%ld\n",res_value,res_totnode);
			// HOME key
			if(code==RAWKEY_HOME  &&  res_value!=0) {
				sel_entry = 0;
			}
			// END key
			if(code==RAWKEY_END  &&  res_value!=res_totnode-1) {
				sel_entry = res_totnode - 1;
			}
			// CuRSOR UP key
			if(code==CURSORUP  &&  res_value!=0) {
						sel_entry = res_value - 1;
			}
			// PAGE UP key
			if(code==RAWKEY_PAGEUP  &&  res_value!=0) {
				IIntuition->GetAttrs(OBJ(OID_LISTBROWSER), LISTBROWSER_Top,&res_value, LISTBROWSER_Bottom,&res_temp, TAG_DONE);
				sel_entry = res_temp - res_value;
				sel_entry = res_value - sel_entry;
//DBUG("  %ld\n",sel_entry);
				if(sel_entry < 0) { sel_entry = 0; }
			}
			// CURSOR DOWN key
			if(code==CURSORDOWN  &&  res_value!=res_totnode-1) {
				sel_entry = res_value + 1;
			}
			// PAGE DOWN key
			if(code==RAWKEY_PAGEDOWN  &&  res_value!=res_totnode-1) {
				IIntuition->GetAttrs(OBJ(OID_LISTBROWSER), LISTBROWSER_Bottom,&res_value, TAG_DONE);
				sel_entry = res_value;
			}

			if(sel_entry != -1) {
				res_prev = selectListEntry(gui->win[WID_MAIN], sel_entry);
				ShowPreview(gui);
			}

			// RETURN/ENTER key
			/*if(code==RAWKEY_RETURN  ||  code==RAWKEY_ENTER)
			{
				LaunchRom(dgg);
			}*/

		}
		break;
		case WMHI_GADGETUP:
DBUG("[WMHI_GADGETUP] code = %ld (0x%08lx)\n",code,code);
			switch(result & WMHI_GADGETMASK)
			{
				case OID_ROMDRAWER:
					res_value = IIntuition->IDoMethod(OBJ(OID_ROMDRAWER), GFILE_REQUEST, gui->win[WID_MAIN]);
					if(res_value) {
						updateList(gui);
						// Sort and refresh new list and chooser info
						IIntuition->DoGadgetMethod(GAD(OID_LISTBROWSER), gui->win[WID_MAIN], NULL,
						                           LBM_SORT, NULL, COL_ROM, LBMSORT_FORWARD, NULL);
						IIntuition->RefreshSetGadgetAttrs(GAD(OID_LISTBROWSER), gui->win[WID_MAIN], NULL,
                                        LISTBROWSER_Selected,0, LISTBROWSER_MakeVisible,0, TAG_DONE);
						// Reset selected rom to 1st entry
						res_prev = 0;
						ShowPreview(gui);
					}
				break;
				case OID_LISTBROWSER:
					IIntuition->GetAttrs(OBJ(OID_LISTBROWSER), LISTBROWSER_RelEvent,&res_value, LISTBROWSER_Selected,&res_temp, TAG_DONE);
					if(res_value == LBRE_DOUBLECLICK) { LaunchRom(gui); }
					else
					{// avoid "reload" already selected ROM (and launching if no ROMs in listbrowser)
DBUG("  Selected: [old]%ld == [new]%ld\n",res_prev,res_temp);
						if(res_temp!=-1  &&  res_prev!=res_temp) {
							res_prev = res_temp;
							ShowPreview(gui);
						}
					}
				break;
				case OID_PREVIEW_BTN:
					LaunchRom(gui);
				break;
				case OID_GAMEPAD_BTN:
DBUG("  OID_GAMEPAD_BTN\n",NULL);
					openGamepadWin(gui); // open gamepad window
				break;
				case OID_FORCESIZE:
					// Refresh settings page Engine/VideoHack gadgets
					gui->forcesize = code;
					IIntuition->SetAttrs(GAD(OID_FS_WIDTH),  GA_Disabled,!gui->forcesize, TAG_DONE);
					IIntuition->SetAttrs(GAD(OID_FS_HEIGHT), GA_Disabled,!gui->forcesize, TAG_DONE);
					IIntuition->RefreshGadgets(GAD(OID_FORCESIZE_GROUP), gui->win[WID_MAIN], NULL);
				break;
				case OID_FS_WIDTH:
					gui->myTT.forcesize_w = code;
				break;
				case OID_FS_HEIGHT:
					gui->myTT.forcesize_h = code;
				break;
				case OID_ABOUT:
				{
					char text_buf[256] = "";
					IUtility->SNPrintf(text_buf, sizeof(text_buf), (STRPTR)GetString(&li,MSG_GUI_ABOUT_TEXT),VERS,DATE);
					DoMessage(text_buf, REQIMAGE_INFO, NULL);
				}
				break;
				case OID_SAVE:
				{
					char str[5] = "";
DBUG("  OID_SAVE: (0x%08lx)\n",gui->wbs);
					// ROMS_DRAWER
DBUG("    ROMS_DRAWER '%s'\n",gui->myTT.romsdrawer);
					SaveToolType(gui->wbs->sm_ArgList->wa_Name, "ROMS_DRAWER", gui->myTT.romsdrawer);
/*
// AUTO_SNAPSHOT
struct Node *n;
IIntuition->GetAttrs(OBJ(OID_SAVESTATES), CHOOSER_SelectedNode,(uint32*)&n, TAG_DONE);
IChooser->GetChooserNodeAttrs(n, CNA_UserData,(APTR)&res_val, TAG_DONE);
if(gui->myTT.autosnapshot==FALSE  &&  res_val==0xC0DEDEAD) { // 0xC0DEDEAD == "auto" entry/item
	SaveToolType(gui->wbs->sm_ArgList->wa_Name, "AUTO_SNAPSHOT", NULL);
}
*/
					// VIEWMODE
					IIntuition->GetAttrs(OBJ(OID_GFXSCALE), SLIDER_Level,&res_value, TAG_DONE);
					str[0] = res_value+0x30; // +0x30 -> "convert" value to char
					SaveToolType(gui->wbs->sm_ArgList->wa_Name, "VIEWMODE", str);
					// FORCESIZE
					if(gui->forcesize) {
						IUtility->SNPrintf(str, sizeof(str), "%ld",gui->myTT.forcesize_w);
						SaveToolType(gui->wbs->sm_ArgList->wa_Name, "FORCESIZE_W", str);
						IUtility->SNPrintf(str, sizeof(str), "%ld",gui->myTT.forcesize_h);
						SaveToolType(gui->wbs->sm_ArgList->wa_Name, "FORCESIZE_H", str);
					}
					else {
						SaveToolType(gui->wbs->sm_ArgList->wa_Name, "FORCESIZE_W", DISABLE_TT);
						SaveToolType(gui->wbs->sm_ArgList->wa_Name, "FORCESIZE_H", DISABLE_TT);
					}
				}
				break;
				case OID_QUIT:
					done = FALSE;
				break;
			}
		} // END switch
	} // END while( (result..

	return done;
}

BOOL make_chooser_list2(BOOL mode, struct List *list, int32 str_num, int32 index)
{
	struct Node *node;
	int32 j;

	if(mode == NEW_LIST) { IExec->NewList(list); }
	for(j=0; j<index; j++)
	{
		node = IChooser->AllocChooserNode(CNA_CopyText, TRUE,
		                                  CNA_Text, GetString(&li, str_num+j),
		                                 TAG_DONE);
		if(node) { IExec->AddTail(list, node); }
		else { return FALSE; }
	}

	return TRUE;
}

// Window/Fullscreen slider level formatting
STRPTR win_fs_lvlFunc(struct Hook *hook, APTR slider, struct TagItem *tags)
{
	uint32 val = IUtility->GetTagData(SLIDER_Level, 0, tags);
DBUG("win_fs_lvlFunc() 0x%08lx (0x%08lx)\n",slider,OBJ(OID_GFXSCALE));
	if(val == 9) { // OBJ(OID_GFXSCALE)'s SLIDER_Max [gui_leftgadgets.h]
		return (STRPTR)GetString(&li, MSG_GUI_GFXSCALE_FS);
	}

	IUtility->SNPrintf(win_fs, sizeof(win_fs), "%s%ld",GetString(&li,MSG_GUI_GFXSCALE_W),val);
	return win_fs;
}


void CreateGUIwindow(struct MGBAGUI *gui)
{
	struct MsgPort *gAppPort = NULL;
	struct ColumnInfo *columninfo;
	uint32 res_totnode;
	//struct Node *node;
	struct Hook *win_fs_lvlHook = IExec->AllocSysObjectTags(ASOT_HOOK, ASOHOOK_Entry,win_fs_lvlFunc, TAG_END); // Window/Fullscreen
DBUG("CreateGUIwindow()\n",NULL);
	WORD max_w_ext = IGraphics->TextLength(&gui->screen->RastPort, GetString(&li,MSG_GUI_TITLE_COL_FMT), IUtility->Strlen(GetString(&li,MSG_GUI_TITLE_COL_FMT))+1); // max rom extension pixel width

	gAppPort = IExec->AllocSysObjectTags(ASOT_PORT, TAG_END);

//DBUG("screentitle height=%ld\n",dgg->screen->BarHeight + 1);
//DBUG("screenfont='%s'/%ld\n",gui->screen->Font->ta_Name,gui->screen->Font->ta_YSize);

	columninfo = IListBrowser->AllocLBColumnInfo(LAST_COL,
	                            LBCIA_Column,COL_ROM, LBCIA_Title, GetString(&li, MSG_GUI_TITLE_COL_ROM), //"Game",
	                                                  LBCIA_AutoSort,TRUE, LBCIA_Sortable,TRUE,
	                                                  LBCIA_Weight, 100,
	                            LBCIA_Column,COL_FMT, LBCIA_Title, GetString(&li, MSG_GUI_TITLE_COL_FMT), //"Format",
	                                                  LBCIA_AutoSort,TRUE, LBCIA_Sortable,TRUE,
	                                                  LBCIA_Width, max_w_ext,
	                           TAG_DONE);

	make_chooser_list2(NEW_LIST, gui->savestates_list, MSG_GUI_SAVESTATES_NO, 1);

/*OBJ(OID_GPAD_WIN) = IIntuition->NewObject(WindowClass, NULL, //"window.class",
        WA_ScreenTitle, VERS" "DATE,
        WA_Title,       "mgbaGUI: Gamepad settings",
        WA_PubScreen,         gui->screen,
        WA_PubScreenFallBack, TRUE,
        WA_DragBar,     TRUE,
        WA_CloseGadget, TRUE,
        //WA_SizeGadget,  TRUE,
        WA_DepthGadget, TRUE,
        WA_Activate,    TRUE,
        //WA_IDCMP, IDCMP_VANILLAKEY | IDCMP_RAWKEY,
        gui->myTT.guifade? WA_FadeTime : TAG_IGNORE, 500000, // duration of transition in microseconds
        WINDOW_IconifyGadget, TRUE,
        //WINDOW_AppPort,       gAppPort,
        //WINDOW_Icon,          gui->iconify,
        WINDOW_Position,    WPOS_CENTERWINDOW,
//WINDOW_RefWindow, NULL,
        WINDOW_PopupGadget, TRUE,
        //WINDOW_JumpScreensMenu, TRUE,
        WINDOW_UniqueID,    "mgbaGUI_Gamepad_win",
        //WINDOW_GadgetHelp, gui->myTT.show_hints,
        WINDOW_Layout, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
         LAYOUT_Orientation,    LAYOUT_ORIENT_VERT,
         LAYOUT_SpaceOuter,     TRUE,
         LAYOUT_HorizAlignment, LALIGN_CENTER,
        TAG_DONE),
TAG_DONE);*/

	OBJ(OID_MAIN) = IIntuition->NewObject(WindowClass, NULL, //"window.class",
        WA_ScreenTitle, VERS" "DATE,
        WA_Title,       "mgbaGUI",
        WA_PubScreen,         gui->screen,
        WA_PubScreenFallBack, TRUE,
        WA_DragBar,     TRUE,
        WA_CloseGadget, TRUE,
        WA_SizeGadget,  TRUE,
        WA_DepthGadget, TRUE,
        WA_Activate,    TRUE,
        WA_IDCMP, IDCMP_VANILLAKEY | IDCMP_RAWKEY,
        gui->myTT.guifade? WA_FadeTime : TAG_IGNORE, 500000, // duration of transition in microseconds
        WINDOW_IconifyGadget, TRUE,
        WINDOW_AppPort,       gAppPort,
        WINDOW_Icon,          gui->iconify,
        WINDOW_Position,    WPOS_CENTERSCREEN,
        WINDOW_PopupGadget, TRUE,
        //WINDOW_JumpScreensMenu, TRUE,
        WINDOW_UniqueID,    "mgbaGUI_win",
        WINDOW_GadgetHelp, gui->myTT.show_hints,
        WINDOW_Layout, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
         LAYOUT_Orientation,    LAYOUT_ORIENT_VERT,
         LAYOUT_SpaceOuter,     TRUE,
         LAYOUT_HorizAlignment, LALIGN_CENTER,
// BANNER
         LAYOUT_AddChild, IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
           GA_ReadOnly, TRUE,
           //BUTTON_Transparent, TRUE,
           BUTTON_BevelStyle,  BVS_BOX,
           BUTTON_RenderImage, OBJ(OID_BANNER_IMG) = IIntuition->NewObject(BitMapClass, NULL, //"bitmap.image",
             //IA_Scalable, TRUE,
             BITMAP_Screen,     gui->screen,
             //BITMAP_Transparent, TRUE,
             BITMAP_SourceFile, "PROGDIR:mgba_banner.png",
           TAG_DONE),
         TAG_DONE), // END of BANNER
         CHILD_WeightedHeight, 0,
         CHILD_WeightedWidth,  0,
// ROM DRAWER + DGEN EXECUTABLE
        LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
          //LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
          //LAYOUT_SpaceOuter,  TRUE,
          LAYOUT_AddChild, OBJ(OID_ROMDRAWER) = IIntuition->NewObject(GetFileClass, NULL,
            GA_ID,        OID_ROMDRAWER,
            GA_RelVerify, TRUE,
            GA_HintInfo,  GetString(&li, MSG_GUI_ROMDRAWER_HELP),
            GETFILE_ReadOnly,    TRUE,
            GETFILE_RejectIcons, TRUE,
            GETFILE_TitleText,   GetString(&li, MSG_GUI_ROMDRAWER_TITLE),
            GETFILE_DrawersOnly, TRUE,
            GETFILE_Drawer,      gui->myTT.romsdrawer,
          TAG_DONE),
          CHILD_Label, IIntuition->NewObject(LabelClass, NULL,
            LABEL_Text, GetString(&li, MSG_GUI_ROMDRAWER),//" ROM drawer:",
          TAG_DONE),
          /*sLAYOUT_AddChild, OBJ(OID_DGENEXE) = IIntuition->NewObject(ChooserClass, NULL,
            //GA_ID,         OID_DGENEXE,
            //GA_RelVerify,  TRUE,
            //GA_Disabled,   gui->myTT.dgensdl_exec==0? FALSE:TRUE,
            GA_Underscore, 0,
            GA_HintInfo,   GetString(&li, MSG_GUI_DGENEXEX_HELP),
            CHOOSER_LabelArray, dgenexec_array,
            CHOOSER_Selected,   gui->myTT.dgensdl_exec - 1,
          TAG_DONE),
          CHILD_WeightedWidth, 0,
          CHILD_Label, IIntuition->NewObject(LabelClass, NULL,
            LABEL_Text, " DGen:",
          TAG_DONE),*/
         TAG_DONE), // END of ROM DRAWER + DGEN EXECUTABLE
         CHILD_WeightedHeight, 0,

// ROM LIST + LEFT_GADGETS
         LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
           //LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
           LAYOUT_SpaceOuter,  TRUE,
           //LAYOUT_SpaceInner,  FALSE,
// ROM LIST
           LAYOUT_AddChild, OBJ(OID_LISTBROWSER) = IIntuition->NewObject(ListBrowserClass, NULL, //"listbrowser.gadget",
             GA_ID,        OID_LISTBROWSER,
             GA_RelVerify, TRUE,
             GA_HintInfo,  GetString(&li, MSG_GUI_LISTBROWSER_HELP),
             //LISTBROWSER_SortColumn,     COL_ROM,
             //LISTBROWSER_AutoFit,        TRUE,
             LISTBROWSER_Labels,         NULL,
             LISTBROWSER_ColumnInfo,     columninfo,
             LISTBROWSER_ColumnTitles,   TRUE,
             LISTBROWSER_ShowSelected,   TRUE,
             LISTBROWSER_Selected,       -1,
             LISTBROWSER_MinVisible,     15,
             //LISTBROWSER_Striping,       LBS_ROWS,
             LISTBROWSER_TitleClickable, TRUE,
             //LISTBROWSER_HorizontalProp, TRUE,
           TAG_DONE), // END of ROM LIST 
           CHILD_MinWidth, 200,
           CHILD_WeightedWidth, 65,
// LEFTSIDE_GADGETS - START
#include "includes/gui_leftsidegads.h"
// LEFTSIDE_GADGETS - END
         TAG_DONE), // END of ROM LIST + LEFTSIDE_GADGETS

// BUTTONS
         LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
           LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
           LAYOUT_SpaceOuter,  TRUE,
           LAYOUT_BevelStyle,  BVS_SBAR_VERT,
           LAYOUT_AddChild, IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
             GA_ID,        OID_ABOUT,
             GA_RelVerify, TRUE,
             GA_Text,      GetString(&li, MSG_GUI_ABOUT_BTN),//"About..."
             GA_HintInfo,  GetString(&li, MSG_GUI_ABOUT_BTN_HELP),
           TAG_DONE),
           LAYOUT_AddChild, IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
             GA_ID,        OID_SAVE,
             GA_RelVerify, TRUE,
             GA_Text,      GetString(&li, MSG_GUI_SAVE_BTN),//"Save settings"
             GA_HintInfo,  GetString(&li, MSG_GUI_SAVE_BTN_HELP),
           TAG_DONE),
           LAYOUT_AddChild, IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
             GA_ID,        OID_QUIT,
             GA_RelVerify, TRUE,
             GA_Text,      GetString(&li, MSG_GUI_QUIT_BTN),//"Quit",
             GA_HintInfo,  GetString(&li, MSG_GUI_QUIT_BTN_HELP),
           TAG_DONE),
         TAG_DONE), // END of BUTTONS
         CHILD_WeightedHeight, 0,

        TAG_DONE), // END of window layout group
	TAG_END);

//IIntuition->SetAttrs(OBJ(OID_FORCESIZE_GROUP), LAYOUT_AlignLabels,OBJ(OID_OPTIONS_GROUP), TAG_DONE);
//IIntuition->SetAttrs(OBJ(OID_OPTIONS_GROUP), LAYOUT_AlignLabels,OBJ(OID_FORCESIZE_GROUP), TAG_DONE);


	if( OBJ(OID_MAIN) ) {
		updateList(gui);
		// Select last launched ROM
		IIntuition->GetAttrs(OBJ(OID_LISTBROWSER), LISTBROWSER_TotalNodes,&res_totnode, TAG_DONE);
		//if(gui->myTT.last_rom_run<0  ||  gui->myTT.last_rom_run>=res_totnode-1) { gui->myTT.last_rom_run = 0; }
		if(gui->myTT.last_rom_run<0  ||  gui->myTT.last_rom_run>res_totnode) { gui->myTT.last_rom_run = 0; }
DBUG("last_rom_run=%ld (of %ld)\n",gui->myTT.last_rom_run+1,res_totnode);
		IIntuition->SetAttrs(OBJ(OID_LISTBROWSER), LISTBROWSER_Selected,gui->myTT.last_rom_run,
		                     LISTBROWSER_MakeVisible,gui->myTT.last_rom_run, TAG_DONE);
		res_prev = gui->myTT.last_rom_run;
	}

	if( (gui->win[WID_MAIN]=(struct Window *)IIntuition->IDoMethod(OBJ(OID_MAIN), WM_OPEN, NULL)) )
	{
		IIntuition->ScreenToFront(gui->win[WID_MAIN]->WScreen);

		// Show preview image
		ShowPreview(gui);

		while(ProcessGUI(gui) != FALSE);
	} // END if( (gui->win[WID_MAIN]=..

	IIntuition->DisposeObject( OBJ(OID_MAIN) );
	OBJ(OID_MAIN) = NULL;
	IIntuition->DisposeObject( OBJ(OID_BANNER_IMG) );
	IIntuition->DisposeObject( OBJ(OID_PREVIEW_IMG) );

	IListBrowser->FreeLBColumnInfo(columninfo);
	IExec->FreeSysObject(ASOT_PORT, gAppPort);

	IExec->FreeSysObject(ASOT_HOOK, win_fs_lvlHook);
}

void updateList(struct MGBAGUI *gui)
{
	STRPTR romdrawer = NULL;
	uint32 res_tot;
DBUG("updateList()\n",NULL);
	// Detach the listbrowser list first
	IIntuition->SetAttrs(OBJ(OID_LISTBROWSER), LISTBROWSER_Labels,NULL, TAG_END);

	// Re-generate romlist with new rom drawer
	IIntuition->GetAttr(GETFILE_Drawer, OBJ(OID_ROMDRAWER), (uint32*)&romdrawer);
	FreeString(&gui->myTT.romsdrawer);
	gui->myTT.romsdrawer = DupStr(romdrawer, -1);
	res_tot = GetRoms(gui->myTT.romsdrawer, gui->romlist);
	if(res_tot == 0) { // "clear" savestates chooser list
		struct Node *node = IChooser->AllocChooserNode(CNA_Text,GetString(&li,MSG_GUI_SAVESTATES_NO), TAG_DONE);
		// Detach chooser list
		IIntuition->SetAttrs(OBJ(OID_SAVESTATES), CHOOSER_Labels,NULL, TAG_DONE);
		// Remove previous savestates chooser list
		free_chooserlist_nodes(gui->savestates_list);
	// Add NO string (MSG_GUI_SAVESTATES_NO) at top/head of the list..
		IExec->AddHead(gui->savestates_list, node);
		//..and re-attach chooser list
		IIntuition->RefreshSetGadgetAttrs(GAD(OID_SAVESTATES), gui->win[WID_MAIN], NULL,
		                                  CHOOSER_Labels,gui->savestates_list, GA_Disabled,FALSE, TAG_DONE);
	}
DBUG("  gui->romlist = 0x%08lx\n",gui->romlist);
	// Re-attach the listbrowser
	IIntuition->SetAttrs(OBJ(OID_LISTBROWSER), LISTBROWSER_SortColumn,COL_ROM,
	                     LISTBROWSER_Labels,gui->romlist, TAG_DONE);
	IIntuition->RefreshSetGadgetAttrs(GAD(OID_TOTALROMS), gui->win[WID_MAIN], NULL,
	                                  BUTTON_VarArgs,&res_tot, TAG_DONE);
}
