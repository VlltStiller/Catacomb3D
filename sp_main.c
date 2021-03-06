/* Catacomb 3-D SDL Port
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

#include <stdlib.h>
#include <SDL/SDL.h>
#include "id_heads.h"

static const int gameTimerRate=70;		// I think 70 is correct, but maybe it should read 35 ?
static long timeCountStart=0;

static int pleaseExit=false;
static int inGame=false;
static int mouseGrabEnabled=true;


// some prototypes for the internal functions
static int translateKey(int Sym);
static void toggleMouseGrab();

int main(int argc, char **argv) {
	int i;
	for (i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "--nograb"))  {
			mouseGrabEnabled = false;
		}
	}

	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_EnableUNICODE(1); // needed to get complete input events

	SPG_Init();
	SPA_Init();

	SPD_SetupGameData();

	SPI_ClearKeysDown();
	US_Startup ();

	InitGame ();

	SDL_PauseAudio(0);
	DemoLoop();

	SDL_Quit();
	return 0;
}

void SP_PollEvents(void) {
	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			pleaseExit = true;
		} else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
			boolean down = event.type == SDL_KEYDOWN;
			if (down && event.key.keysym.sym == SDLK_m) { // override mouse grabbing
				toggleMouseGrab();
			}

			int idKey = translateKey(event.key.keysym.sym);
			if (idKey != 0) {
				SPI_InputKey(idKey, down);
			}
			char c=0;
			if ((event.key.keysym.unicode&0xFF80) == 0) {
				c = event.key.keysym.unicode&0x7F;
			}
			if (down && c != 0) {
				SPI_InputASCII(c);
			}
		} else if (event.type == SDL_MOUSEMOTION) {
			SPI_InputMouseMotion(event.motion.xrel,event.motion.yrel);
		} else if (event.type == SDL_MOUSEBUTTONDOWN) {
			if (event.button.button == SDL_BUTTON_LEFT) {
				SPI_InputKey(but_Mouse1, 1);
			} else if (event.button.button == SDL_BUTTON_RIGHT) {
				SPI_InputKey(but_Mouse2, 1);
			} else if (event.button.button == SDL_BUTTON_MIDDLE) {
				SPI_InputKey(but_Mouse3, 1);
			}
		} else if (event.type == SDL_MOUSEBUTTONUP) {
			if (event.button.button == SDL_BUTTON_LEFT) {
				SPI_InputKey(but_Mouse1, 0);
			} else if (event.button.button == SDL_BUTTON_RIGHT) {
				SPI_InputKey(but_Mouse2, 0);
			} else if (event.button.button == SDL_BUTTON_MIDDLE) {
				SPI_InputKey(but_Mouse3, 0);
			}
		} else if (event.type == SDL_VIDEORESIZE) {
			SDL_ResizeEvent *resize = (SDL_ResizeEvent*)&event;
			SPG_SetWindowSize(resize->w, resize->h);
		}
	}
}

static int translateKey(int Sym) {
	switch (Sym) {
	default: return 0; break;
	case SDLK_RETURN: return sc_Return; break;
	case SDLK_ESCAPE: return sc_Escape; break;
	case SDLK_SPACE: return sc_Space; break;
	case SDLK_BACKSPACE: return sc_BackSpace; break;
	case SDLK_TAB: return sc_Tab; break;
	case SDLK_LALT: return sc_Alt; break;
	case SDLK_RALT: return sc_Alt; break;
	case SDLK_LCTRL: return sc_Control; break;
	case SDLK_RCTRL: return sc_Control; break;
	case SDLK_LSHIFT: return sc_LShift; break;
	case SDLK_RSHIFT: return sc_RShift; break;
	case SDLK_CAPSLOCK: return sc_CapsLock; break;
	
	case SDLK_UP: return sc_UpArrow; break;
	case SDLK_DOWN: return sc_DownArrow; break;
	case SDLK_LEFT: return sc_LeftArrow; break;
	case SDLK_RIGHT: return sc_RightArrow; break;

	case SDLK_PAGEUP: return sc_PgUp; break;
	case SDLK_PAGEDOWN: return sc_PgDn; break;
	case SDLK_INSERT: return sc_Insert; break;
	case SDLK_DELETE: return sc_Delete; break;
	case SDLK_HOME: return sc_Home; break;
	case SDLK_END: return sc_End; break;
	case SDLK_F1: return sc_F1; break;
	case SDLK_F2: return sc_F2; break;
	case SDLK_F3: return sc_F3; break;
	case SDLK_F4: return sc_F4; break;
	case SDLK_F5: return sc_F5; break;
	case SDLK_F6: return sc_F6; break;
	case SDLK_F7: return sc_F7; break;
	case SDLK_F8: return sc_F8; break;
	case SDLK_F9: return sc_F9; break;
	case SDLK_F10: return sc_F10; break;
	case SDLK_F11: return sc_F11; break;
	case SDLK_F12: return sc_F12; break;
	case SDLK_1: return 2; break;
	case SDLK_2: return 3; break;
	case SDLK_3: return 4; break;
	case SDLK_4: return 5; break;
	case SDLK_5: return 6; break;
	case SDLK_6: return 7; break;
	case SDLK_7: return 8; break;
	case SDLK_8: return 9; break;
	case SDLK_9: return 10; break;
	case SDLK_0: return 11; break;
	case SDLK_a: return sc_A; break;
	case SDLK_b: return sc_B; break;
	case SDLK_c: return sc_C; break;
	case SDLK_d: return sc_D; break;
	case SDLK_e: return sc_E; break;
	case SDLK_f: return sc_F; break;
	case SDLK_g: return sc_G; break;
	case SDLK_h: return sc_H; break;
	case SDLK_i: return sc_I; break;
	case SDLK_j: return sc_J; break;
	case SDLK_k: return sc_K; break;
	case SDLK_l: return sc_L; break;
	case SDLK_m: return sc_M; break;
	case SDLK_n: return sc_N; break;
	case SDLK_o: return sc_O; break;
	case SDLK_p: return sc_P; break;
	case SDLK_q: return sc_Q; break;
	case SDLK_r: return sc_R; break;
	case SDLK_s: return sc_S; break;
	case SDLK_t: return sc_T; break;
	case SDLK_u: return sc_U; break;
	case SDLK_v: return sc_V; break;
	case SDLK_w: return sc_W; break;
	case SDLK_x: return sc_X; break;
	case SDLK_y: return sc_Y; break;
	case SDLK_z: return sc_Z; break;
	}
}

static void toggleMouseGrab() {
	mouseGrabEnabled = !mouseGrabEnabled;
	if (inGame) {
		if (mouseGrabEnabled) {
			assert(SDL_WM_GrabInput(SDL_GRAB_ON) == SDL_GRAB_ON);
			SDL_ShowCursor(SDL_DISABLE);
		} else {
			assert(SDL_WM_GrabInput(SDL_GRAB_OFF) == SDL_GRAB_OFF);
			SDL_ShowCursor(SDL_ENABLE);
		}
	}
}



void SP_Exit() {
	pleaseExit = true;
	exit(0);
}

int SP_GameActive() {
	return inGame;
}

void SP_GameEnter() {
	if (mouseGrabEnabled && !inGame) {
		assert(SDL_WM_GrabInput(SDL_GRAB_ON) == SDL_GRAB_ON);
		SDL_ShowCursor(SDL_DISABLE);
		SPI_GetMouseDelta(NULL, NULL);
	}
	inGame = true;
}

void SP_GameLeave() {
	if (mouseGrabEnabled && inGame) {
		assert(SDL_WM_GrabInput(SDL_GRAB_OFF) == SDL_GRAB_OFF);
		SDL_ShowCursor(SDL_ENABLE);
	}
	inGame = false;
}

int SP_StrafeOn() {
	return mouseGrabEnabled;
}

long SP_TimeCount() { // Global time in 70Hz ticks
	return SDL_GetTicks()*70/1000-timeCountStart;
}



// originally from id_mm.c but that doesn't exist anymore

void MM_GetPtr (memptr *baseptr,unsigned long size)
{
	*baseptr = malloc(size);
}

void MM_FreePtr (memptr *baseptr)
{
	free((void*)*baseptr);
	*baseptr = NULL;
}




// id_us_a.asm

static unsigned rndindex=0;

static unsigned rndtable[] = {0,   8, 109, 220, 222, 241, 149, 107,  75, 248, 254, 140,  16,  66,
	74,  21, 211,  47,  80, 242, 154,  27, 205, 128, 161,  89,  77,  36,
	95, 110,  85,  48, 212, 140, 211, 249,  22,  79, 200,  50,  28, 188,
	52, 140, 202, 120,  68, 145,  62,  70, 184, 190,  91, 197, 152, 224,
	149, 104,  25, 178, 252, 182, 202, 182, 141, 197,   4,  81, 181, 242,
	145,  42,  39, 227, 156, 198, 225, 193, 219,  93, 122, 175, 249,   0,
	175, 143,  70, 239,  46, 246, 163,  53, 163, 109, 168, 135,   2, 235,
	25,  92,  20, 145, 138,  77,  69, 166,  78, 176, 173, 212, 166, 113,
	94, 161,  41,  50, 239,  49, 111, 164,  70,  60,   2,  37, 171,  75,
	136, 156,  11,  56,  42, 146, 138, 229,  73, 146,  77,  61,  98, 196,
	135, 106,  63, 197, 195,  86,  96, 203, 113, 101, 170, 247, 181, 113,
	80, 250, 108,   7, 255, 237, 129, 226,  79, 107, 112, 166, 103, 241,
	24, 223, 239, 120, 198,  58,  60,  82, 128,   3, 184,  66, 143, 224,
	145, 224,  81, 206, 163,  45,  63,  90, 168, 114,  59,  33, 159,  95,
	28, 139, 123,  98, 125, 196,  15,  70, 194, 253,  54,  14, 109, 226,
	71,  17, 161,  93, 186,  87, 244, 138,  20,  52, 123, 251,  26,  36,
	17,  46,  52, 231, 232,  76,  31, 221,  84,  37, 216, 165, 212, 106,
	197, 242,  98,  43,  39, 175, 254, 145, 190,  84, 118, 222, 187, 136,
	120, 163, 236, 249 };


unsigned LastRnd;

void US_InitRndT (boolean randomize) {
	if (randomize) {
		rndindex = time(NULL);
	} else {
		rndindex = 0;
	}
}

int US_RndT(void) {
	return rndtable[(rndindex++)&0xFF];
}



int RANDOM(int Max) {
	if (Max != 0) {
		return rand()%Max;
	}
	return 0;
}


/*
==========================
=
= Quit
=
==========================
*/

void Quit (char *error)
{
	US_Shutdown ();
	if (error && *error) {
		printf("ERROR: %s\n", error);
	}
	SP_Exit();
}

