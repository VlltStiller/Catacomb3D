/* Catacomb 3-D Source Code
 * Copyright (C) 1993-2014 Flat Rock Software
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

//
//      ID Engine
//      ID_US.c - User Manager - User interface
//      v1.1d1
//      By Jason Blochowiak
//      Hacked up for Catacomb 3D
//

#include "c3_def.h"


//      Special imports
		ScanCode        firescan;

//      Global variables
static ControlPanelExitType panelExit;
static		boolean         ingame;
//,abortgame,loadedgame;
//static		GameDiff        restartgame = gd_Continue;

//      Internal variables
static  boolean         GameIsDirty,
					QuitToDos,
					CtlPanelDone;

//      Forward reference prototypes
static void     USL_SetupCard(void);

//      Control panel data

#define CtlPanelSX      74
#define CtlPanelSY      48
#define CtlPanelEX      234
#define CtlPanelEY      150
#define CtlPanelW       (CtlPanelEX - CtlPanelSX)
#define CtlPanelH       (CtlPanelEY - CtlPanelSY)

#define TileBase        92

// DEBUG - CGA
#define BackColor               0
#define HiliteColor             (BackColor ^ 12)
#define NohiliteColor   (BackColor ^ 4)

typedef enum
		{
			uc_None,
			uc_Return,
			uc_Abort,
			uc_Quit,
			uc_Loaded,
			uc_SEasy,
			uc_SNormal,
			uc_SHard,
		} UComm;
typedef enum
		{
			uii_Bad,
			uii_Button,uii_RadioButton,uii_Folder
		} UIType;
typedef enum
		{
			ui_Normal = 0,
			ui_Pushed = 1,
			ui_Selected = 2,
			ui_Disabled = 4,
			ui_Separated = 8
		} UIFlags;
#define UISelectFlags (ui_Pushed | ui_Selected | ui_Disabled)

typedef enum
		{
			uic_SetupCard,uic_DrawCard,uic_TouchupCard,
			uic_DrawIcon,uic_Draw,uic_Hit
		} UserCall;

typedef struct  UserItem
		{
				UIType                  type;
				UIFlags                 flags;
				ScanCode                hotkey;
				char                    *text;
				UComm                   comm;
				void                    *child;     // Should be (UserItemGroup *)

				word                    x,y;
		} UserItem;
typedef struct  UserItemGroup
		{
				word                    x,y;
				graphicnums             title;
				ScanCode                hotkey;
				UserItem                *items;
				boolean                 (*custom)(UserCall,struct UserItem *);      // Custom routine

				word                    cursor;
		struct  UserItemGroup  	        *parent;
		} UserItemGroup;

static  char            *BottomS1,*BottomS2,*BottomS3;
static  UComm           Communication;
static  ScanCode        *KeyMaps[] =
					{
						&KbdDefs.button0,
						&KbdDefs.button1,
						&firescan,
						&KbdDefs.up,
						&KbdDefs.right,
						&KbdDefs.down,
						&KbdDefs.left
					};

// Custom routine prototypes
static  boolean USL_ConfigCustom(UserCall call,struct UserItem *item),
				USL_KeyCustom(UserCall call,struct UserItem *item),
				USL_KeySCustom(UserCall call,struct UserItem *item),
				USL_LoadCustom(UserCall call,struct UserItem *item),
				USL_SaveCustom(UserCall call,struct UserItem *item),
				USL_PongCustom(UserCall call,struct UserItem *item);

#define DefButton(key,text)                             uii_Button,ui_Normal,key,text
#define DefRButton(key,text)                    uii_RadioButton,ui_Normal,key,text
#define DefFolder(key,text,child)               uii_Folder,ui_Normal,key,text,uc_None,child
#define CustomGroup(title,key,custom)   0,0,title,key,0,custom
	UserItem holder[] =
	{
		{DefButton(sc_None,"DEBUG")},
		{uii_Bad}
	};
	UserItemGroup   holdergroup = {0,0,CP_MAINMENUPIC,sc_None,holder};

	// Sound menu
	UserItem soundi[] =
	{
		{DefRButton(sc_N,"NO SOUND EFFECTS")},
//		{DefRButton(sc_P,"PC SPEAKER")},
		{DefRButton(sc_A,"ADLIB/SOUNDBLASTER")},
		{uii_Bad}
	};
	UserItemGroup   soundgroup = {8,0,CP_SOUNDMENUPIC,sc_None,soundi};

	// Music menu
	UserItem musici[] =
	{
		{DefRButton(sc_N,"NO MUSIC")},
		{DefRButton(sc_A,"ADLIB/SOUNDBLASTER")},
		{uii_Bad}
	};
	UserItemGroup   musicgroup = {8,0,CP_MUSICMENUPIC,sc_None,musici};

	// New game menu
	UserItem newgamei[] =
	{
		{DefButton(sc_E,"BEGIN EASY GAME"),uc_SEasy},
		{DefButton(sc_N,"BEGIN NORMAL GAME"),uc_SNormal},
		{DefButton(sc_H,"BEGIN HARD GAME"),uc_SHard},
		{uii_Bad}
	};
	UserItemGroup   newgamegroup = {8,0,CP_NEWGAMEMENUPIC,sc_None,newgamei,0,1};

	// Load/Save game menu
	UserItem loadsavegamei[] =
	{
		{uii_Button,ui_Normal,sc_None},
		{uii_Button,ui_Normal,sc_None},
		{uii_Button,ui_Normal,sc_None},
		{uii_Button,ui_Normal,sc_None},
		{uii_Button,ui_Normal,sc_None},
		{uii_Button,ui_Normal,sc_None},
		{uii_Bad}
	};
	UserItemGroup   loadgamegroup = {4,3,CP_LOADMENUPIC,sc_None,loadsavegamei,USL_LoadCustom};
	UserItemGroup   savegamegroup = {4,3,CP_SAVEMENUPIC,sc_None,loadsavegamei,USL_SaveCustom};


	// Keyboard menu
	UserItem keyi[] =
	{
		{DefButton(sc_None,"UP")},
		{DefButton(sc_None,"RIGHT")},
		{DefButton(sc_None,"DOWN")},
		{DefButton(sc_None,"LEFT")},
		{uii_Bad}
	};
	UserItemGroup   keygroup = {0,0,CP_KEYMOVEMENTPIC,sc_None,keyi,USL_KeyCustom};
	UserItem keybi[] =
	{
		{DefButton(sc_J,"FIRE")},
		{DefButton(sc_P,"STRAFE")},
		{uii_Bad}
	};
	UserItemGroup   keybgroup = {0,0,CP_KEYBUTTONPIC,sc_None,keybi,USL_KeyCustom};
	UserItem keysi[] =
	{
		{DefFolder(sc_M,"MOVEMENT",&keygroup)},
		{DefFolder(sc_B,"BUTTONS",&keybgroup)},
		{uii_Bad}
	};
	UserItemGroup   keysgroup = {8,0,CP_KEYBOARDMENUPIC,sc_None,keysi,USL_KeySCustom};

	// Config menu
	UserItem configi[] =
	{
		{DefFolder(sc_S,"SOUND",&soundgroup)},
		{DefFolder(sc_M,"MUSIC",&musicgroup)},
		{uii_Folder,ui_Separated,sc_K,"SETUP CONTROLS",uc_None,&keysgroup},
		{uii_Bad}
	};
	UserItemGroup   configgroup = {8,0,CP_CONFIGMENUPIC,sc_None,configi,USL_ConfigCustom};

	// Main menu
	UserItemGroup   ponggroup = {0,0,0,sc_None,0,USL_PongCustom};
	UserItem rooti[] =
	{
		{DefFolder(sc_N,"NEW GAME",&newgamegroup)},
		{DefFolder(sc_L,"LOAD GAME",&loadgamegroup)},
		{DefFolder(sc_S,"SAVE GAME",&savegamegroup)},
		{DefFolder(sc_C,"CONFIGURE",&configgroup)},
		{DefButton(sc_R,NULL),uc_Return},        // Return to Game/Demo
		{DefButton(sc_E,"END GAME"),uc_Abort},
		{DefFolder(sc_B,"SKULL 'N' BONES",&ponggroup)},
		{DefButton(sc_Q,"QUIT"),uc_Quit},
		{uii_Bad}
	};
	UserItemGroup   rootgroup = {32,4,CP_MAINMENUPIC,sc_None,rooti};
#undef  DefButton
#undef  DefFolder

#define MaxCards        7
	word                    cstackptr;
	UserItemGroup   *cardstack[MaxCards],
				 *topcard;

//      Card stack code
static void
USL_SetupStack(void)
{
	cstackptr = 0;
	cardstack[0] = topcard = &rootgroup;
}

static void
USL_PopCard(void)
{
	if (!cstackptr)
		return;

	topcard = cardstack[--cstackptr];
}

static void
USL_PushCard(UserItemGroup *card)
{
	if (cstackptr == MaxCards - 1)
		return;

	topcard = cardstack[++cstackptr] = card;
}

static void
USL_DrawItemIcon(UserItem *item)
{
	word    flags,tile;

	if (topcard->custom && topcard->custom(uic_DrawIcon,item))
		return;

	flags = item->flags;
	if (flags & ui_Disabled)
		tile = TileBase + ((flags & ui_Selected)? 5 : 4);
	else if ((item->type == uii_RadioButton) && (!(flags & ui_Pushed)))
		tile = TileBase + ((flags & ui_Selected)? 3 : 2);
	else
		tile = TileBase + ((flags & ui_Selected)? 1 : 0);
	VW_DrawTile8(item->x/8,item->y,tile);
}

static void
USL_DrawItem(UserItem *item)
{
	if (topcard->custom && topcard->custom(uic_Draw,item))
		return;

	VW_Bar(CtlPanelSX + 1,item->y,
			CtlPanelEX - CtlPanelSX - 1,8,BackColor);       // Clear out background
	USL_DrawItemIcon(item);
	if ((item->flags & ui_Selected) && !(item->flags & ui_Disabled))
		fontcolor = HiliteColor;
	else
		fontcolor = NohiliteColor;
	px = item->x + 8;
	py = item->y + 1;
	USL_DrawString(item->text);
	fontcolor = F_BLACK;
}

#define MyLine(y)       VW_Hlin(CtlPanelSX + 3,CtlPanelEX - 3,y,12);

static void
USL_DrawBottom(void)
{
	word    w,h;

	fontcolor = NohiliteColor;

	px = CtlPanelSX + 4;
	py = CtlPanelEY - 15;
	USL_DrawString(BottomS1);

	USL_MeasureString(BottomS2,&w,&h);
	px = CtlPanelEX - 4 - w;
	USL_DrawString(BottomS2);

	USL_MeasureString(BottomS3,&w,&h);
	px = CtlPanelSX + ((CtlPanelEX - CtlPanelSX - w) / 2);
	py += h + 1;
	USL_DrawString(BottomS3);

	fontcolor = F_WHITE;
	MyLine(CtlPanelEY - 17);
}

static void
USL_DrawCtlPanelContents(void)
{
	int                             x,y;
	UserItem               		    *item;

	if (topcard->custom && topcard->custom(uic_DrawCard,NULL))
		return;

	if (topcard->title)
	{
		// Draw the title
		MyLine(CtlPanelSY + 7);
		SPG_DrawPic(&guiBuffer, grsegs[topcard->title], (CtlPanelSX + 6)/8*8,CtlPanelSY);

	}

	USL_DrawBottom();

	if (!topcard->items)
		return;

	x = topcard->x + CtlPanelSX;
	if (x % 8)
		x += 8 - (x % 8);
	y = topcard->y + CtlPanelSY + 12;
	for (item = topcard->items;item->type != uii_Bad;item++)
	{
		if (item->flags & ui_Separated)
			y += 8;

		item->x = x;
		item->y = y;
		USL_DrawItem(item);
		y += 8;
	}
	if (topcard->custom)
		topcard->custom(uic_TouchupCard,NULL);
}

static void
USL_DrawCtlPanel(void)
{
	if (topcard->items || topcard->title)
	{
		SPG_ClearBuffer(0);

		// Draw the backdrop
		SPG_DrawPic(&guiBuffer, grsegs[CP_MENUSCREENPIC],0,0);

		// Draw the contents
		USL_DrawCtlPanelContents();
	}

	// Refresh the screen
	SPG_FlipBuffer();
}

static void
USL_DialogSetup(word w,word h,word *x,word *y)
{
	SPG_DrawPic(&guiBuffer, grsegs[CP_MENUMASKPICM], CtlPanelSX/8*8,CtlPanelSY);


	*x = CtlPanelSX + ((CtlPanelW - w) / 2);
	*y = CtlPanelSY + ((CtlPanelH - h) / 2);
	VW_Bar(*x,*y,w + 1,h + 1,BackColor);
	VW_Hlin(*x - 1,*x + w + 1,*y - 1,NohiliteColor);
	VW_Hlin(*x - 1,*x + w + 1,*y + h + 1,NohiliteColor);
	VW_Vlin(*y - 1,*y + h + 1,*x - 1,NohiliteColor);
	VW_Vlin(*y - 1,*y + h + 1,*x + w + 1,NohiliteColor);
}

static void
USL_ShowLoadSave(char *s,char *name)
{
	word    x,y,
			w,h,
			tw,sw;
	char    msg[MaxGameName + 4];

	strcpy(msg,"'");
	strcat(msg,name);
	strcat(msg,"'");
	USL_MeasureString(s,&sw,&h);
	USL_MeasureString(msg,&w,&h);
	tw = ((sw > w)? sw : w) + 6;
	USL_DialogSetup(tw,(h * 2) + 2,&x,&y);
	py = y + 2;
	px = x + ((tw - sw) / 2);
	USL_DrawString(s);
	py += h;
	px = x + ((tw - w) / 2);
	USL_DrawString(msg);

	SPG_FlipBuffer();
	SPI_WaitFor(100);
}

static boolean
USL_CtlDialog(char *s1,char *s2,char *s3)
{
	word            w,h,sh,
				w1,w2,w3,
				x,y;
	ScanCode        c;

	USL_MeasureString(s1,&w1,&h);
	USL_MeasureString(s2,&w2,&h);
	if (s3)
		USL_MeasureString(s3,&w3,&h);
	else
		w3 = 0;
	w = (w1 > w2)? ((w1 > w3)? w1 : w3) : ((w2 > w3)? w2 : w3);
	w += 7;
	sh = h;
	h *= s3? 5 : 4;

	USL_DialogSetup(w,h,&x,&y);

	fontcolor = HiliteColor;
	px = x + ((w - w1) / 2);
	py = y + sh + 1;
	USL_DrawString(s1);
	py += (sh * 2) - 1;

	VW_Hlin(x + 3,x + w - 3,py,NohiliteColor);
	py += 2;

	fontcolor = NohiliteColor;
	px = x + ((w - w2) / 2);
	USL_DrawString(s2);
	py += sh;

	if (s3)
	{
		px = x + ((w - w3) / 2);
		USL_DrawString(s3);
	}

	SPG_FlipBuffer();

	SPI_ClearKeysDown();
	do
	{
		c = SPI_GetLastKey();
		if (c == but_Mouse1) {
			c = sc_Y;
		} else if (c == but_Mouse2) {
			c = sc_Escape;
		}
	} while (c == sc_None);

	SPI_ClearKeysDown();
	USL_DrawCtlPanel();
	return(c == sc_Y);
}

static boolean
USL_ConfirmComm(UComm comm)
{
	boolean confirm,dialog;
	char    *s1,*s2,*s3;

	if (!comm)
		Quit("USL_ConfirmComm() - empty comm");

	confirm = true;
	dialog = false;
	s3 = "ESC TO BACK OUT";
	switch (comm)
	{
	case uc_Abort:
		s1 = "REALLY END CURRENT GAME?";
		s2 = "PRESS Y TO END IT";
		if (ingame && GameIsDirty)
			dialog = true;
		break;
	case uc_Quit:
		s1 = "REALLY QUIT?";
		s2 = "PRESS Y TO QUIT";
		dialog = true;
		break;
	case uc_Loaded:
		s1 = "YOU'RE IN A GAME";
		s2 = "PRESS Y TO LOAD GAME";
		if (ingame && GameIsDirty)
			dialog = true;
		break;
	case uc_SEasy:
	case uc_SNormal:
	case uc_SHard:
		s1 = "YOU'RE IN A GAME";
		s2 = "PRESS Y FOR NEW GAME";
		if (ingame && GameIsDirty)
			dialog = true;
		break;
	}

	confirm = dialog? USL_CtlDialog(s1,s2,s3) : true;
	if (confirm)
	{
		Communication = comm;
		CtlPanelDone = true;
	}
	return(confirm);
}

///////////////////////////////////////////////////////////////////////////
//
//      USL_HandleError() - Handles telling the user that there's been an error
//
///////////////////////////////////////////////////////////////////////////
static void
USL_HandleError(int num)
{
	char    buf[64];

	strcpy(buf,"Error: ");
	if (num < 0)
		strcat(buf,"Unknown");
	else if (num == ENOMEM)
		strcat(buf,"Disk is Full");
	else
		strcat(buf,sys_errlist[num]);

	USL_CtlDialog(buf,"PRESS ANY KEY",NULL);
	SPG_FlipBuffer();

	SPI_ClearKeysDown();
	SPI_WaitForever();

	SPG_FlipBuffer();
}

static boolean
USL_ConfigCustom(UserCall call,UserItem *item)
{
		char    *s;
		word    w,h,
				tw;

	if (call == uic_TouchupCard)
	{
		s = "CONTROL: ";
		USL_MeasureString(s,&w,&h);
		tw = w;
		USL_MeasureString("KEYBOARD",&w,&h);
		tw += w;
		py = CtlPanelEY - 18 - h;
		px = CtlPanelSX + ((CtlPanelW - tw) / 2);
		fontcolor = NohiliteColor;
		USL_DrawString(s);
		USL_DrawString("KEYBOARD");
	}
	item++; // Shut the compiler up
	return(false);
}

static void
USL_CKSetKey(UserItem *item,word i)
{
	boolean         on;
	word            j;
	ScanCode        scan;
	longword        time;

	on = false;
	time = 0;
	SPI_ClearKeysDown();
	fontcolor = HiliteColor;
	do
	{
		if (SP_TimeCount() >= time)
		{
			on ^= true;
			VW_Bar(item->x + 90,item->y,40,8,fontcolor ^ BackColor);
			VW_Bar(item->x + 90 + 1,item->y + 1,40 - 2,8 - 2,BackColor);
			if (on)
				VW_DrawTile8((item->x + 90 + 16)/8,item->y,TileBase + 8);

			SPG_FlipBuffer();

			time = SP_TimeCount() + (TickBase / 2);
		}

		if (SPI_GetLastKey() == sc_LShift) {
			SPI_ClearKeysDown();
		}
	} while (!(scan = SPI_GetLastKey()));

	if (scan != sc_Escape)
	{
		for (j = 0;j < 7;j++)
		{
			if (j == i)
				continue;
			if (*(KeyMaps[j]) == scan) {
				*KeyMaps[j] = 0;
			}
		}
		*KeyMaps[i] = scan;
	} else {
		*KeyMaps[i] = 0;
	}
	SPI_ClearKeysDown();
}

static boolean
USL_KeySCustom(UserCall call,UserItem *item)
{
	return(false);
}

static boolean
USL_KeyCustom(UserCall call,UserItem *item)
{
	boolean result;
	word    i;

	result = false;
	i = (topcard == &keygroup)? (3 + (item - keyi)) : (item - keybi);
	switch (call)
	{
	case uic_SetupCard:
		break;
	case uic_Draw:
		VW_Bar(CtlPanelSX + 1,item->y,
				CtlPanelEX - CtlPanelSX - 1,8,BackColor);       // Clear out background
		USL_DrawItemIcon(item);
		fontcolor = (item->flags & ui_Selected)? HiliteColor : NohiliteColor;
		px = item->x + 8;
		py = item->y + 1;
		USL_DrawString(item->text);
		VW_Bar(item->x + 90,item->y,40,8,fontcolor ^ BackColor);
		VW_Bar(item->x + 90 + 1,item->y + 1,40 - 2,8 - 2,BackColor);
		px = item->x + 90 + 6;
		py = item->y + 1;
		USL_DrawString(SPI_GetScanName(*KeyMaps[i]));
		result = true;
		break;
	case uic_Hit:
		USL_KeyCustom(uic_Draw,item);
		USL_CKSetKey(item,i);
		USL_DrawCtlPanel();
		result = true;
		break;
	}
	return(result);
}

static void
USL_DrawFileIcon(UserItem *item)
{
	word    color;

	item->y = topcard->y + CtlPanelSY + 12;
	item->y += (item - loadsavegamei) * 11;

	fontcolor = (item->flags & ui_Selected)? HiliteColor : NohiliteColor;
	color = fontcolor ^ BackColor;  // Blech!
	VW_Hlin(item->x,item->x + (CtlPanelW - 12),item->y,color);
	VW_Hlin(item->x,item->x + (CtlPanelW - 12),item->y + 9,color);
	VW_Vlin(item->y,item->y + 9,item->x,color);
	VW_Vlin(item->y,item->y + 9,item->x + (CtlPanelW - 12),color);
}

static void
USL_DoLoadGame(UserItem *item)
{
	char            *filename;
	word            n,
				err;
	FILE*                     file;
	SaveGame        *game;

	if (!USL_ConfirmComm(uc_Loaded))
		return;

	n = item - loadsavegamei;
	game = &Games[n];

	USL_ShowLoadSave("Loading",game->name);
	filename = USL_GiveSaveName(n);
	strncpy(panelExit.SavegameToLoad, filename, 1000);
	panelExit.SavegameSkip = sizeof(SaveGame);

/*	err = 0;
	if ((file = fopen(filename,"r")) != NULL)
	{
		if (fread(game,1,sizeof(*game),file) == sizeof(*game))
		{
			if (USL_LoadGame) {
				if (!USL_LoadGame(file, NULL)) {
					USL_HandleError(err = errno);
				}
			}
		}
		else
			USL_HandleError(err = errno);
		fclose(file);
	}
	else
		USL_HandleError(err = errno);
	if (err)
	{
		panelExit.Result = CPE_ABORTGAME;
		Communication = uc_None;
		CtlPanelDone = false;
	}
	else*/
	{
		panelExit.Result = CPE_LOADEDGAME;
	}
	game->present = true;

//	USL_DrawCtlPanel();
}

static boolean
USL_LoadCustom(UserCall call,UserItem *item)
{
	boolean result;
	word    i;

	result = false;
	switch (call)
	{
	case uic_SetupCard:
		for (i = 0;i < MaxSaveGames;i++)
		{
			if (Games[i].present)
				loadsavegamei[i].flags &= ~ui_Disabled;
			else
				loadsavegamei[i].flags |= ui_Disabled;
		}
		break;
	case uic_DrawIcon:
		USL_DrawFileIcon(item);
		result = true;
		break;
	case uic_Draw:
		USL_DrawFileIcon(item);
		VW_Bar(item->x + 1,item->y + 2,CtlPanelW - 12 - 2,7,BackColor);

		i = item - loadsavegamei;
		if (Games[i].present)
			px = item->x + 2;
		else
			px = item->x + 60;
		py = item->y + 2;
		USL_DrawString(Games[i].present? Games[i].name : "Empty");
		result = true;
		break;
	case uic_Hit:
		USL_DoLoadGame(item);
		result = true;
		break;
	}
	return(result);
}

static void
USL_DoSaveGame(UserItem *item)
{
	boolean         ok;
	char            *filename;
	word            n,err;
	FILE            *file;
	SaveGame        *game;

	BottomS1 = "Type name";
	BottomS2 = "Enter accepts";
	USL_DrawCtlPanel();

	n = item - loadsavegamei;
	game = &Games[n];
	fontcolor = HiliteColor;
	VW_Bar(item->x + 1,item->y + 2,CtlPanelW - 12 - 2,7,BackColor);
	game->oldtest = &PrintX;
	ok = US_LineInput(item->x + 2,item->y + 2,
						game->name,game->present? game->name : NULL,
						true,MaxGameName,
						CtlPanelW - 22,0);
	if (!strlen(game->name))
		strcpy(game->name,"Untitled");
	if (ok)
	{
		USL_ShowLoadSave("Saving",game->name);

		filename = USL_GiveSaveName(n);
		err = 0;
		file = fopen(filename,"w");
		if (file != NULL)
		{
			if (fwrite(game,1,sizeof(*game),file) == sizeof(*game))
			{
				if (USL_SaveGame)
					ok = USL_SaveGame(file);
				else
					printf("No save hook !\n");
				if (!ok)
					USL_HandleError(err = errno);
			}
			else
				USL_HandleError(err = ((errno == ENOENT)? ENOMEM : errno));
			fclose(file);
		}
		else
			USL_HandleError(err = ((errno == ENOENT)? ENOMEM : errno));
		if (err)
		{
			remove(filename);
			ok = false;
		}

	}

	if (!game->present)
		game->present = ok;

	if (ok)
		GameIsDirty = false;
	USL_SetupCard();
}

static boolean
USL_SaveCustom(UserCall call,UserItem *item)
{
	word    i;

	switch (call)
	{
	case uic_SetupCard:
		for (i = 0;i < MaxSaveGames;i++)
			loadsavegamei[i].flags &= ~ui_Disabled;
		return(false);
	case uic_Hit:
		USL_DoSaveGame(item);
		return(true);
//              break;
	}
	return(USL_LoadCustom(call,item));
}

#define PaddleMinX      (CtlPanelSX + 3)
#define PaddleMaxX      (CtlPanelEX - 15)
#define BallMinX        (CtlPanelSX + 2)
#define BallMinY        (CtlPanelSY + 12 + 2)
#define BallMaxX        (CtlPanelEX - 6)
#define BallMaxY        (CtlPanelEY - 13)
#define CPaddleY        (BallMinY + 4)
#define KPaddleY        (BallMaxY - 2)
void
USL_DrawPongScore(word k,word c)
{
	fontcolor = HiliteColor;
	PrintY = py = CtlPanelSY + 4;
	px = CtlPanelSX + 6;
	VW_Bar(px,py,42,6,BackColor);
	USL_DrawString("YOU:");
	PrintX = px;
	US_PrintUnsigned(k);
	px = CtlPanelSX + 108;
	VW_Bar(px,py,50,6,BackColor);
	USL_DrawString("COMP:");
	PrintX = px;
	US_PrintUnsigned(c);
}

void
USL_PlayPong(void)
{
	boolean         ball,killball,revdir,done,lastscore;
	word            cycle,
				x,y,
				kx,cx,
				rx,
				bx,by,
				kscore,cscore,
				speedup;
	int                     bdx,bdy;
	longword        balltime,waittime;
	int 		dx;

	kx = cx = PaddleMinX + ((PaddleMaxX - PaddleMinX) / 2);
	bx = by = bdx = bdy = 0;
	kscore = cscore = 0;
	USL_DrawPongScore(0,0);
	cycle = 0;
	revdir = false;
	killball = true;
	done = false;
	lastscore = false;
	do
	{
		waittime = SP_TimeCount();

		SPI_GetMouseDelta(&dx,NULL);
		if (((dx < 0) || SPI_GetKeyDown(sc_LeftArrow)) && (kx > PaddleMinX))
			kx -= 2;
		else if (((dx > 0) || SPI_GetKeyDown(sc_RightArrow)) && (kx < PaddleMaxX))
			kx += 2;

		if (killball)
		{
			ball = false;
			balltime = SP_TimeCount() + TickBase;
			speedup = 10;
			killball = false;
		}

		if (ball && (cycle++ % 3))
		{
			x = (bx >> 2);
			if (!(x & 1))
				x += (US_RndT() & 1);

			if ((cx + 6 < x) && (cx < PaddleMaxX))
				cx += 1;
			else if ((cx + 6 > x) && (cx > PaddleMinX))
				cx -= 1;
		}

		VW_Bar(BallMinX,BallMinY - 1,
				BallMaxX - BallMinX + 5,BallMaxY - BallMinY + 7,
				BackColor);

		SPG_DrawPic(&guiBuffer, grsegs[PADDLESPR], cx&0xFFFE, CPaddleY);
		SPG_DrawPic(&guiBuffer, grsegs[PADDLESPR], kx&0xFFFE, KPaddleY);
		if (ball)
		{
			if
			(
				(((bx + bdx) >> 2) > BallMaxX)
			||      (((bx + bdx) >> 2) < BallMinX)
			)
			{
				SPA_PlaySound(BALLBOUNCESND);
				bdx = -bdx;
			}
			bx += bdx;

			if (((by + bdy) >> 2) > BallMaxY)
			{
				killball = true;
				lastscore = false;
				cscore++;
				SPA_PlaySound(COMPSCOREDSND);
				USL_DrawPongScore(kscore,cscore);
				if (cscore == 21)
				{
					USL_CtlDialog("You lost!","Press any key",NULL);
					done = true;
					continue;
				}
			}
			else if (((by + bdy) >> 2) < BallMinY)
			{
				killball = true;
				lastscore = true;
				kscore++;
				SPA_PlaySound(KEENSCOREDSND);
				USL_DrawPongScore(kscore,cscore);
				if (kscore == 21)
				{
					USL_CtlDialog("You won!","Press any key",NULL);
					done = true;
					continue;
				}
			}
			by += bdy;

			x = bx >> 2;
			y = by >> 2;
			if (!killball)
			{
				if
				(
					(bdy < 0)
				&&      ((y >= CPaddleY) && (y < CPaddleY + 3))
				&&      ((x >= (cx - 5)) && (x < (cx + 11)))
				)
				{
					rx = cx;
					revdir = true;
					SPA_PlaySound(COMPPADDLESND);
				}
				else if
				(
					(bdy > 0)
				&&      ((y >= (KPaddleY - 3)) && (y < KPaddleY))
				&&      ((x >= (kx - 5)) && (x < (kx + 11)))
				)
				{
					if (((bdy >> 2) < 3) && !(--speedup))
					{
						bdy++;
						speedup = 10;
					}
					rx = kx;
					revdir = true;
					SPA_PlaySound(KEENPADDLESND);
				}
				if (revdir)
				{
					bdy = -bdy;
					bdx = ((x + 5 - rx) >> 1) - (1 << 2);
					if (!bdx)
						bdx--;
					revdir = false;
				}
			}
			SPG_DrawPic(&guiBuffer, grsegs[(x & 1)? BALL1PIXELTOTHERIGHTSPR : BALLSPR], x&0xFFFE, y);
		}
		else if (SP_TimeCount() >= balltime)
		{
			ball = true;
			bdx = 1 - (US_RndT() % 3);
			bdy = 2;
			if (lastscore)
				bdy = -bdy;
			bx = (BallMinX + ((BallMaxX - BallMinX) / 2)) << 2;
			by = (BallMinY + ((BallMaxY - BallMinY) / 2)) << 2;
		}
		SPG_FlipBuffer();
		while (waittime == SP_TimeCount())
			;       // DEBUG - do adaptiveness
	} while ((SPI_GetLastKey() != sc_Escape) && !done);
	SPI_ClearKeysDown();
}

static boolean
USL_PongCustom(UserCall call,struct UserItem *item)
{
	if (call != uic_SetupCard)
		return(false);


	SPG_DrawPic(&guiBuffer, grsegs[CP_MENUSCREENPIC],0,0);
	SPG_DrawPic(&guiBuffer, grsegs[CP_PADDLEWARPIC],(CtlPanelSX+56)/8*8,CtlPanelSY);
	VW_Hlin(CtlPanelSX + 3,CtlPanelEX - 3,CtlPanelSY + 12,HiliteColor ^ BackColor);
	VW_Hlin(CtlPanelSX + 3,CtlPanelEX - 3,CtlPanelEY - 7,HiliteColor ^ BackColor);
	USL_PlayPong();

	return(true);
}

//      Flag management stuff
static void
USL_ClearFlags(UserItemGroup *node)
{
	UserItem        *i;

	if (!node->items)
		return;

	for (i = node->items;i->type != uii_Bad;i++)
	{
		i->flags &= ~UISelectFlags;
		if (i->child)
			USL_ClearFlags((UserItemGroup *)i->child);
	}
}

static int
USL_FindPushedItem(UserItemGroup *group)
{
	word            i;
	UserItem        *item;

	for (item = group->items,i = 0;item->type != uii_Bad;item++,i++)
		if (item->flags & ui_Pushed)
			return(i);
	return(-1);
}

static void
USL_SelectItem(UserItemGroup *group,word index,boolean draw)
{
	UserItem        *item;

	if (index != group->cursor)
	{
		item = &group->items[group->cursor];
		item->flags &= ~ui_Selected;
		if (draw)
			USL_DrawItem(item);
	}

	group->cursor = index;
	item = &group->items[group->cursor];
	group->items[group->cursor].flags |= ui_Selected;
	if (draw)
		USL_DrawItem(item);
}

static void
USL_PushItem(UserItemGroup *group,word index,boolean draw)
{
	word            i;
	UserItem        *item;

	USL_SelectItem(group,index,draw);
	for (item = group->items,i = 0;item->type != uii_Bad;item++,i++)
	{
		if (item->type != uii_RadioButton)
			continue;

		if (i == index)
		{
			item->flags |= ui_Pushed;
			if (draw)
				USL_DrawItem(item);
		}
		else if (item->flags & ui_Pushed)
		{
			item->flags &= ~ui_Pushed;
			if (draw)
				USL_DrawItem(item);
		}
	}
}

static void
USL_NextItem(void)
{
	if (topcard->items[topcard->cursor + 1].type == uii_Bad)
		return;
	USL_SelectItem(topcard,topcard->cursor + 1,true);
}

static void
USL_PrevItem(void)
{
	if (!topcard->cursor)
		return;
	USL_SelectItem(topcard,topcard->cursor - 1,true);
}

static void
USL_SetupCard(void)
{
	BottomS1 = "Arrows move";
	BottomS2 = "Enter selects";
	BottomS3 = cstackptr? "ESC to back out" : "ESC to quit";

	USL_SelectItem(topcard,topcard->cursor,false);
	USL_DrawCtlPanel();     // Contents?
}

static void
USL_DownLevel(UserItemGroup *group)
{
	if (!group)
		Quit("USL_DownLevel() - NULL card");
	USL_PushCard(group);
	if (group->custom && group->custom(uic_SetupCard,NULL))
		USL_PopCard();
	USL_SetupCard();
}

static void
USL_UpLevel(void)
{
	if (!cstackptr)
	{
		USL_ConfirmComm(uc_Quit);
		return;
	}

	if (topcard->items)
		topcard->items[topcard->cursor].flags &= ~ui_Selected;
	USL_PopCard();
	USL_SetupCard();
}

static void
USL_DoItem(void)
{
	// DEBUG - finish this routine
	UserItem                *item;

	item = &topcard->items[topcard->cursor];
	if (item->flags & ui_Disabled)
		SPA_PlaySound(NOWAYSND);
	else
	{
		switch (item->type)
		{
		case uii_Button:
			if (!(topcard->custom && topcard->custom(uic_Hit,item)))
				USL_ConfirmComm(item->comm);
			break;
		case uii_RadioButton:
			USL_PushItem(topcard,topcard->cursor,true);
			break;
		case uii_Folder:
			USL_DownLevel(item->child);
			break;
		}
	}
}

static void
USL_SetControlValues(void)
{
	int SoundMode = SPA_GetSoundSource();
	int MusicMode = SPA_GetMusicSource();
	assert(SoundMode >= 0 && SoundMode <= 1);
	assert(MusicMode >= 0 && MusicMode <= 1);
	USL_PushItem(&soundgroup,SoundMode,false);
	USL_PushItem(&musicgroup,MusicMode,false);
	rooti[4].text = ingame? "RETURN TO GAME" : "RETURN TO DEMO";
	if (!ingame)
	{
		rooti[2].flags |= ui_Disabled;  // Save Game
		rooti[5].flags |= ui_Disabled;  // End Game
	}
	rootgroup.cursor = ingame? 4 : 0;
	// DEBUG - write the rest of this
}



///////////////////////////////////////////////////////////////////////////
//
//      USL_SetUpCtlPanel() - Sets the states of the UserItems to reflect the
//              values of all the appropriate variables
//
///////////////////////////////////////////////////////////////////////////
static void
USL_SetUpCtlPanel(void)
{
	int     i;

	// Cache in all of the stuff for the control panel
	for (i = CONTROLS_LUMP_START;i <= CONTROLS_LUMP_END;i++)
		SPD_LoadGrChunk(i);
	for (i = PADDLE_LUMP_START;i <= PADDLE_LUMP_END;i++)
		SPD_LoadGrChunk(i);
	SPD_LoadGrChunk(STARTFONT+1);            // Little font
	SPD_LoadGrChunk(CP_MENUMASKPICM);        // Mask for dialogs

	// Do some other setup
	fontnumber = 1;
	US_SetPrintRoutines(VW_MeasurePropString,VW_DrawPropString);
	fontcolor = F_BLACK;
//	VW_Bar (0,0,320,200,3); // CAT3D patch

	Communication = uc_None;
	USL_ClearFlags(&rootgroup);
	USL_SetControlValues();
	USL_SetupStack();
	USL_SetupCard();

	if (ingame)
		GameIsDirty = true;

	SPI_ClearKeysDown();
}

static void
USL_HandleComm(UComm comm)
{
	switch (comm)
	{
	case uc_Loaded:
	case uc_Return:
		break;
	case uc_Abort:
		panelExit.Result = CPE_ABORTGAME;
		break;
	case uc_Quit:
		QuitToDos = true;
		break;
	case uc_SEasy:
		panelExit.Result = CPE_NEWGAME;
		panelExit.Difficulty = gd_Easy;
		break;
	case uc_SNormal:
		panelExit.Result = CPE_NEWGAME;
		panelExit.Difficulty = gd_Normal;
		break;
	case uc_SHard:
		panelExit.Result = CPE_NEWGAME;
		panelExit.Difficulty = gd_Hard;
		break;

	default:
		Quit("USL_HandleComm() - unknown");
		break;
	}
}

static void
USL_GetControlValues(void)
{
	int     i;
	// DEBUG - write the rest of this
	i = USL_FindPushedItem(&soundgroup);
	SPA_SetSoundSource(i);

	i = USL_FindPushedItem(&musicgroup);
	SPA_SetMusicSource(i);
}

///////////////////////////////////////////////////////////////////////////
//
//      USL_TearDownCtlPanel() - Given the state of the control panel, sets the
//              modes and values as appropriate
//
///////////////////////////////////////////////////////////////////////////
static void
USL_TearDownCtlPanel(void)
{
	USL_GetControlValues();
	if (Communication)
		USL_HandleComm(Communication);

	fontnumber = 0; // Normal font
	fontcolor = F_BLACK;
	if (QuitToDos)
	{
		US_CenterWindow(20,3);
		fontcolor = F_SECONDCOLOR;
		US_PrintCentered("Quitting...");
		fontcolor = F_BLACK;
		SPG_FlipBuffer();
		Quit(NULL);
	}

	SPI_ClearKeysDown();
	SPA_WaitUntilSoundIsDone();
	VW_Bar (0,0,320,200,3); // CAT3D patch
}

///////////////////////////////////////////////////////////////////////////
//
//      US_ControlPanel() - This is the main routine for the control panel
//
///////////////////////////////////////////////////////////////////////////
#define MoveMin 40
ControlPanelExitType US_ControlPanel(boolean Ingame)
{
	boolean         resetitem,on;
	word            i;
	int                     ydelta;
	longword        flashtime;
	UserItem        *item;
	int				dy;

	if ((SPI_GetLastKey() < sc_F1) || (SPI_GetLastKey() > sc_F10))
		SPI_ClearKeysDown();

	panelExit.Result = CPE_NOTHING;
	ingame = Ingame;
	USL_SetUpCtlPanel();
	USL_DrawCtlPanel();

	ydelta = 0;
	for (CtlPanelDone = false,resetitem = on = true;!CtlPanelDone;)
	{
		item = &(topcard->items[topcard->cursor]);

		if (resetitem)
		{
			flashtime = SP_TimeCount() + (TickBase / 2);
			resetitem = false;
		}

		if (SP_TimeCount() >= flashtime)
		{
			on ^= true;
			resetitem = true;
			if (!on)
				item->flags &= ~ui_Selected;
			USL_DrawItemIcon(item);
			item->flags |= ui_Selected;
		}

		SPG_FlipBuffer();
		if (SPG_ResizeNow()) {
			USL_DrawCtlPanel();
		}

		if (SPI_GetLastKey())
		{
			switch (SPI_GetLastKey())
			{
			case sc_UpArrow:
				USL_PrevItem();
				resetitem = true;
				break;
			case sc_DownArrow:
				USL_NextItem();
				resetitem = true;
				break;
			case sc_Return:
				USL_DoItem();
				resetitem = true;
				break;
			case sc_Escape:
				USL_UpLevel();
				resetitem = true;
				break;
			case sc_F1:
				USL_DrawCtlPanel();
				resetitem = true;
				break;
			case but_Mouse1:
				USL_DoItem();
				resetitem = true;
				break;
			case but_Mouse2:
				USL_UpLevel();
				resetitem = true;
				break;
			}

			if (!resetitem && ((SPI_GetLastKey() == KbdDefs.button0) || (SPI_GetLastKey() == KbdDefs.button1) ) )
			{
				USL_DoItem();
				resetitem = true;
			}

			if (!resetitem)
			{
				for (item = topcard->items,i = 0;item->type != uii_Bad;item++,i++)
				{
					if (item->hotkey == SPI_GetLastKey())
					{
						USL_SelectItem(topcard,i,true);
						resetitem = true;
						break;
					}
				}
			}

			SPI_ClearKeysDown();
		}
		else
		{
			SPI_GetMouseDelta(NULL,&dy);
			ydelta += dy;
			if (ydelta < -MoveMin)
			{
				ydelta += MoveMin;
				USL_PrevItem();
				resetitem = true;
			}
			else if (ydelta > MoveMin)
			{
				ydelta -= MoveMin;
				USL_NextItem();
				resetitem = true;
			}
		}
	}

	USL_TearDownCtlPanel();

	return panelExit;
}
