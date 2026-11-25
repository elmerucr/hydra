// ---------------------------------------------------------------------
// host.cpp
// hydra
//
// Copyright © 2026 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#include "host.hpp"

#include <iostream>
#include <cstdio>

namespace {

inline uint32_t video_blend(uint32_t c0, uint32_t c1, uint8_t alpha)
{
    return
        ((((c0 & 0x00ff00ff) + (c1 & 0x00ff00ff)) >> 1) & 0x00ff00ff) |
        ((((c0 & 0x0000ff00) + (c1 & 0x0000ff00)) >> 1) & 0x0000ff00) |
        (static_cast<uint32_t>(alpha) << 24);
}

constexpr std::size_t key_index(input_key_t k)
{
    return static_cast<std::size_t>(k);
}

} // namespace

host_t::host_t()
    : video_window(nullptr)
    , video_renderer(nullptr)
    , video_texture(nullptr)
    , video_texture_bytes_per_pixel(4)
    , video_scanlines(true)
    , video_scanlines_alpha(0xaa)
    , video_blend_lut{}
    , key_current{}
    , key_previous{}
{
    video_init();
}

host_t::~host_t()
{
    video_shutdown();
}

void host_t::video_init_blend_lut()
{
    for (int i = 0; i < 256; i++) {
        const uint32_t c0 = palette[i];
        for (int j = 0; j < 256; j++) {
            video_blend_lut[i][j] = video_blend(c0, palette[j], video_scanlines_alpha);
        }
    }
}

bool host_t::video_init()
{
    video_init_blend_lut();

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "[SDL] SDL_Init failed: " << SDL_GetError() << "\n";
        return false;
    }

    video_window = SDL_CreateWindow(
        "hydra",
        960,
        540,
        SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_RESIZABLE
    );
    if (!video_window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        video_shutdown();
        return false;
    }

    SDL_SetWindowAspectRatio(
        video_window,
        static_cast<float>(PIXELS_PER_SCANLINE) / SCANLINES,
        static_cast<float>(PIXELS_PER_SCANLINE) / SCANLINES
    );
    SDL_SetWindowPosition(video_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    video_renderer = SDL_CreateRenderer(video_window, nullptr);
    if (!video_renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        video_shutdown();
        return false;
    }

    SDL_SetRenderVSync(video_renderer, 1);
    int vsync = 0;
    SDL_GetRenderVSync(video_renderer, &vsync);
    if (vsync) {
        std::printf("[SDL] vsync every %i frame(s)\n", vsync);
    } else {
        std::printf("[SDL] vsync is off\n");
    }

    SDL_SetRenderLogicalPresentation(
        video_renderer,
        PIXELS_PER_SCANLINE,
        SCANLINES,
        SDL_LOGICAL_PRESENTATION_LETTERBOX
    );
    SDL_HideCursor();

    video_texture = SDL_CreateTexture(
        video_renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        PIXELS_PER_SCANLINE,
        2 * SCANLINES
    );
    if (!video_texture) {
        std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << "\n";
        video_shutdown();
        return false;
    }

    video_texture_bytes_per_pixel = static_cast<uint8_t>(SDL_BITSPERPIXEL(video_texture->format) >> 3);
    SDL_SetTextureScaleMode(video_texture, SDL_SCALEMODE_PIXELART);
    SDL_SetTextureBlendMode(video_texture, SDL_BLENDMODE_ADD);

    return true;
}

void host_t::video_shutdown()
{
    if (video_texture) {
        SDL_DestroyTexture(video_texture);
        video_texture = nullptr;
    }
    if (video_renderer) {
        SDL_DestroyRenderer(video_renderer);
        video_renderer = nullptr;
    }
    if (video_window) {
        SDL_DestroyWindow(video_window);
        video_window = nullptr;
    }
    if (SDL_WasInit(SDL_INIT_VIDEO)) {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }
}

void host_t::update_key_current()
{
    const bool *sdl_keys = SDL_GetKeyboardState(nullptr);
    if (!sdl_keys) {
        key_current.fill(false);
        return;
    }

    key_current[key_index(input_key_t::up)]    = sdl_keys[SDL_SCANCODE_UP];
    key_current[key_index(input_key_t::down)]  = sdl_keys[SDL_SCANCODE_DOWN];
    key_current[key_index(input_key_t::left)]  = sdl_keys[SDL_SCANCODE_LEFT];
    key_current[key_index(input_key_t::right)] = sdl_keys[SDL_SCANCODE_RIGHT];
    key_current[key_index(input_key_t::space)] = sdl_keys[SDL_SCANCODE_SPACE];
    key_current[key_index(input_key_t::v)]     = sdl_keys[SDL_SCANCODE_V];
    key_current[key_index(input_key_t::h)]     = sdl_keys[SDL_SCANCODE_H];
    key_current[key_index(input_key_t::x)]     = sdl_keys[SDL_SCANCODE_X];
    key_current[key_index(input_key_t::y)]     = sdl_keys[SDL_SCANCODE_Y];
}

enum events_output_state host_t::events_process_events()
{
    enum events_output_state return_value = NO_EVENT;

    key_previous = key_current;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            return_value = QUIT_EVENT;
        }
    }

    update_key_current();

    return return_value;
}

bool host_t::key_down(input_key_t key) const
{
    return key_current[key_index(key)];
}

bool host_t::key_pressed(input_key_t key) const
{
    const std::size_t idx = key_index(key);
    return key_current[idx] && !key_previous[idx];
}

bool host_t::key_released(input_key_t key) const
{
    const std::size_t idx = key_index(key);
    return !key_current[idx] && key_previous[idx];
} 

void host_t::video_update_screen(const uint8_t *buffer, uint8_t bg_r, uint8_t bg_g, uint8_t bg_b)
{
    if (!video_renderer || !video_texture) {
        return;
    }

    SDL_SetRenderDrawColor(video_renderer, bg_r, bg_g, bg_b, 255);
    SDL_RenderClear(video_renderer);

    void *pixels = nullptr;
    int pitch = 0;

    if (SDL_LockTexture(video_texture, nullptr, &pixels, &pitch)) {
        const int stride = pitch / video_texture_bytes_per_pixel;
        const int row_advance = (stride * 2) - PIXELS_PER_SCANLINE;

        const uint8_t *cur_pix_src = buffer;
        const uint8_t *nxt_pix_src = cur_pix_src + PIXELS_PER_SCANLINE;
        uint32_t *cur_pix_dst = static_cast<uint32_t *>(pixels);
        uint32_t *nxt_pix_dst = cur_pix_dst + stride;

        if (video_scanlines) {
            for (int y = 0; y < SCANLINES; y++) {
                if (y == (SCANLINES - 1)) {
                    nxt_pix_src = cur_pix_src;
                }
                for (int x = 0; x < PIXELS_PER_SCANLINE; x++) {
                    const uint8_t idx0 = *cur_pix_src++;
                    const uint8_t idx1 = *nxt_pix_src++;
                    *cur_pix_dst++ = palette[idx0];
                    *nxt_pix_dst++ = video_blend_lut[idx0][idx1];
                }
                cur_pix_dst += row_advance;
                nxt_pix_dst += row_advance;
            }
        } else {
            for (int y = 0; y < SCANLINES; y++) {
                for (int x = 0; x < PIXELS_PER_SCANLINE; x++) {
                    const uint32_t c0 = palette[*cur_pix_src++];
                    *cur_pix_dst++ = c0;
                    *nxt_pix_dst++ = c0;
                }
                cur_pix_dst += row_advance;
                nxt_pix_dst += row_advance;
            }
        }

        SDL_UnlockTexture(video_texture);
    }

    SDL_RenderTexture(video_renderer, video_texture, nullptr, nullptr);
    SDL_RenderPresent(video_renderer);
}
