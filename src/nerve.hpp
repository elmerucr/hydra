// ---------------------------------------------------------------------
// nerve.hpp
// hydra
//
// Copyright © 2026 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#ifndef NERVE_HPP
#define NERVE_HPP

#include <lua.hpp>

class system_t;

class nerve_t {
private:
    lua_State *L;
    system_t  *sys;

    static int l_set_bg_color(lua_State *L);

public:
    explicit nerve_t(system_t *owner);
    ~nerve_t();

    nerve_t(const nerve_t &) = delete;
    nerve_t &operator=(const nerve_t &) = delete;

    bool run_string(const char *code);
    void call_if_exists(const char *func_name);
};

#endif // NERVE_HPP
