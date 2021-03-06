/* Catacomb Abyss Source Code
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

// C3_PLAY.C

#include "c4_def.h"
#include "gelib.h"
#pragma hdrstop


/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

#define POINTTICS	6
#define PAUSE 300

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

boolean autofire;
boolean enterSaveMenu=false, enterLoadMenu=false;
byte bcolor;
short skytimer=-1,skytimer_reset;
short groundtimer=-1,groundtimer_reset;
static unsigned black=0;
unsigned *skycolor = &black,*groundcolor = &black;
unsigned nocolorchange=0xFFFF;
byte BGFLAGS,				// global that holds all current flags
	  bgflag;				// used by BG changer, this flag is set when done


unsigned sky_daytonight[]={0x0909,0x0101,0x0808,0x0000,0xFFFF};
//unsigned gnd_daytonight[]={0x0202,0xFFFF};

unsigned sky_lightning[]={0x0101,0x0909,0x0f0f,0x0808,0x0000,0xFFFF};

unsigned sky_colors[NUMLEVELS]={0x0000,0x0000,0x0000,0x0000,0x0808,
										  0x0404,0x0000,0x0000,0x0000,0x0000,
										  0x0000,0x0000,0x0000,0x0000,0x0606,
										  0x0000,0x0000,0x0000,0x0000,0x0000,
										  0x0000};
unsigned gnd_colors[NUMLEVELS]={0x0202,0x0202,0x0606,0x0202,0x0707,
										  0x0505,0x0808,0x0606,0x0101,0x0808,
										  0x0606,0x0404,0x0808,0x0c0c,0x0e0e,
										  0x0808,0x0808,0x0c0c,0x0000,0x0707,
										  0x0808};



int			bordertime;

boolean		singlestep,godmode;
int			extravbls;
status_flags    status_flag;
int             status_delay;
char			status_text[500] = {0};

//
// replacing refresh manager
//
unsigned	tics,realtics;

short BeepTime = 0;

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

textinfo MainHelpText;

void CalcBounds (objtype *ob);
void DrawPlayScreen (void);
void PreFullDisplay(void);
void PostFullDisplay(boolean draw_view);


//
// near data map array (wall values only, get text number from data)
//
byte		tilemap[MAPSIZE][MAPSIZE];
byte		spotvis[MAPSIZE][MAPSIZE];
objtype		*actorat[MAPSIZE][MAPSIZE];

int bordertime;
int	objectcount;

void StopMusic(void);
void StartMusic(void);

//===========================================================================


/*
=====================
=
= CheckKeys
=
=====================
*/

void CheckKeys (void)
{
	if (screenfaded)			// don't do anything with a faded screen
		return;

	if (SPI_GetKeyDown(sc_M)&&SPI_GetKeyDown(sc_I)&&SPI_GetKeyDown(sc_K)&&SPI_GetKeyDown(sc_E))
	{
		Win_Create(&renderBufferText, 12,2);
		if (autofire)
		  Win_PrintCentered ("Auto-Bolt OFF");
		else
		  Win_PrintCentered ("Auto-Bolt ON");
		SPG_FlipBuffer();
		SPI_WaitForever();
		autofire ^= 1;
		return;
	}

// F2 - SOUND OPTIONS
//
	if (SPI_GetKeyDown(sc_F2))
	{
		int height=7;
		boolean ChoiceMade = false;

		height++;

		Win_Create(&renderBufferText, 22,height);
		Win_Print( "\n        1 )  NO SOUND \n");
		Win_Print(   "        2 )  PC  AUDIO \n");
		Win_Print("        3 ) ADLIB AUDIO\n");
		Win_Print( "\n       ESC)    EXIT    ");
		SPG_FlipBuffer();

		// Switch audio device ON/OFF & load sounds if there
		// was a change in the device.

		do {

			if (SPI_GetKeyDown(1)) 								// ESC - Exit
				ChoiceMade = true;
			else
			if (SPI_GetKeyDown(2)) 							 	// 1 - No Sound
			{
				SPA_SetSoundSource(SND_OFF);
				ChoiceMade = true;
			}
			else
			if ((SPI_GetKeyDown(4)))		// 3 - AdLib Audio
			{
				SPA_SetSoundSource(SND_ADLIB);
				ChoiceMade = true;
			}

		} while (!ChoiceMade);
		tics = realtics = 1;
		SPI_ClearKeysDown();
	}

deadloop:;
// ESCAPE - quits game
//
	if ((SPI_GetKeyDown(sc_Escape)) || (Flags & FL_DEAD))
	{
		char ch;

		DisplaySMsg("Options", NULL);
		if ((status_flag != S_TIMESTOP) || (Flags & FL_DEAD))
			status_flag = S_NONE;


		if (Flags & FL_DEAD)
		{
			char choices[] = {sc_Escape,sc_R,sc_N,sc_Q,0};
			ch = DisplayMsg("Restore          New          Quit",choices);
		}
		else
		{
			char choices[] = {sc_Escape,sc_S,sc_R,sc_N,sc_Q,0};
			ch = DisplayMsg("Save       Restore       New       Quit",choices);
		}
		DrawText(true);

		switch (ch)
		{
			case sc_S:
				if (!(Flags & FL_DEAD))
					enterSaveMenu = true;
			break;

			case sc_R:
				enterLoadMenu = true;
			break;

			case sc_N:
				DisplaySMsg("Starting anew", NULL);
//				VW_WaitVBL(60);
				playstate = ex_resetgame;
				Flags &= ~FL_DEAD;
				status_flag = S_NONE;
			break;

			case sc_Q:
				DisplaySMsg("FARE THEE WELL!", NULL);
				status_flag = S_NONE;
//				VW_WaitVBL(120);
				VW_FadeOut();
				NormalScreen();
				Quit(NULL);
			break;
		}
		tics = realtics = 1;

		if (status_flag == S_TIMESTOP)
			DisplaySMsg("Time Stopped:     ",NULL);
	}

// F1 - DISPLAY HELP
//
	if (SPI_GetKeyDown(sc_F1))
	{
		boolean nohelp=false;
		extern textinfo MainHelpText;

		VW_FadeOut();

		if (!FindFile("HELP.TXT",NULL,1))
			nohelp = true;

		if (LoadTextFile("HELP.TXT",&MainHelpText))
		{
			VW_Bar(0,0,320,200,0);

			DisplayText(&MainHelpText);
		}
		else
			nohelp = true;

		if (nohelp)
		{
			VW_FadeIn();
			Win_Create(&renderBufferText, 30,5);
			Win_CPrint("\nError loading HELP file.\n");
			Win_CPrint("Press any key.");
			SPI_WaitForever();
			VW_FadeOut();
			nohelp = false;
		}
		FreeTextFile(&MainHelpText);
		CacheScaleds();

		RedrawStatusWindow();
		ThreeDRefresh(*skycolor, *groundcolor);
		VW_FadeIn();
		tics = realtics = 1;
		SPI_ClearKeysDown();
	}

// F3 - SAVE GAME
//
	if ((SPI_GetKeyDown(sc_F3) || enterSaveMenu) && (!(Flags & FL_DEAD)))
	{
		enterSaveMenu = false;
		PreFullDisplay();
		GE_SaveGame();
		PostFullDisplay(true);
		tics = realtics = 1;
		SPI_ClearKeysDown();
	}

// F4 - LOAD GAME
//
	if (SPI_GetKeyDown(sc_F4) || enterLoadMenu)
	{
		enterLoadMenu = false;
		PreFullDisplay();
		if (GE_LoadGame())
		{
			loadedgame = true;
			playstate = ex_loadedgame;
			Flags &= ~FL_DEAD;
			PostFullDisplay(false);
		}
		else
		if (playstate == ex_victorious)
		{
			PostFullDisplay(false);
			Victory(false);
		}
		else
			PostFullDisplay(true);
		tics = realtics = 1;
		SPI_ClearKeysDown();
	}

	if (Flags & FL_DEAD)
		goto deadloop;

//
// F10-? debug keys
//
	if (SPI_GetKeyDown(sc_F10))
	{
		SPI_GetMouseDelta(NULL, NULL);	// Clear accumulated mouse movement
		lasttimecount = SP_TimeCount();
	}
}

//-------------------------------------------------------------------------
// PreFullDisplay()
//-------------------------------------------------------------------------
void PreFullDisplay()
{
	VW_FadeOut();
	VW_Bar(0,0,320,200,0);
}

//-------------------------------------------------------------------------
// PostFullDisplay()
//-------------------------------------------------------------------------
void PostFullDisplay(boolean draw_view)
{
	RedrawStatusWindow();
	if (draw_view)
	{
		ThreeDRefresh(*skycolor, *groundcolor);
		VW_FadeIn();
	}
}



//==========================================================================

/*
===================
=
= DrawPlayScreen
=
===================
*/

void DrawPlayScreen (void)
{
	SPG_ClearBuffer (-1);
	SPD_LoadGrChunk (STATUSPIC);
	SPG_DrawPic(&bottomHUDBuffer, grsegs[STATUSPIC], 0,0);
	RedrawStatusWindow ();
}

//==========================================================================


/*
===================
=
= PlayLoop
=
===================
*/

void PlayLoop (void)
{
	char shot_color[3] = {4,9,14};

	int allgems[5]={GEM_DELAY_TIME,		// used for Q & D comparison
						 GEM_DELAY_TIME,		// for having all gems...
						 GEM_DELAY_TIME,		// the "allgems" declaration MUST
						 GEM_DELAY_TIME,		// match the "gems" declaration in
						 GEM_DELAY_TIME		// the gametype structure!
						};

//	int originx=0;
//	int i=100;
	signed long dx,dy,radius,psin,pcos,newx,newy;
	short jim;
	int		give;
	short objnum;
	signed long ox,oy,xl,xh,yl,yh,px,py,norm_dx,norm_dy;
	short o_radius;

	void (*think)();

	ingame = true;
	playstate = 0;
	gamestate->shotpower = handheight = 0;

	// setup sky/ground colors and effects (based on level)
	//
	switch (gamestate->mapon)
	{
		case 0:
			if (!(BGFLAGS & BGF_NIGHT))
			{
				InitBgChange(3*60,sky_daytonight,-1,NULL,BGF_NIGHT);
				groundcolor = &gnd_colors[0];
			}
			else
			{
				skycolor = &sky_colors[0];
				groundcolor = &gnd_colors[0];
			}
		break;

		default:
			skycolor = &sky_colors[gamestate->mapon];
			groundcolor = &gnd_colors[gamestate->mapon];
			skytimer = groundtimer = -1;
		break;
	}

	RedrawStatusWindow();
	ThreeDRefresh(*skycolor, *groundcolor);
	if (screenfaded)
		VW_FadeIn();

	fizzlein = true;				// fizzle fade in the first refresh
	lasttimecount = SP_TimeCount();
	lastnuke = 0;

	do
	{
		SPI_GetPlayerControl(&control);

		objnum=0;
		objtype *obj;
		for (obj = player;obj;obj = obj->next)
		{
			if ((obj->active >= yes) && (!(FreezeTime && (obj!=player))))
			{


				if (obj->ticcount)
				{
					obj->ticcount-=tics;

					while ( obj->ticcount <= 0)
					{
						think = obj->state->think;
						if (think)
						{
							statetype *oldstate=obj->state;

							think (obj);
							if (!obj->state)
							{
								RemoveObj (obj);
								goto nextactor;
							}

							if (obj->state != oldstate)
								break;
						}

						obj->state = obj->state->next;
						if (!obj->state)
						{
							RemoveObj (obj);
							goto nextactor;
						}

						if (!obj->state->tictime)
						{
							obj->ticcount = 0;
							goto nextactor;
						}

						if (obj->state->tictime>0)
							obj->ticcount += obj->state->tictime;
					}
				}


				think =	obj->state->think;

				if (think)
				{
					think (obj);
					if (!obj->state)
						RemoveObj (obj);
				}
nextactor:;
			}

			// keep a list of objects around the player for radar updates
			//
				if (obj == player)
				{
					px = player->x;
					py = player->y;
					psin = sintable[player->angle];
					pcos = costable[player->angle];
					xl = px-((long)RADAR_WIDTH<<TILESHIFT)/2;
					xh = px+((long)RADAR_WIDTH<<TILESHIFT)/2-1;
					yl = py-((long)RADAR_HEIGHT<<TILESHIFT)/2;
					yh = py+((long)RADAR_HEIGHT<<TILESHIFT)/2;
				}

				if (objnum > MAX_RADAR_BLIPS-2)
					objnum = MAX_RADAR_BLIPS-2;

				ox = obj->x;
				oy = obj->y;


				if ((ox >= xl) && (ox <= xh) && (oy >= yl) && (oy <= yh))
				{
					norm_dx = (dx = px-ox)>>TILESHIFT;
					norm_dy = (dy = oy-py)>>TILESHIFT;

					o_radius = IntSqrt((norm_dx * norm_dx) + (norm_dy * norm_dy));

					if (o_radius < RADAR_RADIUS)
					{
						newx = FixedByFrac(dy,pcos)-FixedByFrac(dx,psin);
						newy = FixedByFrac(dy,psin)+FixedByFrac(dx,pcos);

						RadarXY[objnum][0]=newx>>TILESHIFT;
						RadarXY[objnum][1]=newy>>TILESHIFT;

						// Define color to use for this object...
						//

						switch (obj->obclass)
						{
			// NO GEM NEEDED
			//
					// THE WIZARD! (YOU)
					//
							case playerobj:
								RadarXY[objnum++][2]=15;
							break;

					// WIZARD'S SHOTS
					//
							case pshotobj:
							case bigpshotobj:
								RadarXY[objnum++][2]=shot_color[(SP_TimeCount()*70/1000)%3];
							break;

					// BATS	    						(DK GRAY)
					//
							case batobj:
								if (obj->active == always)
									RadarXY[objnum++][2]=8;
							break;

			// RED GEM
			//
					// EYE, RED DEMON        		(DK RED)
					//
							case eyeobj:
							case reddemonobj:
								if (gamestate->gems[B_RGEM-B_RGEM])
									if (obj->active == always)
										RadarXY[objnum++][2]=4;
							break;

					// RED MAGE							(LT RED)
					//
							case mageobj:
								if (gamestate->gems[B_RGEM-B_RGEM])
									if (obj->active == always)
										RadarXY[objnum++][2]=12;
							break;

			// BLUE GEM
			//
					// WATER TROLL						(LT BLUE)
					//
							case wetobj:
								if (gamestate->gems[B_BGEM-B_RGEM])
									if (obj->active == always)
										RadarXY[objnum++][2]=9;
							break;

					// WATER TROLL						(DK BLUE)
					//
							case demonobj:
								if (gamestate->gems[B_BGEM-B_RGEM])
									if (obj->active == always)
										RadarXY[objnum++][2]=1;
							break;

			// GREEN GEM
			//
					// GREEN TROLL						(LT GREEN)
					//
							case trollobj:
								if (gamestate->gems[B_GGEM-B_RGEM])
									if (obj->active == always)
										RadarXY[objnum++][2]=10;
							break;

					// ORC								(DK GREEN)
					//
							case orcobj:
								if (gamestate->gems[B_GGEM-B_RGEM])
									if (obj->active == always)
										RadarXY[objnum++][2]=2;
							break;

			// YELLOW GEM
			//
					// SPOOK								(BROWN)
					//
							case spookobj:
								if (gamestate->gems[B_YGEM-B_RGEM])
									if (obj->active == always)
										RadarXY[objnum++][2]=6;
							break;

					// SKELETON							(YELLOW)
					//
							case skeletonobj:
								if (gamestate->gems[B_YGEM-B_RGEM])
									if (obj->active == always)
										RadarXY[objnum++][2]=14;
							break;

			// PURPLE GEM
			//
					// ZOMBIE
					//
							case zombieobj:
								if (gamestate->gems[B_PGEM-B_RGEM])
									if (obj->active == always)
										RadarXY[objnum++][2]=13;
							break;

			// ALL GEMS NEEDED
			//
					// NEMESIS
					//
							case grelmobj:
								if (!memcmp(gamestate->gems,allgems,sizeof(gamestate->gems)))
									if (obj->active == always)
										RadarXY[objnum++][2]=15;
							break;
						}
					}
				}
		}
		RadarXY[objnum][2]=-1;		// Signals end of RadarXY list...

		if (bordertime)
		{
			bordertime -= realtics;
			if (bordertime<=0)
			{
				bordertime = 0;
				SPG_SetBorderColor(0);
			}
		}

// RANDOM lightning?
//
	if (BGFLAGS & (BGF_NIGHT|BGF_NOT_LIGHTNING))
		switch (gamestate->mapon)
		{
			case 0:
			case 1:
			case 3:
				if (!RANDOM(120-realtics))
				{
					BGFLAGS &= ~BGF_NOT_LIGHTNING;
					InitBgChange(1,sky_lightning,-1,NULL,BGF_NOT_LIGHTNING);
				}
			break;
		}

// handle sky/ground color changes
//
		if (skytimer != -1)
		{
			skytimer -= realtics;
			if (skytimer < 0)
			{
				skycolor++;
				if (*skycolor == 0xffff)
				{
					skytimer = -1;
					skycolor--;
					if (groundtimer == -1)
						BGFLAGS |= bgflag;
				}
				else
					skytimer = skytimer_reset;
			}
		}

		if (groundtimer != -1)
		{
			groundtimer -= realtics;
			if (groundtimer < 0)
			{
				groundcolor++;
				if (*groundcolor == 0xffff)
				{
					groundtimer = -1;
					groundcolor--;
					if (skytimer == -1)
						BGFLAGS |= bgflag;
				}
				else
					groundtimer = groundtimer_reset;
			}
		}


//
//		Handle FreezeTime counter..
//
		if (FreezeTime)
		{
			if (FreezeTime<20*30)
				if ((BeepTime+=realtics)>=60)
				{
					BeepTime -= 60;
					SPA_PlaySound(TICKSND);
				}

			if ((FreezeTime-=realtics)<=0)
			{
				FreezeTime=0;
				SPA_PlaySound(TIMERETURNSND);
				DisplaySMsg(NULL,NULL);
				status_flag = S_NONE;
			}
		}


// refresh all
//
		SPG_ResizeNow();
		DrawPlayScreen ();

		if (Flags & FL_DEAD)
		{
			SPA_PlaySound (GAMEOVERSND);
			DisplaySMsg("DEAD",NULL);
			DrawHealth();
			if (gamestate->potions)
			{
				 Win_Create(&renderBufferText, 35,3);
				 Win_CPrint("\nYou should use your Cure Potions wisely\n");
				 SPI_WaitForever();
			}
		}

// check for win
//
		if (playstate == ex_victorious)
		{
			Victory(true);
			Flags |= FL_DEAD;
		}

		DisplayStatus(&status_flag);

		ThreeDRefresh (*skycolor, *groundcolor);
		CheckKeys();

	}while (!playstate);
//	StopMusic ();

	ingame = false;
	if (bordertime)
	{
		bordertime = 0;
		SPG_SetBorderColor(0);
	}

	if (abortgame)
		abortgame = false;
}

//--------------------------------------------------------------------------
// IntSqrt() - by Master Programmer, George Leritte!
//--------------------------------------------------------------------------
int IntSqrt(long va)
{
#warning check square root code to coincide with original
	return (int)sqrt((float)va);
/*
asm     mov     AX, word ptr va
asm     mov     DX, word ptr va+2
asm     mov     bx,dx           // {bx = integer square root of dx:ax}
asm     or      bx,ax           // {if dx:ax=0 then return}
asm     jz      isq01
asm     mov     bx,dx
asm     shl     bx,1
asm     or      bl,ah
asm     or      bl,al
asm     dec     bx
asm     add     bx,dx           // { initial guess}
asm     jg      isq10
asm     inc     bx              // { don't return zero}
asm     jg      isq10
asm     mov     bx,7fffh
isq01:;
		  goto    exitrout;

isq10:;
asm     push    ax
asm     push    dx
asm     div     bx
asm     sub     ax,bx
asm     cmp     ax,1
asm     jbe     isq90
asm     cmp     ax,-1
asm     jae     isq90
asm     sar     ax,1
asm     add     bx,ax
asm     pop     dx
asm     pop     ax
asm     jmp     isq10
isq90:;
asm     pop     dx
asm     pop     ax
exitrout:;
asm     mov     ax,bx */
}

//-------------------------------------------------------------------------
// InitBgChange()
//-------------------------------------------------------------------------
void InitBgChange(short stimer, unsigned *scolors, short gtimer, unsigned *gcolors, byte flag)
{
	skytimer_reset = skytimer = stimer;
	if (scolors)
		skycolor = scolors;

	groundtimer_reset = groundtimer = gtimer;
	if (gcolors)
		groundcolor = gcolors;

	bgflag = flag;
}

////////////////////////////////////////////////////////
//
// DisplayStatus
//
//  Stat_Flag -  contains the type of status displayed
//  -- also uses status_delay (global variable) will not
//     change display until this variable is zero.
//  -- heirarchy is determined by the series of if statements,
//        to change it, rearrange th if statements.
//
////////////////////////////////////////////////////////

#define MESSAGEDELAY  25
void DisplayStatus (status_flags *stat_flag)
{
	status_flags temp_status;


	if (*stat_flag == S_TIMESTOP)
	  return;

	if (status_delay > 0)
	{
		DisplaySMsg(status_text, NULL);
		status_delay -= realtics;
		return;
	}
	else
		status_delay = 0;

	// check for a change in status from previous call

	temp_status = S_VIEWING;                             //precaution

	if (SPI_GetKeyDown(sc_Control) || control.fire)
		temp_status = S_MISSLE;

	if (SPI_GetKeyDown(sc_Z) && !SPI_GetKeyDown(sc_F10))
		temp_status = S_ZAPPER;

	if ((SPI_GetKeyDown(sc_X) && !SPI_GetKeyDown(sc_F10)) || SPI_GetKeyDown(sc_Enter))
		temp_status = S_XTER;

	if (control.x)
		temp_status = S_TURN;

	if ((SPI_GetKeyDown(sc_V) || SPI_GetKeyDown(sc_Tab)) && control.x)
		temp_status = S_QTURN;

	if (SPI_GetKeyDown(sc_Alt) && control.x)
		temp_status = S_SIDESTEP;

	if (control.y < 0)
		temp_status = S_ADVANCE;

	if (control.y > 0)
		temp_status = S_RETREAT;

	if (SPI_GetKeyDown(sc_F5))
		temp_status = S_JOYSTICK;

	if (SPI_GetKeyDown(sc_F4))
		temp_status = S_RESTORING;

	if (SPI_GetKeyDown(sc_F3))
		temp_status = S_SAVING;

	if (SPI_GetKeyDown(sc_F2))
		temp_status = S_SND;

	if (SPI_GetKeyDown(sc_F1))
		temp_status = S_HELP;

	if (temp_status == *stat_flag) {
		DisplaySMsg(status_text, NULL);
	} else {
		*stat_flag = temp_status;


		switch (*stat_flag)
		{
			case S_MISSLE:
				DisplaySMsg("Magick Missile", NULL);
				status_delay = MESSAGEDELAY;
			break;

			case S_ZAPPER:
				if (gamestate->bolts)
				{
					DisplaySMsg("Zapper", NULL);
					status_delay = MESSAGEDELAY+10;
				}
			break;

			case S_XTER:
				if (gamestate->nukes)
				{
					DisplaySMsg("Xterminator", NULL);
					status_delay = MESSAGEDELAY+5;
				}
			break;

			case S_TURN:
				DisplaySMsg("Turning", NULL);
				status_delay = MESSAGEDELAY;
			break;

			case S_QTURN:
				DisplaySMsg("Quick Turning", NULL);
				status_delay = MESSAGEDELAY;
			break;

			case S_SIDESTEP:
				DisplaySMsg("Sidestepping", NULL);
				status_delay = MESSAGEDELAY;
			break;

			case S_ADVANCE:
				DisplaySMsg("Advancing", NULL);
				status_delay = MESSAGEDELAY;
			break;

			case S_RETREAT:
				DisplaySMsg("Retreating", NULL);
				status_delay = MESSAGEDELAY;
			break;

			case S_JOYSTICK:
				DisplaySMsg("Adjusting Joystick", NULL);
			break;

			case S_RESTORING:
				DisplaySMsg("Restoring", NULL);
			break;

			case S_SAVING:
				DisplaySMsg("Saving", NULL);
			break;

			case S_SND:
				DisplaySMsg("Select Sound", NULL);
			break;

			case S_HELP:
				DisplaySMsg("Getting Help", NULL);
			break;

			case S_VIEWING:
				DisplaySMsg("Viewing", NULL);
			break;
		}

	}
}
