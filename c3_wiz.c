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

// C3_WIZ.C

#include "c3_def.h"

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

#define NUMSCROLLS	8

#define	SHOWITEMS	9

#define	NUKETIME	40
#define NUMBOLTS	10
#define BOLTTICS	6

#define STATUSCOLOR	4
#define TEXTCOLOR	14

#define SIDEBARWIDTH	5

#define BODYLINE    8
#define POWERLINE	80

#define SPECTILESTART	18


#define SHOTDAMAGE		1
#define BIGSHOTDAMAGE	3


#define PLAYERSPEED	5120
#define RUNSPEED	8192

#define SHOTSPEED	10000

#define LASTWALLTILE	17
#define LASTSPECIALTILE	37

#define FIRETIME	4	// DEBUG 60

#define HANDPAUSE	60

#define COMPASSX	33
#define COMPASSY	0

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

long		lastnuke,lasthand;
int			handheight;
int			boltsleft;

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

int			lasttext,lastcompass;
int			bolttimer;
unsigned	lastfiretime;

int	strafeangle[9] = {0,90,180,270,45,135,225,315,0};


//===========================================================================

void DrawChar (unsigned x, unsigned y, unsigned tile);
void RedrawStatusWindow (void);
void GiveBolt (void);
void TakeBolt (void);
void GiveNuke (void);
void TakeNuke (void);
void GivePotion (void);
void TakePotion (void);
void GiveKey (int keytype);
void TakeKey (int keytype);
void GiveScroll (int scrolltype,boolean show);
void ReadScroll (int scroll);
void GivePoints (int points);
void DrawLevelNumber (int number);
void DrawText (void);
void DrawBars (void);

//----------

void Shoot (void);
void BigShoot (void);
void CastBolt (void);
void CastNuke (void);
void DrinkPotion (void);

//----------

void SpawnPlayer (int tilex, int tiley, int dir);
void Thrust (int angle, unsigned speed);
void T_Player (objtype *ob);

void AddPoints (int points);

void ClipMove (objtype *ob, long xmove, long ymove);
boolean ShotClipMove (objtype *ob, long xmove, long ymove);

//===========================================================================


/*
===============
=
= DrawChar
=
===============
*/

void DrawChar (unsigned x, unsigned y, unsigned tile)
{
	SPG_DrawTile8(&hudSetup, 8*x,y,(byte*)grsegs[STARTTILE8]+64*tile);
}


//===========================================================================

/*
===============
=
= RedrawStatusWindow
=
===============
*/

void RedrawStatusWindow (void)
{
	int	i,j,x;

	DrawLevelNumber (gamestate.mapon);
	lasttext = -1;
	lastcompass = -1;

	j = gamestate.bolts < SHOWITEMS ? gamestate.bolts : SHOWITEMS;
	for (i=0;i<j;i++)
		DrawChar(7+i,144+20,BOLTCHAR);
	j = gamestate.nukes < SHOWITEMS ? gamestate.nukes : SHOWITEMS;
	for (i=0;i<j;i++)
		DrawChar(7+i,144+30,NUKECHAR);
	j = gamestate.potions < SHOWITEMS ? gamestate.potions : SHOWITEMS;
	for (i=0;i<j;i++)
		DrawChar(7+i,144+40,POTIONCHAR);

	x=24;
	for (i=0;i<4;i++)
		for (j=0;j<gamestate.keys[i];j++)
			DrawChar(x++,144+20,KEYCHARS+i);

	x=24;
	for (i=0;i<8;i++)
		if (gamestate.scrolls[i])
			DrawChar(x++,144+30,SCROLLCHARS+i);

	AddPoints(0);

	DrawBars ();
}


//===========================================================================

/*
===============
=
= GiveBolt
=
===============
*/

void GiveBolt (void)
{
	SD_PlaySound (GETBOLTSND);
	if (++gamestate.bolts<=9)
		DrawChar(6+gamestate.bolts,144+20,BOLTCHAR);
}


/*
===============
=
= TakeBolt
=
===============
*/

void TakeBolt (void)
{
	SD_PlaySound (USEBOLTSND);
	if (--gamestate.bolts<=9)
		DrawChar(7+gamestate.bolts,144+20,BLANKCHAR);
}

//===========================================================================

/*
===============
=
= GiveNuke
=
===============
*/

void GiveNuke (void)
{
	SD_PlaySound (GETNUKESND);
	if (++gamestate.nukes<=9)
		DrawChar(6+gamestate.nukes,144+30,NUKECHAR);
}


/*
===============
=
= TakeNuke
=
===============
*/

void TakeNuke (void)
{
	SD_PlaySound (USENUKESND);
	if (--gamestate.nukes<=9)
		DrawChar(7+gamestate.nukes,144+30,BLANKCHAR);
}

//===========================================================================

/*
===============
=
= GivePotion
=
===============
*/

void GivePotion (void)
{
	SD_PlaySound (GETPOTIONSND);
	if (++gamestate.potions<=9)
		DrawChar(6+gamestate.potions,144+40,POTIONCHAR);
}


/*
===============
=
= TakePotion
=
===============
*/

void TakePotion (void)
{
	SD_PlaySound (USEPOTIONSND);
	if (--gamestate.potions<=9)
		DrawChar(7+gamestate.potions,144+40,BLANKCHAR);
}

//===========================================================================

/*
===============
=
= GiveKey
=
===============
*/

void GiveKey (int keytype)
{
	int	i,j,x;

	SD_PlaySound (GETKEYSND);
	gamestate.keys[keytype]++;

	x=24;
	for (i=0;i<4;i++)
		for (j=0;j<gamestate.keys[i];j++)
			DrawChar(x++,144+20,KEYCHARS+i);

}


/*
===============
=
= TakeKey
=
===============
*/

void TakeKey (int keytype)
{
	int	i,j,x;

	SD_PlaySound (USEKEYSND);
	gamestate.keys[keytype]--;

	x=24;
	for (i=0;i<4;i++)
		for (j=0;j<gamestate.keys[i];j++)
			DrawChar(x++,144+20,KEYCHARS+i);

	DrawChar(x,144+20,BLANKCHAR);
}

//===========================================================================

/*
===============
=
= GiveScroll
=
===============
*/

void GiveScroll (int scrolltype,boolean show)
{
	int	i,x;

	SD_PlaySound (GETSCROLLSND);
	gamestate.scrolls[scrolltype] = true;

	x=24;
	for (i=0;i<8;i++)
		if (gamestate.scrolls[i])
			DrawChar(x++,144+30,SCROLLCHARS+i);
	if (show)
		ReadScroll(scrolltype);
}

//===========================================================================

/*
===============
=
= GivePoints
=
===============
*/

void GivePoints (int points)
{
	pointcount = 1;
	pointsleft += points;
}


//===========================================================================

/*
===============
=
= AddPoints
=
===============
*/

void AddPoints (int points)
{
	char	str[10];
	int		len,x,i;

	gamestate.score += points;

	sprintf(str, "%i", gamestate.score);
	len = strlen (str);

	x=24+(8-len);
	for (i=0;i<len;i++)
		DrawChar(x++,144+40,NUMBERCHARS+str[i]-'0');
}


//===========================================================================

/*
===============
=
= GiveChest
=
===============
*/

void GiveChest (void)
{
	SD_PlaySound (GETPOINTSSND);
	GivePoints ((gamestate.mapon+1)*100);
}


//===========================================================================

/*
===============
=
= GiveGoal
=
===============
*/

void GiveGoal (void)
{
	SD_PlaySound (GETPOINTSSND);
	GivePoints (100000);
	playstate = ex_victorious;
}


//===========================================================================

/*
===============
=
= DrawLevelNumber
=
===============
*/

void DrawLevelNumber (int number)
{
	char buf[100];

	SPG_Bar(&hudSetup, 5, 148, 16, 9, STATUSCOLOR);
	sprintf(buf, "%i", number+1);
	SPG_DrawString(&guiSetup, (number<9)?13:5, 148, buf, 0, TEXTCOLOR);
}


//===========================================================================

/*
===============
=
= DrawText
=
===============
*/

void DrawText (void)
{
	unsigned	number;
	char		str[80];
	char 		*text;
	unsigned	temp;

	//
	// draw a new text description if needed
	//
	number = *(byte*)(gamestate.mapsegs[0]+player->tiley*mapwidth+player->tilex)-NAMESTART;

	if ( number>26 )
		number = 0;

	if (number == lasttext)
		return;

	lasttext = number;
	text = mapheaderseg[loadedmap].texts[number];

	if (text == NULL) {
		memset(str, 0, 80);
	} else {
		memcpy (str,text,80);
	}

	SPG_Bar(&hudSetup, 26, 148, 232, 9, STATUSCOLOR);
	int width;
	SPG_MeasureString(str, 0, &width, NULL);
	SPG_DrawString(&guiSetup, 26+(232-width)/2, 148, str, 0, TEXTCOLOR);
}

//===========================================================================

/*
===============
=
= DrawCompass
=
===============
*/

void DrawCompass (void)
{
	int		angle,number;

	//
	// draw the compass if needed
	//
	angle = player->angle-ANGLES/4;
	angle -= ANGLES/32;
	if (angle<0)
		angle+=ANGLES;
	number = angle/(ANGLES/16);
	if (number>15)					// because 360 angles doesn't divide by 16
		number = 15;

	if (number == lastcompass)
		return;

	lastcompass = number;

	SPG_DrawPic(&hudSetup, grsegs[COMPAS1PIC+15-number],8*COMPASSX,144+COMPASSY);
}

//===========================================================================


/*
===============
=
= DrawBars
=
===============
*/

void DrawBars (void)
{
	SPG_Bar(&hudSetup, 34*8, POWERLINE, 40, MAXSHOTPOWER, 1);

//
// shot power
//
	if (gamestate.shotpower)
	{
		SPG_DrawPicSkip(&hudSetup, grsegs[SHOTPOWERPIC], 34*8, POWERLINE, MAXSHOTPOWER-gamestate.shotpower, MAXSHOTPOWER);
	}

//
// body
//
	if (gamestate.body)
	{
		SPG_DrawPicSkip(&hudSetup, grsegs[BODYPIC], 34*8, BODYLINE, 0, gamestate.body);
	}


	if (gamestate.body != MAXBODY)
	{
		SPG_DrawPicSkip(&hudSetup, grsegs[NOBODYPIC], 34*8, BODYLINE, gamestate.body, MAXBODY);
	}
}



/*
=============================================================================

							SHOTS

=============================================================================
*/

void T_Pshot (objtype *ob);


extern	statetype s_pshot1;
extern	statetype s_pshot2;

extern	statetype s_bigpshot1;
extern	statetype s_bigpshot2;


statetype s_pshot1 = {PSHOT1PIC,8,&T_Pshot,&s_pshot2};
statetype s_pshot2 = {PSHOT2PIC,8,&T_Pshot,&s_pshot1};

statetype s_shotexplode = {PSHOT2PIC,8,NULL,NULL};

statetype s_bigpshot1 = {BIGPSHOT1PIC,8,&T_Pshot,&s_bigpshot2};
statetype s_bigpshot2 = {BIGPSHOT2PIC,8,&T_Pshot,&s_bigpshot1};


/*
===================
=
= SpawnPShot
=
===================
*/

void SpawnPShot (void)
{
	SpawnNewObjFrac (player->x,player->y,&s_pshot1,PIXRADIUS*14);
	new->obclass = pshotobj;
	new->speed = SHOTSPEED;
	new->angle = player->angle;
}

void SpawnBigPShot (void)
{
	SpawnNewObjFrac (player->x,player->y,&s_bigpshot1,24*PIXRADIUS);
	new->obclass = bigpshotobj;
	new->speed = SHOTSPEED;
	new->angle = player->angle;
}


/*
===============
=
= T_Pshot
=
===============
*/

void T_Pshot (objtype *ob)
{
	objtype	*check;
	long	xmove,ymove,speed;

//
// check current position for monsters having moved into it
//
	for (check = player->next; check; check=check->next)
		if (check->shootable
		&& ob->xl <= check->xh
		&& ob->xh >= check->xl
		&& ob->yl <= check->yh
		&& ob->yh >= check->yl)
		{
			SD_PlaySound (SHOOTMONSTERSND);
			if (ob->obclass == bigpshotobj)
				ShootActor (check,BIGSHOTDAMAGE);
			else
				ShootActor (check,SHOTDAMAGE);
			ob->state = &s_shotexplode;
			ob->ticcount = ob->state->tictime;
			return;
		}


//
// move ahead, possibly hitting a wall
//
	speed = ob->speed*tics;

	xmove = FixedByFrac(speed,costable[ob->angle]);
	ymove = -FixedByFrac(speed,sintable[ob->angle]);

	if (ShotClipMove(ob,xmove,ymove))
	{
		ob->state = &s_shotexplode;
		ob->ticcount = ob->state->tictime;
		return;
	}

	ob->tilex = ob->x >> TILESHIFT;
	ob->tiley = ob->y >> TILESHIFT;

//
// check final position for monsters hit
//
	for (check = player->next; check; check=check->next)
		if (ob->shootable
		&& ob->xl <= check->xh
		&& ob->xh >= check->xl
		&& ob->yl <= check->yh
		&& ob->yh >= check->yl)
		{
			ShootActor (check,SHOTDAMAGE);
			ob->state = &s_shotexplode;
			ob->ticcount = ob->state->tictime;
			return;
		}

}



/*
=============================================================================

						   PLAYER ACTIONS

=============================================================================
*/


/*
===============
=
= BuildShotPower
=
===============
*/

void BuildShotPower (void)
{
	int		newlines,topline;
	long	i;
	byte *source;

	if (gamestate.shotpower == MAXSHOTPOWER)
		return;

	newlines = 0;
	for (i=lasttimecount-tics;i<lasttimecount;i++)
		newlines += (i&1);

	gamestate.shotpower += newlines;

	if (gamestate.shotpower > MAXSHOTPOWER)
	{
		newlines -= (gamestate.shotpower - MAXSHOTPOWER);
		gamestate.shotpower = MAXSHOTPOWER;
	}
	topline = MAXSHOTPOWER - gamestate.shotpower;

	DrawBars();
}


//===========================================================================

/*
===============
=
= ClearShotPower
=
===============
*/

void ClearShotPower (void)
{
	if (!gamestate.shotpower)
		return;

	SPG_DrawPic(&hudSetup, grsegs[NOSHOTPOWERPIC], 8*34, POWERLINE);

	gamestate.shotpower = 0;
}

//===========================================================================

/*
===============
=
= Shoot
=
===============
*/

void Shoot (void)
{
	ClearShotPower ();
	SD_PlaySound (SHOOTSND);
	SpawnPShot ();
}

//===========================================================================

/*
===============
=
= BigShoot
=
===============
*/

void BigShoot (void)
{
	ClearShotPower ();
	SD_PlaySound (BIGSHOOTSND);
	SpawnBigPShot ();
}

//===========================================================================

/*
===============
=
= CastBolt
=
===============
*/

void CastBolt (void)
{
	if (!gamestate.bolts)
	{
		SD_PlaySound (NOITEMSND);
		return;
	}

	TakeBolt ();
	boltsleft = NUMBOLTS;
	bolttimer = BOLTTICS;
	BigShoot ();
}


/*
===============
=
= ContinueBolt
=
===============
*/

void ContinueBolt (void)
{
	bolttimer-=tics;
	if (bolttimer<0)
	{
		boltsleft--;
		bolttimer = BOLTTICS;
		BigShoot ();
	}
}


//===========================================================================

/*
===============
=
= CastNuke
=
===============
*/

void CastNuke (void)
{
	int	angle;

	if (!gamestate.nukes)
	{
		SD_PlaySound (NOITEMSND);
		return;
	}

	TakeNuke ();
	lastnuke = SP_TimeCount();

	for (angle = 0; angle < ANGLES; angle+= ANGLES/16)
	{
		SpawnNewObjFrac (player->x,player->y,&s_bigpshot1,24*PIXRADIUS);
		new->obclass = bigpshotobj;
		new->speed = SHOTSPEED;
		new->angle = angle;
	}
}

//===========================================================================

/*
===============
=
= DrinkPotion
=
===============
*/

void DrinkPotion (void)
{
	byte	*source;
	unsigned topline;

	if (!gamestate.potions)
	{
		SD_PlaySound (NOITEMSND);
		return;
	}

	TakePotion ();
	gamestate.body = MAXBODY;
	DrawBars();
}



//===========================================================================

/*
===============
=
= ReadScroll
=
===============
*/

extern	boolean	tileneeded[NUMFLOORS];

void ReadScroll (int scroll)
{
	int	i;

//
// make wall pictures purgable
//
	SPD_LoadGrChunk (SCROLLTOPPIC);
	SPD_LoadGrChunk (SCROLL1PIC + scroll);
	SPG_DrawPic(&hudSetup, grsegs[SCROLLTOPPIC],0,0);
	SPG_DrawPic(&hudSetup, grsegs[SCROLL1PIC+scroll],0,32);
	MM_FreePtr (&grsegs[SCROLL1PIC + scroll]);
	MM_FreePtr (&grsegs[SCROLLTOPPIC]);


	SPG_FlipBuffer();
waitkey:
	IN_ClearKeysDown ();
	IN_Ack();

}



/*
===============
=
= TakeDamage
=
===============
*/

void TakeDamage (int points)
{
	byte *source;
	unsigned	topline;

	if (!gamestate.body || bordertime || godmode)
		return;

	if (points >= gamestate.body)
	{
		points = gamestate.body;
		playstate = ex_died;
	}

	bordertime = points*FLASHTICS;
	SPG_SetBorderColor(FLASHCOLOR);

	if (gamestate.body<MAXBODY/3)
		SD_PlaySound (TAKEDMGHURTSND);
	else
		SD_PlaySound (TAKEDAMAGESND);

	gamestate.body -= points;
	DrawBars();
}



/*
=============================================================================

							INTERACTION

=============================================================================
*/


/*
==================
=
= OpenDoor
=
==================
*/

void OpenDoor (uint16_t bx, uint16_t by, uint16_t doorbase)
{
	int x,y;
	uint16_t *map;

	x=bx;
	y=by;
	map = gamestate.mapsegs[0]+mapwidth*y+x;
	while (tilemap[x][y]-doorbase>=0 && tilemap[x][y]-doorbase<4)
	{
		tilemap[x][y] = CASTAT(intptr_t, actorat[x][y]) = *map = 0;
		map--;
		x--;
	}
	x=bx+1;
	map = gamestate.mapsegs[0]+mapwidth*y+x;
	while (tilemap[x][y]-doorbase>=0 && tilemap[x][y]-doorbase<4)
	{
		tilemap[x][y] = CASTAT(intptr_t,actorat[x][y]) = *map = 0;
		map++;
		x++;
	}
	x=bx;
	y=by-1;
	map = gamestate.mapsegs[0]+mapwidth*y+x;
	while (tilemap[x][y]-doorbase>=0 && tilemap[x][y]-doorbase<4)
	{
		tilemap[x][y] = CASTAT(intptr_t,actorat[x][y]) = *map = 0;
		map-=mapwidth;
		y--;
	}
	y=by+1;
	map = gamestate.mapsegs[0]+mapwidth*y+x;
	while (tilemap[x][y]-doorbase>=0 && tilemap[x][y]-doorbase<4)
	{
		tilemap[x][y] = CASTAT(intptr_t,actorat[x][y]) = *map = 0;
		map+=mapwidth;
		y++;
	}
}


/*
==================
=
= HitSpecialTile
=
= Returns true if the move is blocked
=
==================
*/

boolean HitSpecialTile (unsigned x, unsigned y, unsigned tile)
{
	switch (tile)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		if (!gamestate.keys[0])
			return true;
		TakeKey(0);
		OpenDoor (x,y,SPECTILESTART+0);
		return false;

	case 4:
	case 5:
	case 6:
	case 7:
		if (!gamestate.keys[1])
			return true;
		TakeKey(1);
		OpenDoor (x,y,SPECTILESTART+4);
		return false;

	case 8:
	case 9:
	case 10:
	case 11:
		if (!gamestate.keys[2])
			return true;
		TakeKey(2);
		OpenDoor (x,y,SPECTILESTART+8);
		return false;

	case 12:
	case 13:
	case 14:
	case 15:
		if (!gamestate.keys[3])
			return true;
		TakeKey(3);
		OpenDoor (x,y,SPECTILESTART+12);
		return false;

	}

	return true;
}



/*
==================
=
= TouchActor
=
= Returns true if the move is blocked
=
==================
*/

boolean TouchActor (objtype *ob, objtype *check)
{
	if (ob->xh < check->xl || ob->xl > check->xh ||
		ob->yh < check->yl || ob->yl > check->yh)
		return false;				// not quite touching

	switch (check->obclass)
	{
	case bonusobj:
		if (check->temp1 == B_BOLT)
			GiveBolt ();
		else if (check->temp1 == B_NUKE)
			GiveNuke ();
		else if (check->temp1 == B_POTION)
			GivePotion ();
		else if (check->temp1 >= B_RKEY && check->temp1 <= B_BKEY)
			GiveKey (check->temp1-B_RKEY);
		else if (check->temp1 >= B_SCROLL1 && check->temp1 <= B_SCROLL8)
			GiveScroll (check->temp1-B_SCROLL1,true);
		else if (check->temp1 == B_CHEST)
			GiveChest ();
		else if (check->temp1 == B_GOAL)
			GiveGoal ();
		CASTAT(intptr_t,actorat[check->tilex][check->tiley]) = 0;
		RemoveObj (check);

		return false;

	}
	return	true;
}


/*
==================
=
= CalcBounds
=
==================
*/

void CalcBounds (objtype *ob)
{
//
// calculate hit rect
//
  ob->xl = ob->x - ob->size;
  ob->xh = ob->x + ob->size;
  ob->yl = ob->y - ob->size;
  ob->yh = ob->y + ob->size;
}


/*
===================
=
= LocationInActor
=
===================
*/

boolean LocationInActor (objtype *ob)
{
	int	x,y,xmin,ymin,xmax,ymax;
	objtype *check;

	CalcBounds (ob);

	xmin = (ob->x >> TILESHIFT)-2;
	ymin = (ob->y >> TILESHIFT)-2;
	xmax = xmin+5;
	ymax = ymin+5;

	for (x=xmin;x<xmax;x++)
		for (y=ymin;y<ymax;y++)
		{
			if (CASTAT(intptr_t, actorat[x][y])>LASTSPECIALTILE) {
				check = actorat[x][y];
				if (check->shootable && ob->xl <= check->xh
									 && ob->xh >= check->xl
									 && ob->yl <= check->yh
									 && ob->yh >= check->yl)
				return true;
			}
		}

	return false;
}


/*
===================
=
= ClipMove
=
= Only checks corners, so the object better be less than one tile wide!
=
===================
*/

void ClipMove (objtype *ob, long xmove, long ymove)
{
	int			xl,yl,xh,yh,tx,ty,nt1,nt2,x,y;
	long		intersect,basex,basey,pointx,pointy;
	unsigned	inside,total,tile;
	objtype		*check;
	boolean		moveok;

//
// move player and check to see if any corners are in solid tiles
//
	basex = ob->x;
	basey = ob->y;

	ob->x += xmove;
	ob->y += ymove;

	CalcBounds (ob);

	xl = ob->xl>>TILESHIFT;
	yl = ob->yl>>TILESHIFT;

	xh = ob->xh>>TILESHIFT;
	yh = ob->yh>>TILESHIFT;

	for (y=yl;y<=yh;y++)
		for (x=xl;x<=xh;x++)
		{
			check = actorat[x][y];
			if (!check)
				continue;		// blank floor, walk ok

			if ((unsigned)check<=LASTWALLTILE)
				goto blockmove;	// solid wall

			if ((unsigned)check<=LASTSPECIALTILE)
			{
				if ( HitSpecialTile (x,y,(unsigned)check-SPECTILESTART) )
					goto blockmove;		// whatever it was, it blocked the move
				else
					continue;
			}
			TouchActor(ob,check);		// pick up items
		}

//
// check nearby actors
//
	if (LocationInActor(ob))
	{
		ob->x -= xmove;
		if (LocationInActor(ob))
		{
			ob->x += xmove;
			ob->y -= ymove;
			if (LocationInActor(ob))
				ob->x -= xmove;
		}
	}
	return;		// move is OK!


blockmove:

	if (!SD_SoundPlaying())
		SD_PlaySound (HITWALLSND);

	moveok = false;

	do
	{
		xmove /= 2;
		ymove /= 2;
		if (moveok)
		{
			ob->x += xmove;
			ob->y += ymove;
		}
		else
		{
			ob->x -= xmove;
			ob->y -= ymove;
		}
		CalcBounds (ob);
		xl = ob->xl>>TILESHIFT;
		yl = ob->yl>>TILESHIFT;
		xh = ob->xh>>TILESHIFT;
		yh = ob->yh>>TILESHIFT;
		if (tilemap[xl][yl] || tilemap[xh][yl]
		|| tilemap[xh][yh] || tilemap[xl][yh] )
		{
			moveok = false;
			if (xmove>=-2048 && xmove <=2048 && ymove>=-2048 && ymove <=2048)
			{
				ob->x = basex;
				ob->y = basey;
				return;
			}
		}
		else
		{
			if (xmove>=-2048 && xmove <=2048 && ymove>=-2048 && ymove <=2048)
				return;
			moveok = true;
		}
	} while (1);
}


//==========================================================================


/*
===================
=
= ShotClipMove
=
= Only checks corners, so the object better be less than one tile wide!
=
===================
*/

boolean ShotClipMove (objtype *ob, long xmove, long ymove)
{
	int			xl,yl,xh,yh,tx,ty,nt1,nt2,x,y;
	long		intersect,basex,basey,pointx,pointy;
	unsigned	inside,total,tile;
	objtype		*check;
	boolean		moveok;

//
// move shot and check to see if any corners are in solid tiles
//
	basex = ob->x;
	basey = ob->y;

	ob->x += xmove;
	ob->y += ymove;

	CalcBounds (ob);

	xl = ob->xl>>TILESHIFT;
	yl = ob->yl>>TILESHIFT;

	xh = ob->xh>>TILESHIFT;
	yh = ob->yh>>TILESHIFT;

	for (y=yl;y<=yh;y++)
		for (x=xl;x<=xh;x++)
		{
			tile = tilemap[x][y];
			if (tile)
			{
				if ((unsigned)(tile-EXPWALLSTART)<NUMEXPWALLS)
					ExplodeWall (x,y);
				goto blockmove;
			}
		}
	return false;		// move is OK!


blockmove:

	SD_PlaySound (SHOOTWALLSND);

	moveok = false;

	do
	{
		xmove /= 2;
		ymove /= 2;
		if (moveok)
		{
			ob->x += xmove;
			ob->y += ymove;
		}
		else
		{
			ob->x -= xmove;
			ob->y -= ymove;
		}
		CalcBounds (ob);
		xl = ob->xl>>TILESHIFT;
		yl = ob->yl>>TILESHIFT;
		xh = ob->xh>>TILESHIFT;
		yh = ob->yh>>TILESHIFT;
		if (tilemap[xl][yl] || tilemap[xh][yl]
		|| tilemap[xh][yh] || tilemap[xl][yh] )
		{
			moveok = false;
			if (xmove>=-2048 && xmove <=2048 && ymove>=-2048 && ymove <=2048)
			{
				ob->x = basex;
				ob->y = basey;
				return true;
			}
		}
		else
		{
			if (xmove>=-2048 && xmove <=2048 && ymove>=-2048 && ymove <=2048)
				return true;
			moveok = true;
		}
	} while (1);
}



/*
=============================================================================

						   PLAYER CONTROL

=============================================================================
*/



void	T_Player (objtype *ob);

statetype s_player = {0,0,&T_Player,&s_player};

/*
===============
=
= SpawnPlayer
=
===============
*/

void SpawnPlayer (int tilex, int tiley, int dir)
{
	player->obclass = playerobj;
	player->active = true;
	player->tilex = tilex;
	player->tiley = tiley;
	player->x = ((long)tilex<<TILESHIFT)+TILEGLOBAL/2;
	player->y = ((long)tiley<<TILESHIFT)+TILEGLOBAL/2;
	player->state = &s_player;
	player->angle = (1-dir)*90;
	player->size = renderSetup.MinDist;
	CalcBounds (player);
	if (player->angle<0)
		player->angle += ANGLES;
}


/*
===================
=
= Thrust
=
===================
*/

void Thrust (int angle, unsigned speed)
{
	long xmove,ymove;

	if (lasttimecount>>5 != ((lasttimecount-tics)>>5) )
	{
	//
	// walk sound
	//
		if (lasttimecount&32)
			SD_PlaySound (WALK1SND);
		else
			SD_PlaySound (WALK2SND);
	}

	xmove = FixedByFrac(speed,costable[angle]);
	ymove = -FixedByFrac(speed,sintable[angle]);

	ClipMove(player,xmove,ymove);
	player->tilex = player->x >> TILESHIFT;
	player->tiley = player->y >> TILESHIFT;
}



/*
=======================
=
= ControlMovement
=
=======================
*/

void ControlMovement (objtype *ob)
{
	int	angle=0;
	long	speed=0;

	static int mouseAngleRem=0;
	int mouseAngle = mousexmove+mouseAngleRem;
	mouseAngleRem = mouseAngle%10;
	mouseAngle = mouseAngle/10;

	if (c.button1 || SP_StrafeOn())
	{
	//
	// strafing
	//
		//
		// side to side move
		//
		if (SP_StrafeOn()) { // keyboard is used for strafing, mouse changes angle
			ob->angle -= mouseAngle;

			if (ob->angle >= ANGLES)
				ob->angle -= ANGLES;
			if (ob->angle < 0)
				ob->angle += ANGLES;
		} else if (!mousexmove)
			speed = 0;
		else if (mousexmove<0)
			speed = -(long)mousexmove*300;
		else
			speed = -(long)mousexmove*300;

		if (c.xaxis == -1)
		{
			if (running)
				speed += RUNSPEED*tics;
			else
				speed += PLAYERSPEED*tics;
		}
		else if (c.xaxis == 1)
		{
			if (running)
				speed -= RUNSPEED*tics;
			else
				speed -= PLAYERSPEED*tics;
		}
		if (speed > 0)
		{
			if (speed >= TILEGLOBAL)
				speed = TILEGLOBAL-1;
			angle = ob->angle + ANGLES/4;
			if (angle >= ANGLES)
				angle -= ANGLES;
			Thrust (angle,speed);				// move to left
		}
		else if (speed < 0)
		{
			if (speed <= -TILEGLOBAL)
				speed = -TILEGLOBAL+1;
			angle = ob->angle - ANGLES/4;
			if (angle < 0)
				angle += ANGLES;
			Thrust (angle,-speed);				// move to right
		}
	}
	else
	{
	//
	// not strafing
	//

		//
		// turning
		//
		if (c.xaxis == 1)
		{
			ob->angle -= tics;
			if (running)				// fast turn
				ob->angle -= tics;
		}
		else if (c.xaxis == -1)
		{
			ob->angle+= tics;
			if (running)				// fast turn
				ob->angle += tics;
		}

		ob->angle -= mouseAngle;

		if (ob->angle >= ANGLES)
			ob->angle -= ANGLES;
		if (ob->angle < 0)
			ob->angle += ANGLES;

	}

	//
	// forward/backwards move
	//
	if (!mouseymove || SP_StrafeOn())
		speed = 0;
	else if (mouseymove<0)
		speed = -(long)mouseymove*500;
	else
		speed = -(long)mouseymove*200;

	if (c.yaxis == -1)
	{
		if (running)
			speed += RUNSPEED*tics;
		else
			speed += PLAYERSPEED*tics;
	}
	else if (c.yaxis == 1)
	{
		if (running)
			speed -= RUNSPEED*tics;
		else
			speed -= PLAYERSPEED*tics;
	}

	if (speed > 0)
	{
		if (speed >= TILEGLOBAL)
			speed = TILEGLOBAL-1;
		Thrust (ob->angle,speed);			// move forwards
	}
	else if (speed < 0)
	{
		if (speed <= -TILEGLOBAL)
			speed = -TILEGLOBAL+1;
		angle = ob->angle + ANGLES/2;
		if (angle >= ANGLES)
			angle -= ANGLES;
		Thrust (angle,-speed);				// move backwards
	}
}


/*
===============
=
= T_Player
=
===============
*/

void	T_Player (objtype *ob)
{
	int	angle,speed,scroll;
	unsigned	text,tilex,tiley;
	long	lspeed;


	ControlMovement (ob);


	//
	// firing
	//
	if (boltsleft)
	{
		handheight+=(tics<<2);
		if (handheight>MAXHANDHEIGHT)
			handheight = MAXHANDHEIGHT;

		ContinueBolt ();
		lasthand = lasttimecount;
	}
	else
	{
		if (c.button0)
		{
			handheight+=(tics<<2);
			if (handheight>MAXHANDHEIGHT)
				handheight = MAXHANDHEIGHT;

			if ((unsigned)SP_TimeCount()/FIRETIME != lastfiretime)
				BuildShotPower ();
			lasthand = lasttimecount;
		}
		else
		{
			if (lasttimecount > lasthand+HANDPAUSE)
			{
				handheight-=(tics<<1);
				if (handheight<0)
					handheight = 0;
			}

			if (gamestate.shotpower == MAXSHOTPOWER)
			{
				lastfiretime = (unsigned)SP_TimeCount()/FIRETIME;
				BigShoot ();
			}
			else if (gamestate.shotpower)
			{
				lastfiretime = (unsigned)SP_TimeCount()/FIRETIME;
				Shoot ();
			}
		}
	}

	//
	// special actions
	//

	if ( (SP_Keyboard(sc_Space) || SP_Keyboard(sc_H)) && gamestate.body != MAXBODY)
		DrinkPotion ();

	if (SP_Keyboard(sc_B) && !boltsleft)
		CastBolt ();

	if ( (SP_Keyboard(sc_Enter) || SP_Keyboard(sc_N)) && SP_TimeCount()-lastnuke > NUKETIME)
		CastNuke ();

	scroll = SP_LastScan()-2;
	if ( scroll>=0 && scroll<NUMSCROLLS && gamestate.scrolls[scroll])
		ReadScroll (scroll);

	DrawText ();
	DrawCompass ();

}

