////////////////////////////////////////////////////////////////////
//
// WL_MENU.C
// by John Romero (C) 1992 Id Software, Inc.
//
////////////////////////////////////////////////////////////////////

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "n64_main.h"
#include "wl_def.h"

extern int lastgamemusicoffset;

//
// PRIVATE PROTOTYPES
//
int  CP_ReadThis (int);
void SetTextColor (CP_itemtype *items, int hlight);

#ifdef SPEAR
#define STARTITEM       newgame

#else
#ifdef GOODTIMES
#define STARTITEM       newgame

#else
#define STARTITEM       readthis
#endif
#endif

// ENDSTRx constants are defined in foreign.h
char endStrings[9][80] = {
    ENDSTR1,
    ENDSTR2,
    ENDSTR3,
    ENDSTR4,
    ENDSTR5,
    ENDSTR6,
    ENDSTR7,
    ENDSTR8,
    ENDSTR9
};

CP_itemtype MainMenu[] = {
#ifdef JAPAN
    {1, "", CP_NewGame},
    {1, "", CP_Sound},
    {1, "", CP_Control},
    {1, "", NULL},
    {0, "", NULL},
    {1, "", CP_ChangeView},
    {1, "", CP_Cheats},
    {2, "", CP_ReadThis},
    {1, "", CP_ViewScores},
    {1, "", 0}
#else

    {1, STR_NG, CP_NewGame},
    {1, STR_SD, CP_Sound},
    {1, STR_CL, CP_Control},
    {1, STR_LG, NULL},
    {0, STR_SG, NULL},
    {1, STR_CV, CP_ChangeView},
    {1, STR_CH, CP_Cheats},

#ifndef GOODTIMES
#ifndef SPEAR

#ifdef SPANISH
    {2, "Ve esto!", CP_ReadThis},
#else
    {2, "Read This!", CP_ReadThis},
#endif

#endif
#endif

    {1, STR_VS, CP_ViewScores},
    {1, STR_BD, 0}
#endif
};

CP_itemtype SndMenu[] = {
#ifdef JAPAN
    {1, "", 0},
    {1, "", 0},
    {1, "", 0},
    {0, "", 0},
    {0, "", 0},
    {1, "", 0},
    {1, "", 0},
    {1, "", 0},
    {0, "", 0},
    {0, "", 0},
    {1, "", 0},
    {1, "", 0},
#else
    {1, STR_NONE, 0},
    {1, STR_PC, 0},
    {1, STR_ALSB, 0},
    {0, "", 0},
    {0, "", 0},
    {1, STR_NONE, 0},
    {0, STR_DISNEY, 0},
    {1, STR_SB, 0},
    {0, "", 0},
    {0, "", 0},
    {1, STR_NONE, 0},
    {1, STR_ALSB, 0}
#endif
};

enum { CTL_STICKSENS, CTL_EMPTY1, CTL_MOUSESENS, CTL_EMPTY2, CTL_STICKMODE, CTL_ALWAYSRUN };

CP_itemtype CtlMenu[] = {
    {4, STR_STSENS, 0},
    {0, "", 0},
    {4, STR_SENS, 0},
    {0, "", 0},
    {1, STR_STMODE, 0},
    {1, STR_ALRUN, 0}
};

enum { CHT_GODMODE, CHT_NOCLIP, CHT_MAPREVEAL, CHT_BOOST, CHT_SHOWFPS };

CP_itemtype CheatMenu[] = {
    {1, "God Mode", 0},
    {1, "No Clipping", 0},
    {1, "Reveal Map", 0},
    {0, "Give Boost", 0},
    {1, "Show FPS", 0}
};

#ifndef SPEAR
CP_itemtype NewEmenu[] = {
#ifdef JAPAN
#ifdef JAPDEMO
    {1, "", 0},
    {0, "", 0},
    {0, "", 0},
    {0, "", 0},
    {0, "", 0},
    {0, "", 0},
    {0, "", 0},
    {0, "", 0},
    {0, "", 0},
    {0, "", 0},
    {0, "", 0},
    {0, "", 0},
#else
    {1, "", 0},
    {0, "", 0},
    {1, "", 0},
    {0, "", 0},
    {1, "", 0},
    {0, "", 0},
    {1, "", 0},
    {0, "", 0},
    {1, "", 0},
    {0, "", 0},
    {1, "", 0},
    {0, "", 0}
#endif
#else
#ifdef SPANISH
    {1, "Episodio 1\n" "Fuga desde Wolfenstein", 0},
    {0, "", 0},
    {3, "Episodio 2\n" "Operacion Eisenfaust", 0},
    {0, "", 0},
    {3, "Episodio 3\n" "Muere, Fuhrer, Muere!", 0},
    {0, "", 0},
    {3, "Episodio 4\n" "Un Negro Secreto", 0},
    {0, "", 0},
    {3, "Episodio 5\n" "Huellas del Loco", 0},
    {0, "", 0},
    {3, "Episodio 6\n" "Confrontacion", 0}
#else
    {1, "Episode 1\n" "Escape from Wolfenstein", 0},
    {0, "", 0},
    {3, "Episode 2\n" "Operation: Eisenfaust", 0},
    {0, "", 0},
    {3, "Episode 3\n" "Die, Fuhrer, Die!", 0},
    {0, "", 0},
    {3, "Episode 4\n" "A Dark Secret", 0},
    {0, "", 0},
    {3, "Episode 5\n" "Trail of the Madman", 0},
    {0, "", 0},
    {3, "Episode 6\n" "Confrontation", 0}
#endif
#endif
};
#endif


CP_itemtype NewMenu[] = {
#ifdef JAPAN
    {1, "", 0},
    {1, "", 0},
    {1, "", 0},
    {1, "", 0}
#else
    {1, STR_DADDY, 0},
    {1, STR_HURTME, 0},
    {1, STR_BRINGEM, 0},
    {1, STR_DEATH, 0}
#endif
};

CP_itemtype LSMenu[] = {
    {1, "", 0}
};

CP_itemtype CusMenu[] = {
    {1, "", 0},
    {0, "", 0},
    {0, "", 0},
    {1, "", 0},
    {0, "", 0},
    {0, "", 0},
    {1, "", 0},
    {0, "", 0},
    {1, "", 0}
};

// CP_iteminfo struct format: short x, y, amount, curpos, indent;
CP_iteminfo MainItems = { MENU_X, MENU_Y, lengthof(MainMenu), STARTITEM, 24 },
            SndItems  = { SM_X, SM_Y1, lengthof(SndMenu), 0, 52 },
            LSItems   = { LSM_X, LSM_Y, lengthof(LSMenu), 0, 24 },
            CtlItems  = { CTL_X, CTL_Y, lengthof(CtlMenu), -1, 24 },
            CusItems  = { 8, CST_Y + 13 * 2, lengthof(CusMenu), -1, 0},
            CheatItems  = { CHT_X, CHT_Y, lengthof(CheatMenu), -1, 56 },
#ifndef SPEAR
            NewEitems = { NE_X, NE_Y, lengthof(NewEmenu), 0, 88 },
#endif
            NewItems  = { NM_X, NM_Y, lengthof(NewMenu), 2, 24 };

int color_hlite[] = {
    DEACTIVE,
    HIGHLIGHT,
    READHCOLOR,
    0x67,
    HIGHLIGHT,
};

int color_norml[] = {
    DEACTIVE,
    TEXTCOLOR,
    READCOLOR,
    0x6b,
    TEXTCOLOR,
};

int EpisodeSelect[6] = { 1 };


static bool SaveGameAvail = false;
static int StartGame;
static int SoundStatus = 1;
static int pickquick;


////////////////////////////////////////////////////////////////////
//
// Wolfenstein Control Panel!  Ta Da!
//
////////////////////////////////////////////////////////////////////
void
US_ControlPanel ()
{
    int which;

    if (ingame)
    {
        if (CP_CheckQuick ())
            return;
        lastgamemusicoffset = StartCPMusic (MENUSONG);
    }
    else
        StartCPMusic (MENUSONG);
    SetupControlPanel ();

    DrawMainMenu ();
    MenuFadeIn ();
    StartGame = 0;

    //
    // MAIN MENU LOOP
    //
    do
    {
        which = HandleMenu (&MainItems, &MainMenu[0], NULL);

#ifdef SPEAR
#ifndef SPEARDEMO
        IN_ProcessEvents();

        joypad_buttons_t buttons = joypad_get_buttons(JOYPAD_PORT_1);
        //
        // EASTER EGG FOR SPEAR OF DESTINY!
        //
        if (buttons.l && buttons.r)
        {
            VW_FadeOut ();
            StartCPMusic (XJAZNAZI_MUS);
            ClearMemory ();


            VWB_DrawPic (0, 0, IDGUYS1PIC);
            VWB_DrawPic (0, 80, IDGUYS2PIC);

            VW_UpdateScreen ();

            uint16_t pal[256];
            VL_ConvertPalette(grsegs[IDGUYSPALETTE], pal, 256);
            VL_FadeIn (0, 255, pal, 30);

            while (buttons.l || buttons.r)
            {
                IN_ProcessEvents();
                buttons = joypad_get_buttons(JOYPAD_PORT_1);
            }
            IN_Ack ();

            VW_FadeOut ();

            DrawMainMenu ();
            StartCPMusic (MENUSONG);
            MenuFadeIn ();
        }
#endif
#endif

        switch (which)
        {
            case loadgame:
                CP_LoadGame();
                break;
            case savegame:
                CP_SaveGame();
                break;
            case viewscores:
                if (MainMenu[viewscores].routine == NULL)
                {
                    if (CP_EndGame (0))
                        StartGame = 1;
                }
                else
                {
                    DrawMainMenu();
                    MenuFadeIn ();
                }
                break;

            case -1:
            case backtodemo:
                StartGame = 1;
                if (!ingame)
                {
                    StartCPMusic (INTROSONG);
                    VL_FadeOut (0, 255, 0, 0, 0, 10);
                }
                break;

            default:
                if (!StartGame)
                {
                    DrawMainMenu ();
                    MenuFadeIn ();
                }
        }

        //
        // "EXIT OPTIONS" OR "NEW GAME" EXITS
        //
    }
    while (!StartGame);

    //
    // DEALLOCATE EVERYTHING
    //
    CleanupControlPanel ();

    //
    // CHANGE MAINMENU ITEM
    //
    if (startgame || loadedgame)
        EnableEndGameMenuItem();
}

void EnableEndGameMenuItem()
{
    MainMenu[viewscores].routine = NULL;
#ifndef JAPAN
    strcpy (MainMenu[viewscores].string, STR_EG);
#endif
}

////////////////////////
//
// DRAW MAIN MENU SCREEN
//
void
DrawMainMenu (void)
{
#ifdef JAPAN
    VWB_DrawPic (0,0,S_OPTIONSPIC);
#else
    ClearMScreen ();

    VWB_DrawPic (112, 184, C_MOUSELBACKPIC);
    DrawStripes (10);
    VWB_DrawPic (84, 0, C_OPTIONSPIC);

#ifdef SPANISH
    DrawWindow (MENU_X - 8, MENU_Y - 3, MENU_W + 8, MENU_H, BKGDCOLOR);
#else
    DrawWindow (MENU_X - 8, MENU_Y - 3, MENU_W, MENU_H, BKGDCOLOR);
#endif
#endif

    //
    // CHANGE "GAME" AND "DEMO"
    //
    if (ingame)
    {
#ifndef JAPAN

#ifdef SPANISH
        strcpy (&MainMenu[backtodemo].string, STR_GAME);
#else
        strcpy (&MainMenu[backtodemo].string[8], STR_GAME);
#endif

#else
        VWB_DrawPic (12 * 8, 20 * 9, C_MRETGAMEPIC);
        VWB_DrawPic (12 * 8, 18 * 9, C_MENDGAMEPIC);
#endif
        MainMenu[backtodemo].active = 2;
    }
    else
    {
#ifndef JAPAN
#ifdef SPANISH
        strcpy (&MainMenu[backtodemo].string, STR_BD);
#else
        strcpy (&MainMenu[backtodemo].string[8], STR_DEMO);
#endif
#else
        VWB_DrawPic (12 * 8, 20 * 9, C_MRETDEMOPIC);
        VWB_DrawPic (12 * 8, 18 * 9, C_MSCORESPIC);
#endif
        MainMenu[backtodemo].active = 1;
    }

    DrawMenu (&MainItems, &MainMenu[0]);
    VW_UpdateScreen ();
}

#ifndef GOODTIMES
#ifndef SPEAR
////////////////////////////////////////////////////////////////////
//
// READ THIS!
//
////////////////////////////////////////////////////////////////////
int
CP_ReadThis (int blank)
{
    StartCPMusic (CORNER_MUS);
    HelpScreens ();
    StartCPMusic (MENUSONG);
    return true;
}
#endif
#endif


////////////////////////////////////////////////////////////////////
//
// CHECK QUICK-KEYS & QUIT (WHILE IN A GAME)
//
////////////////////////////////////////////////////////////////////
int
CP_CheckQuick (void)
{
#if 0
    switch (scancode)
    {
        //
        // END GAME
        //
        case sc_F7:
            WindowH = 160;
#ifdef JAPAN
            if (GetYorN (7, 8, C_JAPQUITPIC))
#else
            if (Confirm (ENDGAMESTR))
#endif
            {
                playstate = ex_died;
                killerobj = NULL;
                pickquick = gamestate.lives = 0;
            }

            WindowH = 200;
            fontnumber = 0;
            MainMenu[savegame].active = 0;
            return 1;

        //
        // QUICKSAVE
        //
        case sc_F8:
            if (SaveGamesAvail[LSItems.curpos] && pickquick)
            {
                fontnumber = 1;
                Message (STR_SAVING "...");
                CP_SaveGame (1);
                fontnumber = 0;
            }
            else
            {
                VW_FadeOut ();
                if(screenHeight % 200 != 0)
                    VL_ClearScreen(0);

                lastgamemusicoffset = StartCPMusic (MENUSONG);
                pickquick = CP_SaveGame (0);

                SETFONTCOLOR (0, 15);
                VW_FadeOut();
                if(viewsize != 21)
                    DrawPlayScreen ();

                if (!startgame && !loadedgame)
                    ContinueMusic (lastgamemusicoffset);

                if (loadedgame)
                    playstate = ex_abort;
                lasttimecount = GetTimeCount ();
            }
            return 1;

        //
        // QUICKLOAD
        //
        case sc_F9:
            if (SaveGamesAvail[LSItems.curpos] && pickquick)
            {
                char string[100] = STR_LGC;

                fontnumber = 1;

                strcat (string, SaveGameNames[LSItems.curpos]);
                strcat (string, "\"?");

                if (Confirm (string))
                    CP_LoadGame (1);

                fontnumber = 0;
            }
            else
            {
                VW_FadeOut ();
                if(screenHeight % 200 != 0)
                    VL_ClearScreen(0);

                lastgamemusicoffset = StartCPMusic (MENUSONG);
                pickquick = CP_LoadGame (0);    // loads lastgamemusicoffs

                SETFONTCOLOR (0, 15);
                VW_FadeOut();
                if(viewsize != 21)
                    DrawPlayScreen ();

                if (!startgame && !loadedgame)
                    ContinueMusic (lastgamemusicoffset);

                if (loadedgame)
                    playstate = ex_abort;

                lasttimecount = GetTimeCount ();
            }
            return 1;

        //
        // QUIT
        //
        case sc_F10:
            WindowX = WindowY = 0;
            WindowW = 320;
            WindowH = 160;
#ifdef JAPAN
            if (GetYorN (7, 8, C_QUITMSGPIC))
#else
#ifdef SPANISH
            if (Confirm (ENDGAMESTR))
#else
            if (Confirm (endStrings[US_RndT () & 0x7 + (US_RndT () & 1)]))
#endif
#endif
            {
                VW_UpdateScreen ();
                SD_MusicOff ();
                SD_StopSound ();
                MenuFadeOut ();

                Quit (NULL);
            }

            DrawPlayBorder ();
            WindowH = 200;
            fontnumber = 0;
            return 1;
    }

#endif
    return 0;
}


////////////////////////////////////////////////////////////////////
//
// END THE CURRENT GAME
//
////////////////////////////////////////////////////////////////////
int
CP_EndGame (int blank)
{
    int res;
#ifdef JAPAN
    res = GetYorN (7, 8, C_JAPQUITPIC);
#else
    res = Confirm (ENDGAMESTR);
#endif
    DrawMainMenu();
    if(!res) return 0;

    pickquick = gamestate.lives = 0;
    playstate = ex_died;
    killerobj = NULL;

    MainMenu[savegame].active = 0;
    MainMenu[viewscores].routine = CP_ViewScores;
#ifndef JAPAN
    strcpy (MainMenu[viewscores].string, STR_VS);
#endif

    return 1;
}


////////////////////////////////////////////////////////////////////
//
// VIEW THE HIGH SCORES
//
////////////////////////////////////////////////////////////////////
int
CP_ViewScores (int blank)
{
    fontnumber = 0;

#ifdef SPEAR
    StartCPMusic (XAWARD_MUS);
#else
    StartCPMusic (ROSTER_MUS);
#endif

    DrawHighScores ();
    VW_UpdateScreen ();
    MenuFadeIn ();
    fontnumber = 1;

    IN_Ack ();

    StartCPMusic (MENUSONG);
    MenuFadeOut ();

    return 0;
}


////////////////////////////////////////////////////////////////////
//
// START A NEW GAME
//
////////////////////////////////////////////////////////////////////
int
CP_NewGame (int blank)
{
    int which, episode;

#ifndef SPEAR
  firstpart:

    DrawNewEpisode ();
    do
    {
        which = HandleMenu (&NewEitems, &NewEmenu[0], NULL);
        switch (which)
        {
            case -1:
                MenuFadeOut ();
                return 0;

            default:
                if (!EpisodeSelect[which / 2])
                {
                    SD_PlaySound (NOWAYSND);
                    Message ("Please select \"Read This!\"\n"
                             "from the Options menu to\n"
                             "find out how to order this\n" "episode from Apogee.");
                    IN_Ack ();
                    DrawNewEpisode ();
                    which = 0;
                }
                else
                {
                    episode = which / 2;
                    which = 1;
                }
                break;
        }

    }
    while (!which);

    ShootSnd ();

    //
    // ALREADY IN A GAME?
    //
    if (ingame)
#ifdef JAPAN
        if (!GetYorN (7, 8, C_JAPNEWGAMEPIC))
#else
        if (!Confirm (CURGAME))
#endif
        {
            MenuFadeOut ();
            return 0;
        }

    MenuFadeOut ();

#else
    episode = 0;

    //
    // ALREADY IN A GAME?
    //
    DrawNewGame ();
    if (ingame)
        if (!Confirm (CURGAME))
        {
            MenuFadeOut ();

            return 0;
        }

#endif

    DrawNewGame ();
    which = HandleMenu (&NewItems, &NewMenu[0], DrawNewGameDiff);
    if (which < 0)
    {
        MenuFadeOut ();
#ifndef SPEAR
        goto firstpart;
#else
        return 0;
#endif
    }

    ShootSnd ();
    NewGame (which, episode);
    StartGame = 1;
    MenuFadeOut ();

    //
    // CHANGE "READ THIS!" TO NORMAL COLOR
    //
#ifndef SPEAR
#ifndef GOODTIMES
    MainMenu[readthis].active = 1;
#endif
#endif

    pickquick = 0;

    return 0;
}


#ifndef SPEAR
/////////////////////
//
// DRAW NEW EPISODE MENU
//
void
DrawNewEpisode (void)
{
    int i;

#ifdef JAPAN
    VWB_DrawPic (0,0,S_EPISODEPIC);
#else
    ClearMScreen ();
    VWB_DrawPic (112, 184, C_MOUSELBACKPIC);

    DrawWindow (NE_X - 4, NE_Y - 4, NE_W + 8, NE_H + 8, BKGDCOLOR);
    SETFONTCOLOR (READHCOLOR, BKGDCOLOR);
    PrintY = 2;
    WindowX = 0;
#ifdef SPANISH
    US_CPrint ("Cual episodio jugar?");
#else
    US_CPrint ("Which episode to play?");
#endif
#endif

    SETFONTCOLOR (TEXTCOLOR, BKGDCOLOR);
    DrawMenu (&NewEitems, &NewEmenu[0]);

    for (i = 0; i < 6; i++)
        VWB_DrawPic (NE_X + 32, NE_Y + i * 26, C_EPISODE1PIC + i);

    VW_UpdateScreen ();
    MenuFadeIn ();
    WaitKeyUp ();
}
#endif

/////////////////////
//
// DRAW NEW GAME MENU
//
void
DrawNewGame (void)
{
#ifdef JAPAN
    VWB_DrawPic (0,0,S_SKILLPIC);
#else
    ClearMScreen ();
    VWB_DrawPic (112, 184, C_MOUSELBACKPIC);

    SETFONTCOLOR (READHCOLOR, BKGDCOLOR);
    PrintX = NM_X + 20;
    PrintY = NM_Y - 32;

#ifndef SPEAR
#ifdef SPANISH
    US_Print ("Eres macho?");
#else
    US_Print ("How tough are you?");
#endif
#else
    VWB_DrawPic (PrintX, PrintY, C_HOWTOUGHPIC);
#endif

    DrawWindow (NM_X - 5, NM_Y - 10, NM_W, NM_H, BKGDCOLOR);
#endif

    DrawMenu (&NewItems, &NewMenu[0]);
    DrawNewGameDiff (NewItems.curpos);
    VW_UpdateScreen ();
    MenuFadeIn ();
    WaitKeyUp ();
}


////////////////////////
//
// DRAW NEW GAME GRAPHIC
//
void
DrawNewGameDiff (int w)
{
    VWB_DrawPic (NM_X + 185, NM_Y + 7, w + C_BABYMODEPIC);
}


////////////////////////////////////////////////////////////////////
//
// HANDLE SOUND MENU
//
////////////////////////////////////////////////////////////////////
int
CP_Sound (int blank)
{
    int which;

    DrawSoundMenu ();
    MenuFadeIn ();
    WaitKeyUp ();

    do
    {
        which = HandleMenu (&SndItems, &SndMenu[0], NULL);
        //
        // HANDLE MENU CHOICES
        //
        switch (which)
        {
                //
                // SOUND EFFECTS
                //
            case 0:
                if (SoundMode != sdm_Off)
                {
                    SD_WaitSoundDone ();
                    SD_SetSoundMode (sdm_Off);
                    DrawSoundMenu ();
                    WriteConfig();
                }
                break;
            case 1:
                if (SoundMode != sdm_PC)
                {
                    SD_WaitSoundDone ();
                    SD_SetSoundMode (sdm_PC);
                    CA_LoadAllSounds ();
                    DrawSoundMenu ();
                    ShootSnd ();
                    WriteConfig();
                }
                break;
            case 2:
                if (SoundMode != sdm_AdLib)
                {
                    SD_WaitSoundDone ();
                    SD_SetSoundMode (sdm_AdLib);
                    CA_LoadAllSounds ();
                    DrawSoundMenu ();
                    ShootSnd ();
                    WriteConfig();
                }
                break;

                //
                // DIGITIZED SOUND
                //
            case 5:
                if (DigiMode != sds_Off)
                {
                    SD_SetDigiDevice (sds_Off);
                    DrawSoundMenu ();
                    WriteConfig();
                }
                break;
            case 6:
/*                if (DigiMode != sds_SoundSource)
                {
                    SD_SetDigiDevice (sds_SoundSource);
                    DrawSoundMenu ();
                    ShootSnd ();
                    WriteConfig();
                }*/
                break;
            case 7:
                if (DigiMode != sds_SoundBlaster)
                {
                    SD_SetDigiDevice (sds_SoundBlaster);
                    DrawSoundMenu ();
                    ShootSnd ();
                    WriteConfig();
                }
                break;

                //
                // MUSIC
                //
            case 10:
                if (MusicMode != smm_Off)
                {
                    SD_SetMusicMode (smm_Off);
                    DrawSoundMenu ();
                    ShootSnd ();
                    WriteConfig();
                }
                break;
            case 11:
                if (MusicMode != smm_AdLib)
                {
                    SD_SetMusicMode (smm_AdLib);
                    DrawSoundMenu ();
                    ShootSnd ();
                    StartCPMusic (MENUSONG);
                    WriteConfig();
                }
                break;
        }
    }
    while (which >= 0);

    MenuFadeOut ();

    return 0;
}


//////////////////////
//
// DRAW THE SOUND MENU
//
void
DrawSoundMenu (void)
{
    int i, on;


#ifdef JAPAN
    VWB_DrawPic (0,0,S_SOUNDPIC);
#else
    //
    // DRAW SOUND MENU
    //
    ClearMScreen ();
    VWB_DrawPic (112, 184, C_MOUSELBACKPIC);

    DrawWindow (SM_X - 8, SM_Y1 - 3, SM_W, SM_H1, BKGDCOLOR);
    DrawWindow (SM_X - 8, SM_Y2 - 3, SM_W, SM_H2, BKGDCOLOR);
    DrawWindow (SM_X - 8, SM_Y3 - 3, SM_W, SM_H3, BKGDCOLOR);
#endif

    DrawMenu (&SndItems, &SndMenu[0]);
#ifndef JAPAN
    VWB_DrawPic (100, SM_Y1 - 20, C_FXTITLEPIC);
    VWB_DrawPic (100, SM_Y2 - 20, C_DIGITITLEPIC);
    VWB_DrawPic (100, SM_Y3 - 20, C_MUSICTITLEPIC);
#endif

    for (i = 0; i < SndItems.amount; i++)
#ifdef JAPAN
        if (i != 3 && i != 4 && i != 8 && i != 9)
#else
        if (SndMenu[i].string[0])
#endif
        {
            //
            // DRAW SELECTED/NOT SELECTED GRAPHIC BUTTONS
            //
            on = 0;
            switch (i)
            {
                    //
                    // SOUND EFFECTS
                    //
                case 0:
                    if (SoundMode == sdm_Off)
                        on = 1;
                    break;
                case 1:
                    if (SoundMode == sdm_PC)
                        on = 1;
                    break;
                case 2:
                    if (SoundMode == sdm_AdLib)
                        on = 1;
                    break;

                    //
                    // DIGITIZED SOUND
                    //
                case 5:
                    if (DigiMode == sds_Off)
                        on = 1;
                    break;
                case 6:
//                    if (DigiMode == sds_SoundSource)
//                        on = 1;
                    break;
                case 7:
                    if (DigiMode == sds_SoundBlaster)
                        on = 1;
                    break;

                    //
                    // MUSIC
                    //
                case 10:
                    if (MusicMode == smm_Off)
                        on = 1;
                    break;
                case 11:
                    if (MusicMode == smm_AdLib)
                        on = 1;
                    break;
            }

            if (on)
                VWB_DrawPic (SM_X + 24, SM_Y1 + i * 13 + 2, C_SELECTEDPIC);
            else
                VWB_DrawPic (SM_X + 24, SM_Y1 + i * 13 + 2, C_NOTSELECTEDPIC);
        }

    DrawMenuGun (&SndItems);
    VW_UpdateScreen ();
}


//
// DRAW LOAD/SAVE IN PROGRESS
//
void
DrawLSAction (int which)
{
#define LSA_X   96
#define LSA_Y   80
#define LSA_W   130
#define LSA_H   42

    DrawWindow (LSA_X, LSA_Y, LSA_W, LSA_H, TEXTCOLOR);
    DrawOutline (LSA_X, LSA_Y, LSA_W, LSA_H, 0, HIGHLIGHT);
    VWB_DrawPic (LSA_X + 8, LSA_Y + 5, C_DISKLOADING1PIC);

    fontnumber = 1;
    SETFONTCOLOR (0, TEXTCOLOR);
    PrintX = LSA_X + 46;
    PrintY = LSA_Y + 13;

    if (!which)
        US_Print (STR_LOADING "...");
    else
        US_Print (STR_SAVING "...");

    VW_UpdateScreen ();
}

static const char savemagic[32] = ROMNAME " save game";

////////////////////////////////////////////////////////////////////
//
// LOAD SAVED GAMES
//
////////////////////////////////////////////////////////////////////
int
CP_LoadGame (void)
{
    FILE *file;
    char magic[32];

    ShootSnd ();
    DrawLSAction (0);
    file = N64_ReadSave();
    fread(magic, 1, sizeof magic, file);
    if (strncmp(magic, savemagic, 32) == 0)
    {
        loadedgame = true;

        if (LoadTheGame (file, LSA_X + 8, LSA_Y + 5))
        {
            StartGame = 1;
            ShootSnd ();
        }
    }
    fclose (file);

    MainItems.curpos = backtodemo;

    if (!StartGame)
    {
        file = N64_WriteSave();
        memset(magic, 0, sizeof magic);
        fwrite(magic, 0, sizeof magic, file);
        fclose(file);
        SaveGameAvail = false;
        MainMenu[loadgame].active = false;
        DrawMainMenu();
        Message("Save Game Corrupted");

        IN_Ack();
        DrawMainMenu();
    }
    return 1;
}


////////////////////////////////////////////////////////////////////
//
// SAVE CURRENT GAME
//
////////////////////////////////////////////////////////////////////
int
CP_SaveGame (void)
{
    FILE *file;
    ShootSnd ();

    file = N64_WriteSave();
    fwrite (savemagic, 32, 1, file);
    fseek (file, 32, SEEK_SET);

    DrawLSAction (0);
    SaveTheGame (file, LSA_X + 8, LSA_Y + 5);

    fclose (file);

    ShootSnd ();
    SaveGameAvail = true;
    MainMenu[loadgame].active = true;
    MainMenu[savegame].active = false;
    strcpy(MainMenu[savegame].string, STR_SAVED);
    MainItems.curpos = backtodemo;
    DrawMainMenu();
    return 1;
}

int
CP_Cheats (int blank)
{
    int which;

    DrawCheatScreen ();
    MenuFadeIn ();
    WaitKeyUp ();

    do
    {
        which = HandleMenu (&CheatItems, CheatMenu, NULL);
        switch (which)
        {
            case CHT_GODMODE:
                godmode ^= 2;
                DrawCheatScreen ();
                ShootSnd ();
                break;
            case CHT_NOCLIP:
                noclip ^= 1;
                DrawCheatScreen ();
                ShootSnd ();
                break;
            case CHT_MAPREVEAL:
                mapreveal ^= 1;
                DrawCheatScreen ();
                ShootSnd ();
                break;
            case CHT_BOOST:
                if (ingame)
                {
                    gamestate.health = 100;
                    gamestate.ammo = 99;
                    gamestate.keys = 3;
                    GiveWeapon (wp_machinegun);
                    GiveWeapon (wp_chaingun);
                    DrawCheatScreen ();
                    ShootSnd ();
                }
                break;
            case CHT_SHOWFPS:
                fpscounter ^= 1;
                DrawCheatScreen ();
                ShootSnd ();
                break;
        }
    }
    while (which >= 0);

    MenuFadeOut ();

    return 0;
}

////////////////////////////////////////////////////////////////////
//
// DEFINE CONTROLS
//
////////////////////////////////////////////////////////////////////
int
CP_Control (int blank)
{
    int which;

    DrawCtlScreen ();
    MenuFadeIn ();

    do
    {
        which = HandleMenu (&CtlItems, CtlMenu, NULL);
        if (which == CTL_ALWAYSRUN)
        {
            autorun ^= 1;
            DrawCtlScreen ();
            ShootSnd ();
            WriteConfig();
        }
        else if (which == CTL_STICKMODE)
        {
            MoveMode ^= 1;
            DrawCtlScreen ();
            ShootSnd ();
            WriteConfig();
        }
        else if (CtlItems.curpos == CTL_MOUSESENS)
        {
            joypad_8way_t dir = joypad_get_direction(JOYPAD_PORT_1, JOYPAD_2D_LH);
            switch (dir)
            {
                case JOYPAD_8WAY_LEFT:
                    if (mouseadjustment)
                    {
                        mouseadjustment--;
                        SD_PlaySound (MOVEGUN1SND);
                        DrawCtlScreen ();
                        WriteConfig();
                        TicDelay(20);
                    }
                    break;

                case JOYPAD_8WAY_RIGHT:
                    if (mouseadjustment < 9)
                    {
                        mouseadjustment++;
                        SD_PlaySound (MOVEGUN1SND);
                        DrawCtlScreen ();
                        WriteConfig();
                        TicDelay(20);
                    }
                    break;
                default:
                    break;
            }
        }
        else if (CtlItems.curpos == CTL_STICKSENS)
        {
            joypad_8way_t dir = joypad_get_direction(JOYPAD_PORT_1, JOYPAD_2D_LH);
            switch (dir)
            {
                case JOYPAD_8WAY_LEFT:
                    if (stickadjustment)
                    {
                        stickadjustment -= 4;
                        stickadjustment = CLAMP(stickadjustment, 0, 120);
                        SD_PlaySound (MOVEGUN1SND);
                        DrawCtlScreen ();
                        WriteConfig();
                    }
                    break;

                case JOYPAD_8WAY_RIGHT:
                    if (stickadjustment < 120)
                    {
                        stickadjustment += 4;
                        stickadjustment = CLAMP(stickadjustment, 0, 120);
                        SD_PlaySound (MOVEGUN1SND);
                        DrawCtlScreen ();
                        WriteConfig();
                    }
                    break;
                default:
                    break;
            }
        }
    }
    while (which >= 0);

    MenuFadeOut ();

    return 0;
}


///////////////////////////
//
// DRAW CONTROL MENU SCREEN
//
void
DrawCtlScreen (void)
{
    int i;
    int barval;

    ClearMScreen ();
    DrawStripes (10);
    VWB_DrawPic (80, 0, C_CONTROLPIC);
    VWB_DrawPic (112, 184, C_MOUSELBACKPIC);
    DrawWindow (CTL_X - 8, CTL_Y - 5, CTL_W, CTL_H, BKGDCOLOR);
    SETFONTCOLOR (TEXTCOLOR, BKGDCOLOR);

    CtlMenu[CTL_MOUSESENS].active = joypad_get_style(JOYPAD_PORT_2) == JOYPAD_STYLE_MOUSE ? 4 : 0;

    DrawMenu (&CtlItems, CtlMenu);
    int which = CtlItems.curpos;

    for (i = 0; i < CtlItems.amount; i++)
    {
        SetTextColor (&CtlMenu[i], which == i);

        PrintY = CtlItems.y + i * 13;
        barval = -1;

        if (i == CTL_STICKMODE)
        {
            PrintX = 208;
            SETFONTCOLOR (HIGHLIGHT, BKGDCOLOR);
            US_Print(MoveMode ? STR_CSTRAFE : STR_CMOVE);
        }
        else if (i == CTL_ALWAYSRUN)
            VWB_DrawPic (208, PrintY+3, autorun ? C_SELECTEDPIC : C_NOTSELECTEDPIC);
        else if (i == CTL_MOUSESENS)
            barval = 20 * mouseadjustment;
        else if (i == CTL_STICKSENS)
            barval = stickadjustment * 180 / 120;

        if (barval >= 0)
        {
            VWB_Bar (WindowX, PrintY+15, 200, 9, BKGDCOLOR+1);
            DrawOutline (WindowX, PrintY+15, 200, 9, BKGDCOLOR, BKGDCOLOR+2);
            DrawOutline (WindowX + barval, PrintY+15, 20, 9,
                    CtlMenu[i].active ? READCOLOR+2 : BORDCOLOR+2,
                    CtlMenu[i].active ? READCOLOR-2 : BORDCOLOR-2);
            VWB_Bar (WindowX + 1 + barval, PrintY+15+1, 19, 8, CtlMenu[i].active ? READCOLOR : BORDCOLOR);
        }
    }

    //
    // PICK FIRST AVAILABLE SPOT
    //
    if (CtlItems.curpos < 0 || !CtlMenu[CtlItems.curpos].active)
    {
        for (i = 0; i < CtlItems.amount; i++)
        {
            if (CtlMenu[i].active)
            {
                CtlItems.curpos = i;
                break;
            }
        }
    }

    DrawMenuGun (&CtlItems);
    VW_UpdateScreen ();
}


////////////////////////////////////////////////////////////////////
//
// CUSTOMIZE CONTROLS
//
////////////////////////////////////////////////////////////////////
enum
{ FIRE, STRAFE, RUN, OPEN };
char mbarray[4][3] = { "b0", "b1", "b2", "b3" };
int8_t order[4] = { RUN, OPEN, FIRE, STRAFE };


int
CustomControls (int blank)
{
    int which;

    DrawCustomScreen ();
    do
    {
        which = HandleMenu (&CusItems, &CusMenu[0], FixupCustom);
        switch (which)
        {
            case 0:
                DefineMouseBtns ();
                DrawCustMouse (1);
                break;
            case 3:
                DefineJoyBtns ();
                DrawCustJoy (0);
                break;
        }
    }
    while (which >= 0);

    MenuFadeOut ();

    return 0;
}


////////////////////////
//
// DEFINE THE MOUSE BUTTONS
//
void
DefineMouseBtns (void)
{
    CustomCtrls mouseallowed = { 0, 1, 1, 1 };
    EnterCtrlData (2, &mouseallowed, DrawCustMouse, PrintCustMouse, MOUSE);
}


////////////////////////
//
// DEFINE THE JOYSTICK BUTTONS
//
void
DefineJoyBtns (void)
{
    CustomCtrls joyallowed = { 1, 1, 1, 1 };
    EnterCtrlData (5, &joyallowed, DrawCustJoy, PrintCustJoy, JOYSTICK);
}


////////////////////////
//
// ENTER CONTROL DATA FOR ANY TYPE OF CONTROL
//
enum
{ FWRD, RIGHT, BKWD, LEFT };
int moveorder[4] = { LEFT, RIGHT, FWRD, BKWD };

void
EnterCtrlData (int index, CustomCtrls * cust, void (*DrawRtn) (int), void (*PrintRtn) (int),
               int type)
{
    int j,z, exit, tick, redraw, which, x, picked;
    uint64_t lastFlashTime;
    ControlInfo ci;


    ShootSnd ();
    PrintY = CST_Y + 13 * index;
    exit = 0;
    redraw = 1;
    //
    // FIND FIRST SPOT IN ALLOWED ARRAY
    //
    for (j = 0; j < 4; j++)
        if (cust->allowed[j])
        {
            which = j;
            break;
        }

    do
    {
        if (redraw)
        {
            x = CST_START + CST_SPC * which;
            DrawWindow (5, PrintY - 1, 310, 13, BKGDCOLOR);

            DrawRtn (1);
            DrawWindow (x - 2, PrintY, CST_SPC, 11, TEXTCOLOR);
            DrawOutline (x - 2, PrintY, CST_SPC, 11, 0, HIGHLIGHT);
            SETFONTCOLOR (0, TEXTCOLOR);
            PrintRtn (which);
            PrintX = x;
            SETFONTCOLOR (TEXTCOLOR, BKGDCOLOR);
            VW_UpdateScreen ();
            WaitKeyUp ();
            redraw = 0;
        }

        VL_Wait(5);
        ReadAnyControl (&ci);

        //
        // CHANGE BUTTON VALUE?
        //
        if (ci.confirm || ci.cancel)
        {
            lastFlashTime = GetTimeCount();
            tick = picked = 0;
            SETFONTCOLOR (0, TEXTCOLOR);

            while(1)
            {
                joypad_buttons_t button;
                int result = 0;

                //
                // FLASH CURSOR
                //
                if (GetTimeCount() - lastFlashTime > 10)
                {
                    switch (tick)
                    {
                        case 0:
                            VWB_Bar (x, PrintY + 1, CST_SPC - 2, 10, TEXTCOLOR);
                            break;
                        case 1:
                            PrintX = x;
                            US_Print ("?");
                            SD_PlaySound (HITWALLSND);
                    }
                    tick ^= 1;
                    lastFlashTime = GetTimeCount();
                    VW_UpdateScreen ();
                }
                else VL_Wait(5);

                //
                // WHICH TYPE OF INPUT DO WE PROCESS?
                //
                switch (type)
                {
                    case MOUSE:
                        button = joypad_get_buttons_pressed(JOYPAD_PORT_2);
                        if (button.a)
                            result = 1;
                        else if (button.b)
                            result = 2;

                        if (result)
                        {
                            for (z = 0; z < 2; z++)
                                if (order[which] == buttonmouse[z])
                                {
                                    buttonmouse[z] = bt_nobutton;
                                    break;
                                }

                            buttonmouse[result - 1] = order[which];
                            picked = 1;
                            SD_PlaySound (SHOOTDOORSND);
                        }
                        break;

                    case JOYSTICK:
                        button = joypad_get_buttons_pressed(JOYPAD_PORT_1);

                        if (button.raw)
                        {
                            for (z = 0; z < 16; z++)
                                if (button.raw & (1<<z))
                                    result = 1<<z;

                            for (z = 0; z < 16; z++)
                            {
                                if (order[which] == buttonjoy[z])
                                {
                                    buttonjoy[z] = bt_nobutton;
                                    break;
                                }
                            }

                            buttonjoy[result - 1] = order[which];
                            picked = 1;
                            SD_PlaySound (SHOOTDOORSND);
                        }
                        break;
                }

                //
                // EXIT INPUT?
                //
                if (ci.cancel)
                {
                    picked = 1;
                    SD_PlaySound (ESCPRESSEDSND);
                }

                if(picked) break;

                ReadAnyControl (&ci);
            }

            SETFONTCOLOR (TEXTCOLOR, BKGDCOLOR);
            redraw = 1;
            WaitKeyUp ();
            continue;
        }

        if (ci.cancel)
            exit = 1;

        //
        // MOVE TO ANOTHER SPOT?
        //
        switch (ci.dir)
        {
            case dir_West:
                do
                {
                    which--;
                    if (which < 0)
                        which = 3;
                }
                while (!cust->allowed[which]);
                redraw = 1;
                SD_PlaySound (MOVEGUN1SND);
                while (ReadAnyControl (&ci), ci.dir != dir_None) VL_Wait(5);
                break;

            case dir_East:
                do
                {
                    which++;
                    if (which > 3)
                        which = 0;
                }
                while (!cust->allowed[which]);
                redraw = 1;
                SD_PlaySound (MOVEGUN1SND);
                while (ReadAnyControl (&ci), ci.dir != dir_None) VL_Wait(5);
                break;
            case dir_North:
            case dir_South:
                exit = 1;
        }
    }
    while (!exit);

    SD_PlaySound (ESCPRESSEDSND);
    WaitKeyUp ();
    DrawWindow (5, PrintY - 1, 310, 13, BKGDCOLOR);
}


////////////////////////
//
// FIXUP GUN CURSOR OVERDRAW SHIT
//
void
FixupCustom (int w)
{
    static int lastwhich = -1;
    int y = CST_Y + 26 + w * 13;


    VWB_Hlin (7, 32, y - 1, DEACTIVE);
    VWB_Hlin (7, 32, y + 12, BORD2COLOR);
#ifndef SPEAR
    VWB_Hlin (7, 32, y - 2, BORDCOLOR);
    VWB_Hlin (7, 32, y + 13, BORDCOLOR);
#else
    VWB_Hlin (7, 32, y - 2, BORD2COLOR);
    VWB_Hlin (7, 32, y + 13, BORD2COLOR);
#endif

    switch (w)
    {
        case 0:
            DrawCustMouse (1);
            break;
        case 3:
            DrawCustJoy (1);
            break;
    }


    if (lastwhich >= 0)
    {
        y = CST_Y + 26 + lastwhich * 13;
        VWB_Hlin (7, 32, y - 1, DEACTIVE);
        VWB_Hlin (7, 32, y + 12, BORD2COLOR);
#ifndef SPEAR
        VWB_Hlin (7, 32, y - 2, BORDCOLOR);
        VWB_Hlin (7, 32, y + 13, BORDCOLOR);
#else
        VWB_Hlin (7, 32, y - 2, BORD2COLOR);
        VWB_Hlin (7, 32, y + 13, BORD2COLOR);
#endif

        if (lastwhich != w)
            switch (lastwhich)
            {
                case 0:
                    DrawCustMouse (0);
                    break;
                case 3:
                    DrawCustJoy (0);
                    break;
            }
    }

    lastwhich = w;
}


////////////////////////
//
// DRAW CUSTOMIZE SCREEN
//
void
DrawCustomScreen (void)
{
    int i;


#ifdef JAPAN
    VWB_DrawPic (0,0,S_CUSTOMPIC);
    fontnumber = 1;

    PrintX = CST_START;
    PrintY = CST_Y + 26;
    DrawCustMouse (0);

    PrintX = CST_START;
    US_Print ("\n\n\n");
    DrawCustJoy (0);
#else
    ClearMScreen ();
    WindowX = 0;
    WindowW = 320;
    VWB_DrawPic (112, 184, C_MOUSELBACKPIC);
    DrawStripes (10);
    VWB_DrawPic (80, 0, C_CUSTOMIZEPIC);

    //
    // MOUSE
    //
    SETFONTCOLOR (READCOLOR, BKGDCOLOR);
    WindowX = 0;
    WindowW = 320;

#ifndef SPEAR
    PrintY = CST_Y;
    US_CPrint ("Mouse\n");
#else
    PrintY = CST_Y + 13;
    VWB_DrawPic (128, 48, C_MOUSEPIC);
#endif

    SETFONTCOLOR (TEXTCOLOR, BKGDCOLOR);
#ifdef SPANISH
    PrintX = CST_START - 16;
    US_Print (STR_CRUN);
    PrintX = CST_START - 16 + CST_SPC * 1;
    US_Print (STR_COPEN);
    PrintX = CST_START - 16 + CST_SPC * 2;
    US_Print (STR_CFIRE);
    PrintX = CST_START - 16 + CST_SPC * 3;
    US_Print (STR_CSTRAFE "\n");
#else
    PrintX = CST_START;
    US_Print (STR_CRUN);
    PrintX = CST_START + CST_SPC * 1;
    US_Print (STR_COPEN);
    PrintX = CST_START + CST_SPC * 2;
    US_Print (STR_CFIRE);
    PrintX = CST_START + CST_SPC * 3;
    US_Print (STR_CSTRAFE "\n");
#endif
#endif

    DrawWindow (5, PrintY - 1, 310, 13, BKGDCOLOR);
    DrawCustMouse (0);
    US_Print ("\n");


    //
    // JOYSTICK/PAD
    //
#ifndef SPEAR
    SETFONTCOLOR (READCOLOR, BKGDCOLOR);
    US_CPrint ("Joystick/Gravis GamePad\n");
#else
    PrintY += 13;
    VWB_DrawPic (40, 88, C_JOYSTICKPIC);
#endif

#ifdef SPEAR
    VWB_DrawPic (112, 120, C_KEYBOARDPIC);
#endif

    SETFONTCOLOR (TEXTCOLOR, BKGDCOLOR);
#ifdef SPANISH
    PrintX = CST_START - 16;
    US_Print (STR_CRUN);
    PrintX = CST_START - 16 + CST_SPC * 1;
    US_Print (STR_COPEN);
    PrintX = CST_START - 16 + CST_SPC * 2;
    US_Print (STR_CFIRE);
    PrintX = CST_START - 16 + CST_SPC * 3;
    US_Print (STR_CSTRAFE "\n");
#else
    PrintX = CST_START;
    US_Print (STR_CRUN);
    PrintX = CST_START + CST_SPC * 1;
    US_Print (STR_COPEN);
    PrintX = CST_START + CST_SPC * 2;
    US_Print (STR_CFIRE);
    PrintX = CST_START + CST_SPC * 3;
    US_Print (STR_CSTRAFE "\n");
#endif
    DrawWindow (5, PrintY - 1, 310, 13, BKGDCOLOR);
    DrawCustJoy (0);
    US_Print ("\n");


    //
    // PICK STARTING POINT IN MENU
    //
    if (CusItems.curpos < 0)
        for (i = 0; i < CusItems.amount; i++)
            if (CusMenu[i].active)
            {
                CusItems.curpos = i;
                break;
            }


    VW_UpdateScreen ();
    MenuFadeIn ();
}


void
PrintCustMouse (int i)
{
    int j;

    for (j = 0; j < 4; j++)
        if (order[i] == buttonmouse[j])
        {
            PrintX = CST_START + CST_SPC * i;
            US_Print (mbarray[j]);
            break;
        }
}

void
DrawCustMouse (int hilight)
{
    int i, color;


    color = TEXTCOLOR;
    if (hilight)
        color = HIGHLIGHT;
    SETFONTCOLOR (color, BKGDCOLOR);

    CusMenu[0].active = 1;

    PrintY = CST_Y + 13 * 2;
    for (i = 0; i < 4; i++)
        PrintCustMouse (i);
}

void
PrintCustJoy (int i)
{
    int j;

    for (j = 0; j < 4; j++)
    {
        if (order[i] == buttonjoy[j])
        {
            PrintX = CST_START + CST_SPC * i;
            US_Print (mbarray[j]);
            break;
        }
    }
}

void
DrawCustJoy (int hilight)
{
    int i, color;

    color = TEXTCOLOR;
    if (hilight)
        color = HIGHLIGHT;
    SETFONTCOLOR (color, BKGDCOLOR);

    CusMenu[3].active = 1;

    PrintY = CST_Y + 13 * 5;
    for (i = 0; i < 4; i++)
        PrintCustJoy (i);
}

void
DrawCheatScreen (void)
{
    int i, on;

    ClearMScreen ();
    DrawStripes (10);
    VWB_DrawPic (112, 184, C_MOUSELBACKPIC);
    DrawWindow (CHT_X - 8, CHT_Y - 5, CHT_W, CHT_H, BKGDCOLOR);

    WindowX = 0;
    WindowW = 320;

    SETFONTCOLOR (15, 0);
    PrintY = 15;
    PrintX = 160;
    US_CPrint (STR_CH);

    SETFONTCOLOR (TEXTCOLOR, BKGDCOLOR);

    CheatMenu[CHT_BOOST].active = ingame;

    DrawMenu (&CheatItems, CheatMenu);

    //
    // PICK FIRST AVAILABLE SPOT
    //
    if (CheatItems.curpos < 0 || !CheatMenu[CheatItems.curpos].active)
    {
        for (i = 0; i < CheatItems.amount; i++)
        {
            if (CheatMenu[i].active)
            {
                CheatItems.curpos = i;
                break;
            }
        }
    }

    for (i = 0; i < CheatItems.amount; i++)
    {
        on = -1;
        if (i == CHT_GODMODE)
            on = !!godmode;
        else if (i == CHT_MAPREVEAL)
            on = !!mapreveal;
        else if (i == CHT_NOCLIP)
            on = !!noclip;
        else if (i == CHT_SHOWFPS)
            on = !!fpscounter;

        if (on > 0)
            VWB_DrawPic (CHT_X + 30, CHT_Y + i * 13 + 2, C_SELECTEDPIC);
        else if (on == 0)
            VWB_DrawPic (CHT_X + 30, CHT_Y + i * 13 + 2, C_NOTSELECTEDPIC);
    }

    DrawMenuGun (&CheatItems);
    VW_UpdateScreen ();
}


////////////////////////////////////////////////////////////////////
//
// CHANGE SCREEN VIEWING SIZE
//
////////////////////////////////////////////////////////////////////
int
CP_ChangeView (int blank)
{
    int exit = 0, oldview, newview;
    ControlInfo ci;

    WindowX = WindowY = 0;
    WindowW = 320;
    WindowH = 200;
    newview = oldview = viewsize;
    DrawChangeView (oldview);
    MenuFadeIn ();

    do
    {
        CheckPause ();
        VL_Wait(5);
        ReadAnyControl (&ci);
        switch (ci.dir)
        {
            case dir_South:
            case dir_West:
                newview--;
                if (newview < 4)
                    newview = 4;
                if(newview >= 19) DrawChangeView(newview);
                else ShowViewSize (newview);
                VW_UpdateScreen ();
                SD_PlaySound (HITWALLSND);
                TicDelay (10);
                break;

            case dir_North:
            case dir_East:
                newview++;
                if (newview >= 21)
                {
                    newview = 21;
                    DrawChangeView(newview);
                }
                else ShowViewSize (newview);
                VW_UpdateScreen ();
                SD_PlaySound (HITWALLSND);
                TicDelay (10);
                break;
        }

        if (ci.confirm)
            exit = 1;
        else if (ci.cancel)
        {
            SD_PlaySound (ESCPRESSEDSND);
            MenuFadeOut ();
            if(screenHeight % 200 != 0)
                VL_ClearScreen(0);
            return 0;
        }
    }
    while (!exit);

    if (oldview != newview)
    {
        SD_PlaySound (SHOOTSND);
        Message (STR_THINK "...");
        NewViewSize (newview);
    }

    WriteConfig();
    ShootSnd ();
    MenuFadeOut ();
    if(screenHeight % 200 != 0)
        VL_ClearScreen(0);

    return 0;
}


/////////////////////////////
//
// DRAW THE CHANGEVIEW SCREEN
//
void
DrawChangeView (int view)
{
    if(view != 21) VWB_Bar (0, screenHeight - 40, 320, 40, bordercol);

#ifdef JAPAN
    VWB_DrawPic (0,0,S_CHANGEPIC);

    ShowViewSize (view);
#else
    ShowViewSize (view);

    PrintY = screenHeight - 39;
    WindowX = 0;
    WindowY = 320;                                  // TODO: Check this!
    SETFONTCOLOR (HIGHLIGHT, BKGDCOLOR);

    US_CPrint (STR_SIZE1 "\n");
    US_CPrint (STR_SIZE2 "\n");
    US_CPrint (STR_SIZE3);
#endif
    VW_UpdateScreen ();
}


////////////////////////////////////////////////////////////////////
//
// QUIT THIS INFERNAL GAME!
//
////////////////////////////////////////////////////////////////////
int
CP_Quit (int blank)
{
#ifdef JAPAN
    if (GetYorN (7, 11, C_QUITMSGPIC))
#else

#ifdef SPANISH
    if (Confirm (ENDGAMESTR))
#else
    if (Confirm (endStrings[US_RndT () & 0x7 + (US_RndT () & 1)]))
#endif

#endif
    {
        VW_UpdateScreen ();
        SD_MusicOff ();
        SD_StopSound ();
        MenuFadeOut ();
        Quit (NULL);
    }

    DrawMainMenu ();
    return 0;
}


////////////////////////////////////////////////////////////////////
//
// HANDLE INTRO SCREEN (SYSTEM CONFIG)
//
////////////////////////////////////////////////////////////////////
void
IntroScreen (void)
{
#ifdef SPEAR

#define MAINCOLOR       0x4f
#define EMSCOLOR        0x4f
#define XMSCOLOR        0x4f

#else

#define MAINCOLOR       0x6c
#define EMSCOLOR        0x6c    // 0x4f
#define XMSCOLOR        0x6c    // 0x7f

#endif
#define FILLCOLOR       14

//      long memory;
//      long emshere,xmshere;
    int i;
/*      int ems[10]={100,200,300,400,500,600,700,800,900,1000},
                xms[10]={100,200,300,400,500,600,700,800,900,1000};
        int main[10]={32,64,96,128,160,192,224,256,288,320};*/


    //
    // DRAW MAIN MEMORY
    //
#ifdef ABCAUS
    memory = (1023l + mminfo.nearheap + mminfo.farheap) / 1024l;
    for (i = 0; i < 10; i++)
        if (memory >= main[i])
            VWB_Bar (49, 163 - 8 * i, 6, 5, MAINCOLOR - i);

    //
    // DRAW EMS MEMORY
    //
    if (EMSPresent)
    {
        emshere = 4l * EMSPagesAvail;
        for (i = 0; i < 10; i++)
            if (emshere >= ems[i])
                VWB_Bar (89, 163 - 8 * i, 6, 5, EMSCOLOR - i);
    }

    //
    // DRAW XMS MEMORY
    //
    if (XMSPresent)
    {
        xmshere = 4l * XMSPagesAvail;
        for (i = 0; i < 10; i++)
            if (xmshere >= xms[i])
                VWB_Bar (129, 163 - 8 * i, 6, 5, XMSCOLOR - i);
    }
#else
    for (i = 0; i < 10; i++)
        VWB_Bar (49, 163 - 8 * i, 6, 5, MAINCOLOR - i);
    for (i = 0; i < 10; i++)
        VWB_Bar (89, 163 - 8 * i, 6, 5, EMSCOLOR - i);
    for (i = 0; i < 10; i++)
        VWB_Bar (129, 163 - 8 * i, 6, 5, XMSCOLOR - i);
#endif

    //
    // FILL BOXES
    //
    if (joypad_get_style(JOYPAD_PORT_2) == JOYPAD_STYLE_MOUSE)
        VWB_Bar (164, 82, 12, 2, FILLCOLOR);

    if (joypad_is_connected(JOYPAD_PORT_1))
        VWB_Bar (164, 105, 12, 2, FILLCOLOR);

    if (1)
        VWB_Bar (164, 128, 12, 2, FILLCOLOR);

    if (1)
        VWB_Bar (164, 151, 12, 2, FILLCOLOR);

//    if (SoundSourcePresent)
//        VWB_Bar (164, 174, 12, 2, FILLCOLOR);
}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//
// SUPPORT ROUTINES
//
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//
// Clear Menu screens to dark red
//
////////////////////////////////////////////////////////////////////
void
ClearMScreen (void)
{
#ifndef SPEAR
    VWB_Bar (0, 0, 320, 200, BORDCOLOR);
#else
    VWB_DrawPic (0, 0, C_BACKDROPPIC);
#endif
}


////////////////////////////////////////////////////////////////////
//
// Draw a window for a menu
//
////////////////////////////////////////////////////////////////////
void
DrawWindow (int x, int y, int w, int h, int wcolor)
{
    VWB_Bar (x, y, w, h, wcolor);
    DrawOutline (x, y, w, h, BORD2COLOR, DEACTIVE);
}


void
DrawOutline (int x, int y, int w, int h, int color1, int color2)
{
    VWB_Hlin (x, x + w, y, color2);
    VWB_Vlin (y, y + h, x, color2);
    VWB_Hlin (x, x + w, y + h, color1);
    VWB_Vlin (y, y + h, x + w, color1);
}


////////////////////////////////////////////////////////////////////
//
// Setup Control Panel stuff - graphics, etc.
//
////////////////////////////////////////////////////////////////////
void
SetupControlPanel (void)
{
    //
    // CACHE SOUNDS
    //
    SETFONTCOLOR (TEXTCOLOR, BKGDCOLOR);
    fontnumber = 1;
    WindowH = 200;
    if(screenHeight % 200 != 0)
        VL_ClearScreen(0);

    MainMenu[loadgame].active = SaveGameAvail;
    strcpy(MainMenu[savegame].string, STR_SG);

    if (!ingame)
        CA_LoadAllSounds ();
    else
        MainMenu[savegame].active = 1;
}

////////////////////////////////////////////////////////////////////
//
// SEE WHICH SAVE GAME FILES ARE AVAILABLE & READ STRING IN
//
////////////////////////////////////////////////////////////////////
void SetupSaveGames()
{
    FILE *handle = N64_ReadSave();
    if(handle)
    {
        char temp[32];

        fread(temp, 1, 32, handle);
        if (strncmp(temp, savemagic, 32) == 0)
            SaveGameAvail = true;
        fclose(handle);
    }
}

////////////////////////////////////////////////////////////////////
//
// Clean up all the Control Panel stuff
//
////////////////////////////////////////////////////////////////////
void
CleanupControlPanel (void)
{
    fontnumber = 0;
}


////////////////////////////////////////////////////////////////////
//
// Handle moving gun around a menu
//
////////////////////////////////////////////////////////////////////
int
HandleMenu (CP_iteminfo * item_i, CP_itemtype * items, void (*routine) (int w))
{
    static int redrawitem = 1, lastitem = -1;
    int x, y, basey, exit, which, shape;
    int32_t timer;
    uint64_t lastBlinkTime;
    ControlInfo ci;


    which = item_i->curpos;
    x = item_i->x & -8;
    basey = item_i->y - 2;
    y = basey + which * 13;

    VWB_DrawPic (x, y, C_CURSOR1PIC);
    SetTextColor (items + which, 1);
    if (redrawitem)
    {
        PrintX = item_i->x + item_i->indent;
        PrintY = item_i->y + which * 13;
        US_Print ((items + which)->string);
    }
    //
    // CALL CUSTOM ROUTINE IF IT IS NEEDED
    //
    if (routine)
        routine (which);
    VW_UpdateScreen ();

    shape = C_CURSOR1PIC;
    timer = 8;
    exit = 0;
    lastBlinkTime = GetTimeCount ();


    do
    {
        //
        // CHANGE GUN SHAPE
        //
        if (GetTimeCount() - lastBlinkTime > timer)
        {
            lastBlinkTime = GetTimeCount ();
            if (shape == C_CURSOR1PIC)
            {
                shape = C_CURSOR2PIC;
                timer = 8;
            }
            else
            {
                shape = C_CURSOR1PIC;
                timer = 70;
            }
            VWB_DrawPic (x, y, shape);
            if (routine)
                routine (which);
            VW_UpdateScreen ();
        }
        else VL_Wait(5);

        CheckPause ();

        //
        // GET INPUT
        //
        ReadAnyControl (&ci);
        joypad_buttons_t pad = joypad_get_buttons(JOYPAD_PORT_1);
        switch (ci.dir)
        {
                ////////////////////////////////////////////////
                //
                // MOVE UP
                //
            case dir_North:

                EraseGun (item_i, items, x, y, which);

                //
                // ANIMATE HALF-STEP
                //
                if (which && (items + which - 1)->active)
                {
                    y -= 6;
                    DrawHalfStep (x, y);
                }

                //
                // MOVE TO NEXT AVAILABLE SPOT
                //
                do
                {
                    if (!which)
                        which = item_i->amount - 1;
                    else
                        which--;
                }
                while (!(items + which)->active);

                DrawGun (item_i, items, x, &y, which, basey, routine);
                //
                // WAIT FOR BUTTON-UP OR DELAY NEXT MOVE
                //
                TicDelay (20);
                break;

                ////////////////////////////////////////////////
                //
                // MOVE DOWN
                //
            case dir_South:

                EraseGun (item_i, items, x, y, which);
                //
                // ANIMATE HALF-STEP
                //
                if (which != item_i->amount - 1 && (items + which + 1)->active)
                {
                    y += 6;
                    DrawHalfStep (x, y);
                }

                do
                {
                    if (which == item_i->amount - 1)
                        which = 0;
                    else
                        which++;
                }
                while (!(items + which)->active);

                DrawGun (item_i, items, x, &y, which, basey, routine);

                //
                // WAIT FOR BUTTON-UP OR DELAY NEXT MOVE
                //
                TicDelay (20);
                break;
        }

        if ((items + which)->active == 4 && ci.dir != dir_None)
            exit = 1;

        if (ci.confirm)
            exit = 1;

        if (ci.cancel || pad.start)
            exit = 2;

    }
    while (!exit);


    //
    // ERASE EVERYTHING
    //
    if (lastitem != which)
    {
        VWB_Bar (x - 1, y, 25, 16, BKGDCOLOR);
        PrintX = item_i->x + item_i->indent;
        PrintY = item_i->y + which * 13;
        US_Print ((items + which)->string);
        redrawitem = 1;
    }
    else
        redrawitem = 0;

    if (routine)
        routine (which);
    VW_UpdateScreen ();

    item_i->curpos = which;

    lastitem = which;
    switch (exit)
    {
        case 1:
            //
            // CALL THE ROUTINE
            //
            if ((items + which)->routine != NULL)
            {
                ShootSnd ();
                MenuFadeOut ();
                (items + which)->routine (0);
            }
            return which;

        case 2:
            SD_PlaySound (ESCPRESSEDSND);
            return -1;
    }

    return 0;                   // JUST TO SHUT UP THE ERROR MESSAGES!
}


//
// ERASE GUN & DE-HIGHLIGHT STRING
//
void
EraseGun (CP_iteminfo * item_i, CP_itemtype * items, int x, int y, int which)
{
    VWB_Bar (x - 1, y, 25, 16, BKGDCOLOR);
    SetTextColor (items + which, 0);

    PrintX = item_i->x + item_i->indent;
    PrintY = item_i->y + which * 13;
    US_Print ((items + which)->string);
    VW_UpdateScreen ();
}


//
// DRAW HALF STEP OF GUN TO NEXT POSITION
//
void
DrawHalfStep (int x, int y)
{
    VWB_DrawPic (x, y, C_CURSOR1PIC);
    VW_UpdateScreen ();
    SD_PlaySound (MOVEGUN1SND);
    VL_Wait (8 * 100 / 7);
}


//
// DRAW GUN AT NEW POSITION
//
void
DrawGun (CP_iteminfo * item_i, CP_itemtype * items, int x, int *y, int which, int basey,
         void (*routine) (int w))
{
    VWB_Bar (x - 1, *y, 25, 16, BKGDCOLOR);
    *y = basey + which * 13;
    VWB_DrawPic (x, *y, C_CURSOR1PIC);
    SetTextColor (items + which, 1);

    PrintX = item_i->x + item_i->indent;
    PrintY = item_i->y + which * 13;
    US_Print ((items + which)->string);

    //
    // CALL CUSTOM ROUTINE IF IT IS NEEDED
    //
    if (routine)
        routine (which);
    VW_UpdateScreen ();
    SD_PlaySound (MOVEGUN2SND);
}

////////////////////////////////////////////////////////////////////
//
// DELAY FOR AN AMOUNT OF TICS OR UNTIL CONTROLS ARE INACTIVE
//
////////////////////////////////////////////////////////////////////
void
TicDelay (int count)
{
    ControlInfo ci;

    uint64_t startTime = GetTimeCount ();
    do
    {
        VL_Wait(5);
        ReadAnyControl (&ci);
    }
    while (GetTimeCount () - startTime < count && ci.dir != dir_None);
}


////////////////////////////////////////////////////////////////////
//
// Draw a menu
//
////////////////////////////////////////////////////////////////////
void
DrawMenu (CP_iteminfo * item_i, CP_itemtype * items)
{
    int i, which = item_i->curpos;


    WindowX = PrintX = item_i->x + item_i->indent;
    WindowY = PrintY = item_i->y;
    WindowW = 320;
    WindowH = 200;

    for (i = 0; i < item_i->amount; i++)
    {
        SetTextColor (items + i, which == i);

        PrintY = item_i->y + i * 13;
        if ((items + i)->active)
            US_Print ((items + i)->string);
        else
        {
            SETFONTCOLOR (DEACTIVE, BKGDCOLOR);
            US_Print ((items + i)->string);
            SETFONTCOLOR (TEXTCOLOR, BKGDCOLOR);
        }

        US_Print ("\n");
    }
}


////////////////////////////////////////////////////////////////////
//
// SET TEXT COLOR (HIGHLIGHT OR NO)
//
////////////////////////////////////////////////////////////////////
void
SetTextColor (CP_itemtype * items, int hlight)
{
    if (hlight)
    {
        SETFONTCOLOR (color_hlite[items->active], BKGDCOLOR);
    }
    else
    {
        SETFONTCOLOR (color_norml[items->active], BKGDCOLOR);
    }
}


////////////////////////////////////////////////////////////////////
//
// WAIT FOR CTRLKEY-UP OR BUTTON-UP
//
////////////////////////////////////////////////////////////////////
void
WaitKeyUp (void)
{
    ControlInfo ci;
    while (ReadAnyControl (&ci), ci.confirm || ci.cancel)
    {
        IN_ProcessEvents();
    }
}


////////////////////////////////////////////////////////////////////
//
// READ KEYBOARD, JOYSTICK AND MOUSE FOR INPUT
//
////////////////////////////////////////////////////////////////////
static int totalMousex = 0, totalMousey = 0;
void
ReadAnyControl (ControlInfo * ci)
{
    int mouseactive = 0;

    IN_ReadControl (0, ci);
}


////////////////////////////////////////////////////////////////////
//
// DRAW DIALOG AND CONFIRM YES OR NO TO QUESTION
//
////////////////////////////////////////////////////////////////////
int
Confirm (const char *string)
{
    int xit = 0, x, y, tick = 0, lastBlinkTime;
    int whichsnd[2] = { ESCPRESSEDSND, SHOOTSND };
    ControlInfo ci;

    Message (string);
    WaitKeyUp ();

    //
    // BLINK CURSOR
    //
    x = PrintX;
    y = PrintY;
    lastBlinkTime = GetTimeCount();

    do
    {
        ReadAnyControl(&ci);

        if (GetTimeCount() - lastBlinkTime >= 10)
        {
            switch (tick)
            {
                case 0:
                    VWB_Bar (x, y, 8, 13, TEXTCOLOR);
                    break;
                case 1:
                    PrintX = x;
                    PrintY = y;
                    US_Print ("_");
            }
            VW_UpdateScreen ();
            tick ^= 1;
            lastBlinkTime = GetTimeCount();
        }
        else VL_Wait(5);

    }
    while (!ci.confirm && !ci.cancel);

    if (ci.confirm)
    {
        xit = 1;
        ShootSnd ();
    }

    WaitKeyUp ();

    SD_PlaySound ((soundnames) whichsnd[xit]);

    return xit;
}

#ifdef JAPAN
////////////////////////////////////////////////////////////////////
//
// DRAW MESSAGE & GET Y OR N
//
////////////////////////////////////////////////////////////////////
int
GetYorN (int x, int y, int pic)
{
    int xit = 0;
    soundnames whichsnd[2] = { ESCPRESSEDSND, SHOOTSND };
    ControlInfo ci;

    VWB_DrawPic (x * 8, y * 8, pic);
    VW_UpdateScreen ();

    do
    {
        IN_ProcessEvents();
        ReadAnyControl(&ci);
    }
    while (!ci.cancel && !ci.confirm);

    if (ci.confirm)
    {
        xit = 1;
        ShootSnd ();
    }

    while (ci.confirm || ci.cancel)
    {
        IN_ProcessEvents();
        ReadAnyControl(&ci);
    }

    SD_PlaySound (whichsnd[xit]);
    return xit;
}
#endif


////////////////////////////////////////////////////////////////////
//
// PRINT A MESSAGE IN A WINDOW
//
////////////////////////////////////////////////////////////////////
void
Message (const char *string)
{
    int h = 0, w = 0, mw = 0, i, len = (int) strlen(string);
    fontstruct *font;

    fontnumber = 1;
    font = (fontstruct *) grsegs[STARTFONT + fontnumber];
    h = font->height;
    for (i = 0; i < len; i++)
    {
        if (string[i] == '\n')
        {
            if (w > mw)
                mw = w;
            w = 0;
            h += font->height;
        }
        else
            w += font->width[string[i]];
    }

    if (w + 10 > mw)
        mw = w + 10;

    PrintY = (WindowH / 2) - h / 2;
    PrintX = WindowX = 160 - mw / 2;

    DrawWindow (WindowX - 5, PrintY - 5, mw + 10, h + 10, TEXTCOLOR);
    DrawOutline (WindowX - 5, PrintY - 5, mw + 10, h + 10, 0, HIGHLIGHT);
    SETFONTCOLOR (0, TEXTCOLOR);
    US_Print (string);
    VW_UpdateScreen ();
}

////////////////////////////////////////////////////////////////////
//
// THIS MAY BE FIXED A LITTLE LATER...
//
////////////////////////////////////////////////////////////////////
static int lastmusic;

int
StartCPMusic (int song)
{
    int lastoffs;

    lastmusic = song;
    lastoffs = SD_MusicOff ();
    UNCACHEAUDIOCHUNK (STARTMUSIC + lastmusic);

    SD_StartMusic(STARTMUSIC + song);
    return lastoffs;
}

void
FreeMusic (void)
{
    UNCACHEAUDIOCHUNK (STARTMUSIC + lastmusic);
}


///////////////////////////////////////////////////////////////////////////
//
// CHECK FOR PAUSE KEY (FOR MUSIC ONLY)
//
///////////////////////////////////////////////////////////////////////////
void
CheckPause (void)
{
    if (Paused)
    {
        switch (SoundStatus)
        {
            case 0:
                SD_MusicOn ();
                break;
            case 1:
                SD_MusicOff ();
                break;
        }

        SoundStatus ^= 1;
        VW_WaitVBL (3);
        Paused = false;
    }
}


///////////////////////////////////////////////////////////////////////////
//
// DRAW GUN CURSOR AT CORRECT POSITION IN MENU
//
///////////////////////////////////////////////////////////////////////////
void
DrawMenuGun (CP_iteminfo * iteminfo)
{
    int x, y;


    x = iteminfo->x;
    y = iteminfo->y + iteminfo->curpos * 13 - 2;
    VWB_DrawPic (x, y, C_CURSOR1PIC);
}


///////////////////////////////////////////////////////////////////////////
//
// DRAW SCREEN TITLE STRIPES
//
///////////////////////////////////////////////////////////////////////////
void
DrawStripes (int y)
{
#ifndef SPEAR
    VWB_Bar (0, y, 320, 24, 0);
    VWB_Hlin (0, 319, y + 22, STRIPE);
#else
    VWB_Bar (0, y, 320, 22, 0);
    VWB_Hlin (0, 319, y + 23, 0);
#endif
}

void
ShootSnd (void)
{
    SD_PlaySound (SHOOTSND);
}


///////////////////////////////////////////////////////////////////////////
//
// CHECK FOR EPISODES
//
///////////////////////////////////////////////////////////////////////////
void
CheckForEpisodes (void)
{
    struct stat statbuf;

//
// JAPANESE VERSION
//
#ifdef JAPAN
#ifdef JAPDEMO
    if(!stat("rom:/vswap.wj1", &statbuf))
    {
        strcpy (extension, "wj1");
#else
    if(!stat("rom:/vswap.wj6", &statbuf))
    {
        strcpy (extension, "wj6");
#endif
        strcat (configname, extension);
        strcat (SaveName, extension);
        strcat (demoname, extension);
        EpisodeSelect[1] =
            EpisodeSelect[2] = EpisodeSelect[3] = EpisodeSelect[4] = EpisodeSelect[5] = 1;
    }
    else
        Quit ("NO JAPANESE WOLFENSTEIN 3-D DATA FILES to be found!");
    strcpy (graphext, extension);
    strcpy (audioext, extension);
#else

//
// ENGLISH
//
#ifdef UPLOAD
    if(!stat("rom:/vswap.wl1", &statbuf))
        strcpy (extension, "wl1");
    else
        Quit ("NO WOLFENSTEIN 3-D DATA FILES to be found!");
#else
#ifndef SPEAR
    if(!stat("rom:/vswap.wl6", &statbuf))
    {
        strcpy (extension, "wl6");
        NewEmenu[2].active =
            NewEmenu[4].active =
            NewEmenu[6].active =
            NewEmenu[8].active =
            NewEmenu[10].active =
            EpisodeSelect[1] =
            EpisodeSelect[2] = EpisodeSelect[3] = EpisodeSelect[4] = EpisodeSelect[5] = 1;
    }
    else
    {
        if(!stat("rom:/vswap.wl3", &statbuf))
        {
            strcpy (extension, "wl3");
            NewEmenu[2].active = NewEmenu[4].active = EpisodeSelect[1] = EpisodeSelect[2] = 1;
        }
        else
        {
            if(!stat("rom:/vswap.wl1", &statbuf))
                strcpy (extension, "wl1");
            else
                Quit ("NO WOLFENSTEIN 3-D DATA FILES to be found!");
        }
    }
#endif
#endif


#ifdef SPEAR
#ifndef SPEARDEMO
    if(param_mission == 0)
    {
        if(!stat("rom:/vswap.sod", &statbuf))
            strcpy (extension, "sod");
        else
            Quit ("NO SPEAR OF DESTINY DATA FILES TO BE FOUND!");
    }
    else if(param_mission == 1)
    {
        if(!stat("rom:/vswap.sd1", &statbuf))
            strcpy (extension, "sd1");
        else
            Quit ("NO SPEAR OF DESTINY DATA FILES TO BE FOUND!");
    }
    else if(param_mission == 2)
    {
        if(!stat("rom:/vswap.sd2", &statbuf))
            strcpy (extension, "sd2");
        else
            Quit ("NO SPEAR OF DESTINY DATA FILES TO BE FOUND!");
    }
    else if(param_mission == 3)
    {
        if(!stat("rom:/vswap.sd3", &statbuf))
            strcpy (extension, "sd3");
        else
            Quit ("NO SPEAR OF DESTINY DATA FILES TO BE FOUND!");
    }
    else
        Quit ("UNSUPPORTED MISSION!");
    strcpy (graphext, "sod");
    strcpy (audioext, "sod");
#else
    if(!stat("rom:/vswap.sdm", &statbuf))
    {
        strcpy (extension, "sdm");
    }
    else
        Quit ("NO SPEAR OF DESTINY DEMO DATA FILES TO BE FOUND!");
    strcpy (graphext, "sdm");
    strcpy (audioext, "sdm");
#endif
#else
    strcpy (graphext, extension);
    strcpy (audioext, extension);
#endif

    strcat (configname, extension);
    strcat (demoname, extension);

#ifndef SPEAR
#ifndef GOODTIMES
    strcat (helpfilename, extension);
#endif
    strcat (endfilename, extension);
#endif
#endif
}
