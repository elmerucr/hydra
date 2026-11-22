// ---------------------------------------------------------------------
// main.cpp
// hydra
//
// Copyright © 2026 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <lua.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "common.hpp"
#include "rca.hpp"
#include "vdc.hpp"

#include <cstdint>

const uint8_t hero[] = {
    #embed "hero.bin"
};

// 16x16 Heroine with a Sword (0x00 = transparent, 0x01..0x0F = dark to bright greyscale)
// [0..255]: facing front, sword on right (sprite_id 0)
// [256..511]: facing back, sword on right (sprite_id 1)
constexpr uint8_t swordmaster[16 * 32] = {
    0x00, 0x00, 0x00, 0x04, 0x05, 0x05, 0x05, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 0: Top of hair
    0x00, 0x00, 0x04, 0x07, 0x08, 0x08, 0x07, 0x05, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, // Row 1: Hair & sword tip
    0x00, 0x04, 0x0c, 0x0d, 0x0d, 0x0d, 0x0c, 0x06, 0x04, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x0f, 0x00, // Row 2: Headband & blade
    0x00, 0x04, 0x06, 0x0b, 0x0c, 0x0b, 0x0c, 0x06, 0x04, 0x00, 0x00, 0x00, 0x0d, 0x0f, 0x0e, 0x00, // Row 3: Forehead & blade
    0x00, 0x04, 0x05, 0x0b, 0x01, 0x0b, 0x01, 0x05, 0x04, 0x00, 0x00, 0x0d, 0x0f, 0x0e, 0x00, 0x00, // Row 4: Eyes & blade
    0x00, 0x03, 0x05, 0x0a, 0x0b, 0x0b, 0x0a, 0x05, 0x03, 0x00, 0x0d, 0x0f, 0x0e, 0x00, 0x00, 0x00, // Row 5: Cheeks & blade
    0x00, 0x00, 0x03, 0x04, 0x09, 0x09, 0x04, 0x03, 0x00, 0x0d, 0x0f, 0x0e, 0x00, 0x00, 0x00, 0x00, // Row 6: Chin & blade
    0x00, 0x00, 0x03, 0x07, 0x08, 0x08, 0x07, 0x03, 0x08, 0x0b, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 7: Shoulders & crossguard
    0x00, 0x02, 0x0a, 0x07, 0x09, 0x09, 0x07, 0x0a, 0x0b, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 8: Arms, tunic & hilt
    0x00, 0x02, 0x0a, 0x06, 0x08, 0x08, 0x06, 0x0a, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 9: Torso & hands
    0x00, 0x00, 0x02, 0x03, 0x04, 0x04, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 10: Belt
    0x00, 0x00, 0x05, 0x07, 0x08, 0x08, 0x07, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 11: Tunic skirt
    0x00, 0x02, 0x05, 0x06, 0x07, 0x07, 0x06, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 12: Lower tunic
    0x00, 0x00, 0x02, 0x09, 0x00, 0x00, 0x09, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 13: Legs
    0x00, 0x00, 0x02, 0x03, 0x00, 0x00, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 14: Boots
    0x00, 0x01, 0x02, 0x02, 0x00, 0x00, 0x02, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 15: Boot soles

    0x00, 0x00, 0x00, 0x04, 0x05, 0x05, 0x05, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 0 (back): Crown of hair
    0x00, 0x00, 0x04, 0x06, 0x07, 0x07, 0x06, 0x05, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, // Row 1 (back): Back of hair & sword tip
    0x00, 0x04, 0x06, 0x07, 0x08, 0x08, 0x07, 0x06, 0x04, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x0f, 0x00, // Row 2 (back): Back of head & blade
    0x00, 0x04, 0x06, 0x07, 0x08, 0x08, 0x07, 0x06, 0x04, 0x00, 0x00, 0x00, 0x0d, 0x0f, 0x0e, 0x00, // Row 3 (back): Hair shading & blade
    0x00, 0x04, 0x05, 0x06, 0x07, 0x07, 0x06, 0x05, 0x04, 0x00, 0x00, 0x0d, 0x0f, 0x0e, 0x00, 0x00, // Row 4 (back): Hair shading & blade
    0x00, 0x03, 0x05, 0x06, 0x06, 0x06, 0x06, 0x05, 0x03, 0x00, 0x0d, 0x0f, 0x0e, 0x00, 0x00, 0x00, // Row 5 (back): Lower hair & blade
    0x00, 0x00, 0x03, 0x04, 0x05, 0x05, 0x04, 0x03, 0x00, 0x0d, 0x0f, 0x0e, 0x00, 0x00, 0x00, 0x00, // Row 6 (back): Neck/hair & blade
    0x00, 0x00, 0x03, 0x06, 0x07, 0x07, 0x06, 0x03, 0x08, 0x0b, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 7 (back): Back shoulders & crossguard
    0x00, 0x02, 0x08, 0x06, 0x07, 0x07, 0x06, 0x08, 0x0b, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 8 (back): Back/arms & hilt
    0x00, 0x02, 0x08, 0x05, 0x07, 0x07, 0x05, 0x08, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 9 (back): Mid back
    0x00, 0x00, 0x02, 0x03, 0x04, 0x04, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 10 (back): Waist/belt
    0x00, 0x00, 0x05, 0x06, 0x07, 0x07, 0x06, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 11 (back): Back of tunic
    0x00, 0x02, 0x05, 0x06, 0x06, 0x06, 0x06, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 12 (back): Lower back tunic
    0x00, 0x00, 0x02, 0x08, 0x00, 0x00, 0x08, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 13 (back): Back of legs
    0x00, 0x00, 0x02, 0x03, 0x00, 0x00, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 14 (back): Boots
    0x00, 0x01, 0x02, 0x02, 0x00, 0x00, 0x02, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // Row 15 (back): Boot soles
};

const bool scanlines = true;
const uint8_t scanlines_alpha = 0xaa;

// Shared state controlled by Lua
struct AppState {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

// C++ function exposed to Lua: set_bg_color(r, g, b)
static int l_set_bg_color(lua_State* L) {
    AppState *state = static_cast<AppState*>(lua_touserdata(L, lua_upvalueindex(1)));
    state->r = static_cast<uint8_t>(luaL_checkinteger(L, 1));
    state->g = static_cast<uint8_t>(luaL_checkinteger(L, 2));
    state->b = static_cast<uint8_t>(luaL_checkinteger(L, 3));
    return 0;
}

// static int l_pset(lua_State *L) {
// 	uint8_t x = static_cast<uint8_t>(luaL_checkinteger(L, 1));
// }

inline uint32_t video_blend(uint32_t c0, uint32_t c1) {
	return
		((((c0 & 0x00ff00ff) + (c1 & 0x00ff00ff)) >> 1) & 0x00ff00ff) |
		((((c0 & 0x0000ff00) + (c1 & 0x0000ff00)) >> 1) & 0x0000ff00) |
		(scanlines_alpha << 24); // alpha
}


int main(int argc, char* argv[]) {
    alignas(64) uint32_t blend_lut[256][256];

    for (int i=0; i<256; i++) {
        uint32_t c0 = palette[i];
        for (int j=0; j<256; j++) {
            blend_lut[i][j] = video_blend(c0, palette[j]);
        }
    }

	vdc_t vdc;

    // 1. Initialize SDL3
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "[SDL] SDL_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("hydra", 960, 540, SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }
	SDL_SetWindowAspectRatio(window, (float)PIXELS_PER_SCANLINE/SCANLINES, (float)PIXELS_PER_SCANLINE/SCANLINES);
	SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
	SDL_SetRenderVSync(renderer, 1);
	int i;
    SDL_GetRenderVSync(renderer, &i);
	if (i) {
        printf("[SDL] vsync every %i frame(s)\n", i);
    } else {
		printf("[SDL] vsync is off\n");
    }

	SDL_SetRenderLogicalPresentation(renderer, PIXELS_PER_SCANLINE, SCANLINES, SDL_LOGICAL_PRESENTATION_LETTERBOX);
	SDL_HideCursor();				// make sure cursor isn't visible

	SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, PIXELS_PER_SCANLINE, 2*SCANLINES);
	uint8_t vdc_texture_bytes_per_pixel = (SDL_BITSPERPIXEL(texture->format) >> 3);
	SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_PIXELART);
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_ADD);


    std::memcpy(vdc.ram.data() + 0x30000, swordmaster, sizeof(swordmaster));
    std::memcpy(vdc.ram.data() + 0x30200, hero, sizeof(hero));
    

    // 2. Initialize Lua and register C++ callback
    AppState state;
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

	std::cout << "[Lua] Release: " << LUA_RELEASE << "\n"; // e.g., "Lua 5.5.1"

    lua_pushlightuserdata(L, &state);
    lua_pushcclosure(L, l_set_bg_color, 1);
    lua_setglobal(L, "set_bg_color");

    // 3. Run an inline Lua script to set initial color and define a key handler
    const char* lua_script = R"lua(
        print("Running " .. _VERSION .. " inside SDL3!")
		r = 0
		g = 0
		b = 0
        set_bg_color(r, g, b) -- Initial black

        function on_space_pressed()
		  r = math.random(0, 255)
		  g = math.random(0, 255)
		  b = math.random(0, 255)
    	  set_bg_color(r, g, b)
        end
    )lua";

    if (luaL_dostring(L, lua_script) != LUA_OK) {
        std::cerr << "Lua error: " << lua_tostring(L, -1) << "\n";
        lua_pop(L, 1);
    }

    // 4. Main SDL3 event and render loop
    bool running = true;
    SDL_Event event;

    typedef struct {
        int x;
        int y;
        int dx;
        int dy;
    } pos;

    rca_t rca;
    pos swordmaster_array[100];
    for (int i=0;i<100;i++) {
        swordmaster_array[i].x = rca.next_range(0,320-16);
        swordmaster_array[i].y = rca.next_range(0,180-16);
        swordmaster_array[i].dx = rca.next_bit() ? 1 : -1;
        swordmaster_array[i].dy = rca.next_bit() ? 1 : -1;
    }

    for (int i=0; i<64; i++) {
        vdc.ram[0x000000 + i] = 0x18;
        vdc.ram[0x000040 + i] = 0x45;
    }
    vdc.ram[0x80] = 0;
    vdc.ram[0x81] = 1;
    vdc.ram[0x82] = 0;
    vdc.ram[0x83] = 1;
    vdc.ram[0x84] = 0;
    vdc.ram[0x85] = 1;
    vdc.ram[0x86] = 0;
    vdc.ram[0x87] = 1;
    vdc.ram[0x88] = 0;

    int16_t x = -48;
    int16_t y = -48;

    const bool *keys = SDL_GetKeyboardState(NULL);
    bool upside_down;
    bool hor_flip;
    int16_t hero_x = 155;
    int16_t hero_y = 149;
    bool hor_double;
    bool ver_double;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            } else if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_SPACE) {
                // Call Lua function when Space is pressed
                lua_getglobal(L, "on_space_pressed");
                if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
                    std::cerr << "Lua callback error: " << lua_tostring(L, -1) << "\n";
                    lua_pop(L, 1);
                }
            }
        }

        upside_down = keys[SDL_SCANCODE_V] ? true : false;
        hor_flip = keys[SDL_SCANCODE_H] ? true : false;
        hor_double = keys[SDL_SCANCODE_X] ? true : false;
        ver_double = keys[SDL_SCANCODE_Y] ? true : false;
        if (keys[SDL_SCANCODE_RIGHT]) hero_x++;
        if (keys[SDL_SCANCODE_LEFT]) hero_x--;
        if (keys[SDL_SCANCODE_UP]) hero_y--;
        if (keys[SDL_SCANCODE_DOWN]) hero_y++;

        SDL_SetRenderDrawColor(renderer, state.r, state.g, state.b, 255);
        SDL_RenderClear(renderer);

        vdc.clear(0x93);

        vdc.blit_tilemap(0x000080, x, y, 3, 3, hor_flip, upside_down, hor_double, ver_double, true, 0x30000, 16, 16);
        x++; y++;
        if (x==48) { x = -48; y = -48; }

        for (int i=0; i<100; i++) {
            vdc.blit_sprite(
                0x30000,
                swordmaster_array[i].x,
                swordmaster_array[i].y,
                16,
                16,
                swordmaster_array[i].dy == 1 ? 0 : 1,
                swordmaster_array[i].dx == 1 ? false : true,
                false,
                false,
                false
            );
            swordmaster_array[i].x += swordmaster_array[i].dx;
            if (swordmaster_array[i].x < 1 || swordmaster_array[i].x > (320-16)) swordmaster_array[i].dx = -swordmaster_array[i].dx;
            swordmaster_array[i].y += swordmaster_array[i].dy;
            if (swordmaster_array[i].y < 1 || swordmaster_array[i].y > (180-16)) swordmaster_array[i].dy = -swordmaster_array[i].dy;
        }

        // do checkerboard
        vdc.blit_tilemap(0x000080, 43, 43, 3, 3, false, false, false, false, false, 0x000000, 8, 8);

        // hero
        vdc.blit_sprite(0x30200, hero_x, hero_y, 10, 31, 0, hor_flip, upside_down, hor_double, ver_double);

		void *pixels;
		int pitch;

		SDL_LockTexture(texture, nullptr, &pixels, &pitch);

        const int stride = pitch / vdc_texture_bytes_per_pixel;
        const int row_advance = (stride * 2) - PIXELS_PER_SCANLINE;

        const uint8_t *cur_pix_src = vdc.buffer.data();
        const uint8_t *nxt_pix_src = cur_pix_src + PIXELS_PER_SCANLINE;
        uint32_t *cur_pix_dst = static_cast<uint32_t *>(pixels);
        uint32_t *nxt_pix_dst = cur_pix_dst + stride;

        if (scanlines) {
			for (int y = 0; y < SCANLINES; y++) {
				if (y == (SCANLINES - 1)) nxt_pix_src = cur_pix_src;
				for (int x = 0; x < PIXELS_PER_SCANLINE; x++) {
                    const uint8_t idx0 = *cur_pix_src++;
                    const uint8_t idx1 = *nxt_pix_src++;
					*cur_pix_dst++ = palette[idx0];
					*nxt_pix_dst++ = blend_lut[idx0][idx1];
                }
				cur_pix_dst += row_advance;
				nxt_pix_dst += row_advance;
			}
        } else {
            for (int y=0; y < SCANLINES; y++) {
                for (int x=0; x < PIXELS_PER_SCANLINE; x++) {
                    const uint32_t c0 = palette[*cur_pix_src++];
                    *cur_pix_dst++ = c0;
                    *nxt_pix_dst++ = c0;
                }
                cur_pix_dst += row_advance;
                nxt_pix_dst += row_advance;
            }
        }

		SDL_UnlockTexture(texture);

		SDL_RenderTexture(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    // 5. Clean up
    lua_close(L);
	SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
