// ID_VL.C

#include <string.h>
#include "wl_def.h"

// Uncomment the following line, if you get destination out of bounds
// assertion errors and want to ignore them during debugging
//#define IGNORE_BAD_DEST

#ifdef IGNORE_BAD_DEST
#undef assert
#define assert(x) if(!(x)) return
#define assert_ret(x) if(!(x)) return 0
#else
#define assert_ret(x) assert(x)
#endif

boolean fullscreen = true;
boolean usedoublebuffering = true;
unsigned screenWidth = 320;
unsigned screenHeight = 200;
int      screenBits = 16;

unsigned screenPitch;

surface_t *screenBuffer = NULL;
surface_t storedScreen, screenGrab;
unsigned bufferPitch;

boolean	 screenfaded;
unsigned bordercolor;

uint32_t *ylookup;

uint16_t palette1[256], palette2[256];
uint16_t curpal[256] __attribute__((aligned(16)));


#define CASSERT(x) extern int ASSERT_COMPILE[((x) != 0) * 2 - 1];
#define RGB(r, g, b) ((r>>1<<11)|(g>>1<<6)|(b>>1<<1)|1)

uint16_t gamepal[]={
#ifdef SPEAR
    #include "sodpal.inc"
#else
    #include "wolfpal.inc"
#endif
};

CASSERT(lengthof(gamepal) == 256)

//===========================================================================

void VL_Wait(int ms)
{
    uint64_t start = get_ticks_ms();
    while (get_ticks_ms() - start < (uint64_t) ms)
        SD_Poll();
}

void VL_WaitVBL(int a)
{
    VL_Wait(a*8);
}

/*
=======================
=
= VL_Shutdown
=
=======================
*/

void VL_Shutdown (void)
{
    rspq_wait();
    surface_free(&storedScreen);
    surface_free(&screenGrab);
    display_close();

    free (ylookup);
    free (pixelangle);
    free (wallheight);
#if defined(USE_FLOORCEILINGTEX) || defined(USE_CLOUDSKY)
    free (spanstart);

    spanstart = NULL;
#endif
    screenBuffer = NULL;
    ylookup = NULL;
    pixelangle = NULL;
    wallheight = NULL;
}

void VL_ClearScreen(uint8_t c)
{
  memset(screenBuffer->buffer, c, screenBuffer->height * screenBuffer->stride);
}


/*
=======================
=
= VL_SetVGAPlaneMode
=
=======================
*/

void VL_SetVGAPlaneMode (void)
{
    int i;

    resolution_t res = { .width = screenWidth + 32, .height = screenHeight + 16, .interlaced = INTERLACE_OFF };
    display_init(res, screenBits == 16 ? DEPTH_16_BPP : DEPTH_32_BPP, 2, GAMMA_NONE, FILTERS_RESAMPLE);

    memcpy(UncachedUShortAddr(curpal), gamepal, sizeof gamepal);

    storedScreen = surface_alloc(FMT_CI8, screenWidth, screenHeight);
    screenBuffer = &storedScreen;
    VL_ClearScreen(0);

    screenGrab = surface_alloc(FMT_CI8, screenWidth, screenHeight);

    screenPitch = storedScreen.stride;
    bufferPitch = screenPitch;

    ylookup = SafeMalloc(screenHeight * sizeof(*ylookup));
    pixelangle = SafeMalloc(screenWidth * sizeof(*pixelangle));
    wallheight = SafeMalloc(screenWidth * sizeof(*wallheight));
#if defined(USE_FLOORCEILINGTEX) || defined(USE_CLOUDSKY)
    spanstart = SafeMalloc((screenHeight / 2) * sizeof(*spanstart));
#endif

    for (i = 0; i < screenHeight; i++)
        ylookup[i] = i * bufferPitch;
}

/*
=============================================================================

						PALETTE OPS

		To avoid snow, do a WaitVBL BEFORE calling these

=============================================================================
*/

/*
=================
=
= VL_ConvertPalette
=
=================
*/

void VL_ConvertPalette(byte *srcpal, uint16_t *destpal, int numColors)
{
    int i;
    color_t c;
    c.a = 255;

    for(i=0; i<numColors; i++)
    {
        c.r = *srcpal++ * 255 / 63;
        c.g = *srcpal++ * 255 / 63;
        c.b = *srcpal++ * 255 / 63;
        destpal[i] = color_to_packed16(c);
    }
}

/*
=================
=
= VL_FillPalette
=
=================
*/

void VL_FillPalette (int red, int green, int blue)
{
    int i;
    uint16_t pal[256];
    uint16_t c = color_to_packed16(RGBA32(red, green, blue, 255));

    for(i=0; i<256; i++)
    {
        pal[i] = c;
    }

    VL_SetPalette(pal, true);
}

//===========================================================================

/*
=================
=
= VL_SetPalette
=
=================
*/

void VL_SetPalette (const uint16_t *palette, bool forceupdate)
{
    memcpy(UncachedUShortAddr(curpal), palette, sizeof curpal);
    if (forceupdate)
        VW_UpdateScreen();
}


//===========================================================================

/*
=================
=
= VL_GetPalette
=
=================
*/

void VL_GetPalette (uint16_t *palette)
{
    memcpy(palette, UncachedUShortAddr(curpal), sizeof curpal);
}


//===========================================================================

/*
=================
=
= VL_FadeOut
=
= Fades the current palette to the given color in the given number of steps
=
=================
*/

void VL_FadeOut (int start, int end, int red, int green, int blue, int steps)
{
	int		    i,j,orig,delta;
	uint16_t   *origptr, *newptr;
  color_t origc, newc;

    red = red * 255 / 63;
    green = green * 255 / 63;
    blue = blue * 255 / 63;

	VL_WaitVBL(1);
	VL_GetPalette(palette1);
	memcpy(palette2, palette1, sizeof(uint16_t) * 256);

//
// fade through intermediate frames
//
	for (i=0;i<steps;i++)
	{
		origptr = &palette1[start];
		newptr = &palette2[start];
		for (j=start;j<=end;j++)
		{
			origc = color_from_packed16(*origptr);
			newc = color_from_packed16(*newptr);
			orig = origc.r;
			delta = red-orig;
			newc.r = CLAMP(orig + delta * i / steps, 0, 255);
			orig = origc.g;
			delta = green-orig;
			newc.g = CLAMP(orig + delta * i / steps, 0, 255);
			orig = origc.b;
			delta = blue-orig;
			newc.b = CLAMP(orig + delta * i / steps, 0, 255);
			*newptr = color_to_packed16(newc);
			origptr++;
			newptr++;
		}

		VL_WaitVBL(1);
		VL_SetPalette (palette2, true);
	}

//
// final color
//
	VL_FillPalette (red,green,blue);

	screenfaded = true;
}


/*
=================
=
= VL_FadeIn
=
=================
*/

void VL_FadeIn (int start, int end, uint16_t *palette, int steps)
{
	int i,j,delta;
  color_t c, c1, c2;

	VL_WaitVBL(1);
	VL_GetPalette(palette1);
	memcpy(palette2, palette1, sizeof(uint16_t) * 256);

	c2.a = 255;
//
// fade through intermediate frames
//
	for (i=0;i<steps;i++)
	{
		for (j=start;j<=end;j++)
		{
			c = color_from_packed16(palette[j]);
			c1 = color_from_packed16(palette1[j]);
			delta = c.r-(int)c1.r;
			c2.r = CLAMP(c1.r + delta * i / steps, 0, 255);
			delta = c.g-(int)c1.g;
			c2.g = CLAMP(c1.g + delta * i / steps, 0, 255);
			delta = c.b-(int)c1.b;
			c2.b = CLAMP(c1.b + delta * i / steps, 0, 255);
			palette2[j] = color_to_packed16(c2);
		}

		VL_WaitVBL(1);
		VL_SetPalette(palette2, true);
	}

//
// final color
//
	VL_SetPalette (palette, true);
	screenfaded = false;
}

/*
=============================================================================

							PIXEL OPS

=============================================================================
*/

byte *VL_LockSurface(surface_t *surface)
{
  return surface->buffer;
}

void VL_UnlockSurface(surface_t *surface)
{
}

/*
=================
=
= VL_Plot
=
=================
*/

void VL_Plot (int x, int y, int color)
{
    byte *dest;

    assert(x >= 0 && (unsigned) x < screenWidth
            && y >= 0 && (unsigned) y < screenHeight
            && "VL_Plot: Pixel out of bounds!");

    dest = VL_LockSurface(screenBuffer);
    if(dest == NULL) return;

    dest[ylookup[y] + x] = color;

    VL_UnlockSurface(screenBuffer);
}

/*
=================
=
= VL_GetPixel
=
=================
*/

byte VL_GetPixel (int x, int y)
{
    byte col;

    assert_ret(x >= 0 && (unsigned) x < screenWidth
            && y >= 0 && (unsigned) y < screenHeight
            && "VL_GetPixel: Pixel out of bounds!");

    if (!VL_LockSurface(screenBuffer))
        return 0;

    col = ((byte *) screenBuffer->buffer)[ylookup[y] + x];

    VL_UnlockSurface(screenBuffer);

    return col;
}


/*
=================
=
= VL_Hlin
=
=================
*/

void VL_Hlin (unsigned x, unsigned y, unsigned width, int color)
{
    byte *dest;

    assert(x >= 0 && x + width <= screenWidth
            && y >= 0 && y < screenHeight
            && "VL_Hlin: Destination rectangle out of bounds!");

    dest = VL_LockSurface(screenBuffer);
    if(dest == NULL) return;

    memset(dest + ylookup[y] + x, color, width);

    VL_UnlockSurface(screenBuffer);
}


/*
=================
=
= VL_Vlin
=
=================
*/

void VL_Vlin (int x, int y, int height, int color)
{
	byte *dest;

	assert(x >= 0 && (unsigned) x < screenWidth
			&& y >= 0 && (unsigned) y + height <= screenHeight
			&& "VL_Vlin: Destination rectangle out of bounds!");

	dest = VL_LockSurface(screenBuffer);
	if(dest == NULL) return;

	dest += ylookup[y] + x;

	while (height--)
	{
		*dest = color;
		dest += bufferPitch;
	}

	VL_UnlockSurface(screenBuffer);
}


/*
=================
=
= VL_Bar
=
=================
*/

void VL_Bar (int x, int y, int width, int height, int color)
{
    VL_BarScaledCoord(x, y,width, height, color);
}

void VL_BarScaledCoord (int scx, int scy, int scwidth, int scheight, int color)
{
	byte *dest;

	assert(scx >= 0 && (unsigned) scx + scwidth <= screenWidth
			&& scy >= 0 && (unsigned) scy + scheight <= screenHeight
			&& "VL_BarScaledCoord: Destination rectangle out of bounds!");

	dest = VL_LockSurface(screenBuffer);
	if(dest == NULL) return;

	dest += ylookup[scy] + scx;

	while (scheight--)
	{
		memset(dest, color, scwidth);
		dest += bufferPitch;
	}
	VL_UnlockSurface(screenBuffer);
}

/*
============================================================================

							MEMORY OPS

============================================================================
*/


/*
===================
=
= VL_DePlaneVGA
=
= Unweave a VGA graphic to simplify drawing
=
===================
*/

void VL_DePlaneVGA (byte *source, int width, int height)
{
    int  x,y,plane;
    word size,pwidth;
    byte *temp,*dest,*srcline;

    size = width * height;

    if (width & 3)
        Quit ("DePlaneVGA: width not divisible by 4!");

    temp = SafeMalloc(size);

//
// munge pic into the temp buffer
//
    srcline = source;
    pwidth = width >> 2;

    for (plane = 0; plane < 4; plane++)
    {
        dest = temp;

        for (y = 0; y < height; y++)
        {
            for (x = 0; x < pwidth; x++)
                *(dest + (x << 2) + plane) = *srcline++;

            dest += width;
        }
    }

//
// copy the temp buffer back into the original source
//
    memcpy (source,temp,size);

    free (temp);
}


/*
=================
=
= VL_MemToScreenScaledCoord
=
= Draws a block of data to the screen.
=
=================
*/

void VL_MemToScreen (byte *source, int width, int height, int x, int y)
{
    VL_MemToScreenScaledCoord(source, width, height, x, y);
}

void VL_MemToScreenScaledCoord (byte *source, int width, int height, int destx, int desty)
{
    byte *dest;
    int i, j, sci, scj;

    assert(destx >= 0 && destx + width <= screenWidth
            && desty >= 0 && desty + height <= screenHeight
            && "VL_MemToScreenScaledCoord: Destination rectangle out of bounds!");

    dest = VL_LockSurface(screenBuffer);
    if(dest == NULL) return;

    for(j = 0, scj = 0; j < height; j++, scj += 1)
    {
        for(i = 0, sci = 0; i < width; i++, sci += 1)
        {
            byte col = source[(j * width) + i];
            dest[ylookup[scj + desty] + sci + destx] = col;
        }
    }
    VL_UnlockSurface(screenBuffer);
}

/*
=================
=
= VL_MemToScreenScaledCoord
=
= Draws a part of a block of data to the screen.
= The block has the size origwidth*origheight.
= The part at (srcx, srcy) has the size width*height
= and will be painted to (destx, desty).
=
=================
*/

void VL_MemToScreenScaledCoord2 (byte *source, int origwidth, int origheight, int srcx, int srcy,
                                int destx, int desty, int width, int height)
{
    byte *dest;
    int i, j, sci, scj;

    assert(destx >= 0 && destx + width <= screenWidth
            && desty >= 0 && desty + height <= screenHeight
            && "VL_MemToScreenScaledCoord: Destination rectangle out of bounds!");

    dest = VL_LockSurface(screenBuffer);
    if(dest == NULL) return;

    for(j = 0, scj = 0; j < height; j++, scj += 1)
    {
        for(i = 0, sci = 0; i < width; i++, sci += 1)
        {
            byte col = source[((j + srcy) * origwidth) + (i + srcx)];

            dest[ylookup[scj + desty] + sci + destx] = col;
        }
    }
    VL_UnlockSurface(screenBuffer);
}

//==========================================================================

surface_t *VL_GrabScreen(void)
{
    memcpy(screenGrab.buffer, screenBuffer->buffer, screenBuffer->height * screenBuffer->stride);
    return &screenGrab;
}
