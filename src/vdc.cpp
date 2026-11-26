// ---------------------------------------------------------------------
// vdc.cpp
// hydra
//
// Copyright © 2026 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#include "vdc.hpp"

void vdc_t::clear(uint8_t color)
{
    // Fast bulk fill of the entire indexed framebuffer with `color`.
    std::memset(buffer.data(), color, buffer.size());
}

void vdc_t::blit_sprite(
    uint32_t gfx_addr,
    int16_t  dst_x,
    int16_t  dst_y,
    uint8_t  sprite_w,
    uint8_t  sprite_h,
    uint8_t  sprite_id,
    bool     flip_x,
    bool     flip_y,
    bool     double_x,
    bool     double_y
)
{
    // Compute rendered (post-scaling) dimensions in screen pixels.
    // Using `int` avoids uint8_t overflow when doubling sizes up to 255 * 2.
    const int scale_x = double_x ? 2 : 1;
    const int scale_y = double_y ? 2 : 1;
    const int draw_w  = static_cast<int>(sprite_w) * scale_x;
    const int draw_h  = static_cast<int>(sprite_h) * scale_y;

    // 1. Early rejection if the sprite has zero size or lies completely
    //    outside the visible framebuffer rectangle [0..PIXELS_PER_SCANLINE) x
    //    [0..SCANLINES).
    if (sprite_w == 0 || sprite_h == 0 ||
        dst_x >= PIXELS_PER_SCANLINE || dst_y >= SCANLINES ||
        dst_x + draw_w <= 0 || dst_y + draw_h <= 0) {
        return;
    }

    // 2. Clip the scaled destination rectangle against screen boundaries.
    //    Preserve `orig_dst_x` and `orig_dst_y` so we can recover each clipped
    //    pixel's offset relative to the unclipped top-left corner of the sprite.
    const int orig_dst_x = dst_x;
    const int orig_dst_y = dst_y;
    int w_clip = draw_w;
    int h_clip = draw_h;

    if (dst_x < 0) {
        // Left edge is off-screen: `dst_x` is negative, so `w_clip += dst_x`
        // trims the off-screen width and `dst_x` clamps to 0.
        w_clip += dst_x;
        dst_x = 0;
    }
    if (dst_y < 0) {
        // Top edge is off-screen: `dst_y` is negative, so `h_clip += dst_y`
        // trims the off-screen height and `dst_y` clamps to 0.
        h_clip += dst_y;
        dst_y = 0;
    }
    if (dst_x + w_clip > PIXELS_PER_SCANLINE) {
        // Clamp right edge to screen width.
        w_clip = PIXELS_PER_SCANLINE - dst_x;
    }
    if (dst_y + h_clip > SCANLINES) {
        // Clamp bottom edge to screen height.
        h_clip = SCANLINES - dst_y;
    }

    // Defensive guard in case clipping leaves an empty region.
    if (w_clip <= 0 || h_clip <= 0) {
        return;
    }

    // 3. Locate the requested `sprite_id` frame in `ram` and verify that the
    //    entire `sprite_w * sprite_h` frame fits within `RAM_SIZE`.
    uint32_t offset = static_cast<uint32_t>(sprite_id) * sprite_w * sprite_h;
    gfx_addr = (gfx_addr + offset) & (RAM_SIZE - 1);

    const std::size_t sprite_bytes = static_cast<std::size_t>(sprite_w) * sprite_h;
    if (static_cast<std::size_t>(gfx_addr) + sprite_bytes > RAM_SIZE) {
        return;
    }

    // 4. Blit clipped screen pixels by mapping each visible destination pixel
    //    back to its source texel `(sx, sy)` in the unflipped sprite frame:
    //      - `(dst_x - orig_dst_x) + dx` / `(dst_y - orig_dst_y) + dy` gives
    //        the exact offset within the unclipped (and possibly 2x-scaled)
    //        destination box, even when the left or top edge was clipped.
    //      - Shifting right by 1 (`>> 1`) duplicates each source texel across 2
    //        output pixels/scanlines when `double_x` / `double_y` is active.
    //      - Applying `(sprite_w - 1) - unflipped_sx` / `(sprite_h - 1) - unflipped_sy`
    //        mirrors around the full sprite dimensions, keeping clipped edges
    //        aligned with the flipped sprite's opposite side.
    const uint8_t* __restrict sprite_base = ram.data() + gfx_addr;
    uint8_t* __restrict dst_row = buffer.data() + (dst_y * PIXELS_PER_SCANLINE) + dst_x;

    for (int dy = 0; dy < h_clip; ++dy) {
        const int screen_off_y = (dst_y - orig_dst_y) + dy;
        const int unflipped_sy = double_y ? (screen_off_y >> 1) : screen_off_y;
        const int sy = flip_y ? ((static_cast<int>(sprite_h) - 1) - unflipped_sy) : unflipped_sy;
        const uint8_t* __restrict src_row = sprite_base + (sy * static_cast<int>(sprite_w));

        for (int dx = 0; dx < w_clip; ++dx) {
            const int screen_off_x = (dst_x - orig_dst_x) + dx;
            const int unflipped_sx = double_x ? (screen_off_x >> 1) : screen_off_x;
            const int sx = flip_x ? ((static_cast<int>(sprite_w) - 1) - unflipped_sx) : unflipped_sx;

            // Index 0x00 is transparent; only write non-zero palette indices.
            const uint8_t val = src_row[sx];
            if (val) {
                dst_row[dx] = val;
            }
        }

        dst_row += PIXELS_PER_SCANLINE;
    }
}

void vdc_t::blit_tilemap(
    uint32_t map_addr,
    int16_t  dst_x,
    int16_t  dst_y,
    uint8_t  map_w,
    uint8_t  map_h,
    bool     flip_x,
    bool     flip_y,
    bool     double_x,
    bool     double_y,
    bool     repeat_x,
    bool     repeat_y,

    uint32_t gfx_addr,
    uint8_t  sprite_w,
    uint8_t  sprite_h
)
{
    // Nothing to draw if either the map grid or an individual tile has a zero dimension.
    if (map_w == 0 || map_h == 0 || sprite_w == 0 || sprite_h == 0) {
        return;
    }

    // Rendered pixel pitch per tile cell after accounting for optional 2x scaling.
    const int draw_tile_w = static_cast<int>(sprite_w) * (double_x ? 2 : 1);
    const int draw_tile_h = static_cast<int>(sprite_h) * (double_y ? 2 : 1);

    // Euclidean / floor integer division so negative screen offsets (`dst_x`, `dst_y`)
    // map consistently to negative grid indices (`tx`, `ty`) without C++ truncating
    // division toward zero.
    auto floor_div = [](int a, int b) -> int {
        int q = a / b;
        int r = a % b;
        if ((r != 0) && ((r < 0) != (b < 0))) {
            --q;
        }
        return q;
    };

    // 1. Compute the half-open range `[x_start, x_end) x [y_start, y_end)` of
    //    grid cells that overlap the visible screen `[0..PIXELS_PER_SCANLINE-1]`
    //    and `[0..SCANLINES-1]`.
    int x_start = floor_div(-dst_x, draw_tile_w);
    int y_start = floor_div(-dst_y, draw_tile_h);
    int x_end   = floor_div(PIXELS_PER_SCANLINE - 1 - dst_x, draw_tile_w) + 1;
    int y_end   = floor_div(SCANLINES - 1 - dst_y, draw_tile_h) + 1;

    // For non-repeating axes, clamp grid coordinates to the finite `[0, map_w)`
    // and/or `[0, map_h)` map bounds.
    if (!repeat_x) {
        if (x_start < 0) x_start = 0;
        if (x_end > map_w) x_end = map_w;
    }
    if (!repeat_y) {
        if (y_start < 0) y_start = 0;
        if (y_end > map_h) y_end = map_h;
    }
    if (x_start >= x_end || y_start >= y_end) {
        return;
    }

    // 2. Iterate over visible grid cells in row-major order and delegate all
    //    actual pixel drawing and clipping to `blit_sprite`.
    for (int ty = y_start; ty < y_end; ++ty) {
        const int16_t py = static_cast<int16_t>(dst_y + (ty * draw_tile_h));
        // Wrap `ty` into `[0, map_h)` when `repeat_y` is enabled (handling negative `ty`).
        const int map_y = repeat_y ? (((ty % map_h) + map_h) % map_h) : ty;
        const uint32_t row_map_offset = map_addr + (map_y * map_w);

        for (int tx = x_start; tx < x_end; ++tx) {
            // Wrap `tx` into `[0, map_w)` when `repeat_x` is enabled (handling negative `tx`).
            const int map_x = repeat_x ? (((tx % map_w) + map_w) % map_w) : tx;
            const uint8_t tile_id = ram[(row_map_offset + map_x) & (RAM_SIZE - 1)];
            const int16_t px = static_cast<int16_t>(dst_x + (tx * draw_tile_w));

            blit_sprite(
                gfx_addr,
                px,
                py,
                sprite_w,
                sprite_h,
                tile_id,
                flip_x,
                flip_y,
                double_x,
                double_y
            );
        }
    }
}
