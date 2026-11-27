// ---------------------------------------------------------------------
// system.hpp
// hydra
//
// Copyright © 2026 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#ifndef SYSTEM_HPP
#define SYSTEM_HPP

class host_t;
class nerve_t;
class vdc_t;

class system_t {
private:
    bool running;

public:
    system_t();
    ~system_t();

    uint8_t r;
    uint8_t g;
    uint8_t b;

    host_t *host;
    nerve_t *nerve;
    vdc_t *vdc;

    void run();
};

#endif // SYSTEM_HPP
