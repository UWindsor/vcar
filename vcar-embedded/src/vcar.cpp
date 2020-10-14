#include <cstring>
#include <iostream>
#include <logger.h>
#include <stdexcept>
#include <unistd.h>
#include <vcar.h>

int canGatewayCheckMTU(struct ifreq* ifr) {
    if (ifr->ifr_mtu != CANFD_MTU) {
        Log("CAN interface does not support FD.");
        return 0;
    }

    return 1;
}

vcar::vcar(bool halted) {
    if (!halted) {
        launch();
    }
}

bool vcar::launch() {
    if (running) {
        return false;
    }

    if ((sock = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        Log("Couldn't create a socket to send frame to node.");
        throw std::runtime_error("Couldn't create a socket to send frame to node.");
    }

    // Copy CAN interface name to ifr_name m/nb
    strncpy(ifr.ifr_name, VCAR_NAME, IF_NAMESIZE - 1);
    ifr.ifr_name[IF_NAMESIZE - 1] = '\0';
    ifr.ifr_ifindex = if_nametoindex(ifr.ifr_name);

    if (!ifr.ifr_ifindex) {
        Log("Couldn't convert ifr_name to index.");
        throw std::runtime_error("Couldn't convert ifr_name to index.");
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    // Frame size = CAN netdevice space
    if (ioctl(sock, SIOCGIFMTU, &ifr) < 0) {
        Log("Couldn't fit can_frame in netdevice.");
        throw std::runtime_error("Couldn't fit can_frame in netdevice.");
    }

    // Frame is MTU but interface does not support it
    if (!canGatewayCheckMTU(&ifr)) {
        throw std::runtime_error("Frame is MTU but interface does not support it");
    }

    if (setsockopt(sock, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable_canfd, sizeof(enable_canfd))) {
        Log("Couldn't enable CAN FD support.");
        throw std::runtime_error("Couldn't enable CAN FD support.");
    }

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        Log("Couldn't bind to the interface address.");
        throw std::runtime_error("Couldn't bind to the interface address.");
    }

    vcar_future = std::async(std::launch::async, &vcar::run, this);

    return true;
}

void vcar::halt() {
    running = false;
}

void vcar::run() {
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

void vcar::registerNodeAction(uint32_t node_id, uint64_t action_id, void (*node_action)()) {
    if (node_actions.find(node_id) == node_actions.end()) {
        node_actions[node_id] = std::map<uint64_t, void (*)()>();
    }

    node_actions[node_id][action_id] = node_action;
}

int vcar::sendCanFrame(char* sFrame) {
    required_mtu = parse_canframe(sFrame, &frame);

    if (write(sock, &frame, required_mtu) != required_mtu) {
        Log("Couldn't write through socket.");
        return 0;
    }

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

    uint32_t node_id = static_cast<uint32_t>(in_frame.can_id);
    uint64_t action_id = getDataAsUint64FromCanFrame(in_frame);

    std::cout << "can_id: " << std::hex << node_id << std::endl;
    std::cout << "data: " << std::hex << action_id << std::endl;

    if (node_actions.find(node_id) == node_actions.end() || node_actions[node_id].find(action_id) == node_actions[node_id].end()) {
        return 0;
    }

    node_actions[node_id][action_id]();  // Call node action if it exists

    return 1;
}
