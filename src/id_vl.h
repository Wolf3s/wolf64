// ID_VL.H

#ifndef __ID_VL_H_
#define __ID_VL_H_

#include "wl_types.h"
#include <libdragon.h>

// wolf compatability

void Quit (const char *error,...);

//===========================================================================

#define CHARWIDTH		2
#define TILEWIDTH		4

//===========================================================================

extern surface_t *screenBuffer;
extern surface_t screens[2];

extern  boolean  fullscreen, usedoublebuffering;
extern  unsigned screenWidth, screenHeight, screenPitch, bufferPitch;
extern  int      screenBits;

extern	boolean  screenfaded;
extern	unsigned bordercolor;

extern  uint32_t *ylookup;

extern uint16_t gamepal[256];
extern uint16_t curpal[256];

//===========================================================================

//
// VGA hardware routines
//

void VL_Wait(int ms);
void VL_WaitVBL(int a);

void VL_ClearScreen(uint8_t c);

void VL_DePlaneVGA (byte *source, int width, int height);
void VL_SetVGAPlaneMode (void);
void VL_SetTextMode (void);
void VL_Shutdown (void);

void VL_ConvertPalette(byte *srcpal, uint16_t *destpal, int numColors);
void VL_FillPalette (int red, int green, int blue);
void VL_SetPalette  (const uint16_t *palette, bool forceupdate);
void VL_GetPalette  (uint16_t *palette);
void VL_FadeOut     (int start, int end, int red, int green, int blue, int steps);
void VL_FadeIn      (int start, int end, uint16_t *palette, int steps);

byte *VL_LockSurface(surface_t *surface);
void VL_UnlockSurface(surface_t *surface);

byte VL_GetPixel        (int x, int y);
void VL_Plot            (int x, int y, int color);
void VL_Hlin            (unsigned x, unsigned y, unsigned width, int color);
void VL_Vlin            (int x, int y, int height, int color);
void VL_BarScaledCoord  (int scx, int scy, int scwidth, int scheight, int color);
void VL_Bar             (int x, int y, int width, int height, int color);

void VL_DrawPicBare             (int x, int y, byte *pic, int width, int height);
void VL_ScreenToScreen          (surface_t *source, surface_t *dest);
void VL_MemToScreenScaledCoord  (byte *source, int width, int height, int scx, int scy);
void VL_MemToScreenScaledCoord2  (byte *source, int origwidth, int origheight, int srcx, int srcy,
                                    int destx, int desty, int width, int height);

void VL_MemToScreen (byte *source, int width, int height, int x, int y);
surface_t *VL_GrabScreen(void);

#endif
