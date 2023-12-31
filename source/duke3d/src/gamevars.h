//-------------------------------------------------------------------------
/*
Copyright (C) 2010 EDuke32 developers and contributors

This file is part of EDuke32.

EDuke32 is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
//-------------------------------------------------------------------------

#ifndef gamevars_h_
#define gamevars_h_

#include "gamedef.h"

#define MAXGAMEVARS 2048 // must be a power of two
#define MAXVARLABEL 26

// store global game definitions
enum GamevarFlags_t
{
    GAMEVAR_PERPLAYER = 0x00000001,  // per-player variable
    GAMEVAR_PERACTOR  = 0x00000002,  // per-actor variable
    GAMEVAR_USER_MASK = (GAMEVAR_PERPLAYER | GAMEVAR_PERACTOR),
    GAMEVAR_RESET     = 0x00000008,  // INTERNAL, don't use
    GAMEVAR_DEFAULT   = 0x00000100,  // UNUSED, but always cleared for user-defined gamevars
    GAMEVAR_NODEFAULT = 0x00000400,  // don't reset on actor spawn
    GAMEVAR_SYSTEM    = 0x00000800,  // cannot change mode flags...(only default value)
    GAMEVAR_READONLY  = 0x00001000,  // values are read-only (no setvar allowed)
    GAMEVAR_INTPTR    = 0x00002000,  // plValues is a pointer to an int32_t
    GAMEVAR_SHORTPTR  = 0x00008000,  // plValues is a pointer to a short
    GAMEVAR_CHARPTR   = 0x00010000,  // plValues is a pointer to a char
    GAMEVAR_PTR_MASK  = (GAMEVAR_INTPTR | GAMEVAR_SHORTPTR | GAMEVAR_CHARPTR),
    GAMEVAR_NORESET   = 0x00020000,  // var values are not reset when restoring map state
    GAMEVAR_SPECIAL   = 0x00040000,  // flag for structure member shortcut vars
    GAMEVAR_NOMULTI   = 0x00080000,  // don't attach to multiplayer packets
};

#if !defined LUNATIC

// Alignments for per-player and per-actor variables.
#define PLAYER_VAR_ALIGNMENT (sizeof(intptr_t))
#define ACTOR_VAR_ALIGNMENT 16

# define MAXGAMEARRAYS (MAXGAMEVARS>>2) // must be strictly smaller than MAXGAMEVARS
# define MAXARRAYLABEL MAXVARLABEL

enum GamearrayFlags_t
{
    GAMEARRAY_READONLY  = 0x00001000,
    GAMEARRAY_WARN      = 0x00002000,
    GAMEARRAY_NORMAL    = 0x00004000,
    GAMEARRAY_OFCHAR    = 0x00000001,
    GAMEARRAY_OFSHORT   = 0x00000002,
    GAMEARRAY_OFINT     = 0x00000004,
    GAMEARRAY_RESET     = 0x00000008,
    GAMEARRAY_TYPE_MASK = GAMEARRAY_OFCHAR | GAMEARRAY_OFSHORT | GAMEARRAY_OFINT,
    GAMEARRAY_RESTORE   = 0x00000010,
    GAMEARRAY_VARSIZE   = 0x00000020,
    GAMEARRAY_STRIDE2   = 0x00000100,
};

#pragma pack(push,1)
typedef struct
{
    union {
        intptr_t  global;
        intptr_t *pValues;  // array of values when 'per-player', or 'per-actor'
    };
    intptr_t  defaultValue;
    uintptr_t flags;
    char *    szLabel;
} gamevar_t;

typedef struct
{
    char *    szLabel;
    intptr_t *pValues;  // array of values
    intptr_t  size;
    uintptr_t flags;
} gamearray_t;
#pragma pack(pop)

# define GAR_ELTSZ (sizeof(aGameArrays[0].pValues[0]))

extern gamevar_t   aGameVars[MAXGAMEVARS];
extern gamearray_t aGameArrays[MAXGAMEARRAYS];
extern int32_t     g_gameVarCount;
extern int32_t     g_gameArrayCount;

int __fastcall Gv_GetArrayValue(int const id, int index);
int __fastcall Gv_GetVar(int id, int spriteNum, int playerNum);
void __fastcall Gv_SetVar(int const id, int const lValue, int const spriteNum, int const playerNum);
int __fastcall Gv_GetVarX(int id);
void __fastcall Gv_GetManyVars(int const count, int32_t * const rv);
void __fastcall Gv_SetVarX(int const id, int const lValue);

int Gv_GetVarByLabel(const char *szGameLabel,int const lDefault,int const spriteNum,int const playerNum);
int32_t Gv_NewArray(const char *pszLabel,void *arrayptr,intptr_t asize,uint32_t dwFlags);
int32_t Gv_NewVar(const char *pszLabel,intptr_t lValue,uint32_t dwFlags);

static FORCE_INLINE void A_ResetVars(int const spriteNum)
{
    for (bssize_t i = 0; i < g_gameVarCount; ++i)
    {
        if ((aGameVars[i].flags & (GAMEVAR_PERACTOR | GAMEVAR_NODEFAULT)) != GAMEVAR_PERACTOR)
            continue;
        aGameVars[i].pValues[spriteNum] = aGameVars[i].defaultValue;
    }
}

void Gv_DumpValues(void);
void Gv_InitWeaponPointers(void);
void Gv_RefreshPointers(void);
void Gv_ResetVars(void);
int Gv_ReadSave(int32_t kFile);
void Gv_WriteSave(FILE *fil);
#else
extern int32_t g_noResetVars;
extern LUNATIC_CB void (*A_ResetVars)(int32_t spriteNum);
#endif

void Gv_ResetSystemDefaults(void);
void Gv_Init(void);
void Gv_FinalizeWeaponDefaults(void);

#if !defined LUNATIC
#define VM_GAMEVAR_OPERATOR(func, operator)                                                                                 \
    static FORCE_INLINE void __fastcall func(int const id, int32_t const operand)                                                  \
    {                                                                                                                       \
        switch (aGameVars[id].flags & (GAMEVAR_USER_MASK | GAMEVAR_PTR_MASK))                                               \
        {                                                                                                                   \
            default: aGameVars[id].global operator operand; break;                                                          \
            case GAMEVAR_PERPLAYER:                                                                                         \
                if (EDUKE32_PREDICT_FALSE((unsigned)vm.playerNum > MAXPLAYERS - 1))                                         \
                    break;                                                                                                  \
                aGameVars[id].pValues[vm.playerNum] operator operand;                                                       \
                break;                                                                                                      \
            case GAMEVAR_PERACTOR:                                                                                          \
                if (EDUKE32_PREDICT_FALSE((unsigned)vm.spriteNum > MAXSPRITES - 1))                                         \
                    break;                                                                                                  \
                aGameVars[id].pValues[vm.spriteNum] operator operand;                                                       \
                break;                                                                                                      \
            case GAMEVAR_INTPTR: *((int32_t *)aGameVars[id].global) operator(int32_t) operand; break;                       \
            case GAMEVAR_SHORTPTR: *((int16_t *)aGameVars[id].global) operator(int16_t) operand; break;                     \
            case GAMEVAR_CHARPTR: *((uint8_t *)aGameVars[id].global) operator(uint8_t) operand; break;                      \
        }                                                                                                                   \
    }

#if defined(__arm__) || defined(LIBDIVIDE_ALWAYS)
static FORCE_INLINE void __fastcall Gv_DivVar(int const id, int32_t const operand)
{
    if (EDUKE32_PREDICT_FALSE((aGameVars[id].flags & GAMEVAR_PERPLAYER && (unsigned) vm.playerNum > MAXPLAYERS - 1) ||
        (aGameVars[id].flags & GAMEVAR_PERACTOR && (unsigned) vm.spriteNum > MAXSPRITES - 1)))
        return;

    static libdivide_s32_t sdiv;
    static int32_t lastValue;
    libdivide_s32_t *dptr = ((unsigned) operand < DIVTABLESIZE) ? (libdivide_s32_t *) &divtable32[operand] : &sdiv;
    intptr_t *iptr = &aGameVars[id].global;

    if (operand == lastValue || dptr != &sdiv)
        goto skip;

    sdiv = libdivide_s32_gen((lastValue = operand));

skip:
    switch (aGameVars[id].flags & (GAMEVAR_USER_MASK | GAMEVAR_PTR_MASK))
    {
        case GAMEVAR_PERPLAYER: iptr = &aGameVars[id].pValues[vm.playerNum];
        default: break;
        case GAMEVAR_PERACTOR: iptr = &aGameVars[id].pValues[vm.spriteNum]; break;
        case GAMEVAR_INTPTR:
            *((int32_t *)aGameVars[id].global) =
            (int32_t)libdivide_s32_do(*((int32_t *)aGameVars[id].global), dptr);
            return;
        case GAMEVAR_SHORTPTR:
            *((int16_t *)aGameVars[id].global) =
            (int16_t)libdivide_s32_do(*((int16_t *)aGameVars[id].global), dptr);
            return;
        case GAMEVAR_CHARPTR:
            *((uint8_t *)aGameVars[id].global) =
            (uint8_t)libdivide_s32_do(*((uint8_t *)aGameVars[id].global), dptr);
            return;
    }

    *iptr = libdivide_s32_do(*iptr, dptr);
}
#else
VM_GAMEVAR_OPERATOR(Gv_DivVar, /= )
#endif

VM_GAMEVAR_OPERATOR(Gv_AddVar, +=)
VM_GAMEVAR_OPERATOR(Gv_SubVar, -=)
VM_GAMEVAR_OPERATOR(Gv_MulVar, *=)
VM_GAMEVAR_OPERATOR(Gv_ModVar, %=)
VM_GAMEVAR_OPERATOR(Gv_AndVar, &=)
VM_GAMEVAR_OPERATOR(Gv_XorVar, ^=)
VM_GAMEVAR_OPERATOR(Gv_OrVar, |=)

#undef VM_GAMEVAR_OPERATOR

#endif

#endif
