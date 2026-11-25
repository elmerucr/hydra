// ---------------------------------------------------------------------
// main.cpp
// hydra
//
// Copyright © 2026 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#include <iostream>
#include "system.hpp"

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

	system_t *system = new system_t();
	system->run();
	delete system;

    return 0;
}
