//
//      ID Engine
//      ID_SD.c - Sound Manager for Wolfenstein 3D
//      v1.2
//      By Jason Blochowiak
//

//
//      This module handles dealing with generating sound on the appropriate
//              hardware
//
//      Depends on: User Mgr (for parm checking)
//
//      Globals:
//              For User Mgr:
//                      SoundBlasterPresent - SoundBlaster card present?
//                      AdLibPresent - AdLib card present?
//                      SoundMode - What device is used for sound effects
//                              (Use SM_SetSoundMode() to set)
//                      MusicMode - What device is used for music
//                              (Use SM_SetMusicMode() to set)
//                      DigiMode - What device is used for digitized sound effects
//                              (Use SM_SetDigiDevice() to set)
//
//              For Cache Mgr:
//                      NeedsDigitized - load digitized sounds?
//                      NeedsMusic - load music?
//

#include "wl_def.h"
#include "dbopl.h"

#define ORIGSAMPLERATE 7042

static waveform_t FMGenerator;
static waveform_t PCGenerator;

globalsoundpos channelSoundPos[N_CHANNELS];

//      Global variables
        boolean         SoundPositioned;
        SDMode          SoundMode = sdm_AdLib;
        SMMode          MusicMode = smm_AdLib;
        SDSMode         DigiMode = sds_SoundBlaster;
static  byte          **SoundTable;
        int             DigiMap[LASTSOUND];
        int             DigiChannel[STARTMUSIC - STARTDIGISOUNDS];

//      Internal variables
static  boolean                 SD_Started;
static  boolean                 nextsoundpos;
static  soundnames              SoundNumber;
static  soundnames              DigiNumber;
static  word                    SoundPriority;
static  word                    DigiPriority;
static  int                     LeftPosition;
static  int                     RightPosition;

        word                    NumDigi;
        digiinfo                *DigiList;
static  boolean                 DigiPlaying;

//      PC Sound variables
static  volatile byte           pcLastSample;
static  byte * volatile         pcSound;
static  longword                pcLengthLeft;

//      AdLib variables
static  byte * volatile         alSound;
static  byte                    alBlock;
static  longword                alLengthLeft;
static  longword                alTimeCount;
static  Instrument              alZeroInst;

//      Sequencer variables
static  volatile boolean        sqActive;
static  word                   *sqHack;
static  word                   *sqHackPtr;
static  int                     sqHackLen;
static  int                     sqHackSeqLen;
static  longword                sqHackTime;

static const int oplChip = 0;

void Delay (int32_t wolfticks)
{
    if (wolfticks > 0)
        VL_Wait ((wolfticks * 100) / 7);
}


static void SDL_SoundFinished(void)
{
	SoundNumber   = (soundnames)0;
	SoundPriority = 0;
}

static void
SDL_StartPC(void)
{
    mixer_ch_play(N_CHANNELS+2, &PCGenerator);
}

///////////////////////////////////////////////////////////////////////////
//
//      SDL_PCPlaySound() - Plays the specified sound on the PC speaker
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_PCPlaySound(PCSound *sound)
{
        pcLastSample = (byte)-1;
        pcLengthLeft = sound->common.length;
        pcSound = sound->data;
}

///////////////////////////////////////////////////////////////////////////
//
//      SDL_PCStopSound() - Stops the current sound playing on the PC Speaker
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_PCStopSound(void)
{
        pcSound = 0;
}

///////////////////////////////////////////////////////////////////////////
//
//      SDL_ShutPC() - Turns off the pc speaker
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_ShutPC(void)
{
    pcSound = 0;
    mixer_ch_stop(N_CHANNELS+2);
}

// Adapted from Chocolate Doom (chocolate-doom/pcsound/pcsound_sdl.c)
#define SQUARE_WAVE_AMP 0x2000

static void SDL_PCMixCallback(void *ctx, samplebuffer_t *sbuf, int wpos, int len, bool seeking)
{
    static int current_remaining = 0;
    static int current_freq = 0;
    static int phase_offset = 0;

    int16_t *bufptr;
    int16_t this_value;
    int i;

    bufptr = samplebuffer_append(sbuf, len);

    // Fill the output buffer

    for (i=0; i<len; ++i)
    {
        // Has this sound expired? If so, retrieve the next frequency

        while (current_remaining == 0) 
        {
            phase_offset = 0;

            // Get the next frequency to play

            if(pcSound)
            {
                // The PC speaker sample rate is 140Hz (see SDL_t0SlowAsmService)
                current_remaining = param_samplerate / 140;

                if(*pcSound!=pcLastSample)
                {
                    pcLastSample=*pcSound;
					
                    if(pcLastSample)
                        // The PC PIC counts down at 1.193180MHz
                        // So pwm_freq = counter_freq / reload_value
                        // reload_value = pcLastSample * 60 (see SDL_DoFX)
                        current_freq = 1193180 / (pcLastSample * 60);
                    else
                        current_freq = 0;
						
                }
                pcSound++;
                pcLengthLeft--;
                if(!pcLengthLeft)
                {
                    pcSound=0;
                    SoundNumber=(soundnames)0;
                    SoundPriority=0;
                }
            }
            else
            {	
                current_freq = 0;
                current_remaining = 1;
            }
        }

        // Set the value for this sample.

        if (current_freq == 0)
        {
            // Silence

            this_value = 0;
        }
        else 
        {
            int frac;

            // Determine whether we are at a peak or trough in the current
            // sound.  Multiply by 2 so that frac % 2 will give 0 or 1 
            // depending on whether we are at a peak or trough.

            frac = (phase_offset * current_freq * 2) / param_samplerate;

            if ((frac % 2) == 0) 
            {
                this_value = SQUARE_WAVE_AMP;
            }
            else
            {
                this_value = -SQUARE_WAVE_AMP;
            }

            ++phase_offset;
        }

        --current_remaining;

        // Use the same value for the left and right channels.

        *bufptr = this_value;

        bufptr++;
    }
}

void
SD_StopDigitized(void)
{
    DigiPlaying = false;
    DigiNumber = (soundnames) 0;
    DigiPriority = 0;
    SoundPositioned = false;
    if ((DigiMode == sds_PC) && (SoundMode == sdm_PC))
        SDL_SoundFinished();

    switch (DigiMode)
    {
        case sds_PC:
            SDL_PCStopSound();
            break;
        case sds_SoundBlaster:
            for (int i = 0; i < N_CHANNELS; i++)
              mixer_ch_stop(i);
            break;
    }
}

int SD_GetChannelForDigi(int which)
{
    int channel = DigiChannel[which];
    if (channel != -1)
        return channel;

    uint64_t dist = 0, curdist;
    uint64_t cur = get_ticks();
    for (int i = 2; i < N_CHANNELS; i++)
    {
        if (!channelSoundPos[i].valid)
            return i;
        curdist = cur - channelSoundPos[i].started;
        if (curdist > dist)
        {
            dist = curdist;
            channel = i;
        }
    }
    return channel;
}

void SD_SetPosition(int channel, int leftpos, int rightpos)
{
    if((leftpos < 0) || (leftpos > 15) || (rightpos < 0) || (rightpos > 15)
            || ((leftpos == 15) && (rightpos == 15)))
        Quit("SD_SetPosition: Illegal position");

    switch (DigiMode)
    {
        case sds_SoundBlaster:
            mixer_ch_set_vol(channel, (((15 - leftpos) << 4) + 15)/15.0f,
                (((15 - rightpos) << 4) + 15)/15.0f);
            break;
    }
}

int16_t GetSample(float csample, byte *samples, int size)
{
    float s0=0, s1=0, s2=0;
    int cursample = (int) csample;
    float sf = csample - (float) cursample;

    if(cursample-1 >= 0) s0 = (float) (samples[cursample-1] - 128);
    s1 = (float) (samples[cursample] - 128);
    if(cursample+1 < size) s2 = (float) (samples[cursample+1] - 128);

    float val = s0*sf*(sf-1)/2 - s1*(sf*sf-1) + s2*(sf+1)*sf/2;
    int32_t intval = (int32_t) (val * 256);
    if(intval < -32768) intval = -32768;
    else if(intval > 32767) intval = 32767;
    return (int16_t) intval;
}

static void SD_ReadSample(void *ctx, samplebuffer_t *sbuf, int wpos, int wlen, bool seeking)
{
  int8_t *ptr = samplebuffer_append(sbuf, wlen);
  memcpy(ptr, ctx + wpos, wlen);
}

int SD_PlayDigitized(word which,int leftpos,int rightpos)
{
    if (!DigiMode)
        return 0;

    if (which >= NumDigi)
        Quit("SD_PlayDigitized: bad sound number %i", which);

    DigiPlaying = true;

    int channel = SD_GetChannelForDigi(which);

    waveform_t *sample = &channelSoundPos[channel].wave;

    if (which != channelSoundPos[channel].which)
    {
        mixer_ch_stop(channel);

        channelSoundPos[channel].which = which;

        sample->ctx = PM_GetSoundPage(DigiList[which].startpage);
        sample->len = DigiList[which].length;
    }

    mixer_ch_play(channel, sample);
    channelSoundPos[channel].valid = 1;
    channelSoundPos[channel].started = get_ticks();
    SD_SetPosition(channel, leftpos,rightpos);

    return channel;
}

void
SD_SetDigiDevice(SDSMode mode)
{
    if (mode == DigiMode)
        return;

    SD_StopDigitized();

    DigiMode = mode;
}

void
SDL_SetupDigi(void)
{
    // Correct padding enforced by PM_Startup()
    word *soundInfoPage = (word *) (void *) PM_GetPage(ChunksInFile-1);
    NumDigi = (word) PM_GetPageSize(ChunksInFile - 1) / 4;

    DigiList = SafeMalloc(NumDigi * sizeof(*DigiList));
    int i,page;
    for(i = 0; i < NumDigi; i++)
    {
        // Calculate the size of the digi from the sizes of the pages between
        // the start page and the start page of the next sound

        DigiList[i].startpage = swapword(soundInfoPage[i * 2]);
        if((int) DigiList[i].startpage >= ChunksInFile - 1)
        {
            NumDigi = i;
            break;
        }

        int lastPage;
        if(i < NumDigi - 1)
        {
            lastPage = swapword(soundInfoPage[i * 2 + 2]);
            if(lastPage == 0 || lastPage + PMSoundStart > ChunksInFile - 1) lastPage = ChunksInFile - 1;
            else lastPage += PMSoundStart;
        }
        else lastPage = ChunksInFile - 1;

        int size = 0;
        for(page = PMSoundStart + DigiList[i].startpage; page < lastPage; page++)
            size += PM_GetPageSize(page);

        // Don't include padding of sound info page, if padding was added
        if(lastPage == ChunksInFile - 1 && PMSoundInfoPagePadded) size--;

        // Patch lower 16-bit of size with size from sound info page.
        // The original VSWAP contains padding which is included in the page size,
        // but not included in the 16-bit size. So we use the more precise value.
        if((size & 0xffff0000) != 0 && (size & 0xffff) < swapword(soundInfoPage[i * 2 + 1]))
            size -= 0x10000;
        size = (size & 0xffff0000) | swapword(soundInfoPage[i * 2 + 1]);

        DigiList[i].length = size;
    }

    for(i = 0; i < LASTSOUND; i++)
    {
        DigiMap[i] = -1;
        DigiChannel[i] = -1;
    }
}

//      AdLib Code

///////////////////////////////////////////////////////////////////////////
//
//      SDL_ALStopSound() - Turns off any sound effects playing through the
//              AdLib card
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_ALStopSound(void)
{
    alSound = 0;
    alOut(alFreqH + 0, 0);
}

static void
SDL_AlSetFXInst(Instrument *inst)
{
    byte c,m;

    m = 0;      // modulator cell for channel 0
    c = 3;      // carrier cell for channel 0
    alOut(m + alChar,inst->mChar);
    alOut(m + alScale,inst->mScale);
    alOut(m + alAttack,inst->mAttack);
    alOut(m + alSus,inst->mSus);
    alOut(m + alWave,inst->mWave);
    alOut(c + alChar,inst->cChar);
    alOut(c + alScale,inst->cScale);
    alOut(c + alAttack,inst->cAttack);
    alOut(c + alSus,inst->cSus);
    alOut(c + alWave,inst->cWave);

    // Note: Switch commenting on these lines for old MUSE compatibility
//    alOutInIRQ(alFeedCon,inst->nConn);
    alOut(alFeedCon,0);
}

///////////////////////////////////////////////////////////////////////////
//
//      SDL_ALPlaySound() - Plays the specified sound on the AdLib card
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_ALPlaySound(AdLibSound *sound)
{
    Instrument      *inst;
    byte            *data;

    SDL_ALStopSound();

    alLengthLeft = sound->common.length;
    data = sound->data;
    alBlock = ((sound->block & 7) << 2) | 0x20;
    inst = &sound->inst;

    if (!(inst->mSus | inst->cSus))
    {
        Quit("SDL_ALPlaySound() - Bad instrument");
    }

    SDL_AlSetFXInst(inst);
    alSound = (byte *)data;
}

static bool alStarted = false;

///////////////////////////////////////////////////////////////////////////
//
//      SDL_ShutAL() - Shuts down the AdLib card for sound effects
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_ShutAL(void)
{
    if (alStarted)
    {
        alSound = 0;
        alOut(alEffects,0);
        alOut(alFreqH + 0,0);
        SDL_AlSetFXInst(&alZeroInst);
        mixer_ch_stop(N_CHANNELS+0);
        mixer_ch_stop(N_CHANNELS+1);
        alStarted = false;
    }
}

///////////////////////////////////////////////////////////////////////////
//
//      SDL_StartAL() - Starts up the AdLib card for sound effects
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_StartAL(void)
{
    if (!alStarted)
    {
        alOut(alEffects, 0);
        SDL_AlSetFXInst(&alZeroInst);
        mixer_ch_play(N_CHANNELS+0, &FMGenerator);
        alStarted = true;
    }
}

static void SDL_SyncAL(void)
{
    if (SoundMode == sdm_AdLib || MusicMode == smm_AdLib)
        SDL_StartAL();
    else
        SDL_ShutAL();
}

////////////////////////////////////////////////////////////////////////////
//
//      SDL_ShutDevice() - turns off whatever device was being used for sound fx
//
////////////////////////////////////////////////////////////////////////////
static void
SDL_ShutDevice(void)
{
    switch (SoundMode)
    {
        case sdm_PC:
            SDL_ShutPC();
            break;
        default:
            break;
    }
    SoundMode = sdm_Off;
    SDL_SyncAL();
}

///////////////////////////////////////////////////////////////////////////
//
//      SDL_StartDevice() - turns on whatever device is to be used for sound fx
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_StartDevice(void)
{
    switch (SoundMode)
    {
        case sdm_PC:
            SDL_StartPC();
            break;
        case sdm_AdLib:
            SDL_SyncAL();
            break;
    }
    SoundNumber = (soundnames) 0;
    SoundPriority = 0;
}

//      Public routines

///////////////////////////////////////////////////////////////////////////
//
//      SD_SetSoundMode() - Sets which sound hardware to use for sound effects
//
///////////////////////////////////////////////////////////////////////////
boolean
SD_SetSoundMode(SDMode mode)
{
    boolean result = false;
    word    tableoffset;

    SD_StopSound();

    switch (mode)
    {
        case sdm_Off:
            tableoffset = STARTADLIBSOUNDS;
            result = true;
            break;
        case sdm_PC:
            tableoffset = STARTPCSOUNDS;
            result = true;
            break;
        case sdm_AdLib:
            tableoffset = STARTADLIBSOUNDS;
            result = true;
            break;
        default:
            Quit("SD_SetSoundMode: Invalid sound mode %i", mode);
            return false;
    }
    SoundTable = &audiosegs[tableoffset];

    if (result && (mode != SoundMode))
    {
        SDL_ShutDevice();
        SoundMode = mode;
        SDL_StartDevice();
    }

    return(result);
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_SetMusicMode() - sets the device to use for background music
//
///////////////////////////////////////////////////////////////////////////
boolean
SD_SetMusicMode(SMMode mode)
{
    boolean result = false;

    SD_FadeOutMusic();
    while (SD_MusicPlaying())
        VL_Wait(5);

    switch (mode)
    {
        case smm_Off:
            result = true;
            break;
        case smm_AdLib:
            result = true;
            break;
    }

    if (result)
    {
        MusicMode = mode;
        SDL_SyncAL();
    }


    return(result);
}

int numreadysamples = 0;
byte *curAlSound = 0;
byte *curAlSoundPtr = 0;
longword curAlLengthLeft = 0;
int soundTimeCounter = 5;
int samplesPerMusicTick;

void SDL_IMFMusicPlayer(void *ctx, samplebuffer_t *sbuf, int wpos, int len, bool seeking)
{
    int sampleslen = len;
    int16_t *stream16 = samplebuffer_append(sbuf, len);

    while(1)
    {
        if(numreadysamples)
        {
            if(numreadysamples<sampleslen)
            {
                YM3812UpdateOne(oplChip, stream16, numreadysamples);
                stream16 += numreadysamples;
                sampleslen -= numreadysamples;
            }
            else
            {
                YM3812UpdateOne(oplChip, stream16, sampleslen);
                numreadysamples -= sampleslen;
                return;
            }
        }
        soundTimeCounter--;
        if(!soundTimeCounter)
        {
            soundTimeCounter = 5;
            if(curAlSound != alSound)
            {
                curAlSound = curAlSoundPtr = alSound;
                curAlLengthLeft = alLengthLeft;
            }
            if(curAlSound)
            {
                if(*curAlSoundPtr)
                {
                    alOut(alFreqL, *curAlSoundPtr);
                    alOut(alFreqH, alBlock);
                }
                else alOut(alFreqH, 0);
                curAlSoundPtr++;
                curAlLengthLeft--;
                if(!curAlLengthLeft)
                {
                    curAlSound = alSound = 0;
                    SoundNumber = (soundnames) 0;
                    SoundPriority = 0;
                    alOut(alFreqH, 0);
                }
            }
        }
        if(sqActive)
        {
            do
            {
                if(sqHackTime > alTimeCount) break;
                sqHackTime = alTimeCount + swapword(*(sqHackPtr+1));
                alOut(*(byte *) sqHackPtr, *(((byte *) sqHackPtr)+1));
                sqHackPtr += 2;
                sqHackLen -= 4;
            }
            while(sqHackLen>0);
            alTimeCount++;
            if(!sqHackLen)
            {
                sqHackPtr = sqHack;
                sqHackLen = sqHackSeqLen;
                sqHackTime = 0;
                alTimeCount = 0;
            }
        }
        numreadysamples = samplesPerMusicTick;
    }
}

void SD_Poll(void)
{
    if (audio_can_write()) {
      short *buf = audio_write_begin();
      mixer_poll(buf, audio_get_buffer_length());
      audio_write_end();
      for (int i = 0; i < N_CHANNELS; i++)
        channelSoundPos[i].valid = mixer_ch_playing(i);
  }
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_Startup() - starts up the Sound Mgr
//              Detects all additional sound hardware and installs my ISR
//
///////////////////////////////////////////////////////////////////////////
void
SD_Startup(void)
{
    int     i;

    if (SD_Started)
        return;

    audio_init(param_samplerate, 4);
    mixer_init(N_CHANNELS + 3);

    // Init sfx generators

    for (int i = 0; i < lengthof(channelSoundPos); i++)
    {
        waveform_t *wave = &channelSoundPos[i].wave;
        wave->bits = 8;
        wave->channels = 1;
        wave->frequency = ORIGSAMPLERATE;
        wave->loop_len = 0;
        wave->name = "sfx";
        wave->len = 0;
        wave->read = SD_ReadSample;
        wave->ctx = NULL;

        channelSoundPos[i].which = -1;
    }

    // Init music

    samplesPerMusicTick = param_samplerate / 700;    // SDL_t0FastAsmService played at 700Hz

    if(YM3812Init(1,3579545,param_samplerate))
    {
        printf("Unable to create virtual OPL!!\n");
    }

    for(i=1;i<0xf6;i++)
        YM3812Write(oplChip,i,0);

    YM3812Write(oplChip,1,0x20); // Set WSE=1
//    YM3812Write(0,8,0); // Set CSM=0 & SEL=0		 // already set in for statement

    FMGenerator.len = WAVEFORM_UNKNOWN_LEN;
    FMGenerator.bits = 16;
    FMGenerator.channels = 1;
    FMGenerator.name = "IMF";
    FMGenerator.loop_len = 0;
    FMGenerator.frequency = param_samplerate;
    FMGenerator.read = SDL_IMFMusicPlayer;

    alTimeCount = 0;
	
    // Add PC speaker sound mixer
    PCGenerator.len = WAVEFORM_UNKNOWN_LEN;
    PCGenerator.bits = 16;
    PCGenerator.channels = 1;
    PCGenerator.name = "PC";
    PCGenerator.loop_len = 0;
    PCGenerator.frequency = param_samplerate;
    PCGenerator.read = SDL_PCMixCallback;

    SD_SetSoundMode(sdm_AdLib);
    SD_SetMusicMode(smm_AdLib);

    SDL_SetupDigi();

    SD_Started = true;
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_Shutdown() - shuts down the Sound Mgr
//              Removes sound ISR and turns off whatever sound hardware was active
//
///////////////////////////////////////////////////////////////////////////
void
SD_Shutdown(void)
{
    if (!SD_Started)
        return;

    SD_MusicOff();
    SD_StopSound();

    free (DigiList);
    DigiList = NULL;

    SD_Started = false;
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_PositionSound() - Sets up a stereo imaging location for the next
//              sound to be played. Each channel ranges from 0 to 15.
//
///////////////////////////////////////////////////////////////////////////
void
SD_PositionSound(int leftvol,int rightvol)
{
    LeftPosition = leftvol;
    RightPosition = rightvol;
    nextsoundpos = true;
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_PlaySound() - plays the specified sound on the appropriate hardware
//
///////////////////////////////////////////////////////////////////////////
boolean
SD_PlaySound(soundnames sound)
{
    boolean         ispos;
    SoundCommon     *s;
    int             lp,rp;

    lp = LeftPosition;
    rp = RightPosition;
    LeftPosition = 0;
    RightPosition = 0;

    ispos = nextsoundpos;
    nextsoundpos = false;

    if (sound == -1 || (DigiMode == sds_Off && SoundMode == sdm_Off))
        return 0;

    s = (SoundCommon *) SoundTable[sound];

    if ((SoundMode != sdm_Off) && !s)
            Quit("SD_PlaySound() - Uncached sound");

    if ((DigiMode != sds_Off) && (DigiMap[sound] != -1))
    {
        if ((DigiMode == sds_PC) && (SoundMode == sdm_PC))
        {
            if (s->priority < SoundPriority)
                return 0;

            SDL_PCStopSound();

            SD_PlayDigitized(DigiMap[sound],lp,rp);
            SoundPositioned = ispos;
            SoundNumber = sound;
            SoundPriority = s->priority;
        }
        else
        {
#ifdef NOTYET
            if (s->priority < DigiPriority)
                return(false);
#endif

            int channel = SD_PlayDigitized(DigiMap[sound], lp, rp);
            SoundPositioned = ispos;
            DigiNumber = sound;
            DigiPriority = s->priority;
            return channel + 1;
        }

        return(true);
    }

    if (SoundMode == sdm_Off)
        return 0;

    if (!s->length)
        Quit("SD_PlaySound() - Zero length sound");
    if (s->priority < SoundPriority)
        return 0;

    switch (SoundMode)
    {
        case sdm_PC:
            SDL_PCPlaySound((PCSound *)s);
            break;
        case sdm_AdLib:
#ifdef ADDEDFIX // 2
            curAlSound = alSound = 0;                // Tricob
            alOut(alFreqH, 0);
#endif
            SDL_ALPlaySound((AdLibSound *)s);
            break;
    }

    SoundNumber = sound;
    SoundPriority = s->priority;

    return 0;
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_SoundPlaying() - returns the sound number that's playing, or 0 if
//              no sound is playing
//
///////////////////////////////////////////////////////////////////////////
word
SD_SoundPlaying(void)
{
    boolean result = false;

    switch (SoundMode)
    {
        case sdm_PC:
            result = pcSound? true : false;
            break;
        case sdm_AdLib:
            result = alSound? true : false;
            break;
    }

    if (result)
        return(SoundNumber);
    else
        return(false);
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_StopSound() - if a sound is playing, stops it
//
///////////////////////////////////////////////////////////////////////////
void
SD_StopSound(void)
{
    if (DigiPlaying)
        SD_StopDigitized();

    switch (SoundMode)
    {
        case sdm_PC:
            SDL_PCStopSound();
            break;
        case sdm_AdLib:
            SDL_ALStopSound();
            break;
    }

    SoundPositioned = false;

    SDL_SoundFinished();
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_WaitSoundDone() - waits until the current sound is done playing
//
///////////////////////////////////////////////////////////////////////////
void
SD_WaitSoundDone(void)
{
    while (SD_SoundPlaying())
        VL_Wait(5);
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_MusicOn() - turns on the sequencer
//
///////////////////////////////////////////////////////////////////////////
void
SD_MusicOn(void)
{
    sqActive = true;
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_MusicOff() - turns off the sequencer and any playing notes
//      returns the last music offset for music continue
//
///////////////////////////////////////////////////////////////////////////
int
SD_MusicOff(void)
{
    word    i;

    sqActive = false;
    switch (MusicMode)
    {
        case smm_AdLib:
            alOut(alEffects, 0);
            for (i = 0;i < sqMaxTracks;i++)
                alOut(alFreqH + i + 1, 0);
            break;
    }

    return (int) (sqHackPtr-sqHack);
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_StartMusic() - starts playing the music pointed to
//
///////////////////////////////////////////////////////////////////////////
void
SD_StartMusic(int chunk)
{
    SD_MusicOff();

    if (MusicMode == smm_AdLib)
    {
        int32_t chunkLen = CA_CacheAudioChunk(chunk);
        sqHack = (word *)(void *) audiosegs[chunk];     // alignment is correct
        if(*sqHack == 0) sqHackLen = sqHackSeqLen = chunkLen;
        else sqHackLen = sqHackSeqLen = swapword(*sqHack++);
        sqHackPtr = sqHack;
        sqHackTime = 0;
        alTimeCount = 0;
        SD_MusicOn();
    }
}

void
SD_ContinueMusic(int chunk, int startoffs)
{
    int i;

    SD_MusicOff();

    if (MusicMode == smm_AdLib)
    {
        int32_t chunkLen = CA_CacheAudioChunk(chunk);
        sqHack = (word *)(void *) audiosegs[chunk];     // alignment is correct
        if(*sqHack == 0) sqHackLen = sqHackSeqLen = chunkLen;
        else sqHackLen = sqHackSeqLen = swapword(*sqHack++);
        sqHackPtr = sqHack;

        if(startoffs >= sqHackLen)
        {
#ifdef ADDEDFIX // 7                     // Andy, improved by Chris Chokan
            startoffs = 0;
#else
            Quit("SD_StartMusic: Illegal startoffs provided!");
#endif
        }

        // fast forward to correct position
        // (needed to reconstruct the instruments)

        for(i = 0; i < startoffs; i += 2)
        {
            byte reg = *(byte *)sqHackPtr;
            byte val = *(((byte *)sqHackPtr) + 1);
            if(reg >= 0xb1 && reg <= 0xb8) val &= 0xdf;           // disable play note flag
            else if(reg == 0xbd) val &= 0xe0;                     // disable drum flags

            alOut(reg,val);
            sqHackPtr += 2;
            sqHackLen -= 4;
        }
        sqHackTime = 0;
        alTimeCount = 0;

        SD_MusicOn();
    }
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_FadeOutMusic() - starts fading out the music. Call SD_MusicPlaying()
//              to see if the fadeout is complete
//
///////////////////////////////////////////////////////////////////////////
void
SD_FadeOutMusic(void)
{
    switch (MusicMode)
    {
        case smm_AdLib:
            // DEBUG - quick hack to turn the music off
            SD_MusicOff();
            break;
    }
}

///////////////////////////////////////////////////////////////////////////
//
//      SD_MusicPlaying() - returns true if music is currently playing, false if
//              not
//
///////////////////////////////////////////////////////////////////////////
boolean
SD_MusicPlaying(void)
{
    boolean result;

    switch (MusicMode)
    {
        case smm_AdLib:
            result = sqActive;
            break;
        default:
            result = false;
            break;
    }

    return(result);
}
