//-------------------------------------------------------------------------
/*
Copyright (C) 2016 EDuke32 developers and contributors

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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define MAXUSERQUOTES 6

extern int32_t quotebot, quotebotgoal;
extern int32_t user_quote_time[MAXUSERQUOTES];
extern int32_t minitext_lowercase;
extern int32_t minitext_yofs;

enum ScreenTextFlags_t {
    TEXT_XRIGHT          = 0x00000001,
    TEXT_XCENTER         = 0x00000002,
    TEXT_YBOTTOM         = 0x00000004,
    TEXT_YCENTER         = 0x00000008,
    TEXT_INTERNALSPACE   = 0x00000010,
    TEXT_TILESPACE       = 0x00000020,
    TEXT_INTERNALLINE    = 0x00000040,
    TEXT_TILELINE        = 0x00000080,
    TEXT_XOFFSETZERO     = 0x00000100,
    TEXT_XJUSTIFY        = 0x00000200,
    TEXT_YOFFSETZERO     = 0x00000400,
    TEXT_YJUSTIFY        = 0x00000800,
    TEXT_LINEWRAP        = 0x00001000,
    TEXT_UPPERCASE       = 0x00002000,
    TEXT_INVERTCASE      = 0x00004000,
    TEXT_IGNOREESCAPE    = 0x00008000,
    TEXT_LITERALESCAPE   = 0x00010000,
    TEXT_BACKWARDS       = 0x00020000,
    TEXT_GAMETEXTNUMHACK = 0x00040000,
    TEXT_DIGITALNUMBER   = 0x00080000,
    TEXT_BIGALPHANUM     = 0x00100000,
    TEXT_GRAYFONT        = 0x00200000,
};

extern int32_t minitext_(int32_t x, int32_t y, const char *t, int32_t s, int32_t p, int32_t sb);
extern int32_t textsc(int32_t sc);

#define minitextshade(x, y, t, s, p, sb) minitext_(x,y,t,s,p,sb)
#define minitext(x, y, t, p, sb) minitext_(x,y,t,0,p,sb)
#define menutext(x,y,s,p,t) menutext_(x,y,s,p,OSD_StripColors(menutextbuf,t),10+16)
#define gametext(x,y,t,s,dabits) G_PrintGameText(0,STARTALPHANUM, x,y,t,s,0,dabits,0, 0, xdim-1, ydim-1, 65536, 0)
#define gametextscaled(x,y,t,s,dabits) G_PrintGameText(1,STARTALPHANUM, x,y,t,s,0,dabits,0, 0, xdim-1, ydim-1, 65536, 0)
#define gametextpal(x,y,t,s,p) G_PrintGameText(0,STARTALPHANUM, x,y,t,s,p,26,0, 0, xdim-1, ydim-1, 65536, 0)
#define gametextpalbits(x,y,t,s,p,dabits,a) G_PrintGameText(0,STARTALPHANUM, x,y,t,s,p,dabits,0, 0, xdim-1, ydim-1, 65536, a)
#define mpgametext(y, t, s, dabits) G_PrintGameText(4,STARTALPHANUM, 5,y,t,s,0,dabits,0, 0, xdim-1, ydim-1, 65536, 0);

extern int32_t G_GameTextLen(int32_t x, const char *t);
extern int32_t G_PrintGameText(int32_t hack, int32_t tile, int32_t x, int32_t y, const char *t, int32_t s, int32_t p,
                        int32_t o, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t z, int32_t a);

extern int32_t G_GetStringLineLength(const char *text, const char *end, const int32_t iter);
extern int32_t G_GetStringNumLines(const char *text, const char *end, const int32_t iter);
extern char* G_GetSubString(const char *text, const char *end, const int32_t iter, const int32_t length);
extern int32_t G_GetStringTile(int32_t font, char *t, int32_t f);
extern vec2_t G_ScreenTextSize(const int32_t font, int32_t x, int32_t y, const int32_t z, const int32_t blockangle, const char *str, const int32_t o, int32_t xspace, int32_t yline, int32_t xbetween, int32_t ybetween, const int32_t f, const int32_t x1, const int32_t y1, const int32_t x2, const int32_t y2);
extern void G_AddCoordsFromRotation(vec2_t *coords, const vec2_t *unitDirection, const int32_t magnitude);
extern vec2_t G_ScreenText(const int32_t font, int32_t x, int32_t y, const int32_t z, const int32_t blockangle, const int32_t charangle, const char *str, const int32_t shade, int32_t pal, int32_t o, int32_t alpha, int32_t xspace, int32_t yline, int32_t xbetween, int32_t ybetween, const int32_t f, int32_t x1, int32_t y1, int32_t x2, int32_t y2);
extern vec2_t G_ScreenTextShadow(int32_t sx, int32_t sy, const int32_t font, int32_t x, int32_t y, const int32_t z, const int32_t blockangle, const int32_t charangle, const char *str, const int32_t shade, int32_t pal, int32_t o, const int32_t alpha, int32_t xspace, int32_t yline, int32_t xbetween, int32_t ybetween, const int32_t f, int32_t x1, int32_t y1, int32_t x2, int32_t y2);

#ifdef __cplusplus
}
#endif
