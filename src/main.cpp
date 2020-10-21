// TODO: Get the token from REST server to authorize vehicle actions because SQL is a hot garbage

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <rapidjson/document.h>
#include <restbed>
#include <sstream>
#include <string>
#include <vcar.h>

void PostRegisterUser(std::shared_ptr<restbed::Session> session);
void PostVehicle(std::shared_ptr<restbed::Session> session);
void GetVehicle(std::shared_ptr<restbed::Session> session);
void UpdateVehicle(std::shared_ptr<restbed::Session> session);
void DeleteVehicle(std::shared_ptr<restbed::Session> session);

bool running = true;
bool default_halted = true;
std::map<std::string, vcar> vehicles;

int main() {
    vehicles.emplace("test", default_halted);
    //vehicles["test"].launch();

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
    using namespace rapidjson;

    const auto req = session->get_request();
    int content_length = req->get_header("Content-Length", 0);

    session->fetch(content_length, [](const std::shared_ptr<restbed::Session> session, const restbed::Bytes& body) {
        std::stringstream user_ss;
        user_ss << body.data() << std::flush;

        Document user_json;
        user_json.Parse(user_ss.str().c_str());

        if (user_json.HasParseError()) {
             throw std::runtime_error("Error parsing user JSON");
        }

        std::cout << (int)body.size() << " | " << body.data() << std::endl;
        session->close(restbed::OK, "POST Register User");
    });
}

void PostVehicle(std::shared_ptr<restbed::Session> session) {
    using namespace rapidjson;

    const auto req = session->get_request();
    int content_length = req->get_header("Content-Length", 0);

    session->fetch(content_length, [](const std::shared_ptr<restbed::Session> session, const restbed::Bytes& body) {
        //std::cout << (int)body.size() << " | " << body.data() << std::endl;

        std::stringstream vehicle_ss;
        vehicle_ss << body.data() << std::flush;
        vehicle_ss.str().erase(std::remove(vehicle_ss.str().begin(), vehicle_ss.str().end(), '\\'), vehicle_ss.str().end());
        std::cout << vehicle_ss.str() << std::endl;

        Document vehicle_json;
        vehicle_json.Parse(vehicle_ss.str().c_str());

        if (vehicle_json.HasParseError()) {
            throw std::runtime_error("Error parsing user JSON");
        }

        if (vehicles.find(vehicle_json["vid"].GetString()) == vehicles.end()) {
            vehicles.emplace(vehicle_json["vid"].GetString(), default_halted);
        }

        session->close( restbed::OK, "POST Vehicle", { { "Content-Length", "13" } } );
    });
}

void GetVehicle(const std::shared_ptr<restbed::Session> session) {
    using namespace rapidjson;

    const auto req = session->get_request();
    int content_length = req->get_header("Content-Length", 0);

    session->fetch(content_length, [](const std::shared_ptr<restbed::Session> session, const restbed::Bytes& body) {
        std::stringstream vehicle_ss;
        vehicle_ss << body.data() << std::flush;

        Document vehicle_json;
        vehicle_json.Parse(vehicle_ss.str().c_str());

        if (vehicle_json.HasParseError()) {
            throw std::runtime_error("Error parsing user JSON");
        }

        std::cout << (int)body.size() << " | " << body.data() << std::endl;
        session->close( restbed::OK, "GET Vehicle", { { "Content-Length", "13" } } );
    });
}

void UpdateVehicle(const std::shared_ptr<restbed::Session> session) {
    using namespace rapidjson;

    const auto req = session->get_request();
    int content_length = req->get_header("Content-Length", 0);

    session->fetch(content_length, [](const std::shared_ptr<restbed::Session> session, const restbed::Bytes& body) {
        std::stringstream vehicle_ss;
        vehicle_ss << body.data() << std::flush;

        Document vehicle_json;
        vehicle_json.Parse(vehicle_ss.str().c_str());

        if (vehicle_json.HasParseError()) {
            throw std::runtime_error("Error parsing user JSON");
        }

        session->close( restbed::OK, "UPDATE Vehicle", { { "Content-Length", "13" } } );
    });
}

void DeleteVehicle(const std::shared_ptr<restbed::Session> session) {
    using namespace rapidjson;

    const auto req = session->get_request();
    int content_length = req->get_header("Content-Length", 0);

    session->fetch(content_length, [](const std::shared_ptr<restbed::Session> session, const restbed::Bytes& body) {
        std::stringstream vehicle_ss;
        vehicle_ss << body.data() << std::flush;

        Document vehicle_json;
        vehicle_json.Parse(vehicle_ss.str().c_str());

        if (vehicle_json.HasParseError()) {
            throw std::runtime_error("Error parsing user JSON");
        }

        std::cout << (int)body.size() << " | " << body.data() << std::endl;
        session->close( restbed::OK, "DELETE Vehicle", { { "Content-Length", "13" } } );
    });
}