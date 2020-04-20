/*
    Author: Mostapha Rammo
    
    use something like `cansend vcar 5A1#48656c6f` to send data onto the
    can bus. cansend documentation is helpful.

    ONLY LISTENS TO CAN FRAMES BEGINNING WITH 0x00 
*/

#include <iostream>
#include <sstream>
#include <vcar.h>

#include <linux/can.h>

#include <unistd.h>
#include <string.h>
#include <string>

#define NODE_DOOR 0x001
#define NODE_STEERING_WHEEL 0x002

#define ACTION_DOOR_UNLOCK 0xAABBCCDDEEFF1122
#define ACTION_DOOR_LOCK 0x1234567890123456
#define ACTION_DOOR_WINDOW_DOWN 0x000000000002
#define ACTION_DOOR_WINDOW_UP 0x000000000003
#define ACTION_DOOR_WINDOW_STOP 0x000000000004

void door_controller(can_frame);
void node_door_lock();
void node_door_unlock();
void node_door_window_up();
void node_door_window_down();
void node_door_window_up();
void node_door_window_stop();

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
        
        /*
             Check if frame belongs to self and work accordingly
        */
        if (frame.can_id == 0) {
            /*
            std::stringstream cmd;
            cmd << "cansend vcar 5A1#";
            for (int i = 0; i < frame.can_dlc; i++)
                cmd << std::hex << static_cast<int>(frame.data[i]);
            */
            std::cout << "This frame belongs to me!" << std::endl;
            //system(cmd.str().c_str());
        } else if (frame.can_id == NODE_DOOR) {
            std::cout << "This frame belongs to the door" << std::endl;
            door_controller(frame);
        }
    }

    return 0;
}

// Move to a separate class in nodes directory
void door_controller(can_frame frame) {
    switch(getDataAsUint64FromCanFrame(frame)) {
        case ACTION_DOOR_LOCK : node_door_lock(); break;
        case ACTION_DOOR_UNLOCK : node_door_unlock(); break;
        case ACTION_DOOR_WINDOW_UP : node_door_window_up(); break;
        case ACTION_DOOR_WINDOW_DOWN : node_door_window_down(); break;
        case ACTION_DOOR_WINDOW_STOP : node_door_window_stop(); break;
        default: break;
    }
}


// Put these in the door controller or a door.h header?
void node_door_lock() {
    std::cout << "Locked door" << std::endl;;
}

void node_door_unlock() {
    std::cout << "Unlocked door" << std::endl;;
}

void node_door_window_stop() {
    std::cout << "Stopped rolling window" << std::endl;;
}

void node_door_window_down() {
    std::cout << "Rolling window down..." << std::endl;;
}

void node_door_window_up() {
    std::cout << "Rolling window up..." << std::endl;;
}
