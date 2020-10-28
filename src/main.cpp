#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <restbed>
#include <string>
#include <vcar.h>

#define VID_JSON_FILE "vids.json"

void PostRegisterUser(std::shared_ptr<restbed::Session> session);
void PostVehicle(std::shared_ptr<restbed::Session> session);
void GetVehicle(std::shared_ptr<restbed::Session> session);
void UpdateVehicle(std::shared_ptr<restbed::Session> session);
void DeleteVehicle(std::shared_ptr<restbed::Session> session);

std::string getStringFromJSON(rapidjson::Value const& doc);
void parseJSONFromFile(std::string file_name, rapidjson::Document& doc);
void saveJSONToFile(std::string file_name, rapidjson::Value& doc);

rapidjson::Document vids;
bool running = true;
bool default_halted = true;
std::map<std::string, vcar> vehicles;

int main() {
    vehicles.emplace("test", default_halted);
    //vehicles["test"].launch();

    // Load vid files JSON
    parseJSONFromFile(VID_JSON_FILE, vids);

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
        // Clean incoming data JSON
        std::stringstream vehicle_ss;
        vehicle_ss << body.data() << std::flush;
        std::string vjson_str = vehicle_ss.str();
        vjson_str.erase(std::remove(vjson_str.begin(), vjson_str.end(), '\\'), vjson_str.end());  // Erase slashes

        if (*vjson_str.begin() == '\"' && *(vjson_str.end() - 1) == '\"') {  // Erase surrounding quotes
            vjson_str = vjson_str.substr(1, vjson_str.size() - 2);
        }

        // Parse data JSON
        Document vehicle_json;
        vehicle_json.Parse(vjson_str.c_str());
        std::cout << vjson_str.c_str() << std::endl;

        if (vehicle_json.HasParseError()) {
            throw std::runtime_error("Error parsing user JSON");
        }

        std::string email = vehicle_json["email"].GetString();
        std::string vid = vehicle_json["vid"].GetString();

        // New email
        if (!vids.HasMember(email.c_str())) {
            Value vid_list(kArrayType);
            vids.AddMember(Value(email.c_str(), vids.GetAllocator()), vid_list, vids.GetAllocator());
        }

        if (vids[email.c_str()].GetArray().Size() >= 5) {
            session->close(restbed::BAD_REQUEST, "Vehicle limit reached");
            return;
        }

        // Create vid entry
        vids[email.c_str()].GetArray().PushBack(Value(vid.c_str(), vids.GetAllocator()), vids.GetAllocator());

        // Create car
        if (vehicles.find(vid) == vehicles.end()) {
            vehicles.emplace(vid, default_halted);
        }

        saveJSONToFile(VID_JSON_FILE, vids);

        session->close( restbed::CREATED, "New vehicle created");
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

std::string getStringFromJSON(rapidjson::Value const& doc) {
    rapidjson::StringBuffer buffer;
    buffer.Clear();

    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    return std::string(buffer.GetString());
}

void parseJSONFromFile(const std::string file_name, rapidjson::Document& doc) {
    // Load vid file
    std::ifstream fs(file_name);
    fs.seekg(0, fs.end);
    int file_length = fs.tellg();
    fs.seekg(0, fs.beg);

    char* file_content = new char[file_length];
    fs.read(file_content, file_length);

    doc.Parse(file_content);

    delete[] file_content;
}

void saveJSONToFile(const std::string file_name, rapidjson::Value& doc) {
    std::ofstream fs(file_name, std::iostream::trunc);
    fs << getStringFromJSON(doc);
    fs.close();
}