#ifndef VCAR_H
#define VCAR_H

#include <pthread.h>
#include <cstdlib>
#include <ctime>
#include <future>
#include <list>
#include <map>
#include <mutex>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

extern "C" {
    #include <linux/can.h>
    #include <linux/can/raw.h>

    #include <lib.h>
}

#define VCAR_NAME "vcar"

class vcar {
    int sock{};
    struct ifreq ifr {};
    struct sockaddr_can addr {};
    struct canfd_frame frame {};
    int enable_canfd = 1;
    int required_mtu{};

    std::mutex incoming_connections_mutex;
    std::list<std::future<int> > incoming_connections;

    std::future<void> vcar_future;

    std::map<uint32_t, std::map<uint64_t, void (*)()> > node_actions;

    bool running = true;
    void run();

public:
    explicit vcar(bool halted = true);
    virtual ~vcar() = default;

    // Disable copying
    vcar(vcar const&) = delete;
    vcar& operator=(vcar const&) = delete;

    // Default move
    vcar(vcar &&) = delete;
    vcar& operator=(vcar &&) = delete;

    bool launch();
    void halt();

    void registerNodeAction(uint32_t node_id, uint64_t action_id, void (*node_action)());

    int sendCanFrame(char* sFrame);
    int recvCanFrame();
};

#endif
