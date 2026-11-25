// ---------------------------------------------------------------------
// host.hpp
// hydra
//
// Copyright © 2026 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#ifndef HOST_HPP
#define HOST_HPP

#include <array>
#include <cstdint>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "common.hpp"

enum events_output_state {
	QUIT_EVENT = -1,
	NO_EVENT = 0,
	KEYPRESS_EVENT = 1
};

enum class input_key_t : uint8_t {
    up,
    down,
    left,
    right,
    space,
    v,
    h,
    x,
    y,
    count
};

class host_t {
private:
    // video related
    SDL_Window   *video_window;
    SDL_Renderer *video_renderer;
    SDL_Texture  *video_texture;
    alignas(64) uint32_t video_blend_lut[256][256];
    void video_init_blend_lut();
    uint8_t video_texture_bytes_per_pixel;
    bool    video_scanlines;
    uint8_t video_scanlines_alpha;

    std::array<bool, static_cast<std::size_t>(input_key_t::count)> key_current;
    std::array<bool, static_cast<std::size_t>(input_key_t::count)> key_previous;

    void update_key_current();

public:
    host_t();
    ~host_t();

    bool video_init();
    void video_shutdown();
    void video_update_screen(const uint8_t *buffer, uint8_t bg_r, uint8_t bg_g, uint8_t bg_b);

    enum events_output_state events_process_events();

    bool key_down(input_key_t key) const;
    bool key_pressed(input_key_t key) const;
    bool key_released(input_key_t key) const;
};

#endif // HOST_HPP
