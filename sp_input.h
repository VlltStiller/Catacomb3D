#ifndef SP_INPUT_H
#define SP_INPUT_H

#include "id_heads.h"

#define	NumCodes	128

typedef	byte		ScanCode;
#define	sc_None			0
#define	sc_Bad			0xff
#define	sc_Return		0x1c
#define	sc_Enter		sc_Return
#define	sc_Escape		0x01
#define	sc_Space		0x39
#define	sc_BackSpace	0x0e
#define	sc_Tab			0x0f
#define	sc_Alt			0x38
#define	sc_Control		0x1d
#define	sc_CapsLock		0x3a
#define	sc_LShift		0x2a
#define	sc_RShift		0x36
#define	sc_UpArrow		0x48
#define	sc_DownArrow	0x50
#define	sc_LeftArrow	0x4b
#define	sc_RightArrow	0x4d
#define	sc_Insert		0x52
#define	sc_Delete		0x53
#define	sc_Home			0x47
#define	sc_End			0x4f
#define	sc_PgUp			0x49
#define	sc_PgDn			0x51
#define	sc_F1			0x3b
#define	sc_F2			0x3c
#define	sc_F3			0x3d
#define	sc_F4			0x3e
#define	sc_F5			0x3f
#define	sc_F6			0x40
#define	sc_F7			0x41
#define	sc_F8			0x42
#define	sc_F9			0x43
#define	sc_F10			0x44
#define	sc_F11			0x57
#define	sc_F12			0x59

#define	sc_A			0x1e
#define	sc_B			0x30
#define	sc_C			0x2e
#define	sc_D			0x20
#define	sc_E			0x12
#define	sc_F			0x21
#define	sc_G			0x22
#define	sc_H			0x23
#define	sc_I			0x17
#define	sc_J			0x24
#define	sc_K			0x25
#define	sc_L			0x26
#define	sc_M			0x32
#define	sc_N			0x31
#define	sc_O			0x18
#define	sc_P			0x19
#define	sc_Q			0x10
#define	sc_R			0x13
#define	sc_S			0x1f
#define	sc_T			0x14
#define	sc_U			0x16
#define	sc_V			0x2f
#define	sc_W			0x11
#define	sc_X			0x2d
#define	sc_Y			0x15
#define	sc_Z			0x2c

#define	but_Mouse1		0x71
#define	but_Mouse2		0x72
#define	but_Mouse3		0x73

#define	key_None		0
#define	key_Return		0x0d
#define	key_Enter		key_Return
#define	key_Escape		0x1b
#define	key_Space		0x20
#define	key_BackSpace	0x08
#define	key_Tab			0x09
#define	key_Delete		0x7f

typedef	enum { motion_Left = -1,motion_Up = -1, motion_None = 0, motion_Right = 1,motion_Down = 1 } Motion;

typedef	struct {
	boolean		run, fire, strafe, bolt, nuke, potion;
	int			x,y; // mouse delta x,y
	Motion		xaxis,yaxis;
} ControlInfo;

typedef	struct {
	ScanCode	button0,button1,
				up, left, right, down;
} KeyboardDef;

extern	KeyboardDef	KbdDefs;

void SPI_ClearKeysDown(void);
int SPI_GetKeyDown(int Key);
char SPI_GetLastASCII(void);
int SPI_GetLastKey(void);
void SPI_GetMouseDelta(int *X, int *Y);
void SPI_GetPlayerControl(ControlInfo *info);
char *SPI_GetScanName(ScanCode scan);
void SPI_InputASCII(char ASCIICode);
void SPI_InputKey(int Key, int Down);
void SPI_InputMouseMotion(int DX, int DY);
void SPI_WaitForever(void);
boolean SPI_WaitFor(long Delay);

#endif

