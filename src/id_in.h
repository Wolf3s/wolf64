//
//	ID Engine
//	ID_IN.h - Header file for Input Manager
//	v1.0d1
//	By Jason Blochowiak
//

#ifndef	__ID_IN_H_
#define	__ID_IN_H_

#include "n64_main.h"
#include "wl_types.h"

#ifdef	__DEBUG__
#define	__DEBUG_InputMgr__
#endif

typedef	enum		{
						demo_Off,demo_Record,demo_Playback,demo_PlayDone
					} Demo;
typedef	enum		{
						ctrl_Joystick,
						ctrl_Joystick1 = ctrl_Joystick,ctrl_Joystick2,
						ctrl_Mouse
					} ControlType;
typedef	enum		{
						motion_Left = -1,motion_Up = -1,
						motion_None = 0,
						motion_Right = 1,motion_Down = 1
					} Motion;
typedef	enum		{
						dir_North,dir_NorthEast,
						dir_East,dir_SouthEast,
						dir_South,dir_SouthWest,
						dir_West,dir_NorthWest,
						dir_None
					} Direction;
typedef	struct		{
						boolean		confirm, cancel;
						short		x,y;
						Motion		xaxis,yaxis;
						Direction	dir;
					} CursorInfo;
typedef	CursorInfo	ControlInfo;
// Global variables
extern           boolean    MousePresent;
extern  volatile boolean    Paused;


// DEBUG - put names in prototypes
extern	void		IN_Startup(void),IN_Shutdown(void);
extern	void		IN_ReadControl(int,ControlInfo *);
extern	void		IN_StopDemo(void),IN_FreeDemoBuffer(void),
					IN_Ack(void);
extern	boolean		IN_UserInput(longword delay);

void    IN_WaitAndProcessEvents();
void    IN_ProcessEvents();

void    IN_GetJoyDelta(int *dx,int *dy);

void    IN_StartAck(void);
boolean IN_CheckAck (void);

#endif
