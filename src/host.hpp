// ---------------------------------------------------------------------
// host.hpp
// hydra
//
// Copyright © 2026 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#ifndef HOST_HPP
#define HOST_HPP

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "common.hpp"

class host_t {
private:


    bool scanlines = true;
    uint8_t scanlines_alpha = 0xaa;
public:
    host_t();
    ~host_t();
};

#endif // HOST_HPP
