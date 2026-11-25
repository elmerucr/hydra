



#include "system.hpp"
#include "host.hpp"

system_t::system_t()
{
    host = new host_t();
}

system_t::~system_t()
{
    delete host;
}

void system_t::run()
{
    //
}
