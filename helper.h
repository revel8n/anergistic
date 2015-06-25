// Copyright 2010 fail0verflow <master@fail0verflow.com>
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

#ifndef HELPER_H__
#define HELPER_H__

#include "types.h"

inline u32 reg_bit(u32 r, u32 i)
{
    i %= 128;

    return (ctx->regs[r]._u32[(i / 32)] >> (31 - (i & 0x1F))) & 1;
}

inline u8 reg_u8(u32 r, u32 i)
{
    i %= 16;

    return ctx->regs[r]._u8[(i & ~3) + (3 - (i & 3))];
}
inline s8 reg_s8(u32 r, u32 i)
{
    i %= 16;

    return ctx->regs[r]._s8[(i & ~3) + (3 - (i & 3))];
}

inline u16 reg_u16(u32 r, u32 i)
{
    i %= 8;

    return ctx->regs[r]._u16[(i & ~1) + (2 - (i & 1))];
}
inline s16 reg_s16(u32 r, u32 i)
{
    i %= 8;

    return ctx->regs[r]._s16[(i & ~1) + (2 - (i & 1))];
}

inline u32 reg_u32(u32 r, u32 i)
{
    i %= 4;

    return ctx->regs[r]._u32[i];
}
inline s32 reg_s32(u32 r, u32 i)
{
    i %= 4;

    return ctx->regs[r]._s32[i];
}

inline u64 reg_u64(u32 r, u32 i)
{
    i %= 2;

    return ((ctx->regs[r]._u64[i] & 0xFFFFFFFF) << 32) | ((ctx->regs[r]._u64[i] >> 32) & 0xFFFFFFFF);
}
inline s64 reg_s64(u32 r, u32 i)
{
    i %= 2;

    return ((ctx->regs[r]._s64[i] & 0xFFFFFFFF) << 32) | ((ctx->regs[r]._s64[i] >> 32) & 0xFFFFFFFF);
}

void reg2ls(u32 r, u32 addr);
void ls2reg(u32 r, u32 addr);
void get_mask_byte(u32 rt, u32 t);
void get_mask_hword(u32 rt, u32 t);
void get_mask_word(u32 rt, u32 t);
void get_mask_dword(u32 rt, u32 t);
void reg_to_byte(u8 *d, int r);
void byte_to_reg(int r, const u8 *d);
void reg_to_half(u16 *d, int r);
void half_to_reg(int r, const u16 *d);
void reg_to_Bits(u1 *d, int r);
void Bits_to_reg(int r, const u1 *d);
void reg_to_float(float *d, int r);
void float_to_reg(int r, const float *d);
void reg_to_double(double *d, int r);
void double_to_reg(int r, const double *d);
#define rtw ctx->reg[rt]
#define raw ctx->reg[ra]
#define rbw ctx->reg[rb]
#define rcw ctx->reg[rc]
#define rtwp rtw[0]
#define rawp raw[0]
#define rbwp rbw[0]
#define rcwp rcw[0]
#define rthp rth[1]
#define rahp rah[1]
#define rbhp rbh[1]
#define rchp rch[1]
#define rtbp rtb[3]
#define rabp rab[3]
#define rbbp rbb[3]
#define rcbp rcb[3]

#endif
