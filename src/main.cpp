// TODO: Get the token from REST server to authorize vehicle actions because SQL is a hot garbage

#include <cstdlib>
#include <iostream>
#include <memory>
#include <restbed>
#include <string>
#include <vcar.h>

#include <linux/can.h>

void PostRegisterUser(std::shared_ptr<restbed::Session> session);
void PostVehicle(std::shared_ptr<restbed::Session> session);
void GetVehicle(std::shared_ptr<restbed::Session> session);
void UpdateVehicle(std::shared_ptr<restbed::Session> session);
void DeleteVehicle(std::shared_ptr<restbed::Session> session);

bool running = true;

int main() {
    vcar vc;
    //vc.launch();

    /// REST communication setup
    auto register_user = std::make_shared<restbed::Resource>();
    auto register_vehicle = std::make_shared<restbed::Resource>();
    auto vehicle = std::make_shared<restbed::Resource>();

    register_user->set_path("/embedded/v1/register");
    register_vehicle->set_path("/embedded/v1/vehicles");
    vehicle->set_path("/embedded/v1/vehicles/{vid: ([^\\s]+)}");

    register_user->set_method_handler("POST", PostRegisterUser);
    register_vehicle->set_method_handler("POST", PostVehicle);
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

    while (running) {}

    return 0;
}

void PostRegisterUser(const std::shared_ptr<restbed::Session> session) {
    const auto req = session->get_request();
    int content_length = req->get_header("Content-Length", 0);

    session->fetch(content_length, [](const std::shared_ptr<restbed::Session> session, const restbed::Bytes& body) {
        std::cout << (int)body.size() << " | " << body.data() << std::endl;
        session->close(restbed::OK, "POST Register User");
    });
}

void PostVehicle(std::shared_ptr<restbed::Session> session) {
    const auto req = session->get_request();
    int content_length = req->get_header("Content-Length", 0);

    session->fetch(content_length, [](const std::shared_ptr<restbed::Session> session, const restbed::Bytes& body) {
        std::cout << (int)body.size() << " | " << body.data() << std::endl;
        session->close( restbed::OK, "POST Vehicle", { { "Content-Length", "13" } } );
    });
}

void GetVehicle(const std::shared_ptr<restbed::Session> session) {
    const auto req = session->get_request();
    int content_length = req->get_header("Content-Length", 0);

    session->fetch(content_length, [](const std::shared_ptr<restbed::Session> session, const restbed::Bytes& body) {
        std::cout << (int)body.size() << " | " << body.data() << std::endl;
        session->close( restbed::OK, "GET Vehicle", { { "Content-Length", "13" } } );
    });
}

void UpdateVehicle(const std::shared_ptr<restbed::Session> session) {
    const auto req = session->get_request();
    int content_length = req->get_header("Content-Length", 0);

    session->fetch(content_length, [](const std::shared_ptr<restbed::Session> session, const restbed::Bytes& body) {
        std::cout << (int)body.size() << " | " << body.data() << std::endl;
        session->close( restbed::OK, "UPDATE Vehicle", { { "Content-Length", "13" } } );
    });
}

void DeleteVehicle(const std::shared_ptr<restbed::Session> session) {
    const auto req = session->get_request();
    int content_length = req->get_header("Content-Length", 0);

    session->fetch(content_length, [](const std::shared_ptr<restbed::Session> session, const restbed::Bytes& body) {
        std::cout << (int)body.size() << " | " << body.data() << std::endl;
        session->close( restbed::OK, "DELETE Vehicle", { { "Content-Length", "13" } } );
    });
}