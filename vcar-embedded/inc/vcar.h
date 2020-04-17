#include <pthread.h>
#include <cstdlib>
#include <ctime>

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
    int sock;
    struct ifreq ifr;
    struct sockaddr_can addr;
    struct canfd_frame frame;
    int enable_canfd = 1;
    int required_mtu;

public:
    vcar();
    int sendCanFrame(char* sFrame);
};
