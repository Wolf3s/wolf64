#include "wl_def.h"
#include "n64_main.h"


pictabletype	*pictable;

int	    px,py;
byte	fontcolor,backcolor;
int	    fontnumber;

//==========================================================================

void VWB_DrawPropString(const char* string)
{
	fontstruct  *font;
	int		    width, step, height;
	byte	    *source, *dest;
	byte	    ch;
	int i;

	dest = VL_LockSurface(screenBuffer);
	if(dest == NULL) return;

	font = (fontstruct *) grsegs[STARTFONT+fontnumber];
	height = font->height;
	dest += (ylookup[py] + px);

	while ((ch = (byte)*string++)!=0)
	{
		width = step = font->width[ch];
		source = ((byte *)font)+font->location[ch];
		while (width--)
		{
			for(i=0; i<height; i++)
			{
				if(source[i*step])
				{
					dest[ylookup[i]]=fontcolor;
				}
			}

			source++;
			px++;
			dest++;
		}
	}

	VL_UnlockSurface(screenBuffer);
}


void VWL_MeasureString (const char *string, word *width, word *height, fontstruct *font)
{
	*height = font->height;
	for (*width = 0;*string;string++)
		*width += font->width[*((byte *)string)];	// proportional width
}

void VW_MeasurePropString (const char *string, word *width, word *height)
{
	VWL_MeasureString(string,width,height,(fontstruct *)grsegs[STARTFONT+fontnumber]);
}

/*
=============================================================================

				Double buffer management routines

=============================================================================
*/

void VH_UpdateScreen (surface_t *surface)
{
    Present(surface);
}

void VWB_DrawTile8 (int x, int y, int tile)
{
	VL_MemToScreen (grsegs[STARTTILE8]+tile*64,8,8,x,y);
}

void VWB_DrawPic (int x, int y, int chunknum)
{
	int	picnum = chunknum - STARTPICS;
	unsigned width,height;

	x &= ~7;

	width = pictable[picnum].width;
	height = pictable[picnum].height;

	VL_MemToScreen (grsegs[chunknum],width,height,x,y);
}

void VWB_DrawPicScaledCoord (int scx, int scy, int chunknum)
{
	int	picnum = chunknum - STARTPICS;
	unsigned width,height;

	width = pictable[picnum].width;
	height = pictable[picnum].height;

    VL_MemToScreenScaledCoord (grsegs[chunknum],width,height,scx,scy);
}


void VWB_Bar (int x, int y, int width, int height, int color)
{
	VW_Bar (x,y,width,height,color);
}

void VWB_Plot (int x, int y, int color)
{
    VW_Plot(x,y,color);
}

void VWB_Hlin (int x1, int x2, int y, int color)
{
    VW_Hlin(x1,x2,y,color);
}

void VWB_Vlin (int y1, int y2, int x, int color)
{
    VW_Vlin(y1,y2,x,color);
}


/*
=============================================================================

						WOLFENSTEIN STUFF

=============================================================================
*/


/*
===================
=
= FizzleFade
=
= returns true if aborted
=
= It uses maximum-length Linear Feedback Shift Registers (LFSR) counters.
= You can find a list of them with lengths from 3 to 168 at:
= http://www.xilinx.com/support/documentation/application_notes/xapp052.pdf
= Many thanks to Xilinx for this list!!!
=
===================
*/

// XOR masks for the pseudo-random number sequence starting with n=17 bits
static const uint32_t rndmasks[] = {
                    // n    XNOR from (starting at 1, not 0 as usual)
    0x00012000,     // 17   17,14
    0x00020400,     // 18   18,11
    0x00040023,     // 19   19,6,2,1
    0x00090000,     // 20   20,17
    0x00140000,     // 21   21,19
    0x00300000,     // 22   22,21
    0x00420000,     // 23   23,18
    0x00e10000,     // 24   24,23,22,17
    0x01200000,     // 25   25,22      (this is enough for 8191x4095)
};

static uint32_t rndbits_y;
static uint32_t rndmask;

// Returns the number of bits needed to represent the given value
static int log2_ceil(uint32_t x)
{
    int n = 0;
    uint32_t v = 1;
    while(v < x)
    {
        n++;
        v <<= 1;
    }
    return n;
}

void VH_Startup()
{
    int rndbits_x = log2_ceil(screenWidth);
    rndbits_y = log2_ceil(screenHeight);

    int rndbits = rndbits_x + rndbits_y;
    if(rndbits < 17)
        rndbits = 17;       // no problem, just a bit slower
    else if(rndbits > 25)
        rndbits = 25;       // fizzle fade will not fill whole screen

    rndmask = rndmasks[rndbits - 17];
}

boolean FizzleFade (surface_t *source, surface_t *target, int x1, int y1,
    unsigned width, unsigned height, unsigned frames, boolean abortable)
{
    unsigned x, y, p, pixperframe;
    uint64_t frame;
    uint32_t  rndval, lastrndval;

    rndval = lastrndval = 0;
    pixperframe = width * height / frames;

    IN_StartAck ();

    frame = GetTimeCount();
    byte *srcptr = VL_LockSurface(source);
    if(srcptr == NULL) return false;

    do
    {
        IN_ProcessEvents();

        if(abortable && IN_CheckAck ())
        {
            VL_UnlockSurface(source);
            VH_UpdateScreen (source);
            return true;
        }

        byte *destptr = target ? target->buffer : NULL;

        if(destptr != NULL)
        {
            rndval = lastrndval;

            for(p = 0; p < pixperframe; p++)
            {
                //
                // seperate random value into x/y pair
                //

                x = rndval >> rndbits_y;
                y = rndval & ((1 << rndbits_y) - 1);

                //
                // advance to next random element
                //

                rndval = (rndval >> 1) ^ (rndval & 1 ? 0 : rndmask);

                if(x >= width || y >= height)
                {
                    if(rndval == 0)     // entire sequence has been completed
                        goto finished;
                    p--;
                    continue;
                }

                //
                // copy one pixel
                //

                *(destptr + (y1 + y) * screenBuffer->stride + x1 + x)
                    = *(srcptr + (y1 + y) * screenBuffer->stride + x1 + x);

                if(rndval == 0)		// entire sequence has been completed
                    goto finished;
            }

            lastrndval = rndval;

            Present(target);
        }
        else
        {
            for(p = 0; p < pixperframe; p++)
            {
                rndval = (rndval >> 1) ^ (rndval & 1 ? 0 : rndmask);
                if(rndval == 0)
                    goto finished;
            }
        }

        frame++;
        Delay(frame - GetTimeCount());
    } while (1);

finished:
    VL_UnlockSurface(source);
    VH_UpdateScreen (source);
    return false;
}
