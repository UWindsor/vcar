#include <iostream>
#include <sstream>
#include <vcar.h>

#include <linux/can.h>

#include <unistd.h>
#include <string.h>
#include <string>

#define NODE_DOOR_CONTROL_UNIT 0x001
#define NODE_SPEED_CONTROL_UNIT 0x002

// Move these to door control unit
#define ACTION_DOOR_UNLOCK                  0x0000000000000000
#define ACTION_DOOR_LOCK                    0x0000000000000001
#define ACTION_DOOR_WINDOW_DOWN             0x0000000000000002
#define ACTION_DOOR_WINDOW_UP               0x0000000000000003
#define ACTION_DOOR_WINDOW_STOP             0x0000000000000004
#define ACTION_DOOR_TOGGLE_CHILD_LOCK       0x0000000000000005
void door_control_unit(can_frame);
void node_dcu_lock();
void node_dcu_unlock();
void node_dcu_window_up();
void node_dcu_window_down();
void node_dcu_window_up();
void node_dcu_window_stop();
void node_dcu_toggle_child_lock();

// Move these to speed control unit
#define ACTION_SCU_DISABLE_CRUISE_CONTROL   0x0000000000000000
#define ACTION_SCU_ENABLE_CRUISE_CONTROL    0x0000000000000001
void speed_control_unit(can_frame);
void node_scu_enable_cruise_control();
void node_scu_disable_cruise_control();

void send_post(std::string message) {
    std::stringstream cmd;
    cmd << "curl -X POST -H 'Content-Type: application/x-www-form-urlencoded' -d 'message=" << message << "' http://localhost:5000/hook >/dev/null 2>&1";
    system(cmd.str().c_str());
}

int main() {
    
    std::cout << "Welcome to vCar!" << std::endl;

    std::cout << "> Testing connections.." << std::endl;
    int s;
    struct sockaddr_can addr;
    struct ifreq ifr;

    s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (s < 0) {
        // Socket connection failed
        std::cerr << "> Test failed! Could not connect to CAN BUS, Try running configure.sh" << std::endl;
        return 1;
    }

    strcpy(ifr.ifr_name, "vcar");
    ioctl(s, SIOCGIFINDEX, &ifr);

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    bind(s, (struct sockaddr *)&addr, sizeof(addr));

    std::cout << "> Listening on BUS.." << std::endl;
    struct can_frame frame {};
    while (1) {
        int nbytes = read(s, &frame, sizeof(struct can_frame));

        // Error check
        if (nbytes < 0) {
            std::cerr << "> Failed to read from CAN RAW socket" << std::endl;
            continue;
        }
        if (nbytes < sizeof(struct can_frame)) {
            std::cerr << "> Incomplete CAN frame received" << std::endl;
            continue;
        }

        /* CAN frame is populated, do what you will */
        
        // Print out CAN frame data
        std::stringstream can_id;
        can_id << std::hex << static_cast<int>(frame.can_id);

        std::cout << "ID: " << can_id.str() << std::endl;
        std::cout << "DATA: " << std::hex << getDataAsUint64FromCanFrame(frame) << std::endl;

        switch (frame.can_id) {
            case NODE_DOOR_CONTROL_UNIT:    std::cout << "Forwarding frame to door control unit (DCU)..." << std::endl;
                                            door_control_unit(frame);
                                            break;
            case NODE_SPEED_CONTROL_UNIT:   std::cout << "Forwarding frame to speed control unit (SCU)..." << std::endl;
                                            speed_control_unit(frame);
                                            break;
            default:                        std::cout << "No ECU's responded to frame" << std::endl;
                                            break;
        }
    }

    return 0;
}

/*
    DOOR CONTROL UNIT

    Put these in the door_control_unit file
*/
void door_control_unit(can_frame frame) {
    switch(getDataAsUint64FromCanFrame(frame)) {
        case ACTION_DOOR_LOCK : node_dcu_lock(); break;
        case ACTION_DOOR_UNLOCK : node_dcu_unlock(); break;
        case ACTION_DOOR_WINDOW_UP : node_dcu_window_up(); break;
        case ACTION_DOOR_WINDOW_DOWN : node_dcu_window_down(); break;
        case ACTION_DOOR_WINDOW_STOP : node_dcu_window_stop(); break;
        case ACTION_DOOR_TOGGLE_CHILD_LOCK : node_dcu_toggle_child_lock(); break;
        default: break;
    }
}

int door_locked = 1;
int windows_locked = 1;
int child_lock = 0;

void node_dcu_lock() {
    door_locked = 1;
    std::cout << "Locked door" << std::endl;;
    send_post(std::string("DOOR: locked"));
}

void node_dcu_unlock() {
    door_locked = 0;
    std::cout << "Unlocked door" << std::endl;;
}

void node_dcu_window_stop() {
    std::cout << "Stopped rolling window" << std::endl;;
}

void node_dcu_window_down() {
    std::cout << "Rolling window down..." << std::endl;;
}

void node_dcu_window_up() {
    std::cout << "Rolling window up..." << std::endl;;
}

void node_dcu_toggle_child_lock() {
    child_lock = !child_lock;
    std::cout << "Child lock " << (child_lock ? "enabled" : "disabled") << std::endl;
}



/*
    SPEED CONTROL UNIT

    Put these in the speed_control_unit file
*/
void speed_control_unit(can_frame frame) {
    switch(getDataAsUint64FromCanFrame(frame)) {
        case ACTION_SCU_ENABLE_CRUISE_CONTROL : node_scu_enable_cruise_control(); break;
        case ACTION_SCU_DISABLE_CRUISE_CONTROL : node_scu_disable_cruise_control(); break;
        default: std::cout << "No action was taken" << std::endl; break;
    }
}

int cruise_control = 0;
int cruise_control_speed = 0;

void node_scu_enable_cruise_control() {
    cruise_control = 1;
    std::cout << "Cruise control enabled" << std::endl;;
}

void node_scu_disable_cruise_control() {
    cruise_control = 0;
    std::cout << "Cruise control disabled" << std::endl;;
}
