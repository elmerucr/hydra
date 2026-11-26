// ---------------------------------------------------------------------
// system.hpp
// hydra
//
// Copyright © 2026 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include <lua.hpp>
#include "vdc.hpp"

// Shared state controlled by Lua
struct AppState {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

class host_t;
class vdc_t;

class system_t {
private:
    bool running;

    lua_State* L;

    sprite_t hero_girl;
    sprite_t hydra;
    AppState state;

public:
    system_t();
    ~system_t();

    host_t *host;
    vdc_t *vdc;

    void run();
};

#endif // SYSTEM_HPP
