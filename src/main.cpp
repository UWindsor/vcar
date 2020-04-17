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

int main() {
    
    std::cout << "Welcome to vCar!" << std::endl;
    
    std::cout << "> Wiring up the CAN BUS.." << std::endl;
    system("modprobe vcan");
    system("sudo ip link add dev vcar type vcan");
    system("sudo ip link set up vcar");

    std::cout << "> Testing connections.." << std::endl;
    int s;
    struct sockaddr_can addr;
    struct ifreq ifr;

    s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (s < 0) {
        // Socket connection failed
        std::cerr << "> Test failed! Could not connect to CAN BUS" << std::endl;
        return 1;
    }

    strcpy(ifr.ifr_name, "vcar");
    ioctl(s, SIOCGIFINDEX, &ifr);

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    bind(s, (struct sockaddr *)&addr, sizeof(addr));

    std::cout << "> Listening on BUS.." << std::endl;
    struct can_frame frame;
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
        std::cout << "DATA: ";
        for (int i = 0; i < frame.can_dlc; i++) {
            std::cout << std::hex << static_cast<int>(frame.data[i]);
        }
        std::cout << std::endl;
        
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
        }
    }

    return 0;
}
