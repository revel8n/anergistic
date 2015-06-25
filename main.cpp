// Copyright 2010 fail0verflow <master@fail0verflow.com>
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
//#include <unistd.h>

#include "types.h"
#include "main.h"
#include "config.h"
#include "elf.h"
#include "emulate.h"
#include "gdb.h"

struct ctx_t _ctx;
struct ctx_t *ctx;

__declspec( align( 16 ) ) static u8 src_data[16 * 1024 * 1024] = {0};
__declspec( align( 16 ) ) static u8 dst_data[16 * 1024 * 1024] = {0};

#pragma pack(push, 1)

const uint16_t kTEXTURE_BLOB_MAGIC = 0x10;

struct TextureBlobHeader
{
    uint16_t magic;
    uint16_t type;
    uint16_t quality;
    uint16_t width;
    uint16_t height;
    uint16_t unk_0x0A; // TODO: depth?
    float unk_0x0C;
    float unk_0x10;
    float unk_0x14;
    uint32_t unk_0x18;
    uint32_t unk_0x1C;
    uint32_t unk_0x20;
    uint32_t unk_0x24;
    uint16_t num_chunks;
    uint16_t padding_size;
    uint32_t unk_0x2C;
    uint32_t unk_0x30;
};

struct TextureBlobChunk
{
    uint16_t num_batches;
    uint16_t padding;
    uint32_t compressed_size;
    uint32_t decompressed_size;
    uint32_t offset;
};

#pragma pack(pop)

static int gdb_port = -1;
static const char *elf_path = NULL;

int     opterr = 1,             /* if error message should be printed */
  optind = 1,             /* index into parent argv vector */
  optopt,                 /* character checked for validity */
  optreset;               /* reset getopt */
char    *optarg;                /* argument associated with option */

#define BADCH   (int)'?'
#define BADARG  (int)':'
#define EMSG    ""

/*
* getopt --
*      Parse argc/argv argument vector.
*/
int getopt(int nargc, char * const nargv[], const char *ostr)
{
    static char *place = EMSG;              /* option letter processing */
    const char *oli;                        /* option letter list index */

    if (optreset || !*place) {              /* update scanning pointer */
        optreset = 0;
        if (optind >= nargc || *(place = nargv[optind]) != '-') {
            place = EMSG;
            return (-1);
        }
        if (place[1] && *++place == '-') {      /* found "--" */
            ++optind;
            place = EMSG;
            return (-1);
        }
    }                                       /* option letter okay? */
    if ((optopt = (int)*place++) == (int)':' ||
        !(oli = strchr(ostr, optopt))) {
        /*
        * if the user didn't specify '-' as an option,
        * assume it means -1.
        */
        if (optopt == (int)'-')
            return (-1);
        if (!*place)
            ++optind;
        if (opterr && *ostr != ':')
            (void)printf("illegal option -- %c\n", optopt);
        return (BADCH);
    }
    if (*++oli != ':') {                    /* don't need argument */
        optarg = NULL;
        if (!*place)
            ++optind;
    }
    else {                                  /* need an argument */
        if (*place)                     /* no white space */
            optarg = place;
        else if (nargc <= ++optind) {   /* no arg */
            place = EMSG;
            if (*ostr == ':')
                return (BADARG);
            if (opterr)
                (void)printf("option requires an argument -- %c\n", optopt);
            return (BADCH);
        }
        else                            /* white space */
            optarg = nargv[optind];
        place = EMSG;
        ++optind;
    }
    return (optopt);                        /* dump back option letter */
}

void dump_regs(void)
{
	u32 i;

	printf("\nRegister dump:\n");
	printf(" pc:\t%08x\n", ctx->pc);
	for (i = 0; i < 128; i++)
		printf("%.3d:\t%08x %08x %08x %08x\n",
				i,
				ctx->reg[i][0],
				ctx->reg[i][1],
				ctx->reg[i][2],
				ctx->reg[i][3]
				);
}

void dump_ls(void)
{
	FILE *fp;

	printf("dumping local store to " DUMP_LS_NAME "\n");
	fp = fopen(DUMP_LS_NAME, "wb");
	fwrite(ctx->ls, LS_SIZE, 1, fp);
	fclose(fp);
}

void fail(const char *a, ...)
{
	char msg[1024];
	va_list va;

	va_start(va, a);
	vsnprintf(msg, sizeof msg, a, va);
	perror(msg);

#ifdef FAIL_DUMP_REGS
	dump_regs();
#endif

#ifdef FAIL_DUMP_LS
	dump_ls();
#endif

	gdb_deinit();
	exit(1);
}

static void usage(void)
{
	printf("usage: anergistic [-g 1234] [-f image.bin] filename.elf\n");
	exit(1);
}

static void parse_args(int argc, char *argv[])
{
	int c;

	while ((c = getopt(argc, argv, "g:f:s:")) != -1)
    {
		switch (c)
        {
        case 'g':
            gdb_port = strtol(optarg, NULL, 10);
            break;
        case 'f':
            {
                FILE *f = fopen(optarg, "rb");
                if (!f)
                {
                    printf("Input file not found.\n");
                    exit(1);
                }

                fseek(f, 0, SEEK_END);
                u32 size = ftell(f);
                fseek(f, 0, SEEK_SET);
                if (fread(src_data, size, 1, f) != 1)
                {
                    printf("Unable to read input file\n");
                    exit(1);
                }
                fclose(f);
            }
            break;
        case 's':
            break;
        default:
            printf("Unknown argument: %c\n", c);
            usage();
		}
	}

	if (optind != argc - 1)
		usage();

	elf_path = argv[optind];
}

void setup_context()
{
    // setup stack pointer
    _ctx.reg[1][0] = 0x3FFD0; _ctx.reg[1][1] = 0x0; _ctx.reg[1][2] = 0x0; _ctx.reg[1][3] = 0x0;

    wbe32(_ctx.ls + 0x7060, 0x32000600);

    TextureBlobHeader* header = (TextureBlobHeader*)src_data;
    TextureBlobChunk* chunks = (TextureBlobChunk*)(header + 1);

    wbe32(_ctx.ls + 0x3BE40, be16((u8*)&header->type));
    wbe32(_ctx.ls + 0x3BE44, be16((u8*)&header->quality));
    wbe32(_ctx.ls + 0x3BE48, (u32)src_data + be32((u8*)&chunks[0].offset));
    wbe32(_ctx.ls + 0x3BE4C, be32((u8*)&chunks[0].compressed_size));
    wbe32(_ctx.ls + 0x3BE50, (u32)dst_data);
    wbe32(_ctx.ls + 0x3BE54, be32((u8*)&chunks[0].decompressed_size));
    wbe32(_ctx.ls + 0x3BE58, be16((u8*)&chunks[0].num_batches));
    wbe32(_ctx.ls + 0x3BE5C, be16((u8*)&header->width));
    wbe32(_ctx.ls + 0x3BE60, be16((u8*)&header->height));

    wbe32(_ctx.ls + 0x3BE64, be32((u8*)&header->unk_0x0C));
    wbe32(_ctx.ls + 0x3BE68, be32((u8*)&header->unk_0x10));
    wbe32(_ctx.ls + 0x3BE6C, be32((u8*)&header->unk_0x14));

    wbe32(_ctx.ls + 0x3BE70, be32((u8*)&header->unk_0x18));
    wbe32(_ctx.ls + 0x3BE74, be32((u8*)&header->unk_0x1C));
    wbe32(_ctx.ls + 0x3BE78, be32((u8*)&header->unk_0x20));

    wbe32(_ctx.ls + 0x3BE7C, 0);
    wbe32(_ctx.ls + 0x3BE80, 0);
    wbe32(_ctx.ls + 0x3BE84, 0);
    wbe32(_ctx.ls + 0x3BE8C, 0);
}

int main(int argc, char *argv[])
{
	u32 done;
	memset(&_ctx, 0x00, sizeof _ctx);
	ctx = &_ctx;
	parse_args(argc, argv);

#if 0
	u64 local_ptr;
	
	local_ptr = 0xdead0000dead0000ULL;
	
	ctx->reg[3][0] = (u32)(local_ptr >> 32);
	ctx->reg[3][1] = (u32)local_ptr;

	ctx->reg[4][0] = 0xdead0000;
	ctx->reg[4][1] = 0xdead0000;
#endif

	ctx->ls = (u8*)malloc(LS_SIZE);
	if (ctx->ls == NULL)
		fail("Unable to allocate local storage.");
	memset(ctx->ls, 0, LS_SIZE);

#if 0
	wbe64(ctx->ls + 0x3f000, 0x100000000ULL);
	wbe32(ctx->ls + 0x3f008, 0x10000);
	wbe32(ctx->ls + 0x3e000, 0xff);
#endif

	if (gdb_port < 0)
    {
		ctx->paused = 0;
	}
    else
    {
		gdb_init(gdb_port);
		ctx->paused = 1;
		gdb_signal(SIGABRT);
	}

	elf_load(elf_path);

    setup_context();

	done = 0;

	while(done == 0)
    {
		if (ctx->paused == 0 || ctx->paused == 2)
			done = emulate();

		// data watchpoints
		if (done == 2)
        {
			ctx->paused = 0;
			gdb_signal(SIGTRAP);
			done = 0;
		}
		
		if (done != 0)
        {
			printf("emulated() returned, sending SIGSEGV to gdb stub\n");
			ctx->paused = 1;
			done = gdb_signal(SIGSEGV);
		}

		if (done != 0)
        {
#ifdef STOP_DUMP_REGS
			dump_regs();
#endif
#ifdef STOP_DUMP_LS
			dump_ls();
#endif
		}

		//if (ctx->paused == 1)
			gdb_handle_events();
	}
	printf("emulate() returned. we're done!\n");
	dump_ls();
	free(ctx->ls);
	gdb_deinit();
	return 0;
}
