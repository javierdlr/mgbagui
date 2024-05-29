#define CATCOMP_NUMBERS
//#define CATCOMP_BLOCK
//#define CATCOMP_CODE
extern struct LocaleInfo li;

#include "includes.h"
#include "debug.h"

/*
 * Using parts of code:
 * E-UAE - The portable Amiga Emulator
 * AmigaInput joystick driver
 * Copyright 2005 Richard Drummond
 */
#define MAX_INPUT_DEVICES  6
#define MAX_JOYSTICKS  MAX_INPUT_DEVICES
#define MAX_AXES       2
#define MAX_BUTTONS    12

// A handy container to encapsulate the information we
// need when enumerating joysticks on the system.
struct enumPacket {
 APTR             context;
 uint32          *count;
 struct joystick *joyList;
};

// Per-joystick data private to driver
struct joystick {
 AIN_DeviceID     id;
 STRPTR           name;
 AIN_DeviceHandle *handle;
 APTR             context;
 uint32           axisCount;
 uint32           buttonCount;
// uint32           axisBufferOffset[MAX_AXES];
// int32            axisData[MAX_AXES];
 uint32           buttonBufferOffset[MAX_BUTTONS];
// int32            buttonData[MAX_BUTTONS];
};

static APTR joystickContext;
static uint32 joystickCount;
static struct joystick joystickList[MAX_JOYSTICKS];

BOOL enumerateJoysticks(AIN_Device *, void *UserData);
void close_joysticks(void);
unsigned int get_joystick_count(void);
STRPTR get_joystick_name(unsigned int);
//void read_joysticks(void);
int acquire_joy(unsigned int, int);
void unacquire_joy(unsigned int);


void loadConfig(struct MGBAGUI *);
void saveGamepadButtonsToConfig(struct MGBAGUI *);


extern struct AIN_IFace *IAIN;
//extern struct IconIFace *IIcon;
extern struct DOSIFace *IDOS;
extern struct IntuitionIFace *IIntuition;
//extern struct GraphicsIFace *IGraphics;
extern struct UtilityIFace *IUtility;

// the class pointer
extern Class /**ClickTabClass,*/ *ListBrowserClass, *ButtonClass, *LabelClass, *GetFileClass,
             *CheckBoxClass, *ChooserClass, *BitMapClass, *LayoutClass, *WindowClass,
             *RequesterClass, *SpaceClass, *IntegerClass, *GetFileClass, *SliderClass;

extern Object *Objects[LAST_OID];


uint32 getGamepadButton(unsigned int joynum)
{
	struct joystick *joy = &joystickList[joynum-1];
	uint32 ret = OID_GPAD_NO_BTN;

	if(joy->handle != NULL) {
		AIN_InputEvent *event = NULL;
		while( (event=IAIN->AIN_GetEvent(joy->context)) ) {
//DBUG("AIN_GetEvent() 0x%08lx\n",event->Type);
			/*switch(event->Type) {
				case AINET_BUTTON:
					ret = event->Index - joy->buttonBufferOffset[0];
DBUG("Button #%2ld [%ld]\n",ret, event->Value);
				break;
				default: break;
			}*/
			if(event->Type == AINET_BUTTON) {
				ret = event->Index - joy->buttonBufferOffset[0];
DBUG("Button #%2ld [%ld]\n",ret, event->Value);
			}
			IAIN->AIN_FreeEvent(joy->context, event);
		}
	}

	return ret;
}

void processGamepadGUI(struct MGBAGUI *gui)
{
	uint16 code = 0;
	uint32 siggot = 0, w_sigmask = 0, ai_sigmask = 1L << gui->ai_port->mp_SigBit,
	       result = WMHI_LASTMSG, btn_OID = OID_GPAD_NO_BTN, btn;
	BOOL done = TRUE;
	AIN_InputEvent *event = NULL;
	unsigned int joynum = 1;
	struct joystick *joy = &joystickList[joynum-1];

	IIntuition->GetAttr(WINDOW_SigMask, OBJ(OID_GPAD), &w_sigmask);

	// Code taken fro AmiDog's FPSE SDK (http://www.amidog.se/amiga/bin/FPSE/20151029/FPSE-0.10.6-SDK.tar.gz)
	if(joy->handle != NULL) {
		struct TagItem tags[] = { {AINCC_Window,(ULONG)gui->win[WID_GPAD]}, {TAG_DONE,TAG_DONE} };
//DBUG("joy: handle=0x%08lx  context=0x%08lx\n",joy->handle,joy->context);
		IAIN->AIN_SetDeviceParameter(joy->context, joy->handle, AINDP_EVENT, TRUE);
		IAIN->AIN_Set(joy->context, tags);

		while(done != FALSE) {
			siggot = IExec->Wait(w_sigmask | ai_sigmask | SIGBREAKF_CTRL_C);

			if(siggot & SIGBREAKF_CTRL_C) { done = FALSE; break; }

			while( (result=IIntuition->IDoMethod(OBJ(OID_GPAD), WM_HANDLEINPUT, &code)) != WMHI_LASTMSG ) {
//DBUG("result=0x%lx\n",result);
				switch(result & WMHI_CLASSMASK) {
					case WMHI_CLOSEWINDOW:
						done = FALSE;
					break;
					case WMHI_GADGETUP:
DBUG("[WMHI_GADGETUP] code = %ld (0x%08lx)\n",code,code);
						switch(result & WMHI_GADGETMASK) {
							case OID_GPAD_L:
							case OID_GPAD_R:
							case OID_GPAD_B:
							case OID_GPAD_A:
							case OID_GPAD_SEL:
							case OID_GPAD_STA:
							 btn_OID = result & WMHI_GADGETMASK;
DBUG("  btn_OID = 0x%08lx (OID_GPAD_#?)\n",btn_OID);
							break;
							case OID_GPAD_SAVE:
								saveGamepadButtonsToConfig(gui);
							case OID_GPAD_CANCEL:
								done = FALSE;
							break;
						}
					break;
				}
			}

			btn = getGamepadButton(joynum);
			if(btn!=OID_GPAD_NO_BTN  &&  btn_OID!=OID_GPAD_NO_BTN) {
DBUG("  (%ld -> %ld)\n",gui->keyV[btn_OID-OID_GPAD_L],btn);
				gui->keyV[btn_OID-OID_GPAD_L] = btn;
				IIntuition->RefreshSetGadgetAttrs(GAD(btn_OID), gui->win[WID_GPAD], NULL,
				                                  BUTTON_Integer,btn, GA_Selected,FALSE, TAG_END);
				btn_OID = OID_GPAD_NO_BTN; // "reset" btn_OID
			}

		} // END while(done..

		IAIN->AIN_SetDeviceParameter(joy->context, joy->handle, AINDP_EVENT, FALSE);
		// Remove pending AI messages
		while( (event=IAIN->AIN_GetEvent(joy->context)) ) { IAIN->AIN_FreeEvent(joy->context, event); }
	}

}

Object *labelButton(struct MGBAGUI *gui, uint32 objID, CONST_STRPTR lbl, uint32 alignH, uint32 lblPos)
{
//DBUG("labelButton() %ld\n",gui->keyV[objID-OID_GPAD_L]);
	return IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
		LAYOUT_SpaceOuter,     TRUE,
		LAYOUT_HorizAlignment, alignH,
		LAYOUT_LabelColumn,    lblPos,
		LAYOUT_AddChild, OBJ(objID) = IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
			GA_ID,        objID,
			GA_RelVerify, TRUE,
			BUTTON_PushButton,   TRUE,
			BUTTON_DomainString, "88",
			BUTTON_Integer,      gui->keyV[objID-OID_GPAD_L],
		TAG_DONE),
		CHILD_WeightedWidth, 0,
		CHILD_Label, IIntuition->NewObject(LabelClass, NULL, LABEL_Text,lbl, TAG_DONE),
	TAG_DONE);
}

void createGamepadGUI(struct MGBAGUI *gui)
{
	struct ExamineData *dat_bg = IDOS->ExamineObjectTags(EX_StringNameInput,GPAD_BG, TAG_END),
	                   *dat = IDOS->ExamineObjectTags(EX_StringNameInput,GPAD_IMG, TAG_END),
	                   *dat_str = IDOS->ExamineObjectTags(EX_StringNameInput,GPAD_STR, TAG_END);
	IDOS->FreeDosObject(DOS_EXAMINEDATA, dat_bg);
	IDOS->FreeDosObject(DOS_EXAMINEDATA, dat);
	IDOS->FreeDosObject(DOS_EXAMINEDATA, dat_str);

	OBJ(OID_GPAD) = IIntuition->NewObject(WindowClass, NULL, //"window.class",
        WA_ScreenTitle, VERS" "DATE,
        WA_Title,       get_joystick_name(joystickCount),//GetString(&li, MSG_GUI_GPAD_WINTITLE),//"mgbaGUI: Gamepad settings",
        WA_PubScreen,   gui->screen,
        WA_PubScreenFallBack, TRUE,
        WA_DragBar,     TRUE,
        WA_CloseGadget, TRUE,
        //WA_SizeGadget,  TRUE,
        dat_bg? WINDOW_BackFillName : TAG_IGNORE, GPAD_BG,//"keymap_bg.png"
        WA_DepthGadget, TRUE,
        WA_Activate,    TRUE,
        //WA_IDCMP, IDCMP_VANILLAKEY | IDCMP_RAWKEY,
        gui->myTT.guifade? WA_FadeTime : TAG_IGNORE, 500000, // duration of transition in microseconds
        //WINDOW_IconifyGadget, TRUE,
        //WINDOW_AppPort,       gAppPort,
        //WINDOW_Icon,          gui->iconify,
        WINDOW_RefWindow, gui->win[WID_MAIN],
        WINDOW_Position,  WPOS_CENTERWINDOW,
        //WINDOW_PopupGadget, TRUE,
        //WINDOW_JumpScreensMenu, TRUE,
        //WINDOW_UniqueID,    "mgbaGUI_Gamepad_win",
        WINDOW_GadgetHelp, gui->myTT.show_hints,
        WINDOW_Layout, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
         LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
         LAYOUT_SpaceOuter,  TRUE,

// Left + Right
         LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
          LAYOUT_SpaceOuter,    TRUE,
          LAYOUT_VertAlignment, LALIGN_TOP,
          LAYOUT_AddChild, labelButton(gui, OID_GPAD_L, "_L", LALIGN_LEFT, PLACETEXT_RIGHT),
          LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, TAG_DONE),
          LAYOUT_AddChild, labelButton(gui, OID_GPAD_R, "_R", LALIGN_RIGHT, PLACETEXT_LEFT),
         TAG_DONE), // END of Left + Right
         CHILD_WeightedHeight, 0,

//         LAYOUT_AddChild, IIntuition->NewObject(/*SpaceClass,*/ NULL, "space.gadget", TAG_DONE),
         LAYOUT_AddChild, IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
           BUTTON_Transparent, TRUE,
           BUTTON_BevelStyle,  BVS_NONE,
           dat? BUTTON_RenderImage : TAG_IGNORE, OBJ(OID_GPAD_STR) = IIntuition->NewObject(BitMapClass, NULL, //"bitmap.image",
             BITMAP_SourceFile,  GPAD_STR,//"keymap_str.png",
             BITMAP_Screen,      gui->screen,
             BITMAP_Masking,     TRUE,
             BITMAP_Transparent, TRUE,
           TAG_DONE),
         TAG_DONE),

// [image] + B + A
         LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
          //LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
          LAYOUT_VertAlignment, LALIGN_CENTER,
          //LAYOUT_HorizAlignment, LALIGN_CENTER,
          LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, //"space.gadget",
            SPACE_MinWidth,10, TAG_DONE),
          CHILD_WeightedWidth, 0,
          LAYOUT_AddChild, IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
            BUTTON_Transparent, TRUE,
            BUTTON_BevelStyle,  BVS_NONE,
            dat? BUTTON_RenderImage : TAG_IGNORE, OBJ(OID_GPAD_IMG) = IIntuition->NewObject(BitMapClass, NULL, //"bitmap.image",
              BITMAP_SourceFile,  GPAD_IMG,//"keymap.png",
              BITMAP_Screen,      gui->screen,
              BITMAP_Masking,     TRUE,
              BITMAP_Transparent, TRUE,
            TAG_DONE),
          TAG_DONE),
          dat? TAG_IGNORE : CHILD_MinWidth, 100,
          dat? TAG_IGNORE : CHILD_MinHeight, 100,
          CHILD_WeightedWidth,  0,
          CHILD_WeightedHeight, 0,
          LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, //"space.gadget",
            SPACE_MinWidth,20, TAG_DONE),
          CHILD_WeightedWidth, 0,
          LAYOUT_AddChild, labelButton(gui, OID_GPAD_B, "_B", LALIGN_LEFT, PLACETEXT_LEFT),
          CHILD_WeightedWidth,  0,
          CHILD_WeightedHeight, 0,
          LAYOUT_AddChild, labelButton(gui, OID_GPAD_A, "_A", LALIGN_RIGHT, PLACETEXT_RIGHT),
          CHILD_WeightedWidth,  0,
          CHILD_WeightedHeight, 0,
          LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, //"space.gadget",
            SPACE_MinWidth,20, TAG_DONE),
          CHILD_WeightedWidth, 0,
         TAG_DONE), // END of [image] + B + A
         //CHILD_WeightedHeight, 0,

         LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, TAG_DONE),
         /*LAYOUT_AddChild, IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
           BUTTON_Transparent, TRUE,
           BUTTON_BevelStyle,  BVS_NONE,
           dat? BUTTON_RenderImage : TAG_IGNORE, OBJ(OID_GPAD_STR) = IIntuition->NewObject(BitMapClass, NULL, //"bitmap.image",
             BITMAP_SourceFile,  GPAD_STR,//"keymap_str.png",
             BITMAP_Screen,      gui->screen,
             BITMAP_Masking,     TRUE,
             BITMAP_Transparent, TRUE,
           TAG_DONE),
         TAG_DONE),*/

// Select + Start
         LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
          //LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
          LAYOUT_SpaceOuter, TRUE,
          LAYOUT_AddChild, labelButton(gui, OID_GPAD_SEL, "S_elect", LALIGN_RIGHT, PLACETEXT_LEFT),
          LAYOUT_AddChild, labelButton(gui, OID_GPAD_STA, "_Start", LALIGN_LEFT, PLACETEXT_RIGHT),
         TAG_DONE), // END of Select + Start
         CHILD_WeightedHeight, 0,

//         LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, //"space.gadget",
//           SPACE_MinHeight,10, TAG_DONE),
//         CHILD_WeightedHeight, 0,

         LAYOUT_AddChild, OBJ(OID_GPAD_TEXT) = IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
           //GA_ID,        OID_GPAD_TEXT,
           //GA_RelVerify, TRUE,
           GA_ReadOnly, TRUE,
           GA_Text,      GetString(&li, MSG_GUI_GPAD_TEXT),//"(assign gamepad buttons)",
           BUTTON_BevelStyle,   BVS_NONE,
           BUTTON_Transparent, TRUE,
         TAG_DONE),
         CHILD_WeightedHeight, 0,
// BUTTONS
         LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
           LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
           LAYOUT_SpaceOuter,  TRUE,
           LAYOUT_BevelStyle,  BVS_SBAR_VERT,
           LAYOUT_AddChild, IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
             GA_ID,        OID_GPAD_SAVE,
             GA_RelVerify, TRUE,
             GA_Text,      GetString(&li, MSG_GUI_GPAD_SAVE),
             GA_HintInfo,  GetString(&li, MSG_GUI_GPAD_SAVE_HELP),
           TAG_DONE),
           LAYOUT_AddChild, IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
             GA_ID,        OID_GPAD_CANCEL,
             GA_RelVerify, TRUE,
             GA_Text,      GetString(&li, MSG_GUI_GPAD_CANCEL),
             GA_HintInfo,  GetString(&li, MSG_GUI_GPAD_CANCEL_HELP),
           TAG_DONE),
         TAG_DONE), // END of BUTTONS
         CHILD_WeightedHeight, 0,

        TAG_DONE),
	TAG_DONE);
DBUG("  gamepad obj = 0x%08lx\n",OBJ(OID_GPAD));
	if( OBJ(OID_GPAD) ) {
		gui->win[WID_GPAD] = (struct Window *)IIntuition->IDoMethod(OBJ(OID_GPAD), WM_OPEN, NULL);
	}
}


void openGamepadWin(struct MGBAGUI *gui)
{
	struct TagItem tags[] = { {AINCC_Port,(ULONG)gui->ai_port}, {TAG_DONE,TAG_DONE} };
DBUG("openGamepadWin()\n",NULL);
	gui->cfg_file = (STRPTR)IExec->AllocVecTags(CFG_FILE_SIZE, AVT_ClearWithValue,0, TAG_END);

	loadConfig(gui);

	joystickContext = IAIN->AIN_CreateContext(1, tags);
	if(joystickContext) {
		struct enumPacket packet = {joystickContext, &joystickCount, &joystickList[0]};

		//success = 1;
		IAIN->AIN_EnumDevices(joystickContext, enumerateJoysticks, &packet);

//		success = get_joystick_count();
//DBUG("  Found %ld joysticks\n", success);
//		if(success) {
		if( get_joystick_count() ) {
DBUG("  ->%s<-\n", get_joystick_name(joystickCount));
			acquire_joy(joystickCount, 0);

			IIntuition->SetAttrs(OBJ(OID_MAIN), WA_BusyPointer,TRUE, TAG_DONE);
			createGamepadGUI(gui);
DBUG("  gamepad win = 0x%08lx\n",gui->win[WID_GPAD]);
			processGamepadGUI(gui);
			IIntuition->SetAttrs(OBJ(OID_MAIN), WA_BusyPointer,FALSE, TAG_DONE);

			IIntuition->DisposeObject( OBJ(OID_GPAD) );
			IIntuition->DisposeObject( OBJ(OID_GPAD_IMG) );
			IIntuition->DisposeObject( OBJ(OID_GPAD_STR) );
			OBJ(OID_GPAD) = OBJ(OID_GPAD_IMG) = OBJ(OID_GPAD_STR) = NULL;

			unacquire_joy(joystickCount);
		}

		close_joysticks();
	}

	IExec->FreeVec(gui->cfg_file);
	gui->cfg_file = NULL;
}

void loadConfig(struct MGBAGUI *gui)
{
	int32 i, res_int;
	BPTR fhConfFile = IDOS->FOpen(CONFIG_INI, MODE_OLDFILE, 0);
DBUG("loadConfig()\n",NULL);

	if(fhConfFile != ZERO) {
		CONST_STRPTR keyW[] = {"keyL", "keyR", "keyB", "keyA", "keySelect", "keyStart"};
		struct FReadLineData *frld = IDOS->AllocDosObjectTags(DOS_FREADLINEDATA, 0);

		while(IDOS->FReadLine(fhConfFile, frld) > 0) {
			if(frld->frld_LineLength > 1) {
//DBUG("Line is %ld bytes: %s", frld->frld_LineLength, frld->frld_Line);
				if(/*frld->frld_Line[0]!='#'  &&*/  frld->frld_Line[0]!='[') { // skip lines starting with such char(s)
					char kword[32] = "";
					int32 pos = IDOS->SplitName( frld->frld_Line, '=', kword, 0, sizeof(kword) ); // get KEYWORD without VALUE ("keyUp = 1")

					if(IUtility->Strnicmp(kword,"key",3) == 0) {
//DBUG("  keyword: \"%s\"\n",kword);
						for(i=0; i!=6; i++) {
							if(IUtility->Strnicmp(kword,keyW[i],IUtility->Strlen(keyW[i])) == 0) {
								if(IDOS->StrToLong(frld->frld_Line+pos, &res_int) != -1) {
//DBUG("    value: %ld\n",res_int);
									gui->keyV[i] = res_int;
DBUG("  keyword: \"%s\"\n",keyW[i]);
DBUG("    value: %ld\n",gui->keyV[i]);
								}
								break;
							}

						}
					}
					else { IUtility->Strlcat(gui->cfg_file, frld->frld_Line, CFG_FILE_SIZE); }
				} // END if(frld->frld_Line[0]!='#'..
			} // END if(frld->frld_LineLength..
		} // END while(IDOS->FReadLine(f..

		IDOS->FreeDosObject(DOS_FREADLINEDATA, frld);
		IDOS->FClose(fhConfFile);
	} // END if(fhConfFile..
}

void saveGamepadButtonsToConfig(struct MGBAGUI *gui)
{
	int32 i;
	BPTR fhConfFile = IDOS->FOpen(CONFIG_INI, MODE_NEWFILE, 0);
DBUG("saveGamepadButtonsToConfig()\n",NULL);

	if(fhConfFile != ZERO) {
		CONST_STRPTR keyW[] = {"keyL", "keyR", "keyB", "keyA", "keySelect", "keyStart"};
		// Add to config.ini gamepad buttons
		IUtility->Strlcat(gui->cfg_file, "[gba.input.SDLB]\n", CFG_FILE_SIZE);
		for(i=0; i!=6; i++) {
DBUG("  %s = %ld\n",keyW[i],gui->keyV[i]);
			IUtility->SNPrintf(gui->cfg_file, CFG_FILE_SIZE, "%s%s = %ld\n",gui->cfg_file,keyW[i],gui->keyV[i]);
		}
		// Write "new" config.ini
		IDOS->FPuts(fhConfFile, gui->cfg_file);
		IDOS->FClose(fhConfFile);
	}
}


/*
 * Using parts of code:
 * E-UAE - The portable Amiga Emulator
 * AmigaInput joystick driver
 * Copyright 2005 Richard Drummond
 */
// Callback to enumerate joysticks
BOOL enumerateJoysticks(AIN_Device *device, void *UserData)
{
	APTR context = ((struct enumPacket *)UserData)->context;
	uint32 *count = ((struct enumPacket *)UserData)->count;
	struct joystick *joy = &((struct enumPacket *)UserData)->joyList[*count];

	BOOL result = FALSE;

	if(*count < MAX_JOYSTICKS) {
		if(device->Type == AINDT_JOYSTICK) {
			unsigned int i;

			joy->context     = context;
			joy->id          = device->DeviceID;
			joy->name        = (STRPTR)device->DeviceName;
			joy->axisCount   = device->NumAxes;
			joy->buttonCount = device->NumButtons;

			if(joy->axisCount > MAX_AXES) joy->axisCount = MAX_AXES;

			if(joy->buttonCount > MAX_BUTTONS) joy->buttonCount = MAX_BUTTONS;

			// Query offsets in ReadDevice buffer for axes' data
//			for(i=0; i<joy->axisCount; i++)
//				result = IAIN->AIN_Query(joy->context, joy->id, AINQ_AXIS_OFFSET, i, &(joy->axisBufferOffset[i]), 4);

			// Query offsets in ReadDevice buffer for buttons' data
			for(i=0; i<joy->buttonCount; i++) {
				result = /*result &&*/ IAIN->AIN_Query(joy->context, joy->id, AINQ_BUTTON_OFFSET, i, &(joy->buttonBufferOffset[i]), 4);
DBUG("AINQ_BUTTON_OFFSET #%ld = %ld\n",i,joy->buttonBufferOffset[i]);
			}

			if(result  &&  joy->id==4096) {
DBUG("Joystick #%ld (AI ID=%ld) '%s' with %ld axes, %ld buttons\n",*count, joy->id, joy->name, joy->axisCount, joy->buttonCount);
				(*count)++;
			}

		}
	}

	return result;
}

void close_joysticks(void)
{
	unsigned int i = joystickCount;

	while(i-- > 0) {
		struct joystick *joy = &joystickList[i];

		if(joy->handle) {
			IAIN->AIN_ReleaseDevice(joy->context, joy->handle);
			joy->handle = 0;
		}
	}
	joystickCount = 0;

	if(joystickContext) {
		IAIN->AIN_DeleteContext(joystickContext);
		joystickContext = NULL;
	}
}


// Query number of joysticks attached to system
unsigned int get_joystick_count(void)
{
	return joystickCount;
}

STRPTR get_joystick_name(unsigned int joynum)
{
	return (STRPTR)joystickList[joynum-1].name;
}

int acquire_joy(unsigned int joynum, int flags)
{
	struct joystick *joy = &joystickList[joynum-1];
	int result = 0;

	joy->handle = IAIN->AIN_ObtainDevice(joy->context, joy->id);
	if(joy->handle) result = 1;
	else IDOS->Printf("Failed to acquire joy\n");

	return result;
}

void unacquire_joy(unsigned int joynum)
{
	struct joystick *joy = &joystickList[joynum-1];

	if(joy->handle) {
		IAIN->AIN_ReleaseDevice(joy->context, joy->handle);
		joy->handle = 0;
	}
}
