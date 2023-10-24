//
//	ID Engine
//	ID_IN.c - Input Manager
//	v1.0d1
//	By Jason Blochowiak
//

//
//	This module handles dealing with the various input devices
//
//	Depends on: Memory Mgr (for demo recording), Sound Mgr (for timing stuff),
//				User Mgr (for command line parms)
//
//	Globals:
//		LastScan - The keyboard scan code of the last key pressed
//		LastASCII - The ASCII value of the last key pressed
//	DEBUG - there are more globals
//

#include "wl_def.h"
/*
=============================================================================

					GLOBAL VARIABLES

=============================================================================
*/

//
// configuration variables
//
volatile boolean	Paused;

/*
=============================================================================

					LOCAL VARIABLES

=============================================================================
*/


static	boolean		IN_Started;

static	Direction	DirTable[] =		// Quick lookup for total direction
{
    dir_NorthWest,	dir_North,	dir_NorthEast,
    dir_West,		dir_None,	dir_East,
    dir_SouthWest,	dir_South,	dir_SouthEast
};

///////////////////////////////////////////////////////////////////////////
//
//	IN_GetJoyDelta() - Returns the relative movement of the specified
//		joystick (from +/-127)
//
///////////////////////////////////////////////////////////////////////////
void IN_GetJoyDelta(int *dx,int *dy)
{
    if(!joypad_is_connected(JOYPAD_PORT_1))
    {
        *dx = *dy = 0;
        return;
    }

    joypad_inputs_t inputs = joypad_get_inputs(JOYPAD_PORT_1);

    int x = inputs.stick_x;
    int y = inputs.stick_y;
    if (abs(x) < 7)
      x = 0;
    if (abs(y) < 7)
      y = 0;
    *dx = CLAMP(x * (stickadjustment + 8) / 32, -127, 127);
    *dy = CLAMP(-y * (stickadjustment + 8) / 32, -127, 127);
}

void IN_WaitAndProcessEvents()
{
    uint16_t prev = joypad_get_buttons(JOYPAD_PORT_1).raw;
    do {
        joypad_poll();
        SD_Poll();
    } while (joypad_get_buttons(JOYPAD_PORT_1).raw == prev);
}

void IN_ProcessEvents()
{
  joypad_poll();
  SD_Poll();
}


///////////////////////////////////////////////////////////////////////////
//
//	IN_Startup() - Starts up the Input Mgr
//
///////////////////////////////////////////////////////////////////////////
void
IN_Startup(void)
{
	if (IN_Started)
		return;

    IN_Started = true;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_Shutdown() - Shuts down the Input Mgr
//
///////////////////////////////////////////////////////////////////////////
void
IN_Shutdown(void)
{
	if (!IN_Started)
		return;

	IN_Started = false;
}



///////////////////////////////////////////////////////////////////////////
//
//	IN_ReadControl() - Reads the device associated with the specified
//		player and fills in the control info struct
//
///////////////////////////////////////////////////////////////////////////
void
IN_ReadControl(int player,ControlInfo *info)
{
  int			dx,dy;
  Motion		mx,my;

  dx = dy = 0;
  mx = my = motion_None;

  IN_ProcessEvents();

  joypad_8way_t dir = joypad_get_direction(JOYPAD_PORT_1, JOYPAD_2D_LH);

  if (dir == JOYPAD_8WAY_UP_LEFT)
    mx = motion_Left,my = motion_Up;
  else if (dir == JOYPAD_8WAY_UP_RIGHT)
    mx = motion_Right,my = motion_Up;
  else if (dir == JOYPAD_8WAY_DOWN_LEFT)
    mx = motion_Left,my = motion_Down;
  else if (dir == JOYPAD_8WAY_DOWN_RIGHT)
    mx = motion_Right,my = motion_Down;
  else if (dir == JOYPAD_8WAY_UP)
    my = motion_Up;
  else if (dir == JOYPAD_8WAY_DOWN)
    my = motion_Down;
  else if (dir == JOYPAD_8WAY_LEFT)
    mx = motion_Left;
  else if (dir == JOYPAD_8WAY_RIGHT)
    mx = motion_Right;

  joypad_buttons_t pad = joypad_get_buttons_pressed(JOYPAD_PORT_1);

  dx = mx * 127;
  dy = my * 127;

  info->x = dx;
  info->xaxis = mx;
  info->y = dy;
  info->yaxis = my;
  info->confirm = pad.a;
  info->cancel = pad.b;
  info->dir = DirTable[((my + 1) * 3) + (mx + 1)];
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_Ack() - waits for a button or key press.  If a button is down, upon
// calling, it must be released for it to be recognized
//
///////////////////////////////////////////////////////////////////////////

void IN_StartAck(void)
{
    IN_ProcessEvents();
}


boolean IN_CheckAck (void)
{
    int i;

    //
    // see if something has been pressed
    //

    uint16_t buttons = joypad_get_buttons_pressed(JOYPAD_PORT_1).raw;

    for(i = 0; i < 16; i++)
    {
        uint16_t bit = 1 << i;
        if(buttons & bit)
        {
            // Wait until button has been released
            do
            {
                IN_WaitAndProcessEvents();
                buttons = joypad_get_buttons(JOYPAD_PORT_1).raw;
            }
            while(buttons & bit);

            return true;
        }
    }

    IN_ProcessEvents();

    return false;
}


void IN_Ack (void)
{
    IN_StartAck ();

    do
    {
        IN_WaitAndProcessEvents();
    }
    while(!IN_CheckAck ());
}


///////////////////////////////////////////////////////////////////////////
//
//	IN_UserInput() - Waits for the specified delay time (in ticks) or the
//		user pressing a key or a mouse button. If the clear flag is set, it
//		then either clears the key or waits for the user to let the mouse
//		button up.
//
///////////////////////////////////////////////////////////////////////////
boolean IN_UserInput(longword delay)
{
	uint64_t	lasttime;

	lasttime = GetTimeCount();
	IN_StartAck ();
	do
	{
		if (IN_CheckAck())
		{
			return true;
		}
	} while (GetTimeCount() - lasttime < delay);
	return(false);
}

//===========================================================================
