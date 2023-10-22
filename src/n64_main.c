#include "n64_main.h"
#include "id_us.h"
#include "wl_def.h"
#include <usb.h>
#include <malloc.h>

#define SRAM_START_ADDR  0xA8000000
#define PI_BSD_DOM2_LAT ((volatile uint32_t*) 0xA4600024)
#define PI_BSD_DOM2_PWD ((volatile uint32_t*) 0xA4600028)
#define PI_BSD_DOM2_PGS ((volatile uint32_t*) 0xA460002C)
#define PI_BSD_DOM2_RLS ((volatile uint32_t*) 0xA4600030)
#define SAVE_ADDR (SRAM_START_ADDR+sizeof(N64Config))

void N64_Init(void)
{
    debug_init_isviewer();
    debug_init_usblog();
    dfs_init(DFS_DEFAULT_LOCATION);

    rdpq_init();

#if DEBUG_RDP
    rdpq_debug_start();
    rdpq_debug_log(true);
#endif

    joypad_init();
    usb_initialize();

    disable_interrupts();
    *PI_BSD_DOM2_LAT = 0x5;
    *PI_BSD_DOM2_PWD = 0xc;
    *PI_BSD_DOM2_PGS = 0xd;
    *PI_BSD_DOM2_RLS = 0x2;
    enable_interrupts();
}

typedef struct  __attribute__((aligned(16))) {
    word magic;
    uint8_t  sd;
    uint8_t  sm;
    uint8_t DigiMode;
    uint8_t MoveMode;
    uint8_t viewsize;
    uint8_t stickadjustment;
    uint8_t mouseadjustment;
    uint8_t autorun;
    int8_t buttonjoy[16];
    int8_t buttonmouse[2];
    HighScore scores[MaxScores];
} N64Config;

void N64_LoadConfig(void)
{
    N64Config config __attribute__((aligned(16)));

    data_cache_hit_invalidate(&config, sizeof config);
    dma_read_raw_async(&config, SRAM_START_ADDR, sizeof config);
    dma_wait();

    if (config.magic == 0xfefa)
    {
        memcpy(Scores, config.scores, sizeof config.scores);
        memcpy(buttonjoy, config.buttonjoy, sizeof config.buttonjoy);
        memcpy(buttonmouse, config.buttonmouse, sizeof config.buttonmouse);

        viewsize = config.viewsize;
        mouseadjustment = config.mouseadjustment;
        stickadjustment = config.stickadjustment;
        MoveMode = !!config.MoveMode;
        autorun = !!config.autorun;

        if(mouseadjustment<0) mouseadjustment=0;
        else if(mouseadjustment>9) mouseadjustment=9;

        if(viewsize<4) viewsize=4;
        else if(viewsize>21) viewsize=21;

        if(stickadjustment<0) stickadjustment=0;
        else if(stickadjustment>120) stickadjustment=120;

        MainMenu[6].active=1;
        MainItems.curpos=0;
    }
    else
    {
        config.sm = smm_AdLib;
        config.sd = sdm_AdLib;
        config.DigiMode = sds_SoundBlaster;
        viewsize = 20;                          // start with a good size
        mouseadjustment=5;
        stickadjustment=56;
        MoveMode = 0;
        autorun = false;
    }

    SD_SetMusicMode (config.sm);
    SD_SetSoundMode (config.sd);
    SD_SetDigiDevice (config.DigiMode);
}

void N64_SaveConfig(void)
{
    N64Config config __attribute__((aligned(16)));

    config.magic = 0xfefa;
    config.sd = SoundMode;
    config.sm = MusicMode;
    config.DigiMode = DigiMode;
    config.MoveMode = MoveMode;
    config.autorun = autorun;
    memcpy(config.scores, Scores, sizeof config.scores);
    memcpy(config.buttonjoy, buttonjoy, sizeof config.buttonjoy);
    memcpy(config.buttonmouse, buttonmouse, sizeof config.buttonmouse);
    config.viewsize = viewsize;
    config.mouseadjustment = mouseadjustment;
    config.stickadjustment = stickadjustment;

    data_cache_hit_writeback(&config, sizeof config);
    dma_write_raw_async(&config, SRAM_START_ADDR, sizeof config);
    dma_wait();
}

extern uint16_t curpal[256];

void Present(surface_t *surface)
{
    surface_t *cur = display_get();
    rdpq_attach_clear(cur, NULL);
    rdpq_set_mode_copy(false);
    rdpq_mode_tlut(TLUT_RGBA16);
    data_cache_hit_writeback(curpal, sizeof curpal);
    rdpq_tex_upload_tlut(curpal, 0, 256);
    rdpq_tex_blit(surface, 16, 8, NULL);
    rdpq_detach_wait();
    display_show(cur);
}

#define BUFSIZE 4096

typedef struct  __attribute__((aligned(16))){
    byte data[BUFSIZE];
    int offset;
    unsigned long pi;
} SramBuf;

int close_read_sram(void *cookie)
{
    free(cookie);
    return 0;
}

int read_sram(void *cookie, char *buf, int len)
{
    int count;
    int bufoffset = 0;
    SramBuf *b = cookie;

    dma_wait();

    while (len > 0)
    {
        if (b->offset == BUFSIZE)
        {
            data_cache_hit_invalidate(b->data, BUFSIZE);
            dma_read_raw_async(b->data, b->pi, BUFSIZE);
            b->offset = 0;
            b->pi += BUFSIZE;
            dma_wait();
        }
        count = MIN(len, BUFSIZE - b->offset);
        memcpy(&buf[bufoffset], &b->data[b->offset], count);
        b->offset += count;
        bufoffset += count;
        len -= count;
    }
    return 0;
}

FILE *N64_ReadSave(void)
{
    SramBuf *buf = memalign(16, sizeof(SramBuf));
    assertf(buf, "out of memory");
    buf->offset = 0;
    buf->pi = SAVE_ADDR + BUFSIZE;
    data_cache_hit_invalidate(buf->data, BUFSIZE);
    dma_read_raw_async(buf->data, 0, BUFSIZE);
    return funopen(buf, read_sram, NULL, NULL, close_read_sram);
}

int write_sram(void *cookie, const char *buf, int len)
{
    int count;
    int bufoffset = 0;
    SramBuf *b = cookie;
    while (len > 0)
    {
        count = MIN(len, BUFSIZE - b->offset);
        memcpy(&b->data[b->offset], &buf[bufoffset], count);
        b->offset += count;
        bufoffset += count;
        len -= count;
        if (b->offset == BUFSIZE)
        {
            data_cache_hit_writeback(b->data, BUFSIZE);
            dma_write_raw_async(b->data, b->pi, BUFSIZE);
            b->offset = 0;
            b->pi += BUFSIZE;
            dma_wait();
        }
    }
    return 0;
}

int close_write_sram(void *cookie)
{
    SramBuf *b = cookie;
    if (b->offset > 0)
    {
        data_cache_hit_writeback(b->data, BUFSIZE);
        dma_write_raw_async(b->data, b->pi, b->offset);
        dma_wait();
    }
    free(cookie);
    return 0;
}

FILE *N64_WriteSave(void)
{
    SramBuf *buf = memalign(16, sizeof(SramBuf));
    assertf(buf, "out of memory");
    buf->offset = 0;
    buf->pi = SAVE_ADDR;
    return funopen(buf, NULL, write_sram, NULL, close_write_sram);
}
