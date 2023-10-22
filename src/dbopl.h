/*
 *  Copyright (C) 2002-2010  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

//typedef uintptr_t	Bitu;
//typedef intptr_t	Bits;
typedef uint32_t	Bitu;
typedef int32_t		Bits;
typedef uint32_t	Bit32u;
typedef int32_t		Bit32s;
typedef uint16_t	Bit16u;
typedef int16_t		Bit16s;
typedef uint8_t		Bit8u;
typedef int8_t		Bit8s;

#ifdef __cplusplus
extern "C" {
#endif
bool YM3812Init(int numChips, int clock, int rate);
void YM3812Write(int which, Bit32u reg, Bit8u val);
void YM3812UpdateOne(int which, int16_t *stream, int length);

#ifdef __cplusplus
}
#endif
