#ifndef SYSTEM_HPP
#define SYSTEM_HPP

class host_t;

class system_t {
private:
public:
    system_t();
    ~system_t();

    host_t *host;

    void run();
};

#endif // SYSTEM_HPP
