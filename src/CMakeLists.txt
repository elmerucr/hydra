add_subdirectory(lua-5.5.1/src/)

add_library(system STATIC
    host.cpp
    system.cpp
    vdc.cpp
)
