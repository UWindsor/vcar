// TODO: Get the token from REST server to authorize vehicle actions because SQL is a hot garbage

#include <cstdlib>
#include <iostream>
#include <memory>
#include <restbed>
#include <string>
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

/// Everything above here needs to be redone





// TODO: REQUEST FUNCTIONS

void PostRegisterUser(const std::shared_ptr<restbed::Session> session);
void PostRegisterVehicle(const std::shared_ptr<restbed::Session> session);
void GetVehicle(const std::shared_ptr<restbed::Session> session);
void UpdateVehicle(const std::shared_ptr<restbed::Session> session);
void DeleteVehicle(const std::shared_ptr<restbed::Session> session);

bool running = true;

int main() {
    vcar vc;
    //vc.launch();

    // TODO: Registering actions should be done through components or through REST API
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

    /// REST communication setup
    auto register_user = std::make_shared<restbed::Resource>();
    auto register_vehicle = std::make_shared<restbed::Resource>();
    auto vehicle = std::make_shared<restbed::Resource>();

    register_user->set_path("/embedded/v1/register");
    register_vehicle->set_path("/embedded/v1/vehicles");
    vehicle->set_path("/embedded/v1/vehicles/{vid: ([^\\s]+)}");

    register_user->set_method_handler("POST", PostRegisterUser);
    register_vehicle->set_method_handler("POST", PostRegisterVehicle);
    vehicle->set_method_handler("GET", GetVehicle);
    vehicle->set_method_handler("UPDATE", UpdateVehicle);
    vehicle->set_method_handler("DELETE", DeleteVehicle);

    auto settings = std::make_shared<restbed::Settings>();
    settings->set_port(7998);
    settings->set_default_header("Connection", "close");

    restbed::Service service;
    service.publish(register_vehicle);
    service.publish(register_user);
    service.publish(vehicle);
    service.start(settings);

    while (running) {
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
