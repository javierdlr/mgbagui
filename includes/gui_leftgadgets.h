// LEFTSIDE_GADGETS: [BUTTON/IMAGE + RECGAME] + [SAVESTATES + GFXSCALE + FRAMESKIP] + [FORCESIZE] + TOTALROMS
	LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
		LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
		LAYOUT_SpaceOuter,  TRUE,

// BUTTON/IMAGE
		LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
			LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
			LAYOUT_SpaceOuter,  TRUE,
			//LAYOUT_BevelStyle, BVS_GROUP,
			//LAYOUT_SpaceInner,  FALSE,
// BUTTON/IMAGE
			LAYOUT_AddChild, IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
				//LAYOUT_Orientation,    LAYOUT_ORIENT_VERT,
				//LAYOUT_HorizAlignment, LALIGN_CENTER,
				//LAYOUT_SpaceOuter,     FALSE,
				//LAYOUT_SpaceInner,     FALSE,
				LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, //"space.gadget",
					SPACE_MinWidth, 10,
				TAG_DONE),
				LAYOUT_AddChild, OBJ(OID_PREVIEW_BTN) = IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
					GA_ID,         OID_PREVIEW_BTN,
					GA_RelVerify,  TRUE,
					GA_Underscore, 0,
					//GA_Text,       "RUN GAME",
					GA_HintInfo,   GetString(&li, OID_PREVIEW_BTN_HELP),
					BUTTON_BevelStyle,  BVS_THIN,
					//BUTTON_Transparent, TRUE,
					//BUTTON_BackgroundPen, BLOCKPEN,
					//BUTTON_FillPen,       BLOCKPEN,
					BUTTON_RenderImage, OBJ(OID_PREVIEW_IMG) = IIntuition->NewObject(BitMapClass, NULL, //"bitmap.image",
						//IA_Scalable, TRUE,
						BITMAP_Screen, gui->screen,
						//BITMAP_Masking, TRUE,
					TAG_DONE),
				TAG_DONE),
				CHILD_MaxWidth, 256+2,  // pixels width of preview + button border
				CHILD_MinWidth, 256+2,  // pixels width of preview + button border
				CHILD_MaxHeight, 224+2, // pixels height of preview + button border
				CHILD_MinHeight, 224+2, // pixels height of preview + button border
				LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, //"space.gadget",
					SPACE_MinWidth, 10,
				TAG_DONE),
			TAG_DONE), // END of BUTTON/IMAGE
		TAG_DONE), // END of BUTTON/IMAGE + RECGAME
		CHILD_WeightedHeight, 0,

		LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, //"space.gadget",
			SPACE_MinHeight, 10,
		TAG_DONE),
		CHILD_WeightedHeight, 0,

// [SAVESTATES + GFXSCALE + FRAMESKIP] + [FORCESIZE]
		LAYOUT_AddChild, /*OBJ(OID_GAME_OPTIONS) =*/ IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
			LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
			LAYOUT_SpaceOuter,  TRUE,
			//LAYOUT_SpaceInner,  FALSE,
//LAYOUT_HorizAlignment, LALIGN_RIGHT,
			LAYOUT_BevelStyle,  BVS_GROUP,

			LAYOUT_AddChild, OBJ(OID_SAVESTATES) = IIntuition->NewObject(ChooserClass, NULL, //"chooser.gadget",
				GA_ID,         OID_SAVESTATES,
				GA_RelVerify,  TRUE,
				GA_Underscore, 0,
				GA_HintInfo,   GetString(&li, MSG_GUI_SAVESTATES_HELP),
				CHOOSER_Labels, gui->savestates_list,
				//CHOOSER_Selected, 0,
			TAG_DONE),
			//CHILD_WeightedWidth, 0,
			CHILD_Label, IIntuition->NewObject(LabelClass, NULL,// "label.image",
				LABEL_Text, GetString(&li, MSG_GUI_SAVESTATES),//"Load savestate",
			TAG_DONE),
			LAYOUT_AddChild, OBJ(OID_GFXSCALE) = IIntuition->NewObject(SliderClass, NULL,
				//GA_ID,         OID_GFXSCALE,
				//GA_RelVerify,  TRUE,
				GA_HintInfo, GetString(&li, MSG_GUI_GFXSCALE_HELP),
				SLIDER_Level, gui->myTT.viewmode,
				SLIDER_Min,   1,
				SLIDER_Max,   9,
				SLIDER_Ticks,      5,
				SLIDER_ShortTicks, TRUE,
				SLIDER_Orientation, SORIENT_HORIZ,
				SLIDER_LevelMaxLen, 4,
				SLIDER_LevelHook,   win_fs_lvlHook,
				SLIDER_LevelFormat, "%s",
				//SLIDER_LevelFormat, "x%ld",
				SLIDER_LevelPlace,  PLACETEXT_IN,
			TAG_DONE),
			//CHILD_WeightedWidth, 0,
			CHILD_WeightedHeight, 0,
			CHILD_Label, IIntuition->NewObject(LabelClass, NULL,// "label.image",
				LABEL_Text, GetString(&li, MSG_GUI_GFXSCALE),
			TAG_DONE),
			LAYOUT_AddChild, OBJ(OID_FRAMESKIP) = IIntuition->NewObject(SliderClass, NULL,
				//GA_ID,         OID_GFXSCALE,
				//GA_RelVerify,  TRUE,
				GA_HintInfo, GetString(&li, MSG_GUI_FRAMESKIP_HELP),
				//SLIDER_Level, 1,
				SLIDER_Min,  0,
				SLIDER_Max, 10,
				SLIDER_Ticks,      10+1,
				SLIDER_ShortTicks, TRUE,
				SLIDER_Orientation, SORIENT_HORIZ,
				SLIDER_LevelDomain, "88",
				SLIDER_LevelFormat, "%ld",
				//SLIDER_LevelPlace,  PLACETEXT_LEFT,
			TAG_DONE),
			//CHILD_WeightedWidth, 0,
			CHILD_WeightedHeight, 0,
			CHILD_Label, IIntuition->NewObject(LabelClass, NULL,// "label.image",
				LABEL_Text, GetString(&li, MSG_GUI_FRAMESKIP),
			TAG_DONE),
// FORCESIZE
			LAYOUT_AddChild, OBJ(OID_FORCESIZE_GROUP) = IIntuition->NewObject(LayoutClass, NULL, //"layout.gadget",
				LAYOUT_BevelStyle, BVS_SBAR_VERT,
				//LAYOUT_HorizAlignment, LALIGN_RIGHT,
				LAYOUT_SpaceInner, FALSE,
				LAYOUT_SpaceOuter, TRUE,
				LAYOUT_AddChild, OBJ(OID_FORCESIZE) = IIntuition->NewObject(CheckBoxClass, NULL, //"checkbox.gadget",
					GA_ID,        OID_FORCESIZE,
					GA_RelVerify, TRUE,
					GA_Text,      GetString(&li, MSG_GUI_FORCESIZE),//"Force size",
					GA_HintInfo,  GetString(&li, MSG_GUI_FORCESIZE_HELP),
					GA_Selected,  gui->forcesize,
					CHECKBOX_TextPlace, PLACETEXT_LEFT,
				TAG_DONE),
				CHILD_WeightedWidth, 0,
				LAYOUT_AddChild, OBJ(OID_FS_WIDTH) = IIntuition->NewObject(IntegerClass, NULL, //"integer.gadget",
					GA_ID,        OID_FS_WIDTH,
					GA_RelVerify, TRUE,
					GA_Disabled,  !gui->forcesize,
					INTEGER_Number,  gui->myTT.forcesize_w,
					INTEGER_Minimum, FORCESIZE_W_MIN,
					INTEGER_Maximum, 8192,
					INTEGER_MinVisible, 4,
				TAG_DONE),
				//CHILD_WeightedWidth, 0,
				LAYOUT_AddChild, OBJ(OID_FS_HEIGHT) = IIntuition->NewObject(IntegerClass, NULL, //"integer.gadget",
					GA_ID,        OID_FS_HEIGHT,
					GA_RelVerify, TRUE,
					GA_Disabled,  !gui->forcesize,
					INTEGER_Number,  gui->myTT.forcesize_h,
					INTEGER_Minimum, FORCESIZE_H_MIN,
					INTEGER_Maximum, 4320,
					INTEGER_MinVisible, 4,
				TAG_DONE),
				CHILD_Label, IIntuition->NewObject(LabelClass, NULL,// "label.image",
					LABEL_Text, " x",
				TAG_DONE),
				//CHILD_WeightedWidth, 0,
			TAG_DONE),
			CHILD_WeightedHeight, 0,
			//CHILD_WeightedWidth, 0,
			TAG_DONE), // END of [SAVESTATES + GFXSCALE + FRAMESKIP] + [FORCESIZE]
			CHILD_WeightedHeight, 0,

			LAYOUT_AddChild, IIntuition->NewObject(SpaceClass, NULL, //"space.gadget",
				SPACE_MinHeight, 10,
			TAG_DONE),

// TOTALROMS
			LAYOUT_AddChild, OBJ(OID_TOTALROMS) = IIntuition->NewObject(ButtonClass, NULL, //"button.gadget",
				//GA_ID,         OID_TOTALROMS,
				//GA_RelVerify,  TRUE,
				GA_ReadOnly,   TRUE,
				GA_Underscore, 0,
				GA_Text,       GetString(&li, MSG_GUI_TOTALROMS),
				BUTTON_Justification, BCJ_LEFT,
				BUTTON_BevelStyle,    BVS_NONE,
				BUTTON_Transparent,   TRUE,
			TAG_DONE),
			//CHILD_WeightedWidth,  0,
			CHILD_WeightedHeight, 0,

		TAG_DONE), // END of LEFTSIDE_GADGETS
		CHILD_WeightedWidth, 35,
