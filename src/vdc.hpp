// ---------------------------------------------------------------------
// vdc.hpp
// hydra
//
// ---------------------------------------------------------------------

#ifndef VDC_HPP
#define VDC_HPP

#include <array>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <cstring>

#include "common.hpp"

class vdc_t {
public:
    alignas(64) std::array<uint8_t, PIXELS_PER_SCANLINE * SCANLINES> buffer;
    alignas(64) std::vector<uint8_t> ram;

    vdc_t() : ram (RAM_SIZE, 0) {}

    void clear(uint8_t color);

    void blit_sprite(
        uint32_t gfx_addr,
        int16_t  dst_x,
        int16_t  dst_y,
        uint8_t  sprite_w,
        uint8_t  sprite_h,
        uint8_t  sprite_id,
        bool     flip_x,
        bool     flip_y
    );

    void blit_tilemap(
        uint32_t map_addr,
        int16_t  dst_x,
        int16_t  dst_y,
        uint8_t  map_w,
        uint8_t  map_h,
        uint32_t tileset_addr,
        uint8_t  tile_w,
        uint8_t  tile_h,
        bool     flip_x,
        bool     flip_y,
        bool     repeat
    );

};

#endif // VDC_HPP
