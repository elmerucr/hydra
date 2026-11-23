// ---------------------------------------------------------------------
// vdc.hpp
// hydra
//
// Copyright © 2026 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#ifndef VDC_HPP
#define VDC_HPP

#include <array>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <cstring>

#include "common.hpp"

typedef struct {
    uint32_t gfx_addr;
    int16_t  dst_x;
    int16_t  dst_y;
    uint8_t  sprite_w;
    uint8_t  sprite_h;
    uint8_t  sprite_id;
    bool     flip_x;
    bool     flip_y;
    bool     double_x;
    bool     double_y;
} sprite_t;


// Virtual Display Controller (VDC) for the hydra engine.
// Manages the indexed framebuffer (`buffer`, 320x180 by default via
// PIXELS_PER_SCANLINE x SCANLINES) and main system memory (`ram`),
// providing screen clearing, clipped/flipped/2x-scaled sprite blitting,
// and tilemap rendering built on top of `blit_sprite`.
class vdc_t {
public:
    // 64-byte aligned indexed pixel framebuffer (palette indices 0x00..0xFF).
    // Pixel value 0x00 is treated as transparent during blitting.
    alignas(64) std::array<uint8_t, PIXELS_PER_SCANLINE * SCANLINES> buffer;

    // 64-byte aligned virtual RAM holding tilemaps, sprite sheets, and
    // general data addressed by byte offsets (`0 .. RAM_SIZE - 1`).
    alignas(64) std::vector<uint8_t> ram;

    // Initializes virtual RAM with `RAM_SIZE` zero-initialized bytes.
    vdc_t() : ram (RAM_SIZE, 0) {}

    // Fills the entire display framebuffer with a single palette index `color`.
    void clear(uint8_t color);

    // Blits a single sprite frame (`sprite_id`) from `ram` at `gfx_addr`
    // onto `buffer` at signed screen coordinates `(dst_x, dst_y)`.
    //
    // Sprite layout in RAM:
    //   - Each frame is `sprite_w * sprite_h` contiguous bytes in row-major order.
    //   - Frame `sprite_id` begins at `gfx_addr + sprite_id * sprite_w * sprite_h`
    //     (masked with `RAM_SIZE - 1`).
    //
    // Transformations and clipping:
    //   - `double_x` / `double_y` independently double width / height (2x nearest-
    //     neighbor pixel replication) in screen space.
    //   - `flip_x` / `flip_y` mirror the source sprite horizontally / vertically
    //     relative to its unclipped sprite bounds, so partial visibility at screen
    //     edges reveals the opposite side of the sprite cleanly.
    //   - Any portion outside `[0, PIXELS_PER_SCANLINE) x [0, SCANLINES)` is clipped;
    //     index `0x00` is transparent and never overwrites `buffer`.
    void blit_sprite(
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
    );

    // short hand overloaded function that takes a pointer to a sprite_t struct
    inline void blit_sprite(sprite_t *s) {
        blit_sprite(
            s->gfx_addr,
            s->dst_x,
            s->dst_y,
            s->sprite_w,
            s->sprite_h,
            s->sprite_id,
            s->flip_x,
            s->flip_y,
            s->double_x,
            s->double_y
        );
    }

    // Renders a `map_w x map_h` grid of byte-sized tile indices starting at
    // `map_addr` in `ram`, using `blit_sprite` to draw every visible cell.
    //
    // Grid and scaling behavior:
    //   - Each cell's on-screen footprint is `sprite_w * (double_x ? 2 : 1)` by
    //     `sprite_h * (double_y ? 2 : 1)` pixels starting at `(dst_x, dst_y)`.
    //   - When `repeat` is true, grid coordinates `(tx, ty)` wrap infinitely across
    //     `[0, map_w)` and `[0, map_h)` (including negative indices). When false,
    //     rendering is restricted to the finite `map_w x map_h` map bounds.
    //   - `flip_x`, `flip_y`, `double_x`, and `double_y` are forwarded directly to
    //     `blit_sprite` for each individual tile.
    void blit_tilemap(
        uint32_t map_addr,
        int16_t  dst_x,
        int16_t  dst_y,
        uint8_t  map_w,
        uint8_t  map_h,
        bool     flip_x,
        bool     flip_y,
        bool     double_x,
        bool     double_y,
        bool     repeat,
        
        uint32_t gfx_addr,
        uint8_t  sprite_w,
        uint8_t  sprite_h
    );

};

#endif // VDC_HPP
