#include <kos.h>
#include <stdlib.h>
#include <kos/init.h>

#include "gamesettings.h"
#include "renderer/render_util.h"
#include "screens/screens.h"
#include "sound/sound.h"

#include <dc/sound/sound.h>

// Shamelessly yoinked from Falco Girgis and skmp in DCA3
// https://gitlab.com/skmp/dca3-game/-/blob/main/src/common/vmu/vmu.cpp
static float heap_utilization() {
    // Query heap manager/allocator for info
    struct mallinfo mallocInfo = mallinfo();

    // Used bytes are as reported
    size_t usedBlocks = mallocInfo.uordblks;
    // First component of free bytes are as reported
    size_t freeBlocks = mallocInfo.fordblks;

    // End address of region reserved for heap growth
    size_t brkEnd = _arch_mem_top - THD_KERNEL_STACK_SIZE - 1;
    // Amount of bytes the heap has yet to still grow
    size_t brkFree = brkEnd - (uintptr_t)sbrk(0);

    // Total heap space available is free blocks from allocator + unclaimed sbrk() space
    freeBlocks += brkFree;

    return (float)usedBlocks * 100.0f / (float)(usedBlocks + freeBlocks);
}

static float pvr_utilization() {
    uint32_t mb = 8388608;
    return (float)(mb - pvr_mem_available()) / (float)mb * 100.0f;
}

static float snd_utilization() {
    uint32_t avail = snd_mem_available();

    const uint32_t mb = 2097152;
    return (float)(mb-avail) / (float)mb * 100.0f;
}

pvr_init_params_t pvr_params = {
    .opb_sizes = { PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_0 },
    .vertex_buf_size = 512 * 1024,
    .dma_enabled = 0,
    .fsaa_enabled = 0,
    .autosort_disabled = 1,
    .opb_overflow_count = 0,
    .vbuf_doublebuf_disabled = 0
};

void render() {

    pvr_scene_begin();

    render_dr_begin();

    pvr_list_begin(PVR_LIST_OP_POLY);
    screens_render_op();
    pvr_list_finish();

    pvr_list_begin(PVR_LIST_TR_POLY);
    screens_render_tr();
    pvr_list_finish();

    pvr_scene_finish();
}

// float shz_smoothstepf(float edge0, float edge1, float x) SHZ_NOEXCEPT {
//     if (x <= edge0) return 0.0f;
//     if (x >= edge1) return 1.0f;
//
//     float diff = edge1 - edge0;
//
//     // Fast Reciprocal
//     float inv_diff = shz_inv_sqrtf(diff);
//     inv_diff *= inv_diff;
//
//     // scale and bias
//     float t = (x - edge0) * inv_diff;
//
//     // polynomial: t^2 * (3 - 2t)
//     return t * t * shz_fmaf(t, -2.0f, 3.0f);
// }

//48*32
static uint8_t vmu_ico[192] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
    0x00, 0x03, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xf0, 0x00,
    0x00, 0x3f, 0xff, 0xff, 0xfc, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00,
    0x01, 0xff, 0xff, 0xff, 0xff, 0x80, 0x03, 0xfe, 0x7f, 0xff, 0xfb, 0xc0,
    0x07, 0xf9, 0xfc, 0x3e, 0xfb, 0xe0, 0x0f, 0xf7, 0xfb, 0xdf, 0x7b, 0xf0,
    0x0f, 0xf7, 0xf7, 0xef, 0x7b, 0xf0, 0x1f, 0xef, 0xf7, 0xef, 0xbb, 0xf8,
    0x1f, 0xef, 0xf7, 0xef, 0xdb, 0xf8, 0x3f, 0xef, 0xf7, 0xef, 0xeb, 0xfc,
    0x3f, 0xf0, 0xf7, 0xef, 0xf3, 0xfc, 0x1f, 0xff, 0x77, 0xef, 0xeb, 0xf8,
    0x1f, 0xff, 0x77, 0xef, 0xdb, 0xf8, 0x0f, 0xff, 0x77, 0xef, 0x3b, 0xf0,
    0x0f, 0xff, 0x7b, 0xde, 0xfb, 0xf0, 0x07, 0xff, 0x7c, 0x3d, 0xfb, 0xe0,
    0x03, 0xf0, 0xff, 0xff, 0xfb, 0xc0, 0x01, 0xff, 0xff, 0xff, 0xff, 0x80,
    0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xfc, 0x00,
    0x00, 0x0f, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x03, 0xff, 0xff, 0xc0, 0x00,
    0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

KOS_INIT_FLAGS(INIT_DEFAULT | INIT_MALLOCSTATS);
int main(int argc, char** argv) {

    srand(time(nullptr));
    
    vid_set_mode(DM_640x480, PM_RGB565);
    pvr_init(&pvr_params);
    pvr_set_bg_color(0.05f, 0.05f, 0.05f);
    vid_border_color(10, 10, 10);

    gamesettings_load();
    soundengine_init();

    cont_btn_callback(0, CONT_START | CONT_A | CONT_B | CONT_X | CONT_Y, (cont_btn_callback_t)exit);

    screens_set(SCREEN_MAINMENU);

    maple_device_t* vmu_lcd = maple_enum_type(0, MAPLE_FUNC_LCD);
    if (vmu_lcd) {
        vmu_draw_lcd(vmu_lcd, vmu_ico);
    }

    uint64_t last_time = timer_ns_gettime64();
    for(;;) {

        // -- Delta time (ms) --
        uint64_t current_time = timer_ns_gettime64();
        float delta_time = (float)(current_time - last_time);
        delta_time *= 1.0e-6f;
        last_time = current_time;

        soundengine_step(delta_time);
        screens_step(delta_time);

        render();

#ifndef NO_VMU_STATS
        vmu_printf("time: %.1f\nheap: %.2f%%\nsnd:%.2f%%\npvr:%.2f%%", delta_time,
            heap_utilization(), snd_utilization(), pvr_utilization());
#endif
    }

    soundengine_cleanup();
    pvr_shutdown();
    return 0;
}