#ifndef __N64_MAIN_H_
#define __N64_MAIN_H_

#include <libdragon.h>

void N64_Init(void);
void N64_LoadConfig(void);
void N64_SaveConfig(void);

void Present(surface_t *surface);

FILE *N64_ReadSave(void);
FILE *N64_WriteSave(void);

#endif

