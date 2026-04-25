#include "platform_video.h"

#include <cstdio>
#include <cstdlib>
#include <malloc.h>
#include <string.h>
#include <ogcsys.h>
#include <gccore.h>

// Mostly borrowed from https://github.com/Mefiresu/RSDKv5-Decompilation/blob/dev/wii-port/RSDKv5/RSDK/Graphics/GX/GXRenderDevice.cpp
// Credit where credit is due and such yknow

uint8_t *pixelBuffer;
int pixelBufferPitch = 0;

static unsigned char gp_fifo[GX_FIFO_MINSIZE] __attribute__((aligned(32))) = {0};

static void* xfb = NULL;
static GXRModeObj* vmode = NULL;
static GXTexObj fbTex;
static uint32_t *fbGX;

static s16 square[] __attribute__((aligned(32))) = {
    -GAME_WIDTH,  GAME_HEIGHT, 0,
     GAME_WIDTH,  GAME_HEIGHT, 0,
     GAME_WIDTH, -GAME_HEIGHT, 0,
    -GAME_WIDTH, -GAME_HEIGHT, 0
};

static inline void draw_vert(u8 pos, f32 s, f32 t)
{
    f32 scaleFactor = (float)(GAME_WIDTH / 2) / (float)GAME_WIDTH;
    GX_Position1x8(pos);
    GX_TexCoord2f32(s * scaleFactor, t);
}

static inline void draw_square()
{
    Mtx mv;

    guMtxIdentity(mv);
    GX_LoadPosMtxImm(mv, GX_PNMTX0);
    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
    //texel fix makes uvs 1:1 with texels
    draw_vert(0, 0.0, 0.0);
    draw_vert(1, (float)GAME_WIDTH, 0.0);
    draw_vert(2, (float)GAME_WIDTH, (float)GAME_HEIGHT);
    draw_vert(3, 0.0, (float)GAME_HEIGHT);
    GX_End();
}


#define log(x) fprintf(stderr, "debug: %s\r\n", x);

bool initVideo()
{
    VIDEO_Init();

    vmode = VIDEO_GetPreferredMode(NULL);
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(vmode));

	VIDEO_Configure(vmode);

    // Allocate the video buffers
    xfb = (u32 *)SYS_AllocateFramebuffer(vmode);
    DCInvalidateRange(xfb, VIDEO_GetFrameBufferSize(vmode));
    xfb = (u32 *)MEM_K0_TO_K1(xfb);
    VIDEO_ClearFrameBuffer(vmode, xfb, COLOR_BLACK);
    VIDEO_SetNextFramebuffer(xfb);

	VIDEO_SetBlack(false);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (vmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
	else
		while(VIDEO_GetNextField())
			VIDEO_WaitVSync();
	
	// Initialize GX
    GX_Init(gp_fifo, sizeof(gp_fifo));

    GXColor background = { 0, 0, 0, 0xff };
    GX_SetCopyClear(background, 0x00ffffff);

    //pixel center fix
    GX_SetViewport(1.0f / 24.0f, 1.0f / 24.0f, vmode->fbWidth, vmode->efbHeight, 0.0f, 1.0f);
    GX_SetScissor(0, 0, vmode->fbWidth, vmode->efbHeight);

    GX_SetDispCopySrc(0, 0, vmode->fbWidth, vmode->efbHeight);
    GX_SetDispCopyYScale((float)vmode->xfbHeight / (float)vmode->efbHeight);
    GX_SetDispCopyDst(vmode->fbWidth, vmode->xfbHeight);
    GX_SetCopyFilter(vmode->aa, vmode->sample_pattern, GX_TRUE, vmode->vfilter);
    GX_SetFieldMode(vmode->field_rendering, ((vmode->viHeight == 2 * vmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));
    GX_SetPixelFmt(vmode->aa ? GX_PF_RGB565_Z16 : GX_PF_RGB8_Z24, GX_ZC_LINEAR);

    GX_SetDispCopyGamma(GX_GM_1_0);
    GX_SetCullMode(GX_CULL_NONE);

    GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GX_SetColorUpdate(GX_TRUE);

    Mtx44 p;
    guOrtho(p, GAME_HEIGHT, -(GAME_HEIGHT), -(GAME_WIDTH), GAME_WIDTH, 0, 1);
    GX_LoadProjectionMtx(p, GX_ORTHOGRAPHIC);

    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_INDEX8);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    //texel center fix
    Mtx m;
    for (int i = GX_TEXCOORD0; i < GX_MAXCOORD; i++)
        GX_SetTexCoordScaleManually(i, GX_TRUE, 1, 1);

    for (int i = 0; i < 10; i++)
	{
        float s = 1 << (i + 1);

        guMtxScale(m, s, s, s);
        GX_LoadTexMtxImm(m, GX_TEXMTX0 + i * 3, GX_MTX3x4);
    }

    guMtxTrans(m, 1. / 64., 1. / 64., 0);
    GX_LoadTexMtxImm(m, GX_DTTIDENTITY, GX_MTX3x4);

    for (int i = 0; i < 9; i++)
	{
        float x = i % 3 - 1;
        float y = i / 3 - 1;

        if (i % 2 == 0) {
            x /= 2;
            y /= 2;
        }

        x += 1. / 64.;
        y += 1. / 64.;

        guMtxTrans(m, x, y, 0);
        GX_LoadTexMtxImm(m, GX_DTTMTX1 + i * 3, GX_MTX3x4);
    }

	GX_SetBlendMode(GX_BM_NONE, GX_BL_ZERO, GX_BL_ZERO, GX_LO_CLEAR);
	GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
	GX_SetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
	GX_SetZCompLoc(GX_FALSE);

	GX_SetNumChans(0);
	GX_SetNumTexGens(1);
	GX_SetNumIndStages(0);
	GX_SetNumTevStages(1);

	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

	GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLORNULL);

	GX_SetArray(GX_VA_POS, square, 3 * sizeof(s16));

    GX_InvalidateTexAll();

    pixelBufferPitch = GAME_WIDTH * BYTES_PER_PIXEL;
    pixelBuffer = new uint8_t[pixelBufferPitch * GAME_HEIGHT];
    memset(pixelBuffer, 0, pixelBufferPitch * GAME_HEIGHT);
    
	fbGX = (uint32_t *)memalign(32, pixelBufferPitch * GAME_HEIGHT * sizeof(uint32_t) + 32);
    memset(fbGX, 0, pixelBufferPitch * GAME_HEIGHT * sizeof(uint32_t) + 32);
    GX_InitTexObj(&fbTex, fbGX, pixelBufferPitch, GAME_HEIGHT, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
	GX_InitTexObjFilterMode(&fbTex, GX_NEAR, GX_NEAR); // Nearest filtering for 4:3

    return true;
}

void shutdownVideo()
{
    if (fbGX)
        free(fbGX);

    free(MEM_K1_TO_K0(xfb));
}

void blitScreen()
{
    // copy pixel buffer into fbGX
    memcpy(fbGX, pixelBuffer, pixelBufferPitch * GAME_HEIGHT);

    VIDEO_WaitVSync();
    //tiled rendering code
    //declare pointer to pixel xy in xfb
    uint16_t(*xfb1)[vmode->fbWidth] = (uint16_t (*)[vmode->_gx_rmodeobj::fbWidth])(xfb);
    //loop if XFB height greater than EFB height
    for (int y = 0; y < vmode->xfbHeight; y += vmode->efbHeight)
	{
        //vertical clamping for the EFB/XFB copy
        uint8_t clamp = GX_CLAMP_NONE;

        if (y == 0)
            clamp |= GX_CLAMP_TOP;
        if (y + vmode->efbHeight == vmode->xfbHeight)
            clamp |= GX_CLAMP_BOTTOM;

        GX_SetCopyClamp(clamp);

        //loop if XFB width greater than EFB width
        for (int x = 0; x < vmode->fbWidth; x += 640)
		{
            uint16_t efbWidth = MIN(vmode->fbWidth - x, 640);

            //set up scissor region
            GX_SetScissor(x, y - 2, efbWidth, vmode->efbHeight + 4);
            GX_SetScissorBoxOffset(x, y - 2);
            GX_ClearBoundingBox();

            //draw fb texture
            draw_square();

            //copy the buffer
            GX_SetDispCopyFrame2Field(GX_COPY_PROGRESSIVE);
            GX_SetDispCopySrc(0, 2, efbWidth, vmode->efbHeight);
            GX_SetDispCopyDst(vmode->fbWidth, vmode->efbHeight);

            GX_CopyDisp(&xfb1[y][x], GX_TRUE);

            //clear the overlapping lines
            GX_SetDispCopyFrame2Field(0x1);
            GX_SetDispCopySrc(0, 0, efbWidth, 2);
            GX_SetDispCopyDst(0, 0);

            GX_CopyDisp(&xfb1[y][x], GX_TRUE);

            //clear the overlapping lines
            GX_SetDispCopyFrame2Field(0x1);
            GX_SetDispCopySrc(0, 2 + vmode->efbHeight, efbWidth, 2);
            GX_SetDispCopyDst(0, 0);

            GX_CopyDisp(&xfb1[y][x], GX_TRUE);
        }
    }

    GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GX_SetColorUpdate(GX_TRUE);

    GX_DrawDone();

    VIDEO_Flush();
}