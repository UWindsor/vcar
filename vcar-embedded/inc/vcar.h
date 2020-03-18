#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include <lib.h>

class vcar {
    int sock;
    struct ifreq ifr;
    struct sockaddr_can addr;
    struct canfd_frame frame;
    bool enable_canfd = true;
    int required_mtu;

public:
    vcar();
};
