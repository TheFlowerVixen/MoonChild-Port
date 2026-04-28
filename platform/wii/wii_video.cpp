#include "platform_video.h"

#include <cstdio>
#include <cstdlib>
#include <malloc.h>
#include <string.h>
#include <ogcsys.h>
#include <gccore.h>

uint8_t *pixelBuffer = NULL;
int pixelBufferPitch = 0;

static constexpr u32 FIFO_MEM_SIZE = (256 * 1024);
static constexpr GXColor CLEAR_COLOR = (GXColor) { 0x00, 0x00, 0x00, 0xFF };

class CVideo {
public:
    CVideo(void) {}

    void doInit(void);
    void doShutdown(void);

    void doDraw(void);

private:
    void setupMoonChild(void);
    void syncMoonChild(void);
    void drawMoonChild(void);

    void swapFrameBuffer(void) {
        if (mCurrentFrameBuffer == mFrameBuffer[0]) {
            mCurrentFrameBuffer = mFrameBuffer[1];
        }
        else {
            mCurrentFrameBuffer = mFrameBuffer[0];
        }
    }

private:
    bool mScreenBlack;

    GXRModeObj *mVideoMode;

    void *mFrameBuffer[2];
    void *mCurrentFrameBuffer;

    void *mFifoMem;

    void *mMCIndirectTexMem;
    GXTexObj mMCIndirectTexObj;
};
static CVideo sVideo;

/*
 * Implementation
 */

void CVideo::doInit(void) {
    VIDEO_Init();

    mScreenBlack = true;

    mVideoMode = new GXRModeObj;
    *mVideoMode = *VIDEO_GetPreferredMode(NULL);

    mVideoMode->efbHeight = GAME_HEIGHT;
    mVideoMode->fbWidth = GAME_WIDTH;

	mFrameBuffer[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(mVideoMode));
    mFrameBuffer[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(mVideoMode));

    mFifoMem = aligned_alloc(32, FIFO_MEM_SIZE);
	memset(mFifoMem, 0x00, FIFO_MEM_SIZE);

    GX_Init(mFifoMem, FIFO_MEM_SIZE);

    GX_SetCopyClear(CLEAR_COLOR, 0x00FFFFFF);

    GX_SetViewport(0.0f, 0.0f, mVideoMode->fbWidth, mVideoMode->efbHeight, 0.0f, 1.0f);
	GX_SetScissor(0, 0, mVideoMode->fbWidth, mVideoMode->efbHeight);

	f32 yScaleFactor = GX_GetYScaleFactor(mVideoMode->efbHeight, mVideoMode->xfbHeight);
	f32 yLineCount = GX_SetDispCopyYScale(yScaleFactor);

	GX_SetDispCopySrc(0, 0, mVideoMode->fbWidth, mVideoMode->efbHeight);
	GX_SetDispCopyDst(mVideoMode->fbWidth, yLineCount);

	GX_SetCopyFilter(mVideoMode->aa, mVideoMode->sample_pattern, GX_TRUE, mVideoMode->vfilter);

    GX_SetDispCopyGamma(GX_GM_1_0);

    if (mVideoMode->aa) {
		GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
	}
    else {
		GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
    }

	GX_CopyDisp(mFrameBuffer[1], GX_TRUE);

    VIDEO_Configure(mVideoMode);

	VIDEO_SetNextFramebuffer(mFrameBuffer[0]);
    mCurrentFrameBuffer = mFrameBuffer[1];

	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (mVideoMode->viTVMode & VI_NON_INTERLACE) {
        VIDEO_WaitVSync();
    }

    setupMoonChild();
}

void CVideo::doShutdown(void) {}

void CVideo::doDraw(void) {
    syncMoonChild();

    drawMoonChild();

    GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GX_SetColorUpdate(GX_TRUE);
    GX_CopyDisp(mCurrentFrameBuffer, GX_TRUE);
    GX_DrawDone();

    VIDEO_SetNextFramebuffer(mCurrentFrameBuffer);
    if (mScreenBlack) {
        VIDEO_SetBlack(FALSE);
        mScreenBlack = false;
    }
    VIDEO_Flush();
    VIDEO_WaitVSync();

    swapFrameBuffer();
}

void CVideo::setupMoonChild(void) {
    pixelBufferPitch = GAME_WIDTH * BYTES_PER_PIXEL;

    u32 pixelBufferSize = (pixelBufferPitch * GAME_HEIGHT);
    pixelBuffer = (u8 *)aligned_alloc(32, pixelBufferSize);
    memset(pixelBuffer, 0x00, pixelBufferSize);

    u32 sizeofIndirectTexMem = ((GAME_WIDTH + 3) / 4) * ((GAME_HEIGHT + 3) / 4) * 32 * 2;
    mMCIndirectTexMem = aligned_alloc(32, sizeofIndirectTexMem);

    GX_InitTexObj(&mMCIndirectTexObj, mMCIndirectTexMem, GAME_WIDTH, GAME_HEIGHT, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
    GX_InitTexObjFilterMode(&mMCIndirectTexObj, GX_LINEAR, GX_LINEAR);

    printf("pixelBuffer=%p\n", pixelBuffer);
    printf("mMCIndirectTexMem=%p\n", mMCIndirectTexMem);
}

void CVideo::syncMoonChild(void) {
    u8 *dest = reinterpret_cast<u8 *>(mMCIndirectTexMem);

    const u8 *src = reinterpret_cast<const u8 *>(pixelBuffer);
    const u32 srcWidth = GAME_WIDTH;
    const u32 srcHeight = GAME_HEIGHT;

    u32 writeOffset = 0;

    for (u32 yy = 0; yy < GAME_HEIGHT; yy += 4) {
        for (u32 xx = 0; xx < GAME_WIDTH; xx += 4) {
            // The block data is split down into two subblocks:
            //    Subblock A: Alpha and Red channel
            //    Subblock B: Green and Blue channel

            // Subblock A
            for (u32 y = 0; y < 4; y++) {
                if (yy + y >= srcHeight) break;

                const u32 rowBase = srcWidth * (yy + y);

                for (u32 x = 0; x < 4; x++) {
                    if (xx + x >= srcWidth) break;

                    const u32 readOffset = (rowBase + xx + x) * 4;

                    dest[writeOffset + (y * 2 * 4) + (x * 2) + 0] = src[readOffset + 0]; // Alpha channel
                    dest[writeOffset + (y * 2 * 4) + (x * 2) + 1] = src[readOffset + 1]; // Red channel
                }
            }
            writeOffset += 2 * 4 * 4;

            // Subblock B
            for (u32 y = 0; y < 4; y++) {
                if (yy + y >= srcHeight) break;

                const u32 rowBase = srcWidth * (yy + y);

                for (u32 x = 0; x < 4; x++) {
                    if (xx + x >= srcWidth) break;

                    const u32 readOffset = (rowBase + xx + x) * 4;

                    dest[writeOffset + (y * 2 * 4) + (x * 2) + 0] = src[readOffset + 2]; // Green channel
                    dest[writeOffset + (y * 2 * 4) + (x * 2) + 1] = src[readOffset + 3]; // Blue channel
                }
            }
            writeOffset += 2 * 4 * 4;
        }
    }

    u32 sizeofIndirectTexMem = ((GAME_WIDTH + 3) / 4) * ((GAME_HEIGHT + 3) / 4) * 32 * 2;
    DCFlushRange(mMCIndirectTexMem, sizeofIndirectTexMem);
}

void CVideo::drawMoonChild(void) {
    f32 coord[4];
    coord[0] = 0.0f;
    coord[1] = 0.0f;
    coord[2] = 1.0f;
    coord[3] = 1.0f;

    Mtx44 projMtx;
    guOrtho(projMtx, coord[0], coord[2], coord[1], coord[3], 0.0f, 500.0f);
	GX_LoadProjectionMtx(projMtx, GX_ORTHOGRAPHIC);

    GX_SetScissor(0, 0, mVideoMode->fbWidth, mVideoMode->efbHeight);

    Mtx mtx;
    guMtxIdentity(mtx);
    GX_LoadPosMtxImm(mtx, GX_PNMTX0);
    GX_SetCurrentMtx(GX_PNMTX0);

    GX_SetTevDirect(GX_TEVSTAGE0);

    GX_ClearVtxDesc();
    GX_InvVtxCache();
    GX_InvalidateTexAll();

    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    GX_SetNumTexGens(1);
    GX_SetNumChans(0);
    GX_SetNumIndStages(0);
    GX_SetNumTevStages(1);

    GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLORNULL);
    GX_SetTevColorIn(GX_TEVSTAGE0, GX_CC_C0, GX_CC_C1, GX_CC_TEXC, GX_CC_ZERO);
    GX_SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
    GX_SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, TRUE, GX_TEVPREV);
    GX_SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, TRUE, GX_TEVPREV);

    GX_SetColorUpdate(GX_TRUE);
    GX_SetAlphaUpdate(GX_TRUE);

    GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GX_SetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);

    GX_SetCullMode(GX_CULL_NONE);
    GX_SetClipMode(GX_CLIP_ENABLE);

    GX_LoadTexObj(&mMCIndirectTexObj, GX_TEXMAP0);

    GX_SetTevColor(GX_TEVREG0, (GXColor){ 0x00, 0x00, 0x00, 0xFF });
    GX_SetTevColor(GX_TEVREG1, (GXColor){ 0xFF, 0xFF, 0xFF, 0xFF });

    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);

    GX_Position2f32(coord[0], coord[1]);
    GX_TexCoord2f32(0.0f, 0.0f);

    GX_Position2f32(coord[0], coord[3]);
    GX_TexCoord2f32(0.0f, 1.0f);

    GX_Position2f32(coord[2], coord[3]);
    GX_TexCoord2f32(1.0f, 1.0f);

    GX_Position2f32(coord[2], coord[1]);
    GX_TexCoord2f32(1.0f, 0.0f);

    GX_End();
}

/*
 * Public interface
 */

bool initVideo(int argc, char **argv) {
    sVideo.doInit();
    return true; // It probably worked.
}
void shutdownVideo() {
    sVideo.doShutdown();
}
void blitScreen() {
    sVideo.doDraw();
}
