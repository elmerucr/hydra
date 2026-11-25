// ---------------------------------------------------------------------
// nerve.cpp
// hydra
//
// Copyright © 2026 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#include "nerve.hpp"

#include <cstdint>
#include <iostream>

#include "system.hpp"

int nerve_t::l_set_bg_color(lua_State *L)
{
    auto *sys = static_cast<system_t *>(lua_touserdata(L, lua_upvalueindex(1)));
    if (!sys) {
        return 0;
    }

    const uint8_t r = static_cast<uint8_t>(luaL_checkinteger(L, 1));
    const uint8_t g = static_cast<uint8_t>(luaL_checkinteger(L, 2));
    const uint8_t b = static_cast<uint8_t>(luaL_checkinteger(L, 3));

    //sys->set_bg_color(r, g, b);
    return 0;
}

nerve_t::nerve_t(system_t *owner)
    : L(nullptr)
    , sys(owner)
{
    L = luaL_newstate();
    if (!L) {
        std::cerr << "[Lua] Failed to create lua_State\n";
        return;
    }

    luaL_openlibs(L);
    std::cout << "[Lua] Release: " << LUA_RELEASE << "\n";

    lua_pushlightuserdata(L, sys);
    lua_pushcclosure(L, l_set_bg_color, 1);
    lua_setglobal(L, "set_bg_color");
}

nerve_t::~nerve_t()
{
    if (L) {
        lua_close(L);
        L = nullptr;
    }
}

bool nerve_t::run_string(const char *code)
{
    if (!L || !code) {
        return false;
    }

    if (luaL_dostring(L, code) != LUA_OK) {
        std::cerr << "Lua error: " << lua_tostring(L, -1) << "\n";
        lua_pop(L, 1);
        return false;
    }

    return true;
}

void nerve_t::call_if_exists(const char *func_name)
{
    if (!L || !func_name) {
        return;
    }

    lua_getglobal(L, func_name);
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        return;
    }

    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        std::cerr << "Lua callback error: " << lua_tostring(L, -1) << "\n";
        lua_pop(L, 1);
    }
}
