#include <cstring>
#include <logger.h>
#include <unistd.h>
#include <vcar.h>

vcar::vcar() {

}

int canGatewayCheckMTU(struct ifreq* ifr) {
    if (ifr->ifr_mtu != CANFD_MTU) {
        Log("CAN interface does not support FD.");
        return 0;
    }

    return 1;
}

int vcar::sendCanFrame(char* sFrame) {
    if ((sock = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        Log("Couldn't create a socket to send frame to node.");
        return 0;
    }

    required_mtu = parse_canframe(sFrame, &frame);

    // Copy CAN interface name to ifr_name m/nb
    strncpy(ifr.ifr_name, VCAR_NAME, IF_NAMESIZE - 1);
    ifr.ifr_name[IF_NAMESIZE - 1] = '\0';
    ifr.ifr_ifindex = if_nametoindex(ifr.ifr_name);

    if (!ifr.ifr_ifindex) {
        Log("Couldn't convert ifr_name to index.");
        return 0;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    // Frame size = CAN netdevice space
    if (ioctl(sock, SIOCGIFMTU, &ifr) < 0) {
        Log("Couldn't fit can_frame in netdevice.");
        return 0;
    }

    // Frame is MTU but interface does not support it
    if (!canGatewayCheckMTU(&ifr)) {
        return 0;
    }

    if (setsockopt(sock, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable_canfd, sizeof(enable_canfd))) {
        Log("Couldn't enable CAN FD support.");
        return 0;
    }

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        Log("Couldn't bind to the interface address.");
        return 0;
    }

    if (write(sock, &frame, required_mtu) != required_mtu) {
        Log("Couldn't write through socket.");
        return 0;
    }

    close(sock);

    return 1;
}