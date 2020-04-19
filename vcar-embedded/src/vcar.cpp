#include <cstring>
#include <iostream>
#include <logger.h>
#include <sstream>
#include <unistd.h>
#include <vcar.h>

int canGatewayCheckMTU(struct ifreq* ifr) {
    if (ifr->ifr_mtu != CANFD_MTU) {
        Log("CAN interface does not support FD.");
        return 0;
    }

    return 1;
}

vcar::vcar() {
    if ((sock = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        Log("Couldn't create a socket to send frame to node.");
        // TODO: Throw error
    }

    // Copy CAN interface name to ifr_name m/nb
    strncpy(ifr.ifr_name, VCAR_NAME, IF_NAMESIZE - 1);
    ifr.ifr_name[IF_NAMESIZE - 1] = '\0';
    ifr.ifr_ifindex = if_nametoindex(ifr.ifr_name);

    if (!ifr.ifr_ifindex) {
        Log("Couldn't convert ifr_name to index.");
        // TODO: Throw error
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    // Frame size = CAN netdevice space
    if (ioctl(sock, SIOCGIFMTU, &ifr) < 0) {
        Log("Couldn't fit can_frame in netdevice.");
        // TODO: Throw error
    }

    // Frame is MTU but interface does not support it
    if (!canGatewayCheckMTU(&ifr)) {
        // TODO: Throw error
    }

    if (setsockopt(sock, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable_canfd, sizeof(enable_canfd))) {
        Log("Couldn't enable CAN FD support.");
        // TODO: Throw error
    }

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        Log("Couldn't bind to the interface address.");
        // TODO: Throw error
    }

    vcar_future = std::async(std::launch::async, &vcar::start, this);
}

void vcar::start() {
    while (running) {
        /// Incoming communications

        if (incoming_connections.size() < 5 && incoming_connections_mutex.try_lock()) {
            incoming_connections_mutex.unlock();
            incoming_connections.push_back(std::async(std::launch::async, &vcar::recvCanFrame, this));
        }
        else {
            incoming_connections.remove_if([](std::future<int>& connection) {
                return connection.wait_for(std::chrono::milliseconds(5)) == std::future_status::ready;
            });
        }
    }
}

int vcar::sendCanFrame(char* sFrame) {
    required_mtu = parse_canframe(sFrame, &frame);

    if (write(sock, &frame, required_mtu) != required_mtu) {
        Log("Couldn't write through socket.");
        return 0;
    }

    close(sock);

    return 1;
}

int vcar::recvCanFrame() {
    struct can_frame in_frame {};

    int nbytes = read(sock, &in_frame, sizeof(struct can_frame));

    if (nbytes < 0) {
        Log("Failed to read from CAN RAW socket");
        return 0;
    }

    if (nbytes < sizeof(struct can_frame)) {
        Log("Incomplete can frame received");
        return 0;
    }

    std::stringstream can_id;
    can_id << std::hex << static_cast<int>(in_frame.can_id);

    std::cout << "can_id: " << can_id.str() << std::endl;
    std::cout << "data: ";
    for (int i = 0; i < in_frame.can_dlc; i++) {
        std::cout << std::hex << static_cast<int>(in_frame.data[i]);
    }
    std::cout << std::endl;

    // TODO: Parse message

    return 0;
}
