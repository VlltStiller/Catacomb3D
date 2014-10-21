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

// C3_TRACE.C

#include "c3_def.h"

boolean CheckTileCoords(int x, int y) {
	if (x < 0 || y < 0 || x >= 64 || y >= 64 || reallyabsolutelypositivelyaborttrace) {
		reallyabsolutelypositivelyaborttrace = true;
		return 0;
	}
	return 1;
}

byte SafeTilemap(int x, int y) {
	if (x < 0 || y < 0 || x >= 64 || y >= 64) {
		reallyabsolutelypositivelyaborttrace = true;
		return 1;
	}
//	assert(!(x < 0 || y < 0 || x >= 64 || y >= 64));
	return tilemap[x][y];
}


/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/


//
// TESTWALLVISABLE will set the global variable wallvisable to 1 or 0
// depending on if tile.x,tile.y,wallon is visable from focal point
//
// effectively a crude test to see if a wall is backfacing or not
//
#define TESTWALLVISABLE {						\
	if (tile.y<focal.y)                         \
		voffset = 0;                            \
	else if (tile.y==focal.y)                   \
		voffset = 3;                            \
	else                                        \
		voffset = 6;                            \
	if (tile.x==focal.x)                        \
		voffset ++;                             \
	else if (tile.x>focal.x)                    \
		voffset += 2;                           \
	wallvisable = visable[voffset][wallon]; }


/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

boolean	restarttrace, reallyabsolutelypositivelyaborttrace;

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

unsigned	wallvisable,voffset;


fixed edgex,edgey;

int wallon;
int basecolor;

walltype *oldwall;

//
// offsets from upper left corner of a tile to the left and right edges of
// a given wall (NORTH-WEST)
//
fixed point1x[4] = {TILEGLOBAL,TILEGLOBAL,0      ,0       };
fixed point1y[4] = {0      ,TILEGLOBAL,TILEGLOBAL,0       };

fixed point2x[4] = {0      ,TILEGLOBAL,TILEGLOBAL,0       };
fixed point2y[4] = {0     ,0	   ,TILEGLOBAL ,TILEGLOBAL};


//
// offset from tile.x,tile.y of the tile that shares wallon side
// (side is not visable if it is shared)
//
int sharex[4] = { 0, 1, 0,-1};
int sharey[4] = {-1, 0, 1, 0};

//
// amount to move tile.x,tile.y to follow wallon to another tile
//
int followx[4] = {-1, 0, 1, 0};
int followy[4] = { 0,-1, 0, 1};

//
// cornerwall gives the wall on the same tile to start following when the
// wall ends at an empty tile (go around an edge on same tile)
// turnwall gives the wall on tile.x+sharex,tile.y+sharey to start following
// when the wall hits another tile (right angle corner)
//
int cornerwall[4] = {WEST,NORTH,EAST,SOUTH};
int turnwall[4] = {EAST,SOUTH,WEST,NORTH};

//
// wall visabilities in reletive locations
//   -,- means western and southern wall are visible
//   +,0 means eastern but neither northern nor southern wall is visible
//
// -,- 0,- +,-
// -,0 0,0 +,0
// -,+ 0,+ +,+
//
int visable[9][4] =
{
 {0,1,1,0}, {0,0,1,0}, {0,0,1,1},
 {0,1,0,0}, {0,0,0,0}, {0,0,0,1},
 {1,1,0,0}, {1,0,0,0}, {1,0,0,1}
};

int startwall[9] =  {2,2,3, 1,0,3, 1,0,0};
int backupwall[9] = {3,3,0, 2,0,0, 2,1,1};


int	walllength;

/*
=============================================================================

					 FUNCTIONS

=============================================================================
*/

/*
========================
=
= FollowTrace
=
========================
*/

int FollowTrace (fixed tracex, fixed tracey, long deltax, long deltay, int max)
{
	int tx,ty,otx,oty;
	long absdx,absdy,xstep,ystep;

	tx = tracex>>TILESHIFT;
	ty = tracey>>TILESHIFT;

	if (!CheckTileCoords(tx,ty)) { return 0; }

	spotvis[tx][ty] = true;

	absdx=LABS(deltax);
	absdy=LABS(deltay);

	if (absdx>absdy)
	{
		ystep = (deltay<<8)/(absdx>>8);

		if (!ystep)
			ystep = deltay>0 ? 1 : -1;

		oty = (tracey+ystep)>>TILESHIFT;
		if (deltax>0)
		{
//###############
//
// step x by +1
//
//###############
			do
			{
				tx++;
				if (!CheckTileCoords(tx,ty)) { return 0; }
				spotvis[tx][ty] = true;
				tracey+=ystep;
				ty = tracey>>TILESHIFT;

				if (ty!=oty)
				{
					if (SafeTilemap(tx-1,ty))
					{
						tile.x = tx-1;
						tile.y = ty;
						return 1;
					}
					oty = ty;
				}
				if (SafeTilemap(tx,ty))
				{
					tile.x = tx;
					tile.y = ty;
					return 1;
				}
			} while (--max);
			return 0;
		}
		else
		{
//###############
//
// step x by -1
//
//###############
			do
			{
				if (!CheckTileCoords(tx,ty)) { return 0; }
				tx--;
				spotvis[tx][ty] = true;
				tracey+=ystep;
				ty = tracey>>TILESHIFT;

				if (ty!=oty)
				{
					if (SafeTilemap(tx,oty))
					{
						tile.x = tx;
						tile.y = oty;
						return 1;
					}
					oty = ty;
				}
				if (SafeTilemap(tx,ty))
				{
					tile.x = tx;
					tile.y = ty;
					return 1;
				}
			} while (--max);
			return 0;

		}
	}
	else
	{
		xstep = (deltax<<8)/(absdy>>8);
		if (!xstep)
			xstep = deltax>0 ? 1 : -1;


		otx = (tracex+xstep)>>TILESHIFT;
		if (deltay>0)
		{
//###############
//
// step y by +1
//
//###############
			do
			{
				if (!CheckTileCoords(tx,ty)) { return 0; }
				ty++;
				spotvis[tx][ty] = true;
				tracex+=xstep;
				tx = tracex>>TILESHIFT;

				if (tx!=otx)
				{
					if (SafeTilemap(tx,ty-1))
					{
						tile.x = tx;
						tile.y = ty-1;
						return 1;
					}
					otx = tx;
				}
				if (SafeTilemap(tx,ty))
				{
					tile.x = tx;
					tile.y = ty;
					return 1;
				}
			} while (--max);
			return 0;
		}
		else
		{
//###############
//
// step y by -1
//
//###############
			do
			{
				if (!CheckTileCoords(tx,ty)) { return 0; }
				ty--;
				spotvis[tx][ty] = true;
				tracex+=xstep;
				tx = tracex>>TILESHIFT;

				if (tx!=otx)
				{
					if (SafeTilemap(otx,ty))
					{
						tile.x = otx;
						tile.y = ty;
						return 1;
					}
					otx = tx;
				}
				if (SafeTilemap(tx,ty))
				{
					tile.x = tx;
					tile.y = ty;
					return 1;
				}
			} while (--max);
			return 0;
		}

	}

}


//===========================================================================


/*
=================
=
= BackTrace
=
= Traces backwards from edgex,edgey to viewx,viewy to see if a closer
= tile obscures the given point.  If it does, it finishes the wall and
= starts a new one.
= Returns true if a tile is hit.
= Call with a 1 to have it automatically finish the current wall
=
=================
*/

int BackTrace (int finish)
{
	fixed tracex,tracey;
	long deltax,deltay,absdx,absdy;
	int steps,otx,oty,testx,testheight,offset,wall;

	deltax = viewx-edgex;
	deltay = viewy-edgey;

	absdx = LABS(deltax);
	absdy = LABS(deltay);

	if (absdx>absdy)
		steps = ABS(focal.x-(edgex>>TILESHIFT))-1;
	else
		steps = ABS(focal.y-(edgey>>TILESHIFT))-1;

	if (steps<=0)
		return 0;

	otx = tile.x;
	oty = tile.y;
	if (!FollowTrace(edgex,edgey,deltax,deltay,steps))
		return 0;

	//
	// if the start wall is behind the focal point, the trace went too far back
	//
	if (ABS(tile.x-focal.x)<2 && ABS(tile.y-focal.y)<2)	// too close
	{
		if (tile.x == focal.x && tile.y == focal.y)
		{
			tile.x = otx;
			tile.y = oty;
			return 0;
		}

		if (tile.x<focal.x)
		{
			if (tile.y<focal.y)
				wall = SOUTH;
			else
				wall = EAST;
		}
		else if (tile.x==focal.x)
		{
			if (tile.y<focal.y)
				wall = SOUTH;
			else
				wall = NORTH;
		}
		else
		{
			if (tile.y<=focal.y)
				wall = WEST;
			else
				wall = NORTH;
		}

//
// rotate the X value to see if it is behind the view plane
//
		if (TransformX (((long)tile.x<<16)+point1x[wall],((long)tile.y<<16)+point1y[wall]) < renderSetup.FocalLength)
		{
			tile.x = otx;
			tile.y = oty;
			return 0;
		}
	}

//
// if the old wall is still behind a closer wall, ignore the back trace
// and continue on (dealing with limited precision...)
//
	if (finish && !FinishWall ())	// the wall is still behind a forward wall
	{
		tile.x = otx;
		tile.y = oty;
		rightwall->x1 = oldwall->x2;		// common edge with last wall
		rightwall->height1 = oldwall->height2;
		return 0;
	}


	//
	// back up along the intersecting face to find the rightmost wall
	//

	if (tile.y<focal.y)
		offset = 0;
	else if (tile.y==focal.y)
		offset = 3;
	else
		offset = 6;
	if (tile.x==focal.x)
		offset ++;
	else if (tile.x>focal.x)
		offset += 2;

	wallon = backupwall[offset];

	while (SafeTilemap(tile.x,tile.y))
	{
		if (!CheckTileCoords(tile.x,tile.y)) { return 0; }
		tile.x += followx[wallon];
		tile.y += followy[wallon];
	};

	tile.x -= followx[wallon];
	tile.y -= followy[wallon];

	wallon = cornerwall[wallon];	// turn to first visable face

	edgex = ((long)tile.x<<16);
	edgey = ((long)tile.y<<16);

	TransformPoint (edgex+point1x[wallon],edgey+point1y[wallon],&rightwall->x1,&rightwall->height1);

	basecolor = SafeTilemap(tile.x,tile.y);

	return 1;
}

//===========================================================================


/*
=================
=
= ForwardTrace
=
= Traces forwards from edgex,edgey along the line from viewx,viewy until
= a solid tile is hit.  Sets tile.x,tile.y
=
=================
*/

void ForwardTrace (void)
{
	int offset;
	fixed tracex,tracey;
	long deltax,deltay;

	deltax = edgex-viewx;
	deltay = edgey-viewy;

	FollowTrace(edgex,edgey,deltax,deltay,0);

	if (tile.y<focal.y)
		offset = 0;
	else if (tile.y==focal.y)
		offset = 3;
	else
		offset = 6;
	if (tile.x==focal.x)
		offset ++;
	else if (tile.x>focal.x)
		offset += 2;

	wallon = startwall[offset];

//
// start the new wall
//
	edgex = ((long)tile.x<<16);
	edgey = ((long)tile.y<<16);

//
// if entire first wall is invisable, corner
//
	TransformPoint (edgex+point2x[wallon],edgey+point2y[wallon],&rightwall->x2,&rightwall->height2);

	if (SafeTilemap(tile.x+sharex[wallon],tile.y+sharey[wallon]))
	{
		wallon = cornerwall [wallon];
	} else if (rightwall->x2 < (rightwall-1)->x2)
	{
//		printf("DANGER, POTENTIAL CRASH AHEAD !\n");
		wallon = cornerwall [wallon];
	}

//
// transform first point
//

	TransformPoint (edgex+point1x[wallon],edgey+point1y[wallon],&rightwall->x1,&rightwall->height1);

	basecolor = SafeTilemap(tile.x,tile.y);
}


//===========================================================================


/*
=================
=
= FinishWall
=
= Transforms edgex,edgey as the next point of the current wall
= and sticks it in the wall list
=
=================
*/

int FinishWall (void)
{
  char num[20];

  oldwall = rightwall;

	rightwall->color  = basecolor;

  TransformPoint (edgex,edgey,&rightwall->x2,&rightwall->height2);

  if (rightwall->x2 <= (rightwall-1)->x2+2
  && rightwall->height2 < (rightwall-1)->height2 )
	return 0;

  rightwall->walllength = walllength;

  switch (wallon)
  {
  case north:
  case south:
	  rightwall->side = 0;
	  rightwall->planecoord = edgey;
	  break;

  case west:
  case east:
	  rightwall->side = 1;
	  rightwall->planecoord = edgex;
	  break;
  }

  walllength = 1;

  rightwall++;

  return 1;
}

//===========================================================================


/*
=================
=
= InsideCorner
=
=================
*/

void InsideCorner (void)
{
  int offset;

  //
  // the wall turned -90 degrees, so draw what we have, move to the new tile,
  // change wallon, change color, and continue following.
  //
  FinishWall ();

  tile.x += sharex[wallon];
  tile.y += sharey[wallon];

  wallon = turnwall[wallon];

  //
  // if the new wall is visable, continue following it.  Otherwise
  // follow it backwards until it turns
  //
  TESTWALLVISABLE;

  if (wallvisable)
  {
  //
  // just turn to the next wall and continue
  //
    rightwall->x1 = oldwall->x2;		// common edge with last wall
    rightwall->height1 = oldwall->height2;
    basecolor = SafeTilemap(tile.x,tile.y);
    return;			// continue from here
  }
  //
  // back follow the invisable wall until it turns, then follow that
  //
  do
  {
	if (!CheckTileCoords(tile.x,tile.y)) { return; }
	tile.x += followx[wallon];
    tile.y += followy[wallon];
  } while (SafeTilemap(tile.x,tile.y));

  tile.x -= followx[wallon];
  tile.y -= followy[wallon];

  wallon = cornerwall[wallon];	// turn to first visable face

  edgex = ((long)tile.x<<16)+point1x[wallon];
  edgey = ((long)tile.y<<16)+point1y[wallon];

  if (!BackTrace(0))		// backtrace without finishing a wall
  {
    TransformPoint (edgex,edgey,&rightwall->x1,&rightwall->height1);
    basecolor = SafeTilemap(tile.x,tile.y);
  }
}

//===========================================================================


/*
=================
=
= OutsideCorner
=
=================
*/

void OutsideCorner (void)
{
  int offset;

  //
  // edge is the outside edge of a corner, so draw the current wall and
  // turn the corner (+90 degrees)
  //
  FinishWall ();

  tile.x -= followx[wallon];	// backup to the real tile
  tile.y -= followy[wallon];
  wallon = cornerwall[wallon];

  //
  // if the new wall is visable, continue following it.  Otherwise
  // trace a ray from the corner to find a wall in the distance to
  // follow
  //
  TESTWALLVISABLE;

  if (wallvisable)
  {
  //
  // the new wall is visable, so just continue on
  //
    rightwall->x1 = oldwall->x2;		// common edge with last wall
    rightwall->height1 = oldwall->height2;
    return;			// still on same tile, so color is ok
  }

//
// start from a new tile further away
//
  ForwardTrace();		// find the next wall further back
}


//===========================================================================


/*
=================
=
= FollowWalls
=
= Starts a wall edge at the leftmost edge of tile.x,tile.y and follows it
= until something else is seen or the entire view area is covered
=
=================
*/

void FollowWalls (void)
{
	int height,newcolor,offset,wall;

//####################
//
// figure leftmost wall of new tile
//
//####################

	restart:

	walllength = 1;

	if (tile.y<focal.y)
		offset = 0;
	else if (tile.y==focal.y)
		offset = 3;
	else
		offset = 6;
	if (tile.x==focal.x)
		offset ++;
	else if (tile.x>focal.x)
		offset += 2;

	wallon = startwall[offset];

//
// if the start wall is inside a block, skip it by cornering to the second wall
//
	if ( SafeTilemap(tile.x+sharex[wallon],tile.y+sharey[wallon]))
		wallon = cornerwall [wallon];

//
// transform first edge to screen coordinates
//
	edgex = ((long)tile.x<<16);
	edgey = ((long)tile.y<<16);

	TransformPoint (edgex+point1x[wallon],edgey+point1y[wallon],&rightwall->x1,&rightwall->height1);

	basecolor = SafeTilemap(tile.x,tile.y);

//##################
//
// follow the wall as long as possible
//
//##################

advance:

	do	// while ( tile.x != right.x || tile.y != right.y)
	{
		if (!CheckTileCoords(tile.x,tile.y)) { return; }
//
// check for conditions that shouldn't happed...
//
		if (rightwall->x1 >= renderSetup.Width)	// somehow missed right tile...
			return;

		if (rightwall == &walls[DANGERHIGH])
		{
			assert(false);
//
// somethiing got messed up!  Correct by thrusting ahead...
//
			bordertime = 60;
			Thrust(player->angle,TILEGLOBAL/4);
			player->angle+=5;
			if (player->angle>ANGLES)
				player->angle-=ANGLES;
			restarttrace = true;
			return;
		}

		//
		// proceed along wall
		//

		edgex = ((long)tile.x<<16)+point2x[wallon];
		edgey = ((long)tile.y<<16)+point2y[wallon];

		if (BackTrace(1)) {		// went behind a closer wall
			continue;
		}

		//
		// advance to next tile along wall
		//
		tile.x += followx[wallon];
		tile.y += followy[wallon];

		if (SafeTilemap(tile.x+sharex[wallon],tile.y+sharey[wallon]))
		{
			InsideCorner ();		// turn at a corner
			continue;
		}

		newcolor = SafeTilemap(tile.x,tile.y);

		if (!newcolor)		// turn around an edge
		{
			OutsideCorner ();
			continue;
		}

		if (newcolor != basecolor)
		{
//
// wall changed color, so draw what we have and continue following
//
			FinishWall ();
			rightwall->x1 = oldwall->x2;	// new wall shares this edge
			rightwall->height1 = oldwall->height2;
			basecolor = newcolor;

			continue;
		}
		walllength++;
	} while (tile.x != right.x || tile.y != right.y);


//######################
//
// draw the last tile
//
//######################

	edgex = ((long)tile.x<<16)+point2x[wallon];
	edgey = ((long)tile.y<<16)+point2y[wallon];
	FinishWall();

	wallon = cornerwall[wallon];

//
// if the corner wall is visable, draw it
//
	TESTWALLVISABLE;

	if (wallvisable)
	{
		rightwall->x1 = oldwall->x2;		// common edge with last wall
		rightwall->height1 = oldwall->height2;
		edgex = ((long)tile.x<<16)+point2x[wallon];
		edgey = ((long)tile.y<<16)+point2y[wallon];
		FinishWall();
	}

}

//===========================================================================