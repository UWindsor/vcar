#include <iostream>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vcar.h>

#include <linux/can.h>

#define NODE_DOOR_CONTROL_UNIT 0x001
#define NODE_SPEED_CONTROL_UNIT 0x002

// TODO: Move these to door control unit
#define ACTION_DOOR_UNLOCK                  0x0000000000000000
#define ACTION_DOOR_LOCK                    0x0000000000000001
#define ACTION_DOOR_WINDOW_DOWN             0x0000000000000002
#define ACTION_DOOR_WINDOW_UP               0x0000000000000003
#define ACTION_DOOR_WINDOW_STOP             0x0000000000000004
#define ACTION_DOOR_TOGGLE_CHILD_LOCK       0x0000000000000005

void node_dcu_lock();
void node_dcu_unlock();
void node_dcu_window_up();
void node_dcu_window_down();
void node_dcu_window_stop();
void node_dcu_toggle_child_lock();

// TODO: Move these to speed control unit
#define ACTION_SCU_DISABLE_CRUISE_CONTROL   0x0000000000000000
#define ACTION_SCU_ENABLE_CRUISE_CONTROL    0x0000000000000001

void node_scu_enable_cruise_control();
void node_scu_disable_cruise_control();

void send_post(std::string message) {
    std::stringstream cmd;
    cmd << "curl -X POST -H 'Content-Type: application/x-www-form-urlencoded' -d 'message=" << message << "' http://localhost:5000/hook >/dev/null 2>&1";
    system(cmd.str().c_str());
}





#define to_rest_fifo "/tmp/emb-rest"
#define from_rest_fifo "/tmp/rest-emb"

bool running = true;

int main() {
    vcar vc;

    // Register door control actions
    vc.registerNodeAction(NODE_DOOR_CONTROL_UNIT, ACTION_DOOR_UNLOCK, node_dcu_unlock);
    vc.registerNodeAction(NODE_DOOR_CONTROL_UNIT, ACTION_DOOR_LOCK, node_dcu_lock);
    vc.registerNodeAction(NODE_DOOR_CONTROL_UNIT, ACTION_DOOR_WINDOW_DOWN, node_dcu_window_down);
    vc.registerNodeAction(NODE_DOOR_CONTROL_UNIT, ACTION_DOOR_WINDOW_UP, node_dcu_window_up);
    vc.registerNodeAction(NODE_DOOR_CONTROL_UNIT, ACTION_DOOR_WINDOW_STOP, node_dcu_window_stop);
    vc.registerNodeAction(NODE_DOOR_CONTROL_UNIT, ACTION_DOOR_TOGGLE_CHILD_LOCK, node_dcu_toggle_child_lock);

    // Register speed control actions
    vc.registerNodeAction(NODE_SPEED_CONTROL_UNIT, ACTION_SCU_DISABLE_CRUISE_CONTROL, node_scu_disable_cruise_control);
    vc.registerNodeAction(NODE_SPEED_CONTROL_UNIT, ACTION_SCU_ENABLE_CRUISE_CONTROL, node_scu_enable_cruise_control);

    // Setup REST communication pipes
    mkfifo(to_rest_fifo, 0666);
    mkfifo(from_rest_fifo, 0666);

    // REST communication parameters
    int fd;
    char rest_req[500];
    std::string rest_res;

    while (running) {
        fd = open(from_rest_fifo, O_RDONLY);
        read(fd, rest_req, 500);
        close(fd);

        std::cout << "REST request: " << rest_req << std::endl;

        // TODO: Execute request and generate response
        rest_res = "SAMPLE RESPONSE";

        fd = open(to_rest_fifo, O_WRONLY);
        write(fd, rest_res.c_str(), rest_res.size() + 1);
        close(fd);
    }

    return 0;
}

/// Door control unit

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
    send_post(std::string("DOOR: unlocked"));
}

void node_dcu_window_stop() {
    std::cout << "Stopped rolling window" << std::endl;;
    send_post(std::string("DOOR: window rolling stopped"));
}

void node_dcu_window_down() {
    std::cout << "Rolling window down..." << std::endl;;
    send_post(std::string("DOOR: window rolling down"));
}

void node_dcu_window_up() {
    std::cout << "Rolling window up..." << std::endl;;
    send_post(std::string("DOOR: window rolling up"));
}

void node_dcu_toggle_child_lock() {
    child_lock = !child_lock;
    std::string state = child_lock ? "enabled" : "disabled";
    std::cout << "Child lock " << state << std::endl;
    send_post(std::string("DOOR: child lock "+state));
}

/// Speed control unit

int cruise_control = 0;
int cruise_control_speed = 0;

void node_scu_enable_cruise_control() {
    cruise_control = 1;
    std::cout << "Cruise control enabled" << std::endl;
    send_post(std::string("CRUISE CONTROL: enabled"));
}

void node_scu_disable_cruise_control() {
    cruise_control = 0;
    std::cout << "Cruise control disabled" << std::endl;
    send_post(std::string("CRUISE CONTROL: disabled"));
}
