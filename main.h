// Copyright 2010 fail0verflow <master@fail0verflow.com>
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

#ifndef MAIN_H__
#define MAIN_H__

#include "types.h"
#include "reg_t.h"

#pragma pack(push, 1)

struct ctx_t
{
	u8 *ls;
    union
    {
        reg_t regs[128];
        u32 reg[128][4];
    };
	u32 pc;
	u32 paused;
	u32 trap;
};

#pragma pack(pop)

// evil global variable ahead
extern struct ctx_t *ctx;

void fail(const char *a, ...);
void dump_regs(void);
void dump_ls(void);

#define array_size(x) (sizeof((x)) / sizeof(*(x)))

#endif
